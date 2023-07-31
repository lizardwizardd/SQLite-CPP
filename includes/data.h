#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>
#include <exception>

#include "pager.h"
#include "constants.h"
#include "node.h"


//-------------------------------------------------------------------
// Most of the stuff directly related to operations with data is in C
//-------------------------------------------------------------------

void serializeRow(Row*, void*);

void deserialize_row(void*, Row*);

void print_row(Row*);

typedef struct 
{
	Pager* pager;
    uint32_t rootPageNumber;
} Table;

typedef struct
{
    std::shared_ptr<Table> table; // Current table
    uint32_t pageNumber; // Current page number
    uint32_t cellCount; // Current cell number
    bool endOfTable; // Indicates a position one past the last element
} Cursor;

std::unique_ptr<Cursor> tableStart(std::shared_ptr<Table>& table);

std::unique_ptr<Cursor> tableFindKey(std::shared_ptr<Table>& table, uint32_t key);

std::shared_ptr<Table> openDatabase(std::string filename);

std::shared_ptr<Table> createDatabase(std::string filename);

std::shared_ptr<Table> dropDatabase(std::string filename);

void saveAndCloseDatabase(const std::shared_ptr<Table>& table);

void freeTable(const std::shared_ptr<Table>& table);

void saveTable(const std::shared_ptr<Table>& table);

void* cursorValue(std::unique_ptr<Cursor>& cursor);

void cursorAdvance(std::unique_ptr<Cursor>& cursor);

void leafInsert(std::unique_ptr<Cursor>& cursor, uint32_t key, Row* value);

void leafSplitAndInsert(std::unique_ptr<Cursor>& cursor, uint32_t key, Row* value);

std::unique_ptr<Cursor> findLeafNode(std::shared_ptr<Table>& table, 
                                       uint32_t pageNumber, uint32_t key);

void createNewRootNode(std::shared_ptr<Table>& table, uint32_t right_child_page_num);

std::unique_ptr<Cursor> findInternalNode(std::shared_ptr<Table>& table,
                                           uint32_t pageNumber, uint32_t key);

void internalInsert(std::shared_ptr<Table>& table, 
                          uint32_t parent_page_num, uint32_t child_page_num);

void internalSplitAndInsert(std::shared_ptr<Table>& table, 
                                    uint32_t parent_page_num, uint32_t child_page_num);

uint32_t getMaxKey(Pager* pager, void* node);
