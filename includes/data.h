#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>

#include "pager.h"
#include "constants.h"


//-----------------------------------------------------------
// Most of the stuff directly related to data storage is in C
//-----------------------------------------------------------

void serialize_row(Row*, void*);

void deserialize_row(void*, Row*);

void print_row(Row*);

typedef struct 
{
	uint32_t num_rows;
	Pager* pager;
} Table;

typedef struct
{
    std::shared_ptr<Table> table;
    uint32_t row_num;
    bool end_of_table; // Indicates a position one past the last element
} Cursor;

std::unique_ptr<Cursor> table_start(std::shared_ptr<Table>& table);

std::unique_ptr<Cursor> table_end(std::shared_ptr<Table>& table);

std::shared_ptr<Table> db_open(std::string filename);

void db_close(const std::shared_ptr<Table>& table);

void* cursor_value(std::unique_ptr<Cursor>& cursor);

void cursor_advance(std::unique_ptr<Cursor>& cursor);