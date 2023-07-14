#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <memory>
#include <exception>

#include "constants.h"

class InputBuffer
{
private:
    std::string buffer;
    size_t input_length;

public:
    InputBuffer();

    void read_input();
    void read_input_test(std::vector<std::string> &commands);

    const std::string getBuffer() const;
    const size_t getLength() const;
};

void print_prompt();