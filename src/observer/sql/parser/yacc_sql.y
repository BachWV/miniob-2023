
%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <time.h>

#include "common/log/log.h"
#include "common/lang/string.h"
#include "sql/parser/parse_defs.h"
#include "sql/parser/yacc_sql.hpp"
#include "sql/parser/lex_sql.h"
#include "sql/expr/expression.h"
#include "sql/parser/parser_helper_func.h"
#include "sql/expr/parsed_expr.h"

using namespace std;

string token_name(const char *sql_string, YYLTYPE *llocp)
{
  return string(sql_string + llocp->first_column, llocp->last_column - llocp->first_column + 1);
}

int yyerror(YYLTYPE *llocp, const char *sql_string, ParsedSqlResult *sql_result, yyscan_t scanner, const char *msg)
{
  std::unique_ptr<ParsedSqlNode> error_sql_node = std::make_unique<ParsedSqlNode>(SCF_ERROR);
  error_sql_node->error.error_msg = msg;
  error_sql_node->error.line = llocp->first_line;
  error_sql_node->error.column = llocp->first_column;
  sql_result->add_sql_node(std::move(error_sql_node));
  return 0;
}

ArithmeticExpr *create_arithmetic_expression(ArithmeticExpr::Type type,
                                             Expression *left,
                                             Expression *right,
                                             const char *sql_string,
                                             YYLTYPE *llocp)
{
  ArithmeticExpr *expr = new ArithmeticExpr(type, left, right);
  expr->set_name(token_name(sql_string, llocp));
  return expr;
}

%}

%define api.pure full 
%define parse.error verbose
/** 启用位置标识 **/
%locations
%lex-param { yyscan_t scanner }
/** 这些定义了在yyparse函数中的参数 **/
%parse-param { const char * sql_string }
%parse-param { ParsedSqlResult * sql_result }
%parse-param { void * scanner }

//标识tokens, 终结符
%token  SEMICOLON
        CREATE
        DROP
        TABLE
        TABLES
        INDEX
        UNIQUE
        CALC
        SELECT
        DESC
        SHOW
        SYNC
        INSERT
        DELETE
        UPDATE
        LBRACE
        RBRACE
        COMMA
        TRX_BEGIN
        TRX_COMMIT
        TRX_ROLLBACK
        INT_T
        STRING_T
        FLOAT_T
        HELP
        EXIT
        DOT //QUOTE
        INTO
        VALUES
        FROM
        WHERE
        AND
        OR
        SET
        SYM_INNER_JOIN
        ON
        LOAD
        DATA
        INFILE
        EXPLAIN
        EQ
        LT
        GT
        LE
        GE
        NE
        DATE_T
        TEXT_T
        ORDER_BY
        ASC
        SYM_NOT_NULL
        SYM_NULL
        SYM_IS_NULL
        SYM_IS_NOT_NULL
        SYM_IN
        SYM_NOT_IN
        SYM_EXISTS
        SYM_NOT_EXISTS
        GROUP_BY
        HAVING
        SYM_LIKE
        SYM_NOT_LIKE
        AS

/** union 中定义各种数据类型，真实生成的代码也是union类型，所以不能有非POD类型的数据 **/
%union {
  ParsedSqlNode *                   sql_node;
  Value *                           value;
  enum CompOp                       comp;
  RelAttrSqlNode *                  rel_attr;
  std::vector<AttrInfoSqlNode> *    attr_infos;
  AttrInfoSqlNode *                 attr_info;
  Expression *                      expression;
  std::vector<Expression *> *       expression_list;
  std::vector<Value> *              value_row;
  std::vector<std::vector<Value>> *    value_rows;
  std::vector<Value> *              value_list;
  std::vector<RelAttrSqlNode> *     rel_attr_list;
  std::vector<relation_with_alias> *        relation_list;
  relation_with_alias*              rel_with_alias;
  char *                            string;
  int                               number;
  float                             floats;
  OrderByAttrSqlNode*               order_by_attr;
  std::vector<OrderByAttrSqlNode>*  order_by_list;
  std::vector<SetValueSqlNode> *    set_value_list;
  SetValueSqlNode *                 set_value;

  // 重构后，表达式的语法解析树节点
  ExprSqlNode *                     expr_node;
  Conditions*                       expr_node_list;
  ExprSqlSet *                      expr_sql_set;
  AggregateFuncSqlNode*             agg_func;
  std::vector<SelectExprWithAlias>*   select_expr_list;
  SelectExprWithAlias*              select_expr;

  // Function
  FunctionSqlNode*                  function_node;
  std::string*                       std_string;
  std::vector<std::unique_ptr<InnerJoinSqlNode>>*    inner_join_list;
  InnerJoinSqlNode*                 inner_join;
}

/* %token <number> DATE */
%token <string> DATE_STR
%token <number> NUMBER
%token <floats> FLOAT
%token <string> ID
%token <string> SSS
%token <string> MIN MAX AVG COUNT SUM ROUND LENGTH DATE_FORMAT

