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

Table* db_open(std::string filename);

void db_close(Table* table);

void free_table(Table*);

void* row_slot(Table*, uint32_t);