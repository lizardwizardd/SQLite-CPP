#include "../includes/statement.h"


// META COMMANDS

MetaCommandResult do_meta_command(InputBuffer* input_buffer)
{
	if (input_buffer->getBuffer() == ".exit")
	{
		exit(EXIT_SUCCESS);
	}
	else
	{
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

// STATEMENTS

Statement::Statement() : type() { };

PrepareResult Statement::prepare_statement(InputBuffer* input_buffer)
{
	if (input_buffer->getBuffer().compare(0, 6, "insert", 0, 6) == 0)
	{
		type = STATEMENT_INSERT;

		std::string args = input_buffer->getBuffer();
		std::stringstream argStream(args.substr(6, args.size()));

        int32_t tmpId;
        std::string tmpUsername;
        std::string tmpEmail;

		if (!(argStream >> tmpId >> tmpUsername >> tmpEmail))
		{
			return PREPARE_SYNTAX_ERROR;
		}
        if (tmpId < 0)
        {
            return PREPARE_NEGATIVE_ID;
        }
        if (tmpEmail.size() > COLUMN_EMAIL_SIZE)
        {
            return PREPARE_STRING_TOO_LONG;
        }
        if (tmpUsername.size() > COLUMN_USERNAME_SIZE)
        {
            return PREPARE_STRING_TOO_LONG;
        }

        row_to_insert.id = tmpId;
        strcpy_s(row_to_insert.email, tmpEmail.c_str());
        strcpy_s(row_to_insert.username, tmpUsername.c_str());

		return PREPARE_SUCCESS;
	}

	if (input_buffer->getBuffer() == "select")
	{
		type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult Statement::execute_insert(Table* table)
{
	if (table->num_rows >= TABLE_MAX_ROWS)
	{
		return EXECUTE_TABLE_FULL;
	}

	Row* row_to_insert_ptr = &(this->row_to_insert);
	serialize_row(row_to_insert_ptr, row_slot(table, table->num_rows));
	table->num_rows += 1;

	return EXECUTE_SUCCESS;
}

ExecuteResult Statement::execute_select(Table* table)
{
	Row row;
	for (uint32_t i = 0; i < table->num_rows; i++)
	{
		deserialize_row(row_slot(table, i), &row);
		print_row(&row);
	}
	return EXECUTE_SUCCESS;
}

ExecuteResult Statement::execute_statement(Table* table)
{
	switch (type)
	{
	case (STATEMENT_INSERT):
		return execute_insert(table);
	case (STATEMENT_SELECT):
		return execute_select(table);
	}
}

const StatementType Statement::getStatement() const
{
	return type;
}

const Row Statement::getRow() const
{
	return row_to_insert;
}