//非终结符

/** type 定义了各种解析后的结果输出的是什么类型。类型对应了 union 中的定义的成员变量名称 **/
%type <number>              type
%type <value>               value non_negative_value negative_value
%type <number>              number
%type <comp>                comp_op
%type <rel_attr>            rel_attr
%type <attr_infos>          attr_def_list
%type <attr_info>           basic_attr_def
%type <attr_info>           attr_def
%type <value_list>          value_list
%type <expr_node_list>      where
%type <relation_list>       rel_list
%type <rel_with_alias>      rel_with_alias
%type <expression>          expression
%type <expression_list>     expression_list
%type <sql_node>            calc_stmt
%type <sql_node>            select_stmt
%type <sql_node>            insert_stmt
%type <value_rows>          value_rows
%type <value_row>           value_row
%type <sql_node>            update_stmt
%type <sql_node>            delete_stmt
%type <sql_node>            create_table_stmt
%type <sql_node>            drop_table_stmt
%type <sql_node>            show_tables_stmt
%type <sql_node>            desc_table_stmt
%type <sql_node>            create_index_stmt
%type <sql_node>            drop_index_stmt
%type <sql_node>            sync_stmt
%type <sql_node>            begin_stmt
%type <sql_node>            commit_stmt
%type <sql_node>            rollback_stmt
%type <sql_node>            load_data_stmt
%type <sql_node>            explain_stmt
%type <sql_node>            set_variable_stmt
%type <sql_node>            help_stmt
%type <sql_node>            exit_stmt
%type <sql_node>            create_table_select_stmt
%type <sql_node>            command_wrapper
// commands should be a list but I use a single command instead
%type <sql_node>            commands
%type <select_expr_list>    select_expr_list
%type <select_expr>         select_expr
// order by
%type <order_by_list>       order_by
%type <order_by_list>       order_by_list
%type <order_by_attr>       order_by_attr
%type <set_value_list>      set_value_list
%type <set_value>           set_value
// aggregate func
%type <agg_func>            agg_func
// group by
%type <rel_attr_list>       group_by
%type <rel_attr_list>       group_by_list 
// function
%type <function_node>       function_node
%type <std_string>          alias
%type <string>              all_id

// expr
%type <expr_node>           expr identifier
%type <expr_node_list>      expr_list having
%type <sql_node>            sub_query
%type <expr_sql_set>        expr_set
%type <inner_join>          inner_join
%type <inner_join_list>     inner_join_list

%nonassoc SYM_IS_NULL SYM_IS_NOT_NULL SYM_IN SYM_NOT_IN
%left EQ LT GT LE GE NE
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS SYM_LIKE SYM_NOT_LIKE
%%

commands: command_wrapper opt_semicolon  //commands or sqls. parser starts here.
  {
    std::unique_ptr<ParsedSqlNode> sql_node = std::unique_ptr<ParsedSqlNode>($1);
    sql_result->add_sql_node(std::move(sql_node));
  }
  ;

command_wrapper:
    calc_stmt
  | select_stmt
  | insert_stmt
  | update_stmt
  | delete_stmt
  | create_table_stmt
  | drop_table_stmt
  | show_tables_stmt
  | desc_table_stmt
  | create_index_stmt
  | drop_index_stmt
  | sync_stmt
  | begin_stmt
  | commit_stmt
  | rollback_stmt
  | load_data_stmt
  | explain_stmt
  | set_variable_stmt
  | help_stmt
  | exit_stmt
  | create_table_select_stmt
    ;

exit_stmt:      
    EXIT {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      $$ = new ParsedSqlNode(SCF_EXIT);
    };

help_stmt:
    HELP {
      $$ = new ParsedSqlNode(SCF_HELP);
    };

sync_stmt:
    SYNC {
      $$ = new ParsedSqlNode(SCF_SYNC);
    }
    ;

begin_stmt:
    TRX_BEGIN  {
      $$ = new ParsedSqlNode(SCF_BEGIN);
    }
    ;

commit_stmt:
    TRX_COMMIT {
      $$ = new ParsedSqlNode(SCF_COMMIT);
    }
    ;

rollback_stmt:
    TRX_ROLLBACK  {
      $$ = new ParsedSqlNode(SCF_ROLLBACK);
    }
    ;

drop_table_stmt:    /*drop table 语句的语法解析树*/
    DROP TABLE ID {
      $$ = new ParsedSqlNode(SCF_DROP_TABLE);
      $$->drop_table.relation_name = $3;
      free($3);
    };

