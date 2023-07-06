#include "../includes/data.h"


void serialize_row(Row* source, void* destination) 
{
    char* dest = static_cast<char*>(destination);
    memcpy(dest + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(dest + USERNAME_OFFSET, source->username, USERNAME_SIZE);
    memcpy(dest + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) 
{
    char* src = static_cast<char*>(source);
    memcpy(&(destination->id), src + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), src + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), src + EMAIL_OFFSET, EMAIL_SIZE);
}

void print_row(Row* row)
{
    std::cout << "(" << row->id << ", " << row->username << ", " << row->email << ")" << std::endl;
}

std::unique_ptr<Cursor> table_start(std::shared_ptr<Table>& table)
{
    std::unique_ptr<Cursor> cursor = std::make_unique<Cursor>();
    cursor->table = table;
    cursor->row_num = 0;
    cursor->end_of_table = (table->num_rows == 0);

    return cursor;
}

std::unique_ptr<Cursor> table_end(std::shared_ptr<Table>& table)
{
    std::unique_ptr<Cursor> cursor = std::make_unique<Cursor>();
    cursor->table = table;
    cursor->row_num = table->num_rows;
    cursor->end_of_table = true;

    return cursor;
}

std::shared_ptr<Table> db_open(std::string filename)
{
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->getFileLength() / ROW_SIZE;

    std::shared_ptr<Table> table = std::make_unique<Table>();

    table->pager = pager;
    table->num_rows = num_rows;
    return table;
}

void db_close(const std::shared_ptr<Table>& table)
{
    Pager* pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < num_full_pages; i++)
    {
        if (pager->pages[i] == NULL)
        {
            continue;
        }
        pager->pager_flush(i, PAGE_SIZE);
        delete pager->pages[i];
        pager->pages[i] = NULL;
    }

    // There may be a partial page to write to the end of the file
    // This should not be needed after we switch to a B-tree
    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0)
    {
        uint32_t page_num = num_full_pages;
        if (pager->pages[page_num] != NULL)
        {
            pager->pager_flush(page_num, num_additional_rows * ROW_SIZE);
            delete pager->pages[page_num];
            pager->pages[page_num] = NULL;
        }
    }

    if (CloseHandle(pager->file_handle) == 0)
    {
        std::cout << "Error closing db file." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        void* page = pager->pages[i];
        if (page)
        {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    delete pager;
}

void* cursor_value(std::unique_ptr<Cursor>& cursor)
{
    uint32_t row_num = cursor->row_num;
    uint32_t page_num = row_num / ROWS_PER_PAGE;

    void* page = cursor->table->pager->get_page(page_num);

    // Offset from the beginninng of a page
    uint32_t row_offset = row_num % ROWS_PER_PAGE; // in rows
    uint32_t byte_offset = row_offset * ROW_SIZE;  // in bytes

    return static_cast<char*>(page) + byte_offset;
}

void cursor_advance(std::unique_ptr<Cursor>& cursor)
{
    cursor->row_num += 1;
    if (cursor->row_num >= cursor->table->num_rows)
    {
        cursor->end_of_table = true;
    }
}