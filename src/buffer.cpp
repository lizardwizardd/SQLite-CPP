#include "../includes/buffer.h"


InputBuffer::InputBuffer() : buffer(""), input_length(0) { }

void InputBuffer::read_input()
{
	std::getline(std::cin, buffer);

	if (std::cin.fail())
	{
		std::cout << "Error reading input\n";
		exit(EXIT_FAILURE);
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

const size_t InputBuffer::getLength()
{
	return input_length;
}

void print_prompt()
{
	std::cout << "db > ";
}