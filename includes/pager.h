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

#include "constants.h"


class Pager
{
public:
    HANDLE file_handle;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];

public:
    void* getPages();

    uint32_t getFileLength();

    void* get_page(uint32_t page_num);

    void pager_flush(uint32_t page_num, uint32_t size);
};

Pager* pager_open(std::string filename);