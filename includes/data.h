#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>
#include <exception>

#include "pager.h"
#include "constants.h"
#include "node.h"


//-----------------------------------------------------------
// Most of the stuff directly related to data storage is in C
//-----------------------------------------------------------

void serialize_row(Row*, void*);

void deserialize_row(void*, Row*);

void print_row(Row*);

typedef struct 
{
	Pager* pager;
    uint32_t root_page_num;
} Table;

typedef struct
{
    std::shared_ptr<Table> table; // Current table
    uint32_t page_num; // Current page number
    uint32_t cell_num; // Current cell number
    bool end_of_table; // Indicates a position one past the last element
} Cursor;

std::unique_ptr<Cursor> table_start(std::shared_ptr<Table>& table);

std::unique_ptr<Cursor> table_find(std::shared_ptr<Table>& table, uint32_t key);

std::shared_ptr<Table> db_open(std::string filename);

void db_close(const std::shared_ptr<Table>& table);

void* cursor_value(std::unique_ptr<Cursor>& cursor);

void cursor_advance(std::unique_ptr<Cursor>& cursor);

void leaf_node_insert(std::unique_ptr<Cursor>& cursor, uint32_t key, Row* value);

void leaf_node_split_and_insert(std::unique_ptr<Cursor>& cursor, uint32_t key, Row* value);

std::unique_ptr<Cursor> leaf_node_find(std::shared_ptr<Table>& table, 
                                       uint32_t page_num, uint32_t key);

void create_new_root(std::shared_ptr<Table>& table, uint32_t right_child_page_num);

std::unique_ptr<Cursor> internal_node_find(std::shared_ptr<Table>& table,
                                           uint32_t page_num, uint32_t key);

void internal_node_insert(std::shared_ptr<Table>& table, 
                          uint32_t parent_page_num, uint32_t child_page_num);

void internal_node_split_and_insert(std::shared_ptr<Table>& table, 
                                    uint32_t parent_page_num, uint32_t child_page_num);

uint32_t get_node_max_key(Pager* pager, void* node);