show_tables_stmt:
    SHOW TABLES {
      $$ = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
    ;

desc_table_stmt:
    DESC ID  {
      $$ = new ParsedSqlNode(SCF_DESC_TABLE);
      $$->desc_table.relation_name = $2;
      free($2);
    }
    ;

create_index_stmt:    /*create index 语句的语法解析树*/
    CREATE INDEX ID ON ID LBRACE rel_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $3;
      create_index.relation_name = $5;
      if ($7 != nullptr) {
        std::vector<std::string> old_list;
        for(auto& [ori_name, alias]: *$7){
          create_index.attribute_names.push_back(ori_name);
        }
        delete $7;
      }
      create_index.is_unique = false;
      free($3);
      free($5);
    }
    | CREATE UNIQUE INDEX ID ON ID LBRACE rel_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = $$->create_index;
      create_index.index_name = $4;
      create_index.relation_name = $6;
      create_index.is_unique = true;
      if ($8 != nullptr) {
        std::vector<std::string> old_list;
        for(auto& [ori_name, alias]: *$8){
          create_index.attribute_names.push_back(ori_name);
        }
        delete $8;
      }
      free($4);
      free($6);
    }
    ;

drop_index_stmt:      /*drop index 语句的语法解析树*/
    DROP INDEX ID ON ID
    {
      $$ = new ParsedSqlNode(SCF_DROP_INDEX);
      $$->drop_index.index_name = $3;
      $$->drop_index.relation_name = $5;
      free($3);
      free($5);
    }
    ;
create_table_stmt:    /*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE
    {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = $$->create_table;
      create_table.relation_name = $3;
      free($3);

      std::vector<AttrInfoSqlNode> *src_attrs = $6;

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
      }
      create_table.attr_infos.emplace_back(*$5);
      std::reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete $5;
    }
    ;
attr_def_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA attr_def attr_def_list
    {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<AttrInfoSqlNode>;
      }
      $$->emplace_back(*$2);
      delete $2;
    }
    ;

attr_def:
    basic_attr_def 
    {
      $1->nullable = true;
      $$ = $1;
    }
    | basic_attr_def SYM_NOT_NULL 
    {
      $1->nullable = false;
      $$ = $1;
    }
    | basic_attr_def SYM_NULL 
    {
      $1->nullable = true;
      $$ = $1;
    }
    ;

basic_attr_def:
    ID type LBRACE number RBRACE 
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->length = $4;
      free($1);
    }
    | ID type
    {
      $$ = new AttrInfoSqlNode;
      $$->type = (AttrType)$2;
      $$->name = $1;
      $$->length = 4;
      free($1);
    } 
    | ID TEXT_T
    {
      $$ = new AttrInfoSqlNode;
      $$->type = TEXTS;
      $$->name = $1;
      $$->length = 32;
      free($1);
    }
    ;

number:
    NUMBER {$$ = $1;}
    ;
// 这里的$$=type中的type是在value中定义的
type:
    INT_T      { $$=INTS; }
    | STRING_T { $$=CHARS; }
    | FLOAT_T  { $$=FLOATS; }
    | DATE_T   { $$=DATES; }
    ;
insert_stmt:        /*insert   语句的语法解析树*/
    INSERT INTO ID VALUES value_rows
    {
      $$ = new ParsedSqlNode(SCF_INSERT);
      $$->insertion.relation_name = $3;
      $$->insertion.value_rows.swap(*$5);
      free($3);
    }
    ;

value_rows:
    value_row
    {
      $$ = new std::vector<std::vector<Value>>();
      $$->emplace_back(*$1);
    }
    | value_rows COMMA value_row
    {
      $$ = $1;
      $$->emplace_back(*$3);
    }
    ;

value_row:
    LBRACE value value_list RBRACE
    {
      $$ = new std::vector<Value>();
      if($3 != nullptr){
        $$->swap(*$3);
        delete $3;
      }
      $$->emplace_back(*$2);
      std::reverse($$->begin(), $$->end());
    }
    ;

value_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | COMMA value value_list  { 
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<Value>;
      }
      $$->emplace_back(*$2);
      delete $2;
    }
    ;

// 修复1-1中，-1被识别为1个token的BUG，将value分为negative和non-negative。expr中只允许non-negative，同时不影响其它部分
// 如果value这么改后，expr中还引用value，则expr -> - expr; 和 value -> - number会产生rr冲突
// expr中有-expr这条产生式，所以只需要使用non_negative_value，即可应对全部情况
value:
    non_negative_value {
      $$ = $1;
    }
    | negative_value {
      $$ = $1;
    }
    ;

non_negative_value:
    NUMBER {
      $$ = new Value((int)$1);
      @$ = @1;
    }
    |FLOAT {
      $$ = new Value((float)$1);
      @$ = @1;
    }
    |SSS {
      // 这里为什么要-2
      // A: 注意这里有双引号
      char *tmp = common::substr($1,1,strlen($1)-2);
      if(strlen(tmp) > 65535){
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("string too long"));
      }
      $$ = new Value(tmp);
      free(tmp);
    }
    |DATE_STR{
      int time;
      if(!CheckTimeRange($1, time)){
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("date invalid"));
      }
      $$ = new Value(time, AttrType::DATES);
    }
    |SYM_NULL {
      $$ = new Value();
      $$->set_type(AttrType::NULL_TYPE);
    }
    ;

