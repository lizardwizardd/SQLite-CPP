#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100


typedef struct
{
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE + 1];
	char email[COLUMN_EMAIL_SIZE + 1];
} Row;


const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

//-----------------------------------------------------------
// Most of the stuff directly related to data storage is in C
//-----------------------------------------------------------

void serialize_row(Row*, void*);

void deserialize_row(void*, Row*);

void print_row(Row*);

typedef struct 
{
	uint32_t num_rows;
	void* pages[TABLE_MAX_PAGES];
} Table;

Table* new_table();

void free_table(Table*);

void* row_slot(Table*, uint32_t);