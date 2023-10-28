#pragma once

#include "sql/parser/parse_defs.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/stmt.h"
#include <vector>

class CreateTableSelectStmt: public Stmt{
public:
  CreateTableSelectStmt() = default;
  virtual ~CreateTableSelectStmt() = default;
  StmtType type() const override { return StmtType::CREATE_TABLE_SELECT; }
  static RC create(Db *db, CreateTableSelectSqlNode &sql_node, Stmt *&stmt);

  std::string& created_table_name(){
    return created_table_name_;
  }
  std::vector<AttrInfoSqlNode>& created_table_infos(){
    return created_table_infos_;
  }
  SelectStmt*& original_table_stmt(){
    return original_table_stmt_;
  }
private:
  std::string created_table_name_;
  std::vector<AttrInfoSqlNode> created_table_infos_;
  SelectStmt* original_table_stmt_;
};