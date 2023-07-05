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

std::unique_ptr<Table> db_open(std::string filename);

void db_close(const std::unique_ptr<Table>& table);

void* row_slot(const std::unique_ptr<Table>& table, uint32_t);