#include "../includes/data.h"


void serialize_row(Row* source, void* destination) {
    char* dest = static_cast<char*>(destination);
    memcpy(dest + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(dest + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(dest + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    char* src = static_cast<char*>(source);
    memcpy(&(destination->id), src + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), src + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), src + EMAIL_OFFSET, EMAIL_SIZE);
}

void print_row(Row* row)
{
    std::cout << "(" << row->id << ", " << row->username << ", " << row->email << ")" << std::endl;
}

Table* new_table()
{
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table* table)
{
    for (int i = 0; table->pages[i]; i++)
    {
        free(table->pages[i]);
    }
    free(table);
}

void* row_slot(Table* table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table->pages[page_num];
    if (page == NULL)
    {
        // Allocate memory only when we try to access page
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    // Offset from the beginninng of a page
    uint32_t row_offset = row_num % ROWS_PER_PAGE; // in rows
    uint32_t byte_offset = row_offset * ROW_SIZE;  // in bytes

    return static_cast<char*>(page) + byte_offset;
}