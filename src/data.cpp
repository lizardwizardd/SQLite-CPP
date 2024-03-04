#include "../includes/data.h"

// Copy a row into a file
void serializeRow(Row* source, void* destination) 
{
    char* destPtr = static_cast<char*>(destination);
    memcpy(destPtr + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destPtr + USERNAME_OFFSET, source->username, USERNAME_SIZE);
    memcpy(destPtr + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

// Copy data from file into a row 
void deserializeRow(void* source, Row* destination) 
{
    char* srcPtr = static_cast<char*>(source);
    memcpy(&(destination->id), srcPtr + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), srcPtr + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), srcPtr + EMAIL_OFFSET, EMAIL_SIZE);
}

void printRow(Row* row)
{
    if (row->id == 0) // row is marked as deleted
        return;

    std::cout << "(" << row->id << ", " << row->username
              << ", " << row->email << ")" << std::endl;
}

Table::Table(std::unique_ptr<Pager> pager, uint32_t rootPageNumber) : 
    pager(std::move(pager)), 
    rootPageNumber(rootPageNumber) { }

std::unique_ptr<Cursor> tableStart(std::shared_ptr<Table>& table)
{
    // Search for the lowest id node
    std::unique_ptr<Cursor> cursor =  tableFindKey(table, 0);
    
    void* node = table->pager->getPage(cursor->pageNumber);
    uint32_t cellCount = *leafGetCellCount(node);
    cursor->endOfTable = (cellCount == 0);

    return cursor;
}

// Return the position of a given key. 
// If the key is not present, return the position where it should be inserted
std::unique_ptr<Cursor> tableFindKey(std::shared_ptr<Table>& table, const uint32_t key)
{
    uint32_t rootPageNumber = table->rootPageNumber;
    void* rootNode = table->pager->getPage(rootPageNumber);

    if (nodeGetType(rootNode) == NODE_LEAF)
    {
        return findLeafNode(table, rootPageNumber, key);
    }
    else
    {
        return findInternalNode(table, rootPageNumber, key);
    }
}

std::shared_ptr<Table> openDatabase(std::string filename)
{
    std::shared_ptr<Table> table = std::make_shared<Table>(openPager(filename), 0);

    if (table->pager->getPageCount() == 0)
    {
        // New database file. Initialize page 0 as leaf node
        void* rootNode = table->pager->getPage(0);
        leafInitialize(rootNode);
        setRootNode(rootNode, true);
    }

    return table;
}

std::shared_ptr<Table> createDatabase(std::string filename)
{
    std::shared_ptr<Table> table = std::make_shared<Table>(createPager(filename), 0);

    if (table->pager->getPageCount() == 0)
    {
        // New database file. Initialize page 0 as leaf node
        void* rootNode = table->pager->getPage(0);
        leafInitialize(rootNode);
        setRootNode(rootNode, true);
    }

    return table;
}

// Delete a .db file by a given filename
std::shared_ptr<Table> dropDatabase(std::string filename)
{
    if (!DeleteFileA(filename.c_str()))
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            throw std::exception("File not found.");
        }
        else if (GetLastError() == ERROR_SHARING_VIOLATION)
        {
            throw std::exception("Can't drop while table is open.");
        }
        else
        {
            throw std::runtime_error("Unable to delete file.");
        }
    }

    return nullptr;
}

// Save, then free memory and close table
void saveAndCloseDatabase(const std::shared_ptr<Table>& table)
{
    for (uint32_t i = 0; i < table->pager->getPageCount(); i++)
    {
        if (table->pager->pages[i] == NULL)
        {
            continue;
        }
        table->pager->pagerFlush(i);
        delete table->pager->pages[i];
        table->pager->pages[i] = NULL;
    }

    if (CloseHandle(table->pager->getFileHandle()) == 0)
    {
        throw std::runtime_error("Error closing db file.");
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        void* page = table->pager->pages[i];
        if (page)
        {
            free(page);
            table->pager->pages[i] = NULL;
        }
    }
}