negative_value:
    '-' NUMBER {
      $$ = new Value((int)-$2);
    }
    |'-' FLOAT {
      $$ = new Value((float)-$2);
    }
    ;
    
delete_stmt:    /*  delete 语句的语法解析树*/
    DELETE FROM ID where 
    {
      $$ = new ParsedSqlNode(SCF_DELETE);
      $$->deletion.relation_name = $3;
      if ($4 != nullptr) {
        $$->deletion.conditions.type = $4->type;
        $$->deletion.conditions.exprs.swap($4->exprs);
        delete $4;
      }
      free($3);
    }
    ;
update_stmt:      /*  update 语句的语法解析树*/
    UPDATE ID SET set_value_list where
    {
      $$ = new ParsedSqlNode(SCF_UPDATE);
      $$->update.relation_name = $2;
      if ($4 != nullptr) {
        $$->update.set_value_list.swap(*$4);
        delete $4;
      }
      if ($5 != nullptr){
        $$->update.conditions.type = $5->type;
        $$->update.conditions.exprs.swap($5->exprs);
        delete $5;
      }
      free($2);
      
    }
    ;
set_value_list:
  set_value {
    $$ = new std::vector<SetValueSqlNode>;
    $$->emplace_back(std::move(*$1));
    delete $1;
  }
  | set_value COMMA set_value_list
  {
    $$ = $3;
    $$->emplace_back(std::move(*$1));
    delete $1;
  }
  ;

set_value:
    ID EQ expr
    {
      $$ = new SetValueSqlNode;
      $$->attr_name = $1;
      $$->rhs_expr.reset($3);
      free($1);
    }
    ;

/* 添加了inner_join_list，合并代码时注意，右边非终结符的标号不要搞错了！ */
select_stmt:        /*  select 语句的语法解析树*/
    SELECT select_expr_list FROM rel_list inner_join_list where group_by having order_by 
    {
      $$ = new ParsedSqlNode(SCF_SELECT);
      if ($2 != nullptr) {
        $$->selection.select_exprs.swap(*$2);
        delete $2;
      }
      if ($4 != nullptr) {
        $$->selection.relations.swap(*$4);
        delete $4;
      }

      if ($5 != nullptr) {
        for (auto &inner_join: *$5)
        {
          std::string join_rel_name = inner_join->join_relation;
          $$->selection.relations.push_back(std::make_pair(join_rel_name, join_rel_name));

          $$->selection.conditions.exprs.insert($$->selection.conditions.exprs.end(), 
            std::make_move_iterator(inner_join->join_conditions.exprs.begin()), 
            std::make_move_iterator(inner_join->join_conditions.exprs.end()));
        }
        delete $5;
      }

      // 当前AND和OR混合使用，则结果未定义
      if ($6 != nullptr) {
        $$->selection.conditions.type = $6->type;
        $$->selection.conditions.exprs.swap($6->exprs);
        $$->selection.conditions.exprs.insert($$->selection.conditions.exprs.end(),
          std::make_move_iterator($6->exprs.begin()), std::make_move_iterator($6->exprs.end()));
        delete $6;
      }

      if ($7 != nullptr) {
        $$->selection.group_by_attrs.swap(*$7);
        std::reverse($$->selection.group_by_attrs.begin(), $$->selection.group_by_attrs.end());
        delete $7;
      }

      if($8 != nullptr){
        $$->selection.having_attrs.type = $8->type;
        $$->selection.having_attrs.exprs.swap($8->exprs);
        delete $8;
      }

      if ($9 != nullptr) {
        $$->selection.order_by_attrs.swap(*$9);
        std::reverse($$->selection.order_by_attrs.begin(), $$->selection.order_by_attrs.end());
        delete $9;
      }
    }
    | SELECT select_expr_list{
      $$ = new ParsedSqlNode(SCF_SELECT);
      $$->selection.select_exprs.swap(*$2);
      delete $2;
    }
    ;

group_by:
      /* empty */
    {
      $$ = nullptr;
    }
    | GROUP_BY group_by_list {
      $$ = $2;  
    }
    ;

// rel_attr_node类型可以规约为attr_list，又可以规约成group_by，会不会有二义性
// 
group_by_list:
    rel_attr 
    {
      $$ = new std::vector<RelAttrSqlNode>;
      $$->emplace_back(*$1);
      delete $1;
    }
    | rel_attr COMMA group_by_list
    {
      $$ = $3;
      $$->emplace_back(*$1);
      delete $1;
    }

calc_stmt:
    CALC expression_list
    {
      $$ = new ParsedSqlNode(SCF_CALC);
      std::reverse($2->begin(), $2->end());
      $$->calc.expressions.swap(*$2);
      delete $2;
    }
    ;

