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
    std::string field_name;          // 关联的表字段原名
};

class View
{
public:
    View(const std::string &name, std::vector<ViewColumnAttr> &&view_columns, std::unique_ptr<SelectSqlNode> select_def,
        bool insertable, bool deletable, bool updatable)
        : view_name_(name), view_columns_(std::move(view_columns)), select_def_(std::move(select_def)) {
            insertable_ = insertable;
            deletable_ = deletable;
            updatable_ = updatable;
        }

    const std::string &name() { return view_name_; }
    const std::vector<ViewColumnAttr> &view_columns() { return view_columns_; }

    RC resolve_select(Db *db, ExprResolveContext *glob_ctx, std::unique_ptr<SelectStmt> &result) const;

    bool insertable() const { return insertable_; }
    bool deletable() const { return deletable_; }
    bool updatable() const { return updatable_; }

private:
    std::string view_name_;
    std::vector<ViewColumnAttr> view_columns_;
    std::unique_ptr<SelectSqlNode> select_def_;

    // 是否可插入、删除、更新，在ViewStmt创建时分析。
    // 如果这里标记了不可插入、删除，则一定不可以。如果标记可以，还需要运行时分析
    bool insertable_, deletable_, updatable_;
};

struct ResolvedView
{
    ResolvedView(View *view, std::unique_ptr<SelectStmt> select);

    View *view_;
    std::unique_ptr<SelectStmt> select_;
};

using TableSrc = std::variant<Table*, ResolvedView>;

/* 
 * 工具类：把引用视图列的表达式重写为引用视图基表的表达式
 * 注意：map的正确性由调用者保证，逻辑上只应该映射一个表，和该表的一些列，不能映射多个表 
 */
class ParsedExprMapper
{
public:
    void add_table_map(const std::string &origin_table_name, const std::string &target_table_name) {
        table_name_map_[origin_table_name] = target_table_name;
    }
    void add_field_map(const std::string &origin_field_name, const std::string &target_field_name) {
        field_name_map_[origin_field_name] = target_field_name;
    }

    void map_expr(ExprSqlNode *expr);

private:
    std::unordered_map<std::string, std::string> table_name_map_;
    std::unordered_map<std::string, std::string> field_name_map_;
};