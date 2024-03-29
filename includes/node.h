#pragma once

#include <vector>
#include <memory>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>

#include "constants.h"

typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

void nodeSetType(void* node, NodeType type);
void setRootNode(void* node, bool is_root);
bool isRootNode(void* node);
uint32_t* getParent(void* node);
NodeType nodeGetType(void* node);

void leafInitialize(void* node);
void* leafGetValue(void* node, uint32_t cellCount);
void* leafGetCell(void* node, uint32_t cellCount);
uint32_t* leafGetCellCount(void* node);
uint32_t* leafGetKey(void* node, uint32_t cellCount);
uint32_t* leafGetNextLeaf(void* node);

void internalInitialize(void* node);
void internalUpdateKey(void* node, uint32_t old_key, uint32_t new_key);
uint32_t internalFindChild(void* node, uint32_t key);
uint32_t* internalGetKeyCount(void* node);
uint32_t* internalGetRightChild(void* node);
uint32_t* internalGetCell(void* node, uint32_t cellCount);
uint32_t* internalGetChild(void* node, uint32_t child_num);
uint32_t* internalGetKey(void* node, uint32_t key_num);