expression_list:
    expression
    {
      $$ = new std::vector<Expression*>;
      $$->emplace_back($1);
    }
    | expression COMMA expression_list
    {
      if ($3 != nullptr) {
        $$ = $3;
      } else {
        $$ = new std::vector<Expression *>;
      }
      $$->emplace_back($1);
    }
    ;
expression:
    expression '+' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::ADD, $1, $3, sql_string, &@$);
    }
    | expression '-' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::SUB, $1, $3, sql_string, &@$);
    }
    | expression '*' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::MUL, $1, $3, sql_string, &@$);
    }
    | expression '/' expression {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::DIV, $1, $3, sql_string, &@$);
    }
    | LBRACE expression RBRACE {
      $$ = $2;
      $$->set_name(token_name(sql_string, &@$));
    }
    | '-' expression %prec UMINUS {
      $$ = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, $2, nullptr, sql_string, &@$);
    }
    | non_negative_value {
      $$ = new ValueExpr(*$1);
      $$->set_name(token_name(sql_string, &@$));
      delete $1;
    }
    ;

/* 属性匹配了table.field和单独的feild */
rel_attr:
    ID {
      $$ = new RelAttrSqlNode;
      $$->attribute_name = $1;
      free($1);
    }
    | ID DOT ID {
      $$ = new RelAttrSqlNode;
      $$->relation_name  = $1;
      $$->attribute_name = $3;
      free($1);
      free($3);
    }
    ;

