#include "../includes/data.h"


void serialize_row(Row* source, void* destination) 
{
    char* dest = static_cast<char*>(destination);
    memcpy(dest + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(dest + USERNAME_OFFSET, source->username, USERNAME_SIZE);
    memcpy(dest + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) 
{
    char* src = static_cast<char*>(source);
    memcpy(&(destination->id), src + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), src + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), src + EMAIL_OFFSET, EMAIL_SIZE);
}

void print_row(Row* row)
{
    std::cout << "(" << row->id << ", " << row->username
              << ", " << row->email << ")" << std::endl;
}

std::unique_ptr<Cursor> table_start(std::shared_ptr<Table>& table)
{
    // Search for the lowest id node
    std::unique_ptr<Cursor> cursor =  table_find(table, 0);
    
    void* node = table->pager->get_page(cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    cursor->end_of_table = (num_cells == 0);

    return cursor;
}

// Return the position of a given key. 
// If the key is not present, return the position where it should be inserted
std::unique_ptr<Cursor> table_find(std::shared_ptr<Table>& table, uint32_t key)
{
    uint32_t root_page_num = table->root_page_num;
    void* root_node = table->pager->get_page(root_page_num);

    if (get_node_type(root_node) == NODE_LEAF)
    {
        return leaf_node_find(table, root_page_num, key);
    }
    else
    {
        return internal_node_find(table, root_page_num, key);
    }
}

std::shared_ptr<Table> db_open(std::string filename)
{
    Pager* pager = pager_open(filename);

    std::shared_ptr<Table> table = std::make_unique<Table>();

    table->pager = pager;
    table->root_page_num = 0;

    if (pager->num_pages == 0)
    {
        // New database file. Initialize page 0 as leaf node
        void* root_node = pager->get_page(0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }

    return table;
}

void db_close(const std::shared_ptr<Table>& table)
{
    Pager* pager = table->pager;

    for (uint32_t i = 0; i < pager->num_pages; i++)
    {
        if (pager->pages[i] == NULL)
        {
            continue;
        }
        pager->pager_flush(i);
        delete pager->pages[i];
        pager->pages[i] = NULL;
    }

    if (CloseHandle(pager->file_handle) == 0)
    {
        throw std::runtime_error("Error closing db file.");
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        void* page = pager->pages[i];
        if (page)
        {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    delete pager;
}

void* cursor_value(std::unique_ptr<Cursor>& cursor)
{
    uint32_t page_num = cursor->page_num;
    void* page = cursor->table->pager->get_page(page_num);

    return leaf_node_value(page, cursor->cell_num);
}

void cursor_advance(std::unique_ptr<Cursor>& cursor)
{
    uint32_t page_num = cursor->page_num;
    void* node = cursor->table->pager->get_page(page_num);

    cursor->cell_num += 1;
    if (cursor->cell_num >= (*leaf_node_num_cells(node)))
    {
        // Advance to next leaf node
        uint32_t next_page_num = *leaf_node_next_leaf(node);
        if (next_page_num == 0)
        {
            // This was rightmost leaf
            cursor->end_of_table = true;
        }
        else
        {
            cursor->page_num = next_page_num;
            cursor->cell_num = 0;
        }
    }
}

void leaf_node_insert(std::unique_ptr<Cursor>& cursor, uint32_t key, Row* value)
{
    void* node = cursor->table->pager->get_page(cursor->page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS)
    {
        // Node full
        leaf_node_split_and_insert(cursor, key, value);
        return;
    }

    if (cursor->cell_num < num_cells)
    {
        // Make room for new cell
        for (uint32_t i = num_cells; i > cursor->cell_num; i--)
        {
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

void leaf_node_split_and_insert(std::unique_ptr<Cursor>& cursor, uint32_t key, Row* value)
{
    // Create a new node and move half the cells over.
    // Insert the new value in one of the two nodes.
    // Update parent or create a new parent.

    void* old_node = cursor->table->pager->get_page(cursor->page_num);
    uint32_t old_max = get_node_max_key(cursor->table->pager, old_node);
    uint32_t new_page_num = cursor->table->pager->get_unused_page_num();
    void* new_node = cursor->table->pager->get_page(new_page_num);
    initialize_leaf_node(new_node);
    *node_parent(new_node) = *node_parent(old_node);
    *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
    *leaf_node_next_leaf(old_node) = new_page_num;

    // All existing keys plus new key should be divided
    // evenly between old (left) and new (right) nodes.
    // Starting from the right, move each key to correct position.

    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) 
    {
        void* destination_node;
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) 
        {
            destination_node = new_node;
        } 
        else
        {
            destination_node = old_node;
        }
        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destination = leaf_node_cell(destination_node, index_within_node);

        if (i == cursor->cell_num) 
        {
            serialize_row(value, leaf_node_value(destination_node, index_within_node));
            *leaf_node_key(destination_node, index_within_node) = key;
        } 
        else if (i > cursor->cell_num)
        {
            memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        }
        else
        {
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    // Update cell count on both leaf nodes

    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node)) 
    {
        return create_new_root(cursor->table, new_page_num);
    } 
    else
    {
        uint32_t parent_page_num = *node_parent(old_node);
        uint32_t new_max = get_node_max_key(cursor->table->pager, old_node);
        void* parent = cursor->table->pager->get_page(parent_page_num);

        update_internal_node_key(parent, old_max, new_max);
        internal_node_insert(cursor->table, parent_page_num, new_page_num);

        return;
    }
}

// Search table for a node that contains the given key
std::unique_ptr<Cursor> leaf_node_find(std::shared_ptr<Table>& table, 
                                       uint32_t page_num, uint32_t key)
{
    void* node = table->pager->get_page(page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    std::unique_ptr<Cursor> cursor = std::make_unique<Cursor>();
    cursor->table = table;
    cursor->page_num = page_num;

    // Binary search
    uint32_t min_index = 0;
    uint32_t one_past_max_index = num_cells;
    while (one_past_max_index != min_index)
    {
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = *leaf_node_key(node, index);
        if (key == key_at_index)
        {
            cursor->cell_num = index;
            return cursor;
        }
        if (key < key_at_index)
        {
            one_past_max_index = index;
        }
        else
        {
            min_index = index + 1;
        }
    }
    cursor->cell_num = min_index;

    return cursor;
}

void create_new_root(std::shared_ptr<Table>& table, uint32_t right_child_page_num)
{
    // Handle splitting the root.
    // Old root copied to new page, becomes left child.
    // Address of right child passed in.
    // Re-initialize root page to contain the new root node.
    // New root node points to two children.

    void* root = table->pager->get_page(table->root_page_num);
    void* right_child = table->pager->get_page(right_child_page_num);
    uint32_t left_child_page_num = table->pager->get_unused_page_num();
    void* left_child = table->pager->get_page(left_child_page_num);

    if (get_node_type(root) == NODE_INTERNAL) 
    {
        initialize_internal_node(right_child);
        initialize_internal_node(left_child);
    }

    // Left child has data copied from old root
    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    if (get_node_type(left_child) == NODE_INTERNAL) 
    {
        void* child;
        for (int i = 0; i < *internal_node_num_keys(left_child); i++)
        {
            child = table->pager->get_page(*internal_node_child(left_child,i));
            *node_parent(child) = left_child_page_num;
        }
        child = table->pager->get_page(*internal_node_right_child(left_child));
        *node_parent(child) = left_child_page_num;
    }

    // Root node is a new internal node with one key and two children
    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(table->pager, left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
    *node_parent(left_child) = table->root_page_num;
    *node_parent(right_child) = table->root_page_num;
}

// Search table for a node that contains the given key
std::unique_ptr<Cursor> internal_node_find(std::shared_ptr<Table>& table, 
                                           uint32_t page_num, uint32_t key)
{
    void* node = table->pager->get_page(page_num);

    uint32_t child_index = internal_node_find_child(node, key);
    uint32_t child_num = *internal_node_child(node, child_index);
    void* child = table->pager->get_page(child_num);
    switch (get_node_type(child)) 
    {
        case NODE_LEAF:
            return leaf_node_find(table, child_num, key);
        case NODE_INTERNAL:
            return internal_node_find(table, child_num, key);
    }
}

// Return the index of the child which should contain the given key
uint32_t internal_node_find_child(void* node, uint32_t key)
{
    uint32_t num_keys = *internal_node_num_keys(node);

    //Binary search
    uint32_t min_index = 0;
    uint32_t max_index = num_keys; // there is one more child than key

    while (min_index != max_index) 
    {
        uint32_t index = (min_index + max_index) / 2;
        uint32_t key_to_right = *internal_node_key(node, index);

        if (key_to_right >= key) 
        {
            max_index = index;
        } 
        else
        {
            min_index = index + 1;
        }
    }

    return min_index;
}

void internal_node_insert(std::shared_ptr<Table>& table, uint32_t parent_page_num, uint32_t child_page_num)
{
    // Add a new child/key pair to parent that corresponds to child

    void* parent = table->pager->get_page(parent_page_num);
    void* child = table->pager->get_page(child_page_num);
    uint32_t child_max_key = get_node_max_key(table->pager, child);
    uint32_t index = internal_node_find_child(parent, child_max_key);

    uint32_t original_num_keys = *internal_node_num_keys(parent);

    if (original_num_keys >= INTERNAL_NODE_MAX_KEYS) 
    {
        internal_node_split_and_insert(table, parent_page_num, child_page_num);
        return;
    }

    uint32_t right_child_page_num = *internal_node_right_child(parent);

    // An internal node with a right child of INVALID_PAGE_NUM is empty
    if (right_child_page_num == INVALID_PAGE_NUM) {
        *internal_node_right_child(parent) = child_page_num;
        return;
    }

    void* right_child = table->pager->get_page(right_child_page_num);

    // If we are already at the max number of cells for a node, we cannot increment
    // before splitting. Incrementing without inserting a new key/child pair
    // and immediately calling internal_node_split_and_insert has the effect
    // of creating a new key at (max_cells + 1) with an uninitialized value

    *internal_node_num_keys(parent) = original_num_keys + 1;

    if (child_max_key > get_node_max_key(table->pager, right_child)) {
        /* Replace right child */
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) =  get_node_max_key(table->pager, right_child);
        *internal_node_right_child(parent) = child_page_num;
    } 
    else
    {
        /* Make room for the new cell */
        for (uint32_t i = original_num_keys; i > index; i--) 
        {
            void* destination = internal_node_cell(parent, i);
            void* source = internal_node_cell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }
        *internal_node_child(parent, index) = child_page_num;
        *internal_node_key(parent, index) = child_max_key;
    }
}

void internal_node_split_and_insert(std::shared_ptr<Table>& table, uint32_t parent_page_num,
                          uint32_t child_page_num) 
{
    uint32_t old_page_num = parent_page_num;
    void* old_node = table->pager->get_page(parent_page_num);
    uint32_t old_max = get_node_max_key(table->pager, old_node);

    void* child = table->pager->get_page(child_page_num); 
    uint32_t child_max = get_node_max_key(table->pager, child);

    uint32_t new_page_num = table->pager->get_unused_page_num();

    /*
    Declaring a flag before updating pointers which
    records whether this operation involves splitting the root -
    if it does, we will insert our newly created node during
    the step where the table's new root is created. If it does
    not, we have to insert the newly created node into its parent
    after the old node's keys have been transferred over. We are not
    able to do this if the newly created node's parent is not a newly
    initialized root node, because in that case its parent may have existing
    keys aside from our old node which we are splitting. If that is true, we
    need to find a place for our newly created node in its parent, and we
    cannot insert it at the correct index if it does not yet have any keys
    */
    uint32_t splitting_root = is_node_root(old_node);

    void* parent;
    void* new_node;
    if (splitting_root) {
        create_new_root(table, new_page_num);
        parent = table->pager->get_page(table->root_page_num);
        /*
        If we are splitting the root, we need to update old_node to point
        to the new root's left child, new_page_num will already point to
        the new root's right child
        */
        old_page_num = *internal_node_child(parent,0);
        old_node = table->pager->get_page(old_page_num);
    } else {
        parent = table->pager->get_page(*node_parent(old_node));
        new_node = table->pager->get_page(new_page_num);
        initialize_internal_node(new_node);
    }
    
    uint32_t* old_num_keys = internal_node_num_keys(old_node);

    uint32_t cur_page_num = *internal_node_right_child(old_node);
    void* cur = table->pager->get_page(cur_page_num);

    
    // First put right child into new node and set right child of old node to invalid page number
    internal_node_insert(table, new_page_num, cur_page_num);
    *node_parent(cur) = new_page_num;
    *internal_node_right_child(old_node) = INVALID_PAGE_NUM;

    // For each key until you get to the middle key, move the key and the child to the new node
    for (int i = INTERNAL_NODE_MAX_KEYS - 1; i > INTERNAL_NODE_MAX_KEYS / 2; i--)
    {
        cur_page_num = *internal_node_child(old_node, i);
        cur = table->pager->get_page(cur_page_num);

        internal_node_insert(table, new_page_num, cur_page_num);
        *node_parent(cur) = new_page_num;

        (*old_num_keys)--;
    }

    // Set child before middle key, which is now the highest key, to be node's right child,
    //and decrement number of keys
    *internal_node_right_child(old_node) = *internal_node_child(old_node,*old_num_keys - 1);
    (*old_num_keys)--;

    // Determine which of the two nodes after the split should contain the child to be inserted,
    // and insert the child
    uint32_t max_after_split = get_node_max_key(table->pager, old_node);

    uint32_t destination_page_num = child_max < max_after_split ? old_page_num : new_page_num;

    internal_node_insert(table, destination_page_num, child_page_num);
    *node_parent(child) = destination_page_num;

    update_internal_node_key(parent, old_max, get_node_max_key(table->pager, old_node));

    if (!splitting_root) 
    {
        internal_node_insert(table,*node_parent(old_node),new_page_num);
        *node_parent(new_node) = *node_parent(old_node);
    }
}

uint32_t get_node_max_key(Pager* pager, void* node) 
{
    if (get_node_type(node) == NODE_LEAF) 
    {
        return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
    void* right_child = pager->get_page(*internal_node_right_child(node));
    return get_node_max_key(pager, right_child);
}
