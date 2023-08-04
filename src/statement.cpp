#include "../includes/statement.h"


// META COMMANDS

MetaCommandResult doMetaCommand(std::shared_ptr<InputBuffer> inputBuffer, 
                                  const std::shared_ptr<Table>& table)
{
	if (inputBuffer->getBuffer() == ".exit")
	{
        saveAndCloseDatabase(table);
		exit(EXIT_SUCCESS);
	}
    if (inputBuffer->getBuffer() == ".save")
	{
        saveTable(table);
		std::cout << "Executed." << std::endl;
        return MetaCommandResult::META_COMMAND_SUCCESS;
	}
    else if (inputBuffer->getBuffer() == ".btree")
    {
        printTree(table->pager, 0, 0);
        return MetaCommandResult::META_COMMAND_SUCCESS;
    }
    else if (inputBuffer->getBuffer() == ".constants")
    {
        printConstants();
        return MetaCommandResult::META_COMMAND_SUCCESS;
    }
	else
	{
		return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

// STATEMENTS

Statement::Statement() : type(), tableName("") { };

PrepareResult Statement::prepareStatement(InputBuffer* inputBuffer)
{
    if (inputBuffer->getBuffer().compare(0, 12, "create table", 0, 12) == 0)
    {
        type = StatementType::STATEMENT_CREATE;

        std::string args = inputBuffer->getBuffer();
        std::stringstream argStream(args.substr(12, args.size()));

        std::string _tableName;

        if (!(argStream >> _tableName))
        {
            return PrepareResult::PREPARE_SYNTAX_ERROR;
        }

        this->tableName = _tableName;

        return PrepareResult::PREPARE_SUCCESS;
    }
    else if (inputBuffer->getBuffer().compare(0, 10, "open table", 0, 10) == 0)
    {
        type = StatementType::STATEMENT_OPEN;

        std::string args = inputBuffer->getBuffer();
        std::stringstream argStream(args.substr(10, args.size()));

        std::string _tableName;

        if (!(argStream >> _tableName))
        {
            return PrepareResult::PREPARE_SYNTAX_ERROR;
        }

        this->tableName = _tableName;

        return PrepareResult::PREPARE_SUCCESS;
    }
    else if (inputBuffer->getBuffer().compare(0, 10, "drop table", 0, 10) == 0)
    {
        type = StatementType::STATEMENT_DROP;

        std::string args = inputBuffer->getBuffer();
        std::stringstream argStream(args.substr(10, args.size()));

        std::string _tableName;

        if (!(argStream >> _tableName))
        {
            return PrepareResult::PREPARE_SYNTAX_ERROR;
        }

        this->tableName = _tableName;

        return PrepareResult::PREPARE_SUCCESS;
    }
	else if (inputBuffer->getBuffer().compare(0, 6, "insert", 0, 6) == 0)
	{
		type = StatementType::STATEMENT_INSERT;

		std::string args = inputBuffer->getBuffer();
		std::stringstream argStream(args.substr(6, args.size()));

        int32_t _id;
        std::string _username;
        std::string _email;

		if (!(argStream >> _id >> _username >> _email))
		{
			return PrepareResult::PREPARE_SYNTAX_ERROR;
		}
        if (_id < 1)
        {
            return PrepareResult::PREPARE_NEGATIVE_ID;
        }
        if (_email.size() > COLUMN_EMAIL_SIZE)
        {
            return PrepareResult::PREPARE_STRING_TOO_LONG;
        }
        if (_username.size() > COLUMN_USERNAME_SIZE)
        {
            return PrepareResult::PREPARE_STRING_TOO_LONG;
        }

        rowToInsert.id = _id;
        strcpy_s(rowToInsert.email, _email.c_str());
        strcpy_s(rowToInsert.username, _username.c_str());

		return PrepareResult::PREPARE_SUCCESS;
	}

	if (inputBuffer->getBuffer() == "select")
	{
		type = StatementType::STATEMENT_SELECT;
		return PrepareResult::PREPARE_SUCCESS;
	}

	return PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT;
}

// Change cached table to the new one if created successfully,
// don't change chached table if an error occured
ExecuteResult Statement::executeCreate(std::shared_ptr<Table>& table)
{
    std::shared_ptr<Table> _table;
    try
    {
        _table = std::move(createDatabase(tableName + ".db"));
    }
    catch (...)
    {
        if (GetLastError() == ERROR_FILE_EXISTS)
        {
            return ExecuteResult::EXECUTE_ERROR_FILE_EXISTS;
        }
        else
        {
            return ExecuteResult::EXECUTE_ERROR_WHILE_CREATING;
        }
    }

    // New table created successfully
    if (table != nullptr)
        saveAndCloseDatabase(table);
    table = _table;

    return ExecuteResult::EXECUTE_SUCCESS;
}

// Change cached table to the new one if opened successfully,
// don't change chached table if an error occured
ExecuteResult Statement::executeOpen(std::shared_ptr<Table>& table)
{
    std::shared_ptr<Table> _table;
    try
    {
        _table = std::move(openDatabase(tableName + ".db"));
    }
    catch (...)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return ExecuteResult::EXECUTE_ERROR_FILE_NOT_FOUND;
        }
        else
        {
            return ExecuteResult::EXECUTE_ERROR_WHILE_OPENING;
        }
    }

    // New table opened successfully
    if (table != nullptr)
        saveAndCloseDatabase(table);
    table = _table;

    return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult Statement::executeDrop(std::shared_ptr<Table>& table)
{
    this->attempts++;
    try
    {
        dropDatabase(tableName + ".db");
    }
    catch (...)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return ExecuteResult::EXECUTE_ERROR_FILE_NOT_FOUND;
        }
        // SHARING_VIOLATION happens when file can't be accessed
        // because it's currently open
        else if (GetLastError() == ERROR_SHARING_VIOLATION)
        {
            // Close file first
            CloseHandle(table->pager->getFileHandle());

            // Check the attempt count to avoid infinite loop
            if (this->attempts > 2)
                throw std::runtime_error("Failed to access the file.");

            // Free cached table before closing
            freeTable(table);

            // Try again
            executeDrop(table);

            // Assign cached table to nullptr
            table = nullptr;
        }
        else
        {
            return ExecuteResult::EXECUTE_ERROR_WHILE_DROPPING;
        }
    }

    return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult Statement::executeInsert(std::shared_ptr<Table>& table)
{
    if (table == nullptr)
    {
        return ExecuteResult::EXECUTE_TABLE_NOT_SELECTED;
    }

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
            return ExecuteResult::EXECUTE_DUPLICATE_KEY;
        }
    }

	leafInsert(cursor, this->rowToInsert.id, &rowToInsert);

	return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult Statement::executeSelect(std::shared_ptr<Table>& table)
{
    if (table == nullptr)
    {
        return ExecuteResult::EXECUTE_TABLE_NOT_SELECTED;
    }

    std::unique_ptr<Cursor> cursor = tableStart(table);

	Row row;
	while (!(cursor->endOfTable))
	{
		deserializeRow(cursorValue(cursor), &row);
		printRow(&row);
        cursorAdvance(cursor);
	}

	return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult Statement::executeStatement(std::shared_ptr<Table>& table)
{
	switch (type)
	{
    case(StatementType::STATEMENT_CREATE):
        return executeCreate(table);
	case (StatementType::STATEMENT_INSERT):
		return executeInsert(table);
	case (StatementType::STATEMENT_SELECT):
		return executeSelect(table);
    case(StatementType::STATEMENT_OPEN):
        return executeOpen(table);
    case(StatementType::STATEMENT_DROP):
        return executeDrop(table);
    default:
        throw std::exception("Unknown statement.");
	}
}

const StatementType Statement::getStatement() const
{
	return this->type;
}

const Row Statement::getRow() const
{
	return this->rowToInsert;
}

const std::string Statement::getTableName() const
{
    return this->tableName;
}

// DEBUG FUNCTIONS

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

void printTree(const std::unique_ptr<Pager>& pager, uint32_t pageNumber, uint32_t indentation_level) {
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
