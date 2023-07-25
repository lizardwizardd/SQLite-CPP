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
    std::shared_ptr<Table> table;
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