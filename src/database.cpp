#include "../includes/database.h"

Database::Database(int argc, char** argv) :
    argc(argc), argv(argv), cachedTable(nullptr),  inputBuffer(nullptr) {}

Database::~Database()
{ }

void Database::handleMetaCommand()
{
    switch (doMetaCommand(inputBuffer, cachedTable))
    {
        case MetaCommandResult::META_COMMAND_SUCCESS:
            break;
        case MetaCommandResult::META_COMMAND_SYNTAX_ERROR:
            printErrorMessage("Syntax error. Could not parse statement.");
            break;
        case MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND:
            printErrorMessage("Unrecognized command: " + inputBuffer->getBuffer());
            break;
    }
}

void Database::handleStatement()
{
    Statement statement;
    switch (statement.prepareStatement(inputBuffer.get()))
    {
        case PrepareResult::PREPARE_SUCCESS:
            break;
        case PrepareResult::PREPARE_NEGATIVE_ID:
            printErrorMessage("ID must be positive.");
            return;
        case PrepareResult::PREPARE_STRING_TOO_LONG:
            printErrorMessage("String is too long.");
            return;
        case PrepareResult::PREPARE_SYNTAX_ERROR:
            printErrorMessage("Syntax error. Could not parse statement.");
            return;
        case PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT:
            printErrorMessage("Unrecognized keyword at the start of "+ 
                               inputBuffer->getBuffer());
            return;
        default:
            throw std::exception("Unknown statement.");
    }

    switch (statement.executeStatement(cachedTable))
    {
        case ExecuteResult::EXECUTE_SUCCESS:
            std::cout << "Executed." << std::endl;
            break;
        case ExecuteResult::EXECUTE_DUPLICATE_KEY:
            std::cout << "Error: Duplicate key." << std::endl;
            break;
        case ExecuteResult::EXECUTE_KEY_DOES_NOT_EXIST:
            std::cout << "Error: Key does not exist." << std::endl;
            break;
        case ExecuteResult::EXECUTE_TABLE_FULL:
            std::cout << "Error: Table full." << std::endl;
            break;
        case ExecuteResult::EXECUTE_ERROR_WHILE_CREATING:
            std::cout << "Error: Failed to create a table. " << std::endl;
            break;
        case ExecuteResult::EXECUTE_ERROR_WHILE_OPENING:
            std::cout << "Error: Failed to open a table. Error code: " << GetLastError() << std::endl;
            break;
        case ExecuteResult::EXECUTE_ERROR_WHILE_DROPPING:
            std::cout << "Error: Failed to drop a table. Error code: " << GetLastError() << std::endl;
            break;
        case ExecuteResult::EXECUTE_TABLE_NOT_SELECTED:
            std::cout << "Error: Table not opened. Use \"create/open table [name]\" to create/open a table" << std::endl;
            break;
        case ExecuteResult::EXECUTE_ERROR_FILE_NOT_FOUND:
            std::cout << "Error: Table with the name \"" + statement.getTableName() + ".db\" was not found." << std::endl;
            break;
        case ExecuteResult::EXECUTE_ERROR_FILE_EXISTS:
            std::cout << "Error: Table with the name \"" + statement.getTableName() + ".db\" already exists." << std::endl;
            break;
        default:
            throw std::exception("Unknown statement result.");
    }
}

void Database::printErrorMessage(const std::string& message)
{
    std::cout << "Error: " << message << std::endl;
}

void Database::run()
{
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
