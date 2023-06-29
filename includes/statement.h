#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include "buffer.h"
#include "data.h"
#include "pager.h"

#include "constants.h"


// META COMMANDS

typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer*, Table*);


// STATEMENTS

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef enum { PREPARE_SUCCESS, PREPARE_NEGATIVE_ID, PREPARE_STRING_TOO_LONG, PREPARE_UNRECOGNIZED_STATEMENT, PREPARE_SYNTAX_ERROR } PrepareResult;

typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;


class Statement
{
private:
	StatementType type;

	Row row_to_insert;

public:
	Statement();

	PrepareResult prepare_statement(InputBuffer*);

	ExecuteResult execute_insert(Table*);

	static ExecuteResult execute_select(Table*);

	ExecuteResult execute_statement(Table*);

	const StatementType getStatement() const;

	const Row getRow() const;
};