#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../includes/database.h"
#include "../includes/buffer.h"
#include "../includes/constants.h"
#include "../includes/data.h"
#include "../includes/pager.h"
#include "../includes/statement.h"

#include <sstream>
#include <algorithm>

int argc_global = 0;
char** argv_global;

// Custom stream buffer to capture output
// Buffer has to be flushed after each line for the output capturer to work 
class OutputCapturer : public std::stringbuf 
{
private:
    std::vector<std::string> outputs;

public:
    virtual int sync() 
    {
        std::string output = str();
        if (!output.empty())
        {
            std::istringstream iss(output);
            std::string line;
            while (std::getline(iss, line))
            {
                outputs.push_back(line);
            }
            str(""); // Clear the buffer
        }
        return 0;
    }

    std::vector<std::string> getOutputs() const 
    {
        return outputs;
    }
};

void Database::runTest(std::vector<std::string>& commands)
{
    std::reverse(commands.begin(), commands.end());

    if (argc < 2)
    {
        std::cout << "Must supply a database filename." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string filename = argv[1];
    table = std::move(db_open(filename));
    input_buffer = std::make_shared<InputBuffer>();

    while (!commands.empty())
    {
        if (commands.back() == ".exit")
        {
            db_close(table);
            return;
        }
        //print_prompt(); // dont print prompt in the test version to make expected outputs more simple
        input_buffer->read_input_test(commands);

        if (input_buffer->getBuffer().front() == '.')
        {   
            handleMetaCommand();
        }
        else
        {
            handleStatement();
        }
    }
}

void InputBuffer::read_input_test(std::vector<std::string> &commands)
{
	this->buffer = commands.back();
    commands.pop_back();

	// Remove spaces and newline characters
	while (buffer.back() == ' ' || buffer.back() == '\n')
	{
		buffer.pop_back();
	}

	input_length = buffer.size();
}


class DB_TEST : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Redirect std::cout to an std::ostringstream
        originalOutputBuffer = std::cout.rdbuf(&outputCapturer);
    }

    void TearDown() override
    {
        // Restore the original std::cout buffer
        std::cout.rdbuf(originalOutputBuffer);
    }

    OutputCapturer outputCapturer;
    std::streambuf* originalOutputBuffer;
};


TEST_F(DB_TEST, InsertAndSelect)
{
    std::vector<std::string> commands = {
        "insert 1 SomeName randomaddress@gmail.com",
        "select",
        ".exit"
    };
    std::vector<std::string> expect = {
        "Executed.",
        "(1, SomeName, randomaddress@gmail.com)",
        "Executed."
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, PersistenceBetweenFiles)
{
    std::vector<std::string> commands = {
        "select",
        ".exit"
    };
    std::vector<std::string> expect = {
        "(1, SomeName, randomaddress@gmail.com)",
        "Executed."
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, InsertMaxLengthStrings)
{
    std::string longName(32, 'a');
    std::string longEmail(255, 'a');
    
    std::vector<std::string> commands = {
        "insert 2 " + longName + " " + longEmail, 
        ".exit"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ("Executed.", outputCapturer.getOutputs().back());
}

TEST_F(DB_TEST, ErrorWhenStringsTooLong)
{
    std::string longName(33, 'a');
    std::string longEmail(256, 'a');
    
    std::vector<std::string> commands = {
        "insert 3 " + longName + " " + longEmail, 
        ".exit"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ("Error: String is too long.", outputCapturer.getOutputs().back());
}

TEST_F(DB_TEST, ErrorWhenNegativeId)
{   
    std::vector<std::string> commands = {
        "insert -1 aa aaaaaa", 
        ".exit"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ("Error: ID must be positive.", outputCapturer.getOutputs().back());
}

TEST_F(DB_TEST, PrintConstants)
{   
    std::vector<std::string> commands = {
        ".constants", 
        ".exit"
    };
    std::vector<std::string> expect = {
        "Constants:",
        "ROW_SIZE: 293",
        "COMMON_NODE_HEADER_SIZE: \x6",
        "LEAF_NODE_HEADER_SIZE: 10",
        "LEAF_NODE_CELL_SIZE: 297",
        "LEAF_NODE_SPACE_FOR_CELLS: 4086",
        "LEAF_NODE_MAX_CELLS: 13"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, ErrorWhenFull)
{
    std::vector<std::string> commands;
    for (int i = 1; i < 1400; i++)
    {
        commands.push_back("insert " + std::to_string(i) + " aaaaaaaaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" );
    }

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ("Error: Table full.", outputCapturer.getOutputs().back());
}

int main(int argc, char** argv) 
{
    argc_global = argc;
    argv_global = argv;
    // Initialize the Google Test framework
    testing::InitGoogleTest(&argc_global, argv_global);
    // Run all tests
    return RUN_ALL_TESTS();
}
