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

typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult doMetaCommand(std::shared_ptr<InputBuffer>, const std::shared_ptr<Table>&);

void printConstants();

void indent(uint32_t level);

void printTree(Pager* pager, uint32_t pageNumber, uint32_t indentation_level);

// STATEMENTS

typedef enum 
{
    STATEMENT_CREATE,
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_DROP,
    STATEMENT_OPEN
} StatementType;

typedef enum { 
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID, 
    PREPARE_STRING_TOO_LONG, 
    PREPARE_UNRECOGNIZED_STATEMENT, 
    PREPARE_SYNTAX_ERROR 
} PrepareResult;

typedef enum { 
    EXECUTE_SUCCESS, 
    EXECUTE_DUPLICATE_KEY, 
    EXECUTE_TABLE_FULL,
    EXECUTE_TABLE_NOT_SELECTED,
    EXECUTE_ERROR_WHILE_CREATING,
    EXECUTE_ERROR_WHILE_OPENING,
    EXECUTE_ERROR_FILE_EXISTS,
    EXECUTE_ERROR_WHILE_DROPPING,
    EXECUTE_ERROR_SHARING_VIOLATION,
    EXECUTE_ERROR_FILE_NOT_FOUND
} ExecuteResult;


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

    ExecuteResult executeDrop(std::shared_ptr<Table>& table);

	static ExecuteResult executeSelect(std::shared_ptr<Table>& table);

	ExecuteResult executeStatement(std::shared_ptr<Table>& table);

	const StatementType getStatement() const;

    const std::string getTableName() const;

	const Row getRow() const;
};