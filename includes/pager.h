#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <exception>

#include "constants.h"

#ifdef _WIN32
#include <Windows.h>
#endif


class Pager
{
private:
#ifdef _WIN32
    HANDLE fileHandle;
#else
    int fileHandle; // file descriptor instead of Windows.h handle
#endif
    uint32_t fileLength;
    uint32_t pageCount;

public:
    void* pages[TABLE_MAX_PAGES];

public:
#ifdef _WIN32
    Pager(HANDLE fileHandle, uint32_t fileLength, uint32_t pageCount);

    HANDLE& getFileHandle();
#else
    Pager(int fileHandle, uint32_t fileLength, uint32_t pageCount);

    int& getFileHandle();
#endif
    void* getPages();
    
    uint32_t& getPageCount();

    uint32_t getFileLength();

    void* getPage(uint32_t pageNumber);

    void pagerFlush(uint32_t pageNumber);

    uint32_t getUnusedPageNumber();
};

std::unique_ptr<Pager> openPager(std::string filename);

std::unique_ptr<Pager> createPager(std::string filename);