#include "../includes/buffer.h"


InputBuffer::InputBuffer() : buffer(""), input_length(0) { }

void InputBuffer::read_input()
{
	std::getline(std::cin, buffer);

	if (std::cin.fail())
	{
        throw std::runtime_error("Error reading input");
	}

	// Remove spaces and newline characters
	while (buffer.back() == ' ' || buffer.back() == '\n')
	{
		buffer.pop_back();
	}

	input_length = buffer.size();
}

const std::string InputBuffer::getBuffer() const
{
	return buffer;
}

const size_t InputBuffer::getLength() const
{
	return input_length;
}

void print_prompt()
{
	std::cout << "db > ";
}