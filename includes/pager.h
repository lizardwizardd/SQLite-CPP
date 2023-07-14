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
    HANDLE file_handle;
    uint32_t file_length;
    uint32_t num_pages;
    void* pages[TABLE_MAX_PAGES];

public:
    void* getPages();

    uint32_t getFileLength();

    void* get_page(uint32_t page_num);

    void pager_flush(uint32_t page_num);

    uint32_t get_unused_page_num();
};

Pager* pager_open(std::string filename);