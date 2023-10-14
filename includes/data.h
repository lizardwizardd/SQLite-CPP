#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>
#include <exception>

#include "pager.h"
#include "constants.h"
#include "node.h"


//------------------------------------------------------------------------
// Most of the operations with data are lower-level for better performance
//------------------------------------------------------------------------

void serializeRow(Row*, void*);

void deserializeRow(void*, Row*);

void printRow(Row*);

class Table 
{
public:
	std::unique_ptr<Pager> pager;
    uint32_t rootPageNumber;

public:
    Table(std::unique_ptr<Pager> pager, uint32_t rootPageNumber);
};

class Cursor
{
public:
    std::shared_ptr<Table> table; // current table
    uint32_t pageNumber; // current page number
    uint32_t cellCount; // number of cells
    bool endOfTable; // indicates a position one past the last element

public:
    Cursor& operator++(int);
};

std::unique_ptr<Cursor> tableStart(std::shared_ptr<Table>& table);

std::unique_ptr<Cursor> tableFindKey(std::shared_ptr<Table>& table, const uint32_t key);

std::shared_ptr<Table> openDatabase(std::string filename);

std::shared_ptr<Table> createDatabase(std::string filename);

std::shared_ptr<Table> dropDatabase(std::string filename);

void saveAndCloseDatabase(const std::shared_ptr<Table>& table);

void freeTable(const std::shared_ptr<Table>& table);

void saveTable(const std::shared_ptr<Table>& table);

void* cursorValue(std::unique_ptr<Cursor>& cursor);

void cursorAdvance(std::unique_ptr<Cursor>& cursor);

void leafInsert(std::unique_ptr<Cursor>& cursor, const uint32_t key, Row* value);

void leafUpdate(std::unique_ptr<Cursor>& cursor, Row* value);

void leafSplitAndInsert(std::unique_ptr<Cursor>& cursor, const uint32_t key, Row* value);

std::unique_ptr<Cursor> findLeafNode(std::shared_ptr<Table>& table, 
                                       uint32_t pageNumber, const uint32_t key);

void createNewRootNode(std::shared_ptr<Table>& table, uint32_t right_child_page_num);

std::unique_ptr<Cursor> findInternalNode(std::shared_ptr<Table>& table,
                                           uint32_t pageNumber, const uint32_t key);

void internalInsert(std::shared_ptr<Table>& table, 
                          uint32_t parent_page_num, uint32_t child_page_num);

void internalSplitAndInsert(std::shared_ptr<Table>& table,
                                    uint32_t parent_page_num, uint32_t child_page_num);

uint32_t getMaxKey(const std::unique_ptr<Pager>& pager, void* node);
