#include "../includes/node.h"

uint32_t* leaf_node_num_cells(void* node)
{
    char* char_ptr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(char_ptr + LEAF_NODE_NUM_CELLS_OFFSET);
}

void* leaf_node_cell(void* node, uint32_t cell_num)
{
    char* char_ptr = reinterpret_cast<char*>(node);
    return reinterpret_cast<void*>(char_ptr + LEAF_NODE_HEADER_SIZE + 
                                   cell_num * LEAF_NODE_CELL_SIZE);
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num)
{
    return reinterpret_cast<uint32_t*>(leaf_node_cell(node, cell_num));
}

void* leaf_node_value(void* node, uint32_t cell_num)
{
    char* char_ptr = reinterpret_cast<char*>(leaf_node_cell(node, cell_num));
    return reinterpret_cast<void*>(char_ptr + LEAF_NODE_KEY_SIZE);
}

void initialize_leaf_node(void* node)
{
    set_node_type(node, NODE_LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
}

NodeType get_node_type(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    uint8_t value = *reinterpret_cast<uint8_t*>(charPtr + NODE_TYPE_OFFSET);
    return static_cast<NodeType>(value);
}

void set_node_type(void* node, NodeType type)
{
    char* charPtr = reinterpret_cast<char*>(node);
    uint8_t value = static_cast<uint8_t>(type);
    *reinterpret_cast<uint8_t*>(charPtr + NODE_TYPE_OFFSET) = value;
}

uint32_t* internal_node_num_keys(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* internal_node_right_child(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_HEADER_SIZE +
                                       cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* internal_node_child(void* node, uint32_t child_num)
{
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys)
    {
        throw std::runtime_error("Tried to access child_num " + std::to_string(child_num) + 
                                 " > " + std::to_string(num_keys));
    }
    else if (child_num == num_keys)
    {
        return internal_node_right_child(node);
    }
    else
    {
        return internal_node_cell(node, child_num);
    }
}

uint32_t* internal_node_key(void* node, uint32_t key_num)
{
    return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t get_node_max_key(void* node) 
{
    switch (get_node_type(node)) 
    {
        case NODE_INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node) - 1);
        case NODE_LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
}

bool is_node_root(void* node) 
{
    uint8_t value = *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(node) + IS_ROOT_OFFSET);
    return static_cast<bool>(value);
}

void set_node_root(void* node, bool is_root) 
{
    uint8_t value = is_root;
    *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(node) + IS_ROOT_OFFSET) = value;
}

void initialize_internal_node(void* node) 
{
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
}