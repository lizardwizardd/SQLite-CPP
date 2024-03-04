#include "../includes/node.h"

uint32_t* leafGetCellCount(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + LEAF_NODE_NUM_CELLS_OFFSET);
}

void* leafGetCell(void* node, uint32_t cellCount)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<void*>(charPtr + LEAF_NODE_HEADER_SIZE + 
                                   cellCount * LEAF_NODE_CELL_SIZE);
}

uint32_t* leafGetKey(void* node, uint32_t cellCount)
{
    return reinterpret_cast<uint32_t*>(leafGetCell(node, cellCount));
}

void* leafGetValue(void* node, uint32_t cellCount)
{
    char* charPtr = reinterpret_cast<char*>(leafGetCell(node, cellCount));
    return reinterpret_cast<void*>(charPtr + LEAF_NODE_KEY_SIZE);
}

uint32_t* leafGetNextLeaf(void* node) 
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + LEAF_NODE_NEXT_LEAF_OFFSET);
}

void leafInitialize(void* node)
{
    nodeSetType(node, NODE_LEAF);
    setRootNode(node, false);
    *leafGetCellCount(node) = 0;
    *leafGetNextLeaf(node) = 0;  // 0 is no sibling
}

NodeType nodeGetType(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    uint8_t value = *reinterpret_cast<uint8_t*>(charPtr + NODE_TYPE_OFFSET);
    return static_cast<NodeType>(value);
}

void nodeSetType(void* node, NodeType type)
{
    char* charPtr = reinterpret_cast<char*>(node);
    uint8_t value = static_cast<uint8_t>(type);
    *reinterpret_cast<uint8_t*>(charPtr + NODE_TYPE_OFFSET) = value;
}

uint32_t* internalGetKeyCount(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* internalGetRightChild(void* node)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* internalGetCell(void* node, uint32_t cellCount)
{
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_HEADER_SIZE +
                                       cellCount * INTERNAL_NODE_CELL_SIZE);
}

// Get internal node child by its number
uint32_t* internalGetChild(void* node, uint32_t child_num)
{
    uint32_t keyCount = *internalGetKeyCount(node);
    if (child_num > keyCount)
    {
        throw std::runtime_error("Tried to access child_num " + std::to_string(child_num) + 
                                 " > " + std::to_string(keyCount) + ".");
    }
    else if (child_num == keyCount)
    {
        uint32_t* rightChild = internalGetRightChild(node);
        if (*rightChild == INVALID_PAGE_NUM)
        {
            throw std::runtime_error("Tried to access right child, but the page was invalid.");
        }
        return rightChild;
    }
    else
    {
        uint32_t* child = internalGetCell(node, child_num);
        if (*child == INVALID_PAGE_NUM)
        {
            throw std::runtime_error("Tried to access child " + std::to_string(child_num) +
                                     " , but the page was invalid.");
        }
        return child;
    }
}

// Get one of internal node keys by number
uint32_t* internalGetKey(void* node, uint32_t key_num)
{
    char* charPtr = reinterpret_cast<char*>(internalGetCell(node, key_num));
    return reinterpret_cast<uint32_t*>(charPtr + INTERNAL_NODE_CHILD_SIZE);
}

bool isRootNode(void* node) 
{
    uint8_t value = *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(node) + IS_ROOT_OFFSET);
    return static_cast<bool>(value);
}

void setRootNode(void* node, bool is_root) 
{
    uint8_t value = is_root;
    *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(node) + IS_ROOT_OFFSET) = value;
}

void internalInitialize(void* node) 
{
    nodeSetType(node, NODE_INTERNAL);
    setRootNode(node, false);
    *internalGetKeyCount(node) = 0;
    
    // Making sure right child number does not initialize with 0
    *internalGetRightChild(node) = INVALID_PAGE_NUM;
}

uint32_t* getParent(void* node)
{ 
    char* charPtr = reinterpret_cast<char*>(node);
    return reinterpret_cast<uint32_t*>(charPtr + PARENT_POINTER_OFFSET);
}

void internalUpdateKey(void* node, uint32_t old_key, uint32_t new_key) 
{
    uint32_t oldChildIndex = internalFindChild(node, old_key);
    *internalGetKey(node, oldChildIndex) = new_key;
}
