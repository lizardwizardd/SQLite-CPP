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
#include <Windows.h>
#include <exception>

#include "constants.h"


class Pager
{
private:
    HANDLE fileHandle;
    uint32_t fileLength;
    uint32_t pageCount;

public:
    void* pages[TABLE_MAX_PAGES];

    Pager(HANDLE fileHandle, uint32_t fileLength, uint32_t pageCount);

    void* getPages();
    HANDLE& getFileHandle();
    uint32_t& getPageCount();
    uint32_t getFileLength();
    void* getPage(uint32_t pageNumber);
    uint32_t getUnusedPageNumber();

    void pagerFlush(uint32_t pageNumber);
};

std::unique_ptr<Pager> openPager(std::string filename);
std::unique_ptr<Pager> createPager(std::string filename);