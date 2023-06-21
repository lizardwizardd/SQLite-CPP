#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <memory>

class InputBuffer
{
private:
	std::string buffer;
	size_t input_length;

public:
	InputBuffer();

    void read_input();
    void read_input_debug(std::vector<std::string> &commands);

const std::string getBuffer() const;

const size_t getLength();

};

void print_prompt();