rel_list:
    rel_with_alias{
      $$ = new std::vector<relation_with_alias>;
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    | rel_list COMMA rel_with_alias {
      $$ = $1;
      $$->emplace_back(std::move(*$3));
      free($3);
    }
    ;

rel_with_alias:
    ID alias {
      $$ = new relation_with_alias;
      $$->first = std::string($1);
      free($1);
      if ($2 != nullptr)
      {
        $$->second = *$2;
        delete $2;
      }
    }

inner_join_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | inner_join_list inner_join
    {
      if ($1 == nullptr)
       $1 = new std::vector<std::unique_ptr<InnerJoinSqlNode>>;
      $1->emplace_back($2);
      $$ = $1;
    }
    ;

inner_join:
  SYM_INNER_JOIN ID ON expr_list
  {
    $$ = new InnerJoinSqlNode;
    $$->join_relation = $2;
    $$->join_conditions = std::move(*$4);
    free($2);
    delete $4;
  }
  | SYM_INNER_JOIN ID
  {
    $$ = new InnerJoinSqlNode;
    $$->join_relation = $2;
    free($2);
  }

where:
    /* empty */
    {
      $$ = nullptr;
    }
    | WHERE expr_list {
      $$ = $2;
    }
    ;

expr_list:
    expr {
      $$ = new Conditions;
      $$->exprs.emplace_back($1);
    }
    | expr_list AND expr {
      $$ = $1;
      $$->exprs.emplace_back($3);
      $$->type = Conditions::ConjunctionType::AND;
    }
    | expr_list OR expr {
      $$ = $1;
      $$->exprs.emplace_back($3);
      $$->type = Conditions::ConjunctionType::OR;
    }
    ;

expr:
    non_negative_value
    {
      $$ = new ValueExprSqlNode(token_name(sql_string, &@$), *$1);
    }
    | identifier
    {
      $$ = $1;
    }
    | agg_func 
    {
      $$ = new AggIdentifierExprSqlNode(*$1);
      delete $1;
    }
    | function_node
    {
      $$ = new FunctionExprSqlNode(token_name(sql_string, &@$), std::move(*$1));
      delete $1;
    }
    | '+' expr %prec UMINUS
    {
      $$ = $2;
    }
    | '-' expr %prec UMINUS
    {
      std::unique_ptr<ExprSqlNode> sub_expr($2);
      $$ = new NegativeArithExprSqlNode(token_name(sql_string, &@$), std::move(sub_expr));
    }
    | expr '+' expr
    {
      std::unique_ptr<ExprSqlNode> left($1);
      std::unique_ptr<ExprSqlNode> right($3);
      $$ = new BinaryArithExprSqlNode(token_name(sql_string, &@$), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Add);
    }
    | expr '-' expr
    {
      std::unique_ptr<ExprSqlNode> left($1);
      std::unique_ptr<ExprSqlNode> right($3);
      $$ = new BinaryArithExprSqlNode(token_name(sql_string, &@$), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Sub);
    }
    | expr '*' expr
    {
      std::unique_ptr<ExprSqlNode> left($1);
      std::unique_ptr<ExprSqlNode> right($3);
      $$ = new BinaryArithExprSqlNode(token_name(sql_string, &@$), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Mul);
    }
    | expr '/' expr
    {
      std::unique_ptr<ExprSqlNode> left($1);
      std::unique_ptr<ExprSqlNode> right($3);
      $$ = new BinaryArithExprSqlNode(token_name(sql_string, &@$), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Div);
    }
    | expr comp_op expr %prec NE
    {
      std::unique_ptr<ExprSqlNode> left($1);
      std::unique_ptr<ExprSqlNode> right($3);
      $$ = new CompareExprSqlNode(token_name(sql_string, &@$), std::move(left), std::move(right), $2);
    }
    | expr SYM_LIKE SSS
    {
      std::unique_ptr<ExprSqlNode> left($1);
      char *tmp = common::substr($3, 1, strlen($3) - 2);
      $$ = new LikeExprSqlNode(token_name(sql_string, &@$), std::move(left), tmp, false);
      free(tmp);
    }
    | expr SYM_NOT_LIKE SSS
    {
      std::unique_ptr<ExprSqlNode> left($1);
      char *tmp = common::substr($3, 1, strlen($3) - 2);
      $$ = new LikeExprSqlNode(token_name(sql_string, &@$), std::move(left), tmp, true);
      free(tmp);
    }
    | expr SYM_IN LBRACE sub_query RBRACE
    {
      std::unique_ptr<ExprSqlNode> child($1);
      $$ = new QuantifiedCompSubqueryExprSqlNode(token_name(sql_string, &@$), std::move(child), $4, IN);
    }
    | expr SYM_IN LBRACE expr_set RBRACE
    {
      std::unique_ptr<ExprSqlNode> child($1);
      $$ = new QuantifiedCmpExprSetExprSqlNode(token_name(sql_string, &@$), std::move(child), IN, $4);
    }
    | expr SYM_NOT_IN LBRACE sub_query RBRACE
    {
      std::unique_ptr<ExprSqlNode> child($1);
      $$ = new QuantifiedCompSubqueryExprSqlNode(token_name(sql_string, &@$), std::move(child), $4, NOT_IN);
    }
    | expr SYM_NOT_IN LBRACE expr_set RBRACE
    {
      std::unique_ptr<ExprSqlNode> child($1);
      $$ = new QuantifiedCmpExprSetExprSqlNode(token_name(sql_string, &@$), std::move(child), NOT_IN, $4);
    }
    | expr SYM_IS_NULL
    {
      std::unique_ptr<ExprSqlNode> child($1);
      $$ = new PredNullExprSqlNode(token_name(sql_string, &@$), std::move(child), IS_NULL);
    }
    | expr SYM_IS_NOT_NULL
    {
      std::unique_ptr<ExprSqlNode> child($1);
      $$ = new PredNullExprSqlNode(token_name(sql_string, &@$), std::move(child), IS_NOT_NULL);
    }
    | LBRACE expr RBRACE
    {
      $$ = $2;
    }
    | LBRACE sub_query RBRACE
    {
      $$ = new ScalarSubqueryExprSqlNode(token_name(sql_string, &@$), $2);
    }
    | SYM_EXISTS LBRACE sub_query RBRACE
    {
      $$ = new ExistentialSubqueryExprSqlNode(token_name(sql_string, &@$), $3, EXISTS);
    }
    | SYM_NOT_EXISTS LBRACE sub_query RBRACE
    {
      $$ = new ExistentialSubqueryExprSqlNode(token_name(sql_string, &@$), $3, NOT_EXISTS);
    }
    ;

identifier:
    ID
    {
      $$ = new IdentifierExprSqlNode(token_name(sql_string, &@$), std::string(), $1);
      free($1);
    }
    | ID DOT ID
    {
      $$ = new IdentifierExprSqlNode(token_name(sql_string, &@$), $1, $3);
      free($1);
      free($3);
    }
    | '*'
    {
      $$ = new IdentifierExprSqlNode(token_name(sql_string, &@$), std::string(), "*");
    }
    | ID DOT '*'
    {
      $$ = new IdentifierExprSqlNode(token_name(sql_string, &@$), $1, "*");
      free($1);
    }
    ;

sub_query:
    select_stmt
    {
      $$ = $1;
    }
    ;

expr_set:
  /* empty */
  {
    $$ = nullptr;
  }
  | expr
  {
    $$ = new ExprSqlSet;
    $$->emplace_back($1);
  }
  | expr_set COMMA expr
  {
    $$ = $1;
    $$->emplace_back($3);
  }

// order by
// 之所以这么写的原因是 "field1, field2, firld3" 中逗号要匹配
order_by:
    /* empty */
    {
      $$ = nullptr;
    }
    | ORDER_BY order_by_list {
      $$ = $2;  
    }
    ;

order_by_list:
    /* empty */
    {
      $$ = nullptr;
    }
    | order_by_attr 
    {
      $$ = new std::vector<OrderByAttrSqlNode>;
      $$->emplace_back(*$1);
      delete $1;
    }
    | order_by_attr COMMA order_by_list
    {
      $$ = $3;
      $$->emplace_back(*$1);
      delete $1;
    }

order_by_attr:
    rel_attr{
      $$ = new OrderByAttrSqlNode;
      $$->attr = *$1;
      $$->is_asc = true;
      delete $1;
    }
    | rel_attr ASC
    {
      $$ = new OrderByAttrSqlNode;
      $$->attr = *$1;
      $$->is_asc = true;
      delete $1;
    }
    | rel_attr DESC
    {
      $$ = new OrderByAttrSqlNode;
      $$->attr = *$1;
      $$->is_asc = false;
      delete $1;
    }

comp_op:
      EQ { $$ = EQUAL_TO; }
    | LT { $$ = LESS_THAN; }
    | GT { $$ = GREAT_THAN; }
    | LE { $$ = LESS_EQUAL; }
    | GE { $$ = GREAT_EQUAL; }
    | NE { $$ = NOT_EQUAL; }
    ;

load_data_stmt:
    LOAD DATA INFILE SSS INTO TABLE ID 
    {
      char *tmp_file_name = common::substr($4, 1, strlen($4) - 2);
      
      $$ = new ParsedSqlNode(SCF_LOAD_DATA);
      $$->load_data.relation_name = $7;
      $$->load_data.file_name = tmp_file_name;
      free($7);
      free(tmp_file_name);
    }
    ;

explain_stmt:
    EXPLAIN command_wrapper
    {
      $$ = new ParsedSqlNode(SCF_EXPLAIN);
      $$->explain.sql_node = std::unique_ptr<ParsedSqlNode>($2);
    }
    ;

set_variable_stmt:
    SET ID EQ value
    {
      $$ = new ParsedSqlNode(SCF_SET_VARIABLE);
      $$->set_variable.name  = $2;
      $$->set_variable.value = *$4;
      free($2);
      delete $4;
    }
    ;

agg_func:
    /* empty */
    /* {
      $$ = nullptr;
    } */
    SUM LBRACE rel_attr RBRACE{
      $$ = new AggregateFuncSqlNode();
      $$->agg_op = AggregateOp::AGG_SUM;
      $$->attr = *$3;
      $$->name = token_name(sql_string, &@$);
      free($1);
      delete $3;
    }
    | MAX LBRACE rel_attr RBRACE{
      $$ = new AggregateFuncSqlNode();
      $$->agg_op = AggregateOp::AGG_MAX;
      $$->attr = *$3;
      $$->name = token_name(sql_string, &@$);
      free($1);
      delete $3;
    }
    | MIN LBRACE rel_attr RBRACE{
      $$ = new AggregateFuncSqlNode();
      $$->agg_op = AggregateOp::AGG_MIN;
      $$->attr = *$3;
      $$->name = token_name(sql_string, &@$);
      free($1);
      delete $3;
    }
    | AVG LBRACE rel_attr RBRACE{
      $$ = new AggregateFuncSqlNode();
      $$->agg_op = AggregateOp::AGG_AVG;
      $$->attr = *$3;
      $$->name = token_name(sql_string, &@$);
      free($1);
      delete $3;
    }
    | COUNT LBRACE rel_attr RBRACE{
      $$ = new AggregateFuncSqlNode();
      $$->agg_op = AggregateOp::AGG_COUNT;
      $$->attr = *$3;
      $$->name = token_name(sql_string, &@$);
      free($1);
      delete $3;
    }
    | COUNT LBRACE '*' RBRACE{
      $$ = new AggregateFuncSqlNode();
      $$->agg_op = AggregateOp::AGG_COUNT;

      $$->attr = RelAttrSqlNode();
      $$->attr.relation_name  = "";
      $$->attr.attribute_name = "*";
      $$->name = token_name(sql_string, &@$);
      free($1);
    }
    ;

// 写完之后放到上面去
select_expr:
    expr alias {
      $$ = new SelectExprWithAlias();
      $$->expr_.reset($1);
      if ($2 != nullptr)
      {
        $$->alias_ = std::move(*$2);
        delete $2;
      }
    }
    ;

select_expr_list:
    select_expr{
      $$ = new std::vector<SelectExprWithAlias>;
      $$->emplace_back(std::move(*$1));
      delete $1;
    }
    | select_expr_list COMMA select_expr{
      $$ = $1;
      $$->emplace_back(std::move(*$3));
      delete $3;
    }
    ;

having:
    /* empty */
    {
      $$ = nullptr;
    }
    | HAVING expr_list {
      $$ = $2;  
    }
    ;

function_node:
    ROUND LBRACE FLOAT RBRACE{
      // round(3.1415)
      $$ = new FunctionSqlNode();
      $$->function_kernel = make_unique<RoundFunctionKernel>(true, true, 0, $3);
      $$->is_const = true;
      $$->virtual_field_name = token_name(sql_string, &@$);
      free($1);
    }
    | ROUND LBRACE FLOAT COMMA NUMBER RBRACE{
      // round(3.1415, 2)
      $$ = new FunctionSqlNode();
      $$->function_kernel = make_unique<RoundFunctionKernel>(true, false, $5, $3);
      $$->is_const = true;
      $$->virtual_field_name = token_name(sql_string, &@$);
      free($1);
    }
    | LENGTH LBRACE SSS RBRACE{
      // length('str')  // bug: length('date') error 
      $$ = new FunctionSqlNode();
      std::string s = std::string($3).substr(1, strlen($3) - 2);
      $$->function_kernel = make_unique<LengthFunctionKernel>(true, s);
      $$->is_const = true;
      $$->virtual_field_name = token_name(sql_string, &@$);
      free($1);
    }
    | DATE_FORMAT LBRACE DATE_STR COMMA SSS RBRACE{
      // date_format("2017-06-15", "%y")
      $$ = new FunctionSqlNode();
      std::string s = std::string($3).substr(1, strlen($3) - 2);
      std::string format_without_quotation = std::string($5).substr(1, strlen($5) - 2);
      $$->function_kernel = make_unique<FormatFunctionKernel>(true, format_without_quotation, s);
      $$->is_const = true;
      $$->virtual_field_name = token_name(sql_string, &@$);
      free($1);
    }
    | ROUND LBRACE rel_attr RBRACE{
      // round(score)
      $$ = new FunctionSqlNode();
      $$->function_kernel = make_unique<RoundFunctionKernel>(false, true, 0, 0);
      $$->is_const = false;
      $$->virtual_field_name = token_name(sql_string, &@$);
      $$->rel_attr = *$3;
      free($1);
      delete $3;
    }
    | ROUND LBRACE rel_attr COMMA NUMBER RBRACE{
      // round(score, 2)
      $$ = new FunctionSqlNode();
      $$->function_kernel = make_unique<RoundFunctionKernel>(false, false, $5, 0);
      $$->is_const = false;
      $$->virtual_field_name = token_name(sql_string, &@$);
      $$->rel_attr = *$3;
      free($1);
      delete $3;
    }
    | LENGTH LBRACE rel_attr RBRACE{
      // length('str')
      $$ = new FunctionSqlNode();
      $$->function_kernel = make_unique<LengthFunctionKernel>(false, "");
      $$->is_const = false;
      $$->virtual_field_name = token_name(sql_string, &@$);
      $$->rel_attr = *$3;
      free($1);
      delete $3;
    }
    | DATE_FORMAT LBRACE rel_attr COMMA SSS RBRACE{
      // date_format(date_field, "%y")
      $$ = new FunctionSqlNode();
      std::string format_without_quotation = std::string($5).substr(1, strlen($5) - 2);
      $$->function_kernel = make_unique<FormatFunctionKernel>(false, format_without_quotation, "");
      $$->is_const = false;
      $$->virtual_field_name = token_name(sql_string, &@$);
      $$->rel_attr = *$3;
      free($1);
      delete $3;
    }
    ;

alias:
    /* empty */
    {
      $$ = nullptr;
    }
    | all_id{
      $$ = new std::string($1);
      free($1);
    }
    | AS all_id{
      $$ = new std::string($2);
      free($2);
    }

create_table_select_stmt:
    CREATE TABLE ID as select_stmt {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE_SELECT);
      CreateTableSelectSqlNode &create_table_select = $$->create_table_select;
      create_table_select.selected_sql_node_ = std::move($5->selection);
      delete $5;
      create_table_select.created_table_name_ = $3;
      free($3);
    }
    | CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE as select_stmt {
      $$ = new ParsedSqlNode(SCF_CREATE_TABLE_SELECT);
      CreateTableSelectSqlNode &create_table_select = $$->create_table_select;
      create_table_select.selected_sql_node_ = std::move($9->selection);
      delete $9;
      create_table_select.created_table_name_ = $3;
      free($3);
      delete $5;
      delete $6;
    }
    ;

as:
    /* empty */
    {
    }
    | AS{
    }
    ;

// 如果出现聚集函数名，内置函数名和ID冲突，将文法中ID换成all_id
all_id:
  ID {
    $$ = $1;
  }
  | SUM {
    $$ = $1;
  }
  | MAX {
    $$ = $1;
  }
  | MIN {
    $$ = $1;
  }
  | AVG {
    $$ = $1;
  }
  | COUNT {
    $$ = $1;
  }
  | ROUND {
    $$ = $1;
  }
  | LENGTH {
    $$ = $1;
  }
  | DATE_FORMAT {
    $$ = $1;
  }

opt_semicolon: /*empty*/
    | SEMICOLON
    ;
%%
//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, ParsedSqlResult *sql_result) {
  /* yydebug = 1; */
  yyscan_t scanner;
  yylex_init(&scanner);
  scan_string(s, scanner);
  int result = yyparse(s, sql_result, scanner);
  yylex_destroy(scanner);
  return result;
}
