#pragma once

#include <vector>
#include <memory>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>

#include "constants.h"

typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

uint32_t* leafGetCellCount(void* node);

void* LeafGetCell(void* node, uint32_t cellCount);

uint32_t* leafGetKey(void* node, uint32_t cellCount);

void* leafGetValue(void* node, uint32_t cellCount);

uint32_t* leafGetNextLeaf(void* node);

void leafInitialize(void* node);

NodeType nodeGetType(void* node);

void nodeSetType(void* node, NodeType type);

uint32_t* internalGetKeyCount(void* node);

uint32_t* internalGetRightChild(void* node);

uint32_t* internalGetCell(void* node, uint32_t cellCount);

uint32_t* internalGetChild(void* node, uint32_t child_num);

uint32_t* internalGetKey(void* node, uint32_t key_num);

bool isRootNode(void* node);

void setRootNode(void* node, bool is_root);

void internalInitialize(void* node);

uint32_t* getParent(void* node);

void internalUpdateKey(void* node, uint32_t old_key, uint32_t new_key);

uint32_t internalFindChild(void* node, uint32_t key);
