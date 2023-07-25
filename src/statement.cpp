#include "../includes/statement.h"


// META COMMANDS

MetaCommandResult doMetaCommand(std::shared_ptr<InputBuffer> inputBuffer, 
                                  const std::shared_ptr<Table>& table)
{
	if (inputBuffer->getBuffer() == ".exit")
	{
        closeDatabase(table);
		exit(EXIT_SUCCESS);
	}
    else if (inputBuffer->getBuffer() == ".btree")
    {
        printTree(table->pager, 0, 0);
        return META_COMMAND_SUCCESS;
    }
    else if (inputBuffer->getBuffer() == ".constants")
    {
        printConstants();
        return META_COMMAND_SUCCESS;
    }
	else
	{
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

void printConstants()
{
    std::cout << "Constants:" << std::endl;
    std::cout << "ROW_SIZE: " << ROW_SIZE << std::endl;
    std::cout << "COMMON_NODE_HEADER_SIZE: " << COMMON_NODE_HEADER_SIZE << std::endl;
    std::cout << "LEAF_NODE_HEADER_SIZE: " << LEAF_NODE_HEADER_SIZE << std::endl;
    std::cout << "LEAF_NODE_CELL_SIZE: " << LEAF_NODE_CELL_SIZE << std::endl;
    std::cout << "LEAF_NODE_SPACE_FOR_CELLS: " << LEAF_NODE_SPACE_FOR_CELLS << std::endl;
    std::cout << "LEAF_NODE_MAX_CELLS: " << LEAF_NODE_MAX_CELLS << std::endl;
}

// Utility function to plrint spaces for printTree
void indent(uint32_t level) 
{
    for (uint32_t i = 0; i < level; i++) 
    {
        std::cout << "    ";
    }
}

void printTree(Pager* pager, uint32_t pageNumber, uint32_t indentation_level) {
    void* node = pager->getPage(pageNumber);
    uint32_t keyCount, child;

    switch (nodeGetType(node)) 
    {
        case (NODE_LEAF):
            keyCount = *leafGetCellCount(node);
            indent(indentation_level);
            std::cout << "- leaf (size " << keyCount << ")\n";
            for (uint32_t i = 0; i < keyCount; i++)
            {
                indent(indentation_level + 1);
                std::cout << "- " << *leafGetKey(node, i) << "\n";
            }
            break;
        case (NODE_INTERNAL):
            keyCount = *internalGetKeyCount(node);
            indent(indentation_level);
            std::cout << "- internal (size " << keyCount << ")\n";
            if (keyCount > 0)
            {
                for (uint32_t i = 0; i < keyCount; i++)
                {
                    child = *internalGetChild(node, i);
                    printTree(pager, child, indentation_level + 1);

                    indent(indentation_level + 1);
                    std::cout << "- key " << *internalGetKey(node, i) << "\n";
                }
                child = *internalGetRightChild(node);
                printTree(pager, child, indentation_level + 1);
            }
            break;
    }
    std::cout << std::flush;
}

// STATEMENTS

Statement::Statement() : type() { };

PrepareResult Statement::prepareStatement(InputBuffer* inputBuffer)
{
	if (inputBuffer->getBuffer().compare(0, 6, "insert", 0, 6) == 0)
	{
		type = STATEMENT_INSERT;

		std::string args = inputBuffer->getBuffer();
		std::stringstream argStream(args.substr(6, args.size()));

        int32_t tmpId;
        std::string tmpUsername;
        std::string tmpEmail;

		if (!(argStream >> tmpId >> tmpUsername >> tmpEmail))
		{
			return PREPARE_SYNTAX_ERROR;
		}
        if (tmpId < 1)
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

        rowToInsert.id = tmpId;
        strcpy_s(rowToInsert.email, tmpEmail.c_str());
        strcpy_s(rowToInsert.username, tmpUsername.c_str());

		return PREPARE_SUCCESS;
	}

	if (inputBuffer->getBuffer() == "select")
	{
		type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult Statement::executeInsert(std::shared_ptr<Table>& table)
{
    // Point cursor at the position for a new key 
	uint32_t keyToInsert = this->rowToInsert.id;
    std::unique_ptr<Cursor> cursor = tableFindKey(table, keyToInsert);

    void* node = table->pager->getPage(cursor->pageNumber);
    uint32_t cellCount = *leafGetCellCount(node);

    if (cursor->cellCount < cellCount)
    {
        uint32_t keyAtIndex = *leafGetKey(node, cursor->cellCount);
        if (keyAtIndex == keyToInsert)
        {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

	leafInsert(cursor, this->rowToInsert.id, &rowToInsert);

	return EXECUTE_SUCCESS;
}

ExecuteResult Statement::executeSelect(std::shared_ptr<Table>& table)
{
    std::unique_ptr<Cursor> cursor = tableStart(table);

	Row row;
	while (!(cursor->endOfTable))
	{
		deserialize_row(cursorValue(cursor), &row);
		print_row(&row);
        cursorAdvance(cursor);
	}

	return EXECUTE_SUCCESS;
}

ExecuteResult Statement::executeStatement(std::shared_ptr<Table>& table)
{
	switch (type)
	{
	case (STATEMENT_INSERT):
		return executeInsert(table);
	case (STATEMENT_SELECT):
		return executeSelect(table);
	}
}

const StatementType Statement::getStatement() const
{
	return type;
}

const Row Statement::getRow() const
{
	return rowToInsert;
}
