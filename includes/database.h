#pragma once

#include <iostream>
#include <string>
#include <exception>

#include "../includes/buffer.h"
#include "../includes/statement.h"
#include "../includes/data.h"
#include "../includes/pager.h"
#include "constants.h"

class Database 
{
private:
    // With current implementation, every time a new table is accessed,
    // old table gets saved and removed from cache, and a new one gets cached.
    std::shared_ptr<Table> cachedTable;
    std::shared_ptr<InputBuffer> inputBuffer;

    int argc;
    char** argv;

public:
    Database(int argc, char** argv);

    void handleMetaCommand();
    void handleStatement();

    void printErrorMessage(const std::string& message);

    void run();
    void runTest(std::vector<std::string>& commands);

    ~Database();
};