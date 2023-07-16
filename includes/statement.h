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

MetaCommandResult do_meta_command(std::shared_ptr<InputBuffer>, const std::shared_ptr<Table>&);

void print_constants();

void indent(uint32_t level);

void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level);

// STATEMENTS

typedef enum 
{ 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
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
    EXECUTE_TABLE_FULL 
} ExecuteResult;


class Statement
{
private:
	StatementType type;

	Row row_to_insert;

public:
	Statement();

	PrepareResult prepare_statement(InputBuffer*);

	ExecuteResult execute_insert(std::shared_ptr<Table>& table);

	static ExecuteResult execute_select(std::shared_ptr<Table>& table);

	ExecuteResult execute_statement(std::shared_ptr<Table>& table);

	const StatementType getStatement() const;

	const Row getRow() const;
};