#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <exception>

#include "constants.h"
#include "buffer.h"
#include "data.h"
#include "pager.h"
#include "node.h"


// META COMMANDS

enum class MetaCommandResult {
	META_COMMAND_SUCCESS,
    META_COMMAND_SYNTAX_ERROR,
	META_COMMAND_UNRECOGNIZED_COMMAND
};

MetaCommandResult doMetaCommand(std::shared_ptr<InputBuffer>, 
                                const std::shared_ptr<Table>&);

void printConstants();

void indent(uint32_t level);

void printTree(const std::unique_ptr<Pager>& pager,
               uint32_t pageNumber, uint32_t indentation_level);

// STATEMENTS

enum class StatementType {
    STATEMENT_CREATE,
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_UPDATE,
    STATEMENT_DROP,
    STATEMENT_OPEN
};

enum class PrepareResult { 
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID, 
    PREPARE_STRING_TOO_LONG, 
    PREPARE_UNRECOGNIZED_STATEMENT, 
    PREPARE_SYNTAX_ERROR 
};

enum class ExecuteResult { 
    EXECUTE_SUCCESS, 
    EXECUTE_DUPLICATE_KEY,
    EXECUTE_KEY_DOES_NOT_EXIST,
    EXECUTE_TABLE_FULL,
    EXECUTE_TABLE_NOT_SELECTED,
    EXECUTE_ERROR_WHILE_CREATING,
    EXECUTE_ERROR_WHILE_OPENING,
    EXECUTE_ERROR_FILE_EXISTS,
    EXECUTE_ERROR_WHILE_DROPPING,
    EXECUTE_ERROR_FILE_NOT_FOUND
};


class Statement
{
private:
	StatementType type;

	Row rowToInsert;

    std::string tableName;

    // Track statement execution attemts for stopping recursive executions,
    // in case a function keeps failing
    uint8_t attempts = 0;

public:
	Statement();

	PrepareResult prepareStatement(InputBuffer*);

    ExecuteResult executeCreate(std::shared_ptr<Table>& table);

    ExecuteResult executeOpen(std::shared_ptr<Table>& table);

	ExecuteResult executeInsert(std::shared_ptr<Table>& table);

	ExecuteResult executeUpdate(std::shared_ptr<Table>& table);

    ExecuteResult executeDrop(std::shared_ptr<Table>& table);

	static ExecuteResult executeSelect(std::shared_ptr<Table>& table);

	ExecuteResult executeStatement(std::shared_ptr<Table>& table);

	const StatementType getStatement() const;

    const std::string getTableName() const;

	const Row getRow() const;
};