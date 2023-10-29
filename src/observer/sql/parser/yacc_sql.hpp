/* A Bison parser, made by GNU Bison 3.8.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_YACC_SQL_HPP_INCLUDED
# define YY_YY_YACC_SQL_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    SEMICOLON = 258,               /* SEMICOLON  */
    CREATE = 259,                  /* CREATE  */
    DROP = 260,                    /* DROP  */
    TABLE = 261,                   /* TABLE  */
    TABLES = 262,                  /* TABLES  */
    INDEX = 263,                   /* INDEX  */
    UNIQUE = 264,                  /* UNIQUE  */
    CALC = 265,                    /* CALC  */
    SELECT = 266,                  /* SELECT  */
    DESC = 267,                    /* DESC  */
    SHOW = 268,                    /* SHOW  */
    SYNC = 269,                    /* SYNC  */
    INSERT = 270,                  /* INSERT  */
    DELETE = 271,                  /* DELETE  */
    UPDATE = 272,                  /* UPDATE  */
    LBRACE = 273,                  /* LBRACE  */
    RBRACE = 274,                  /* RBRACE  */
    COMMA = 275,                   /* COMMA  */
    TRX_BEGIN = 276,               /* TRX_BEGIN  */
    TRX_COMMIT = 277,              /* TRX_COMMIT  */
    TRX_ROLLBACK = 278,            /* TRX_ROLLBACK  */
    INT_T = 279,                   /* INT_T  */
    STRING_T = 280,                /* STRING_T  */
    FLOAT_T = 281,                 /* FLOAT_T  */
    HELP = 282,                    /* HELP  */
    EXIT = 283,                    /* EXIT  */
    DOT = 284,                     /* DOT  */
    INTO = 285,                    /* INTO  */
    VALUES = 286,                  /* VALUES  */
    FROM = 287,                    /* FROM  */
    WHERE = 288,                   /* WHERE  */
    AND = 289,                     /* AND  */
    OR = 290,                      /* OR  */
    SET = 291,                     /* SET  */
    SYM_INNER_JOIN = 292,          /* SYM_INNER_JOIN  */
    ON = 293,                      /* ON  */
    LOAD = 294,                    /* LOAD  */
    DATA = 295,                    /* DATA  */
    INFILE = 296,                  /* INFILE  */
    EXPLAIN = 297,                 /* EXPLAIN  */
    EQ = 298,                      /* EQ  */
    LT = 299,                      /* LT  */
    GT = 300,                      /* GT  */
    LE = 301,                      /* LE  */
    GE = 302,                      /* GE  */
    NE = 303,                      /* NE  */
    DATE_T = 304,                  /* DATE_T  */
    TEXT_T = 305,                  /* TEXT_T  */
    ORDER_BY = 306,                /* ORDER_BY  */
    ASC = 307,                     /* ASC  */
    SYM_NOT_NULL = 308,            /* SYM_NOT_NULL  */
    SYM_NULL = 309,                /* SYM_NULL  */
    SYM_IS_NULL = 310,             /* SYM_IS_NULL  */
    SYM_IS_NOT_NULL = 311,         /* SYM_IS_NOT_NULL  */
    SYM_IN = 312,                  /* SYM_IN  */
    SYM_NOT_IN = 313,              /* SYM_NOT_IN  */
    SYM_EXISTS = 314,              /* SYM_EXISTS  */
    SYM_NOT_EXISTS = 315,          /* SYM_NOT_EXISTS  */
    GROUP_BY = 316,                /* GROUP_BY  */
    HAVING = 317,                  /* HAVING  */
    SYM_LIKE = 318,                /* SYM_LIKE  */
    SYM_NOT_LIKE = 319,            /* SYM_NOT_LIKE  */
    AS = 320,                      /* AS  */
    DATE_STR = 321,                /* DATE_STR  */
    NUMBER = 322,                  /* NUMBER  */
    FLOAT = 323,                   /* FLOAT  */
    ID = 324,                      /* ID  */
    SSS = 325,                     /* SSS  */
    MIN = 326,                     /* MIN  */
    MAX = 327,                     /* MAX  */
    AVG = 328,                     /* AVG  */
    COUNT = 329,                   /* COUNT  */
    SUM = 330,                     /* SUM  */
    ROUND = 331,                   /* ROUND  */
    LENGTH = 332,                  /* LENGTH  */
    DATE_FORMAT = 333,             /* DATE_FORMAT  */
    UMINUS = 334                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 125 "yacc_sql.y"

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

#line 181 "yacc_sql.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int yyparse (const char * sql_string, ParsedSqlResult * sql_result, void * scanner);


#endif /* !YY_YY_YACC_SQL_HPP_INCLUDED  */
