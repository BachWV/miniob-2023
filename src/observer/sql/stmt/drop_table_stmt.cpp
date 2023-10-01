#include "sql/stmt/drop_table_stmt.h"
#include "common/log/log.h"

RC DropTableStmt::create(const DropTableSqlNode &drop_table, Stmt *&stmt)
{
    stmt = new DropTableStmt(drop_table.relation_name);
    LOG_TRACE("drop table stmt: table name %s", drop_table.relation_name.c_str());
    return RC::SUCCESS;
}