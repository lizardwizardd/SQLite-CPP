#include "../includes/database.h"
#include "database.h"

Database::Database()
{
    table = new_table();
    input_buffer = std::make_shared<InputBuffer>();
}


Database::~Database()
{
    delete table;
}


void Database::run()
{
    while (true)
    {
        print_prompt();
        input_buffer->read_input();

        if (input_buffer->getBuffer().front() == '.')
        {
            switch (do_meta_command(input_buffer.get()))
            {
                case META_COMMAND_SUCCESS:
                    continue;
                case PREPARE_SYNTAX_ERROR:
                    std::cout << "Syntax error. Could not parse statement." << std::endl;
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    std::cout << "Unrecognized command: " << input_buffer->getBuffer() << std::endl;
                    continue;
            }
        }

        Statement statement;
        switch (statement.prepare_statement(input_buffer.get()))
        {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_NEGATIVE_ID:
                std::cout << "ID must be positive." << std::endl;
                continue;
            case PREPARE_STRING_TOO_LONG:
                std::cout << "String is too long." << std::endl;
                continue;
            case PREPARE_SYNTAX_ERROR:
                std::cout << "Syntax error. Could not parse statement." << std::endl;
                continue;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "Unrecognized keyword at the start of " << input_buffer->getBuffer() << std::endl;
                continue;
        }

        switch (statement.execute_statement(table))
        {
            case EXECUTE_SUCCESS:
                std::cout << "Executed.\n";
                break;
            case EXECUTE_TABLE_FULL:
                std::cout << "Error: Table full.\n";
                break;
        }
    }
}


void Database::runDebug(std::vector<std::string> &commands, std::vector <std::string> &results)
{
    while (true)
    {
        print_prompt();
        if (commands.empty())
        {
            return;
        }
        input_buffer->read_input_debug(commands);

        if (input_buffer->getBuffer().front() == '.')
        {
            switch (do_meta_command(input_buffer.get()))
            {
                case META_COMMAND_SUCCESS:
                    continue;
                case PREPARE_SYNTAX_ERROR:
                    std::cout << "Syntax error. Could not parse statement." << std::endl;
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    std::cout << "Unrecognized command: " << input_buffer->getBuffer() << std::endl;
                    continue;
            }
        }

        Statement statement;
        switch (statement.prepare_statement(input_buffer.get()))
        {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_NEGATIVE_ID:
                std::cout << "ID must be positive." << std::endl;
                continue;
            case PREPARE_STRING_TOO_LONG:
                std::cout << "String is too long." << std::endl;
                continue;
            case PREPARE_SYNTAX_ERROR:
                std::cout << "Syntax error. Could not parse statement.\n";
                continue;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "Unrecognized keyword at the start of " << input_buffer->getBuffer() << std::endl;
                continue;
        }

        switch (statement.execute_statement(table))
        {
            case EXECUTE_SUCCESS:
                std::cout << "Executed." << std::endl;
                break;
            case EXECUTE_TABLE_FULL:
                std::cout << "Error: Table full." << std::endl;
                break;
        }
    }
}