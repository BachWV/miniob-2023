#pragma once

#include "sql/parser/parse_defs.h"
#include "sql/expr/parsed_expr.h"

class SelectStmt;
class Db;

struct ViewColumnAttr
{
    std::string column_name;  // 视图定义的列名。目前默认视图的列和查询的列一对一，不会在创建视图后对视图查询的表进行alter操作
    bool is_field_identifier;  // 是不是简单的对子表字段的引用，聚集函数、内置函数、其它表达式都不是

    // 若is_field_identifier为true，则下面的数据有效
    std::string related_table_name;  // 关联的表名，这个表可以是另一个视图名
    std::string field_name;          // 字段名
};

class View
{
public:
    View(const std::string &name, std::vector<ViewColumnAttr> &&view_columns, std::unique_ptr<SelectSqlNode> select_def,
        bool insertable, bool deletable)
        : view_name_(name), view_columns_(std::move(view_columns)), select_def_(std::move(select_def)) {
            insertable_ = insertable;
            deletable_ = deletable;
        }

    const std::string &name() { return view_name_; }
    const std::vector<ViewColumnAttr> &view_columns() { return view_columns_; }

    RC resolve_select(Db *db, ExprResolveContext *glob_ctx, std::unique_ptr<SelectStmt> &result) const;

private:
    std::string view_name_;
    std::vector<ViewColumnAttr> view_columns_;
    std::unique_ptr<SelectSqlNode> select_def_;

    // 是否可插入、删除，在ViewStmt创建时分析。是否可更新需要运行时分析
    // 如果这里标记了不可插入、删除，则一定不可以。如果标记可以，还需要运行时分析
    // 这里标记的是定义视图的子查询不满足一些条件，导致视图不可更新、插入的情况
    bool insertable_, deletable_;
};

struct ResolvedView
{
    ResolvedView(View *view, std::unique_ptr<SelectStmt> select);

    View *view_;
    std::unique_ptr<SelectStmt> select_;
};