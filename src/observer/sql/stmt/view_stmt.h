#pragma once

#include "sql/stmt/stmt.h"
#include "sql/parser/parse_defs.h"

class Db;
class View;

/* 目前视图信息在内存中维护，不考虑持久化 */
class ViewStmt: public Stmt
{
public:
    ViewStmt(std::unique_ptr<View> view);

    StmtType type() const override
    {
        return StmtType::CREATE_VIEW;
    }

    std::unique_ptr<View> owns_view();

    static RC create(Db *db, CreateViewSqlNode &view_node, Stmt *&stmt);

private:

    std::unique_ptr<View> view_;
};