// Save table without closing
void saveTable(const std::shared_ptr<Table>& table)
{
    for (uint32_t i = 0; i < table->pager->getPageCount(); i++)
    {
        if (table->pager->pages[i] == NULL)
        {
            continue;
        }
        table->pager->pagerFlush(i);
    }
}

// Free memory withount closing
void freeTable(const std::shared_ptr<Table>& table)
{
    for (uint32_t i = 0; i < table->pager->getPageCount(); i++)
    {
        if (table->pager->pages[i] == NULL)
        {
            continue;
        }
        delete table->pager->pages[i];
        table->pager->pages[i] = NULL;
    }
}

void* cursorValue(std::unique_ptr<Cursor>& cursor)
{
    uint32_t pageNumber = cursor->pageNumber;
    void* page = cursor->table->pager->getPage(pageNumber);

    return leafGetValue(page, cursor->cellCount);
}

void cursorAdvance(std::unique_ptr<Cursor>& cursor)
{
    uint32_t pageNumber = cursor->pageNumber;
    void* node = cursor->table->pager->getPage(pageNumber);

    cursor->cellCount += 1;
    if (cursor->cellCount >= (*leafGetCellCount(node)))
    {
        // Advance to next leaf node
        uint32_t nextPageNum = *leafGetNextLeaf(node);
        if (nextPageNum == 0)
        {
            // This was rightmost leaf
            cursor->endOfTable = true;
        }
        else
        {
            cursor->pageNumber = nextPageNum;
            cursor->cellCount = 0;
        }
    }
}

// Same as cursorAdvance
Cursor& Cursor::operator++(int)
{
    void* node = this->table->pager->getPage(pageNumber);
    
    cellCount += 1;
    if (cellCount >= (*leafGetCellCount(node)))
    {
        // Advance to next leaf node
        uint32_t nextPageNum = *leafGetNextLeaf(node);
        if (nextPageNum == 0)
        {
            // This was rightmost leaf
            endOfTable = true;
        }
        else
        {
            pageNumber = nextPageNum;
            cellCount = 0;
        }
    }

    return *this;
}

