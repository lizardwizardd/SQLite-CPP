#include "../includes/buffer.h"


InputBuffer::InputBuffer() : buffer(""), inputLength(0) { }

// Read console input
void InputBuffer::readInput()
{
	std::getline(std::cin, buffer);

	if (std::cin.fail())
	{
        throw std::runtime_error("Error reading input.");
	}

	// Remove spaces and newline characters
	while (buffer.back() == ' ' || buffer.back() == '\n')
	{
		buffer.pop_back();
	}

	inputLength = buffer.size();
}

const std::string InputBuffer::getBuffer() const
{
	return buffer;
}

const size_t InputBuffer::getLength() const
{
	return inputLength;
}

void printPrompt()
{
	std::cout << "db > ";
}