#include "../includes/database.h"

Database::Database(int argc, char** argv) :
    argc(argc), argv(argv), table(nullptr),  input_buffer(nullptr) {}

Database::~Database()
{ }

void Database::handleMetaCommand()
{
    switch (do_meta_command(input_buffer, table))
    {
        case META_COMMAND_SUCCESS:
            break;
        case PREPARE_SYNTAX_ERROR:
            printErrorMessage("Syntax error. Could not parse statement.");
            break;
        case META_COMMAND_UNRECOGNIZED_COMMAND:
            printErrorMessage("Unrecognized command: " + input_buffer->getBuffer());
            break;
    }
}

void Database::handleStatement()
{
    Statement statement;
    switch (statement.prepare_statement(input_buffer.get()))
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
            printErrorMessage("Unrecognized keyword at the start of " + input_buffer->getBuffer());
            return;
    }

    switch (statement.execute_statement(table))
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
        std::cout << "Must supply a database filename." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string filename = argv[1];
    table = std::move(db_open(filename));
    input_buffer = std::make_shared<InputBuffer>();

    while (true)
    {
        print_prompt();
        input_buffer->read_input();

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
