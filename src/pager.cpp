#include "../includes/pager.h"

void* Pager::getPages()
{
    return pages;
}

uint32_t Pager::getFileLength()
{
    return file_length;
}

void* Pager::get_page(uint32_t page_num)
{
    if (page_num > TABLE_MAX_PAGES)
    {
        std::cout << "Tried to fetch page number out of bounds. " << 
            page_num << " > " << TABLE_MAX_PAGES << std::endl;
        exit(EXIT_FAILURE);
    }

    if (this->pages[page_num] == NULL)
    {
        // Cache miss. Allocate memory and load from file
        void* page = new char[PAGE_SIZE];
        this->num_pages = this->file_length / PAGE_SIZE;

        // We might save a partial page at the end of the file
        if (this->file_length % PAGE_SIZE)
        {
            this->num_pages++;
        }

        if (page_num <= this->num_pages)
        {
            DWORD bytes_read;
            DWORD seek_position = page_num * PAGE_SIZE;

            SetFilePointer(this->file_handle, seek_position, nullptr, FILE_BEGIN);
            if (!ReadFile(this->file_handle, page, PAGE_SIZE, &bytes_read, nullptr))
            {
                std::cout << "Error reading file: " << GetLastError() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        this->pages[page_num] = page;
        
        if (page_num >= this->num_pages)
        {
            this->num_pages = page_num + 1;
        }
    }
    return this->pages[page_num];
}


Pager* pager_open(std::string filename)
{
    HANDLE file_handle = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle == INVALID_HANDLE_VALUE) {
        std::cout << "Unable to open file" << std::endl;
        exit(EXIT_FAILURE);
    }

    DWORD file_length = GetFileSize(file_handle, NULL);

    Pager* pager = new Pager();
    pager->file_handle = file_handle;
    pager->file_length = file_length;
    pager->num_pages = file_length / PAGE_SIZE;

    if (file_length % PAGE_SIZE != 0)
    {
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) 
    {
        pager->pages[i] = NULL;
    }

    return pager;
}


void Pager::pager_flush(uint32_t page_num)
{
    if (this->pages[page_num] == NULL)
    {
        std::cout << "Tried to flush null page" << std::endl;
        exit(EXIT_FAILURE);
    }

    DWORD bytes_written;
    DWORD seek_position = page_num * PAGE_SIZE;
    SetFilePointer(this->file_handle, seek_position, nullptr, FILE_BEGIN);

    if (!WriteFile(this->file_handle, this->pages[page_num], PAGE_SIZE, &bytes_written, nullptr))
    {
        std::cout << "Error writing: " << GetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Until we start recycling free pages, new pages will always
// go onto the end of the database file
uint32_t Pager::get_unused_page_num()
{
    return num_pages;
}