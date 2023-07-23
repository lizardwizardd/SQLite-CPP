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
#include <iterator>

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
        throw std::runtime_error("Must supply a database filename.");
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
        //print_prompt(); 
        // Dont print prompt in the test version to make expected outputs simpler
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
    std::string longName;
    std::string longEmail;

    void SetUp() override
    {
        longName = std::string(32, 'a');
        longEmail = std::string(255, 'a');
        
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


// Get an int key from row
int getRowKey(const std::string& command)
{
    size_t start = command.find("insert ") + 7;
    size_t end = command.find(" ", start);
    return std::stoi(command.substr(start, end - start));
}

// Custom function to format the entry string
std::string formatRow(const std::string& command) 
{
    size_t start = command.find("insert ") + 7;
    size_t end = command.find(" ", start);
    std::string key = command.substr(start, end - start);

    start = command.find_first_not_of(" ", end);
    end = command.find_first_of(" ", start);
    std::string name = command.substr(start, end - start);

    start = command.find_first_not_of(" ", end);
    end = command.find_first_of(" ", start);
    std::string email = command.substr(start, end - start);

    return "(" + key + ", " + name + ", " + email + ")";
}

void getExpect(std::vector<std::string>& commands, std::vector<std::string>& expect)
{
    std::sort(commands.begin(), commands.end(), []
             (const std::string& lhs, const std::string& rhs)
             {return getRowKey(lhs) < getRowKey(rhs);});
    for (const auto& row : commands)
    {
        expect.push_back(formatRow(row));
    }
}


TEST_F(DB_TEST, InsertAndSelect)
{
    std::vector<std::string> commands = {
        "insert 1 " + longName + " " + longEmail,
        "select",
        ".exit"
    };
    std::vector<std::string> expect = {
        "Executed.",
        "(1, " + longName + ", " + longEmail + ")",
        "Executed."
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    ASSERT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, PersistenceBetweenFiles)
{
    std::vector<std::string> commands = {
        "select",
        ".exit"
    };
    std::vector<std::string> expect = {
        "(1, " + longName + ", " + longEmail + ")",
        "Executed."
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    ASSERT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, InsertDuplicateKey)
{
    std::vector<std::string> commands = {
        "insert 1 Josh_Sawyer josh.sawyer@example.com",
        ".exit"
    };
    std::vector<std::string> expect = {
        "Error: Duplicate key.",
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    ASSERT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, InsertStringTooLong)
{
    std::string longName(33, 'a');
    std::string longEmail(256, 'a');
    
    std::vector<std::string> commands = {  
        "insert 99 " + longName + " " + longEmail, 
        ".exit"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    ASSERT_EQ("Error: String is too long.", outputCapturer.getOutputs().back());
}

TEST_F(DB_TEST, ErrorWhenNegativeId)
{   
    std::vector<std::string> commands = {
        "insert -1 aa aaaaaa", 
        ".exit"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    ASSERT_EQ("Error: ID must be positive.", outputCapturer.getOutputs().back());
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
        "LEAF_NODE_HEADER_SIZE: 14",
        "LEAF_NODE_CELL_SIZE: 297",
        "LEAF_NODE_SPACE_FOR_CELLS: 4082",
        "LEAF_NODE_MAX_CELLS: 13"
    };

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    EXPECT_EQ(expect, outputCapturer.getOutputs());
}

TEST_F(DB_TEST, Insert40Rows)
{
    std::vector<std::string> commands = {
        "insert 17 Ashley_Wilson ashley.wilson@example.com",
        "insert 33 Megan_Clark megan.clark@example.com",
        "insert 11 Samantha_Scott samantha.scott@example.com",
        "insert 2 Jessica_Miller jessica.miller@example.com",
        "insert 26 Rachel_Allen rachel.allen@example.com",
        "insert 35 Joshua_Wright joshua.wright@example.com",
        "insert 4 Benjamin_Carter benjamin.carter@example.com",
        "insert 39 Michael_Johnson michael.johnson@example.com",
        "insert 31 Lauren_Watson lauren.watson@example.com",
        "insert 20 Stephanie_Mitchell stephanie.mitchell@example.com",
        "insert 14 Christopher_Taylor christopher.taylor@example.com",
        "insert 28 Jennifer_Lee jennifer.lee@example.com",
        "insert 7 Kimberly_Flores kimberly.flores@example.com",
        "insert 23 Joseph_Wood joseph.wood@example.com",
        "insert 10 Brandon_Sanchez brandon.sanchez@example.com",
        "insert 6 Amanda_Baker amanda.baker@example.com",
        "insert 25 Joshua_Hall joshua.hall@example.com",
        "insert 12 Sarah_Hernandez sarah.hernandez@example.com",
        "insert 38 Joseph_Ramirez joseph.ramirez@example.com",
        "insert 19 Lauren_Martinez lauren.martinez@example.com",
        "insert 30 Jessica_Hall jessica.hall@example.com",
        "insert 13 Nicholas_Roberts nicholas.roberts@example.com",
        "insert 27 Daniel_Moore daniel.moore@example.com",
        "insert 9 Madison_Moore madison.moore@example.com",
        "insert 5 Emily_Williams emily.williams@example.com",
        "insert 3 Kevin_King kevin.king@example.com",
        "insert 29 Amanda_Davis amanda.davis@example.com",
        "insert 15 Ryan_Garcia ryan.garcia@example.com",
        "insert 36 Emily_Perez emily.perez@example.com",
        "insert 24 Heather_Nelson heather.nelson@example.com",
        "insert 37 Tyler_Garcia tyler.garcia@example.com",
        "insert 8 Samantha_Jones samantha.jones@example.com",
        "insert 40 Nicholas_Young nicholas.young@example.com",
        "insert 21 David_Davis david.davis@example.com",
        "insert 22 Ashley_Richardson ashley.richardson@example.com",
        "insert 32 Jeffrey_Torres jeffrey.torres@example.com",
        "insert 18 Matthew_Anderson matthew.anderson@example.com",
        "insert 16 Christopher_Gonzalez christopher.gonzalez@example.com",
    };

    std::vector<std::string> expect;
    expect.push_back("(1, " + longName + ", " + longEmail + ")");
    getExpect(commands, expect);
    expect.push_back("Executed.");

    commands.push_back("select");
    commands.push_back(".exit");

    Database databaseTest(argc_global, argv_global);
    databaseTest.runTest(commands);

    std::vector<std::string> out = outputCapturer.getOutputs();
    std::vector<std::string> outSelect(out.begin() + 38, out.end());

    EXPECT_EQ(expect, outSelect);
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
