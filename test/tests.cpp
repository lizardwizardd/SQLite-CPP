#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../includes/database.h"

#include <algorithm>
#include <sstream>

// Custom stream buffer to capture output
// Buffer has to be flushed after each line for the output capturer to work 
class OutputCapturer : public std::stringbuf {
public:
    virtual int sync() {
        std::string output = str();
        if (!output.empty()) {
            outputs.push_back(output);
            str(""); // Clear the buffer
        }
        return 0;
    }

    std::vector<std::string> getOutputs() const {
        return outputs;
    }

private:
    std::vector<std::string> outputs;
};

// Helper function to run script and capture output
// Gets input as a vector of strings 
std::vector<std::string> RunScript(std::vector<std::string>& commands) 
{
    std::vector <std::string> results;

    Database db;
    db.runDebug(commands, results);

    return results;
}

TEST(DB_TEST, InsertLongStrings)
{
    std::vector<std::string> commands;
    std::vector <std::string> results;
    std::vector <std::string> expect;
    
    std::string longUsername(32, 'a');
    std::string longEmail(255, 'a');

    commands = {
        "insert 1 " + longUsername + " " + longEmail,
        "select"
    };
    expect = {
        "db > Executed.\n",
        "db > (1, " + longUsername + ", " + longEmail + ")\n",
        "Executed.\n",
    };
    std::reverse(commands.begin(), commands.end());

    // Redirect std::cout to an std::ostringstream
    OutputCapturer outputCapturer;
    std::streambuf* originalOutputBuffer = std::cout.rdbuf(&outputCapturer);

    results = RunScript(commands);

    // Restore the original std::cout buffer
    std::cout.rdbuf(originalOutputBuffer);

    // Write outputs into a vector
    std::vector<std::string> capturedOutputs = outputCapturer.getOutputs();

    EXPECT_EQ(expect, capturedOutputs);
}

TEST(DB_TEST, ErrorWhenFull)
{
    std::vector<std::string> commands;
    std::vector <std::string> results;
    std::vector <std::string> expect;

    for (int i = 1; i < 1401; i++)
    {
        commands.push_back("insert " + std::to_string(i) + " MyUsername adress@gmail.com");
    }
    std::reverse(commands.begin(), commands.end());

    // Redirect std::cout to an std::ostringstream
    OutputCapturer outputCapturer;
    std::streambuf* originalOutputBuffer = std::cout.rdbuf(&outputCapturer);

    results = RunScript(commands);

    // Restore the original std::cout buffer
    std::cout.rdbuf(originalOutputBuffer);

    // Write outputs into a vector
    std::vector<std::string> capturedOutputs = outputCapturer.getOutputs();

    EXPECT_EQ("db > Error: Table full.\n", capturedOutputs[capturedOutputs.size() - 2]);
}


int main(int argc, char** argv) 
{
    // Initialize the Google Test framework
    testing::InitGoogleTest(&argc, argv);
    // Run all tests
    return RUN_ALL_TESTS();
}
