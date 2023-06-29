#include "../includes/database.h"

Database::Database(int argc, char** argv) :
    argc(argc), argv(argv), table(NULL), input_buffer(NULL) {}

Database::~Database()
{
    delete table;
}

void Database::run()
{
    // Exit if database file name was not provided
    if (argc < 2)
    {
        std::cout << "Must supply a database filename." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string filename = argv[1];
    table = db_open(filename);
    input_buffer = std::make_shared<InputBuffer>();

    while (true)
    {
        print_prompt();
        input_buffer->read_input();

        if (input_buffer->getBuffer().front() == '.')
        {
            switch (do_meta_command(input_buffer.get(), table))
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
