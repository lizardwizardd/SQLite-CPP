#include "../includes/database.h"

Database::Database(int argc, char** argv) :
    argc(argc), argv(argv), table(nullptr),  inputBuffer(nullptr) {}

Database::~Database()
{ }

void Database::handleMetaCommand()
{
    switch (doMetaCommand(inputBuffer, table))
    {
        case META_COMMAND_SUCCESS:
            break;
        case PREPARE_SYNTAX_ERROR:
            printErrorMessage("Syntax error. Could not parse statement.");
            break;
        case META_COMMAND_UNRECOGNIZED_COMMAND:
            printErrorMessage("Unrecognized command: " + inputBuffer->getBuffer());
            break;
    }
}

void Database::handleStatement()
{
    Statement statement;
    switch (statement.prepareStatement(inputBuffer.get()))
    {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_NEGATIVE_ID:
            printErrorMessage("ID must be positive.");
            return;
        case PREPARE_STRING_TOO_LONG:
            printErrorMessage("String is too long.");
            return;
        case PREPARE_SYNTAX_ERROR:
            printErrorMessage("Syntax error. Could not parse statement.");
            return;
        case PREPARE_UNRECOGNIZED_STATEMENT:
            printErrorMessage("Unrecognized keyword at the start of "+ 
                               inputBuffer->getBuffer());
            return;
    }

    switch (statement.executeStatement(table))
    {
        case EXECUTE_SUCCESS:
            std::cout << "Executed." << std::endl;
            break;
        case EXECUTE_DUPLICATE_KEY:
            std::cout << "Error: Duplicate key." << std::endl;
            break;
        case EXECUTE_TABLE_FULL:
            std::cout << "Error: Table full." << std::endl;
            break;
    }
}

void Database::printErrorMessage(const std::string& message)
{
    std::cout << "Error: " << message << std::endl;
}

void Database::run()
{
    if (argc < 2)
    {
        throw std::runtime_error("Must supply a database filename.");
    }

    std::string filename = argv[1];
    table = std::move(openDatabase(filename));
    inputBuffer = std::make_shared<InputBuffer>();

    while (true)
    {
        printPrompt();
        inputBuffer->readInput();

        if (inputBuffer->getBuffer().front() == '.')
        {
            handleMetaCommand();
        }
        else
        {
            handleStatement();
        }
    }
}
