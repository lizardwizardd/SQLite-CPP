#include "../includes/statement.h"


// META COMMANDS

MetaCommandResult do_meta_command(std::shared_ptr<InputBuffer> input_buffer, 
                                  const std::shared_ptr<Table>& table)
{
	if (input_buffer->getBuffer() == ".exit")
	{
        db_close(table);
		exit(EXIT_SUCCESS);
	}
    else if (input_buffer->getBuffer() == ".btree")
    {
        print_tree(table->pager, 0, 0);
        return META_COMMAND_SUCCESS;
    }
    else if (input_buffer->getBuffer() == ".constants")
    {
        print_constants();
        return META_COMMAND_SUCCESS;
    }
	else
	{
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

void print_constants()
{
    std::cout << "Constants:" << std::endl;
    std::cout << "ROW_SIZE: " << ROW_SIZE << std::endl;
    std::cout << "COMMON_NODE_HEADER_SIZE: " << COMMON_NODE_HEADER_SIZE << std::endl;
    std::cout << "LEAF_NODE_HEADER_SIZE: " << LEAF_NODE_HEADER_SIZE << std::endl;
    std::cout << "LEAF_NODE_CELL_SIZE: " << LEAF_NODE_CELL_SIZE << std::endl;
    std::cout << "LEAF_NODE_SPACE_FOR_CELLS: " << LEAF_NODE_SPACE_FOR_CELLS << std::endl;
    std::cout << "LEAF_NODE_MAX_CELLS: " << LEAF_NODE_MAX_CELLS << std::endl;
}

void indent(uint32_t level) 
{
    for (uint32_t i = 0; i < level; i++) 
    {
        std::cout << "    ";
    }
}

void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level) 
{
    void* node = pager->get_page(page_num);
    uint32_t num_keys, child;

    switch (get_node_type(node)) 
    {
        case (NODE_LEAF):
            num_keys = *leaf_node_num_cells(node);
            indent(indentation_level);
            std::cout << "- leaf (size " << num_keys << ")\n";
            for (uint32_t i = 0; i < num_keys; i++) 
            {
                indent(indentation_level + 1);
                std::cout << "- " << *leaf_node_key(node, i) << "\n";
            }
            break;
        case (NODE_INTERNAL):
            num_keys = *internal_node_num_keys(node);
            indent(indentation_level);
            std::cout << "- internal (size " << num_keys << ")\n";
            for (uint32_t i = 0; i < num_keys; i++) {
                child = *internal_node_child(node, i);
                print_tree(pager, child, indentation_level + 1);

                indent(indentation_level + 1);
                std::cout << "- key " << *internal_node_key(node, i) << "\n";
            }
            child = *internal_node_right_child(node);
            print_tree(pager, child, indentation_level + 1);
            break;
    }

    std::cout << std::flush;
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

ExecuteResult Statement::execute_insert(std::shared_ptr<Table>& table)
{
    // Point cursor at the position for a new key 
	uint32_t key_to_insert = this->row_to_insert.id;
    std::unique_ptr<Cursor> cursor = table_find(table, key_to_insert);

    void* node = table->pager->get_page(table->root_page_num);
    uint32_t num_cells = (*leaf_node_num_cells(node));

    if (cursor->cell_num < num_cells)
    {
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if (key_at_index == key_to_insert)
        {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

	leaf_node_insert(cursor, this->row_to_insert.id, &row_to_insert);

	return EXECUTE_SUCCESS;
}

ExecuteResult Statement::execute_select(std::shared_ptr<Table>& table)
{
    std::unique_ptr<Cursor> cursor = table_start(table);

	Row row;
	while (!(cursor->end_of_table))
	{
		deserialize_row(cursor_value(cursor), &row);
		print_row(&row);
        cursor_advance(cursor);
	}

	return EXECUTE_SUCCESS;
}

ExecuteResult Statement::execute_statement(std::shared_ptr<Table>& table)
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
