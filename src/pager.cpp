#include "../includes/pager.h"

void* Pager::getPages()
{
    return pages;
}

uint32_t Pager::getFileLength()
{
    return fileLength;
}

Pager::Pager(HANDLE fileHandle, uint32_t fileLength, uint32_t pageCount) : 
    fileHandle(fileHandle), fileLength(fileLength), pageCount(pageCount) { }

void* Pager::getPage(uint32_t pageNumber)
{
    if (pageNumber > TABLE_MAX_PAGES)
    {
        throw std::runtime_error("Ran out of pages. Increase TABLE_MAX_PAGES if you need more. "
            + std::to_string(pageNumber) + " > " + std::to_string(TABLE_MAX_PAGES) + ".");
    }

    if (this->pages[pageNumber] == NULL)
    {
        // Cache miss. Allocate memory and load from file
        void* page = new char[PAGE_SIZE];
        this->pageCount = this->fileLength / PAGE_SIZE;

        // We might save a partial page at the end of the file
        if (this->fileLength % PAGE_SIZE)
        {
            this->pageCount++;
        }

        if (pageNumber <= this->pageCount)
        {
            DWORD bytesRead;
            DWORD seekPosition = pageNumber * PAGE_SIZE;

            SetFilePointer(this->fileHandle, seekPosition, nullptr, FILE_BEGIN);
            if (!ReadFile(this->fileHandle, page, PAGE_SIZE, &bytesRead, nullptr))
            {
                throw std::runtime_error("Error reading file: " + std::to_string(GetLastError()));
            }
        }
        this->pages[pageNumber] = page;
        
        if (pageNumber >= this->pageCount)
        {
            this->pageCount = pageNumber + 1;
        }
    }
    return this->pages[pageNumber];
}

HANDLE& Pager::getFileHandle()
{
    return fileHandle;
}

uint32_t& Pager::getPageCount()
{
    return pageCount;
}

// Open pager from an existing .db file
std::unique_ptr<Pager> openPager(std::string filename)
{
    HANDLE fileHandle = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                        0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (GetLastError() == ERROR_FILE_NOT_FOUND )
    {
        throw std::exception("File not found.");
    }

    if (fileHandle == INVALID_HANDLE_VALUE) 
    {
        throw std::runtime_error("Unable to open file.");
    }

    DWORD fileLength = GetFileSize(fileHandle, NULL);

    std::unique_ptr<Pager> pager = std::make_unique<Pager>(fileHandle, fileLength, 
                                                           fileLength / PAGE_SIZE);

    if (fileLength % PAGE_SIZE != 0)
    {
        throw std::runtime_error("Db file is not a whole number of pages. Corrupt file.");
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) 
    {
        pager->pages[i] = NULL;
    }

    return pager;
}

// Create a pager in a new .db file, throw an exception if it already exists
std::unique_ptr<Pager> createPager(std::string filename)
{
    HANDLE fileHandle = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                        0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (GetLastError() == ERROR_FILE_EXISTS)
    {
        throw std::exception("File already exists.");
    }

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Unable to open file.");
    }

    DWORD fileLength = GetFileSize(fileHandle, NULL);

    std::unique_ptr<Pager> pager = std::make_unique<Pager>(fileHandle, fileLength, 
                                                           fileLength / PAGE_SIZE);

    if (fileLength % PAGE_SIZE != 0)
    {
        throw std::runtime_error("Db file is not a whole number of pages. Corrupt file.");
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) 
    {
        pager->pages[i] = NULL;
    }

    return pager;
}

// Flush pager into a file
void Pager::pagerFlush(uint32_t pageNumber)
{
    if (this->pages[pageNumber] == NULL)
    {
        throw std::runtime_error("Db file is not a whole number of pages. Corrupt file.");
    }

    DWORD bytesWritten;
    DWORD seekPosition = pageNumber * PAGE_SIZE;
    SetFilePointer(this->fileHandle, seekPosition, nullptr, FILE_BEGIN);

    if (!WriteFile(this->fileHandle, this->pages[pageNumber], PAGE_SIZE, &bytesWritten, nullptr))
    {
        throw std::runtime_error("Error while writing. Error code: " + std::to_string(GetLastError()));
    }
}

uint32_t Pager::getUnusedPageNumber()
{
    // New pages are always on top of the file since free pages are not reused
    return pageCount;
}
