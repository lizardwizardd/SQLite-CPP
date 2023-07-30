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
public:
    HANDLE fileHandle;
    uint32_t fileLength;
    uint32_t pageCount;
    void* pages[TABLE_MAX_PAGES];

public:
    void* getPages();

    uint32_t getFileLength();

    void* getPage(uint32_t pageNumber);

    void pagerFlush(uint32_t pageNumber);

    uint32_t getUnusedPageNumber();
};

Pager* openPager(std::string filename);

Pager* createPager(std::string filename);