// Inserts a new key-value pair into the leaf node of the B-tree
void leafInsert(std::unique_ptr<Cursor>& cursor, const uint32_t key, Row* value)
{
    void* node = cursor->table->pager->getPage(cursor->pageNumber);

    // Check if node is full
    uint32_t cellCount = *leafGetCellCount(node);
    if (cellCount >= LEAF_NODE_MAX_CELLS)
    {
        // Node full
        leafSplitAndInsert(cursor, key, value);
        return;
    }

    // Make room for new cell if necessary
    if (cursor->cellCount < cellCount)
    {
        for (uint32_t i = cellCount; i > cursor->cellCount; i--)
        {
            memcpy(leafGetCell(node, i), leafGetCell(node, i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    *(leafGetCellCount(node)) += 1;
    *(leafGetKey(node, cursor->cellCount)) = key;
    serializeRow(value, leafGetValue(node, cursor->cellCount));
}

void leafUpdate(std::unique_ptr<Cursor>& cursor, Row* value)
{
    void* node = cursor->table->pager->getPage(cursor->pageNumber);
    serializeRow(value, leafGetValue(node, cursor->cellCount));
}

void leafDelete(std::unique_ptr<Cursor>& cursor)
{
    void* node = cursor->table->pager->getPage(cursor->pageNumber);
    void* cellToDelete = leafGetValue(node, cursor->cellCount);

    // Replace key with 0 without erasing other data to make undo possible
    uint32_t deletedKeyMarker = 0;
    memcpy(static_cast<char*>(cellToDelete) + ID_OFFSET,
           &deletedKeyMarker, ID_SIZE);
}

// Splits a leaf node and inserts a new key-value pair into the appropriate node
void leafSplitAndInsert(std::unique_ptr<Cursor>& cursor, const uint32_t key, Row* value)
{
    // Create a new node and move half the cells over.
    // Insert the new value in one of the two nodes.
    // Update parent or create a new parent.

    void* oldNode = cursor->table->pager->getPage(cursor->pageNumber);
    uint32_t oldMax = getMaxKey(cursor->table->pager, oldNode);
    uint32_t newPageNumber = cursor->table->pager->getUnusedPageNumber();
    void* newNode = cursor->table->pager->getPage(newPageNumber);
    leafInitialize(newNode);
    *getParent(newNode) = *getParent(oldNode);
    *leafGetNextLeaf(newNode) = *leafGetNextLeaf(oldNode);
    *leafGetNextLeaf(oldNode) = newPageNumber;

    // After dividing all keys between left and right nodes,
    // move each key to correct position, starting from the right
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) 
    {
        void* destinationNode;
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) 
        {
            destinationNode = newNode;
        } 
        else
        {
            destinationNode = oldNode;
        }
        uint32_t indexInNode = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destination = leafGetCell(destinationNode, indexInNode);

        if (i == cursor->cellCount) 
        {
            serializeRow(value, leafGetValue(destinationNode, indexInNode));
            *leafGetKey(destinationNode, indexInNode) = key;
        } 
        else if (i > cursor->cellCount)
        {
            memcpy(destination, leafGetCell(oldNode, i - 1), LEAF_NODE_CELL_SIZE);
        }
        else
        {
            memcpy(destination, leafGetCell(oldNode, i), LEAF_NODE_CELL_SIZE);
        }
    }

    // Update cell count on both leaf nodes
    *(leafGetCellCount(oldNode)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leafGetCellCount(newNode)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (isRootNode(oldNode)) 
    {
        return createNewRootNode(cursor->table, newPageNumber);
    } 
    else
    {
        uint32_t parentPageNumber = *getParent(oldNode);
        uint32_t new_max = getMaxKey(cursor->table->pager, oldNode);
        void* parent = cursor->table->pager->getPage(parentPageNumber);

        internalUpdateKey(parent, oldMax, new_max);
        internalInsert(cursor->table, parentPageNumber, newPageNumber);

        return;
    }
}

// Search table for a node that contains the given key
std::unique_ptr<Cursor> findLeafNode(std::shared_ptr<Table>& table, 
                                       uint32_t pageNumber, const uint32_t key)
{
    void* node = table->pager->getPage(pageNumber);
    uint32_t cellCount = *leafGetCellCount(node);

    std::unique_ptr<Cursor> cursor = std::make_unique<Cursor>();
    cursor->table = table;
    cursor->pageNumber = pageNumber;

    // Binary search
    uint32_t minIndex = 0;
    uint32_t onePastMaxIndex = cellCount;
    while (onePastMaxIndex != minIndex)
    {
        uint32_t index = (minIndex + onePastMaxIndex) / 2;
        uint32_t keyAtIndex = *leafGetKey(node, index);
        if (key == keyAtIndex)
        {
            cursor->cellCount = index;
            return cursor;
        }
        if (key < keyAtIndex)
        {
            onePastMaxIndex = index;
        }
        else
        {
            minIndex = index + 1;
        }
    }
    cursor->cellCount = minIndex;

    return cursor;
}

void createNewRootNode(std::shared_ptr<Table>& table, uint32_t rightChildPageNum)
{
    // Handle splitting the root.
    // Old root copied to new page, becomes left child.
    // Address of right child passed in.
    // Re-initialize root page to contain the new root node.
    // New root node points to two children.

    void* root = table->pager->getPage(table->rootPageNumber);
    void* rightChild = table->pager->getPage(rightChildPageNum);
    uint32_t leftChildPageNumber = table->pager->getUnusedPageNumber();
    void* leftChild = table->pager->getPage(leftChildPageNumber);

    if (nodeGetType(root) == NODE_INTERNAL) 
    {
        internalInitialize(rightChild);
        internalInitialize(leftChild);
    }

    // Left child has data copied from old root
    memcpy(leftChild, root, PAGE_SIZE);
    setRootNode(leftChild, false);

    if (nodeGetType(leftChild) == NODE_INTERNAL) 
    {
        void* child;
        for (uint32_t i = 0; i < *internalGetKeyCount(leftChild); i++)
        {
            child = table->pager->getPage(*internalGetChild(leftChild,i));
            *getParent(child) = leftChildPageNumber;
        }
        child = table->pager->getPage(*internalGetRightChild(leftChild));
        *getParent(child) = leftChildPageNumber;
    }

    // Root node is a new internal node with one key and two children
    internalInitialize(root);
    setRootNode(root, true);
    *internalGetKeyCount(root) = 1;
    *internalGetChild(root, 0) = leftChildPageNumber;
    uint32_t leftChildMaxKey = getMaxKey(table->pager, leftChild);
    *internalGetKey(root, 0) = leftChildMaxKey;
    *internalGetRightChild(root) = rightChildPageNum;
    *getParent(leftChild) = table->rootPageNumber;
    *getParent(rightChild) = table->rootPageNumber;
}

// Search table for a node that contains the given key
std::unique_ptr<Cursor> findInternalNode(std::shared_ptr<Table>& table, 
                                         uint32_t pageNumber, const uint32_t key)
{
    void* node = table->pager->getPage(pageNumber);

    uint32_t childIndex = internalFindChild(node, key);
    uint32_t childNum = *internalGetChild(node, childIndex);
    void* child = table->pager->getPage(childNum);
    switch (nodeGetType(child)) 
    {
        case NODE_LEAF:
            return findLeafNode(table, childNum, key);
        case NODE_INTERNAL:
            return findInternalNode(table, childNum, key);
        default:
            throw std::exception("Unknown node type.");
    }
}

// Return the index of the child which should contain the given key
uint32_t internalFindChild(void* node, const uint32_t key)
{
    uint32_t numKeys = *internalGetKeyCount(node);

    //Binary search
    uint32_t minIndex = 0;
    uint32_t maxIndex = numKeys; // there is one more child than key

    while (minIndex != maxIndex) 
    {
        uint32_t index = (minIndex + maxIndex) / 2;
        uint32_t keyToRight = *internalGetKey(node, index);

        if (keyToRight >= key) 
        {
            maxIndex = index;
        } 
        else
        {
            minIndex = index + 1;
        }
    }

    return minIndex;
}

void internalInsert(std::shared_ptr<Table>& table, uint32_t parentPageNumber,
                    uint32_t childPageNumber)
{
    // Add a new child/key pair to parent that corresponds to child
    void* parent = table->pager->getPage(parentPageNumber);
    void* child = table->pager->getPage(childPageNumber);
    uint32_t childMaxKey = getMaxKey(table->pager, child);
    uint32_t index = internalFindChild(parent, childMaxKey);

    uint32_t originalKeyCount = *internalGetKeyCount(parent);

    if (originalKeyCount >= INTERNAL_NODE_MAX_KEYS) 
    {
        internalSplitAndInsert(table, parentPageNumber, childPageNumber);
        return;
    }

    uint32_t rightChildPageNum = *internalGetRightChild(parent);

    // An internal node with a right child of INVALID_PAGE_NUM is empty
    if (rightChildPageNum == INVALID_PAGE_NUM) {
        *internalGetRightChild(parent) = childPageNumber;
        return;
    }

    void* rightChild = table->pager->getPage(rightChildPageNum);

    // If we are already at the max number of cells for a node, we cannot increment
    // before splitting. Incrementing without inserting a new key/child pair
    // and immediately calling internalSplitAndInsert has the effect
    // of creating a new key at (max_cells + 1) with an uninitialized value

    *internalGetKeyCount(parent) = originalKeyCount + 1;

    if (childMaxKey > getMaxKey(table->pager, rightChild)) {
        /* Replace right child */
        *internalGetChild(parent, originalKeyCount) = rightChildPageNum;
        *internalGetKey(parent, originalKeyCount) =  getMaxKey(table->pager, rightChild);
        *internalGetRightChild(parent) = childPageNumber;
    } 
    else
    {
        // Make room for the new cell
        for (uint32_t i = originalKeyCount; i > index; i--) 
        {
            void* destination = internalGetCell(parent, i);
            void* source = internalGetCell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }
        *internalGetChild(parent, index) = childPageNumber;
        *internalGetKey(parent, index) = childMaxKey;
    }
}

void internalSplitAndInsert(std::shared_ptr<Table>& table, uint32_t parentPageNumber,
                          uint32_t childPageNumber) 
{
    uint32_t oldPageNumber = parentPageNumber;
    void* oldNode = table->pager->getPage(parentPageNumber);
    uint32_t oldMax = getMaxKey(table->pager, oldNode);

    void* child = table->pager->getPage(childPageNumber); 
    uint32_t maxChild = getMaxKey(table->pager, child);

    uint32_t newPageNumber = table->pager->getUnusedPageNumber();

    uint32_t splittingRoot = isRootNode(oldNode);
    
    // If splitting a root node, insert the new node while creating new root
    // Otherwise, insert the created node into ints parent

    void* parent;
    void* newNode;
    if (splittingRoot)
    {
        createNewRootNode(table, newPageNumber);
        parent = table->pager->getPage(table->rootPageNumber);
        
        // Update oldNode to point to the new root's left child
        // New_page_num will already point to the new root's right child

        oldPageNumber = *internalGetChild(parent,0);
        oldNode = table->pager->getPage(oldPageNumber);
    }
    else 
    {
        parent = table->pager->getPage(*getParent(oldNode));
        newNode = table->pager->getPage(newPageNumber);
        internalInitialize(newNode);
    }
    
    uint32_t* oldNumKeys = internalGetKeyCount(oldNode);

    uint32_t currentPageNumber = *internalGetRightChild(oldNode);
    void* cur = table->pager->getPage(currentPageNumber);

    
    // Insert right child into new node and set right child of old node to invalid page number
    internalInsert(table, newPageNumber, currentPageNumber);
    *getParent(cur) = newPageNumber;
    *internalGetRightChild(oldNode) = INVALID_PAGE_NUM;

    // For each key until the middle key, move the key and the child to the new node
    for (int i = INTERNAL_NODE_MAX_KEYS - 1; i > INTERNAL_NODE_MAX_KEYS / 2; i--)
    {
        currentPageNumber = *internalGetChild(oldNode, i);
        cur = table->pager->getPage(currentPageNumber);

        internalInsert(table, newPageNumber, currentPageNumber);
        *getParent(cur) = newPageNumber;

        (*oldNumKeys)--;
    }

    // Set child before middle key (its now the max child) to be node's right child,
    *internalGetRightChild(oldNode) = *internalGetChild(oldNode, *oldNumKeys - 1);
    (*oldNumKeys)--;

    // Insert the child into max node
    uint32_t maxAfterSplit = getMaxKey(table->pager, oldNode);

    uint32_t destinationPageNum = maxChild < maxAfterSplit ? oldPageNumber : newPageNumber;

    internalInsert(table, destinationPageNum, childPageNumber);
    *getParent(child) = destinationPageNum;

    internalUpdateKey(parent, oldMax, getMaxKey(table->pager, oldNode));

    if (!splittingRoot) 
    {
        internalInsert(table,*getParent(oldNode), newPageNumber);
        *getParent(newNode) = *getParent(oldNode);
    }
}

// Get current max key in node
uint32_t getMaxKey(const std::unique_ptr<Pager>& pager, void* node) 
{
    if (nodeGetType(node) == NODE_LEAF) 
    {
        return *leafGetKey(node, *leafGetCellCount(node) - 1);
    }
    void* rightChild = pager->getPage(*internalGetRightChild(node));
    return getMaxKey(pager, rightChild);
}
