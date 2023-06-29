#pragma once

#include <iostream>
#include <string>
#include "../includes/buffer.h"
#include "../includes/statement.h"
#include "../includes/data.h"
#include "../includes/pager.h"

#include "constants.h"

class Database 
{
private:
    Table* table;
    std::shared_ptr<InputBuffer> input_buffer;

    int argc;
    char** argv;

public:
    Database(int argc, char** argv);

    ~Database();

    void run();

    void runDebug(std::vector<std::string> &commands, std::vector <std::string> &results);
};