%top{
/*
这里的代码会被复制到lex_sql.cpp的最开始位置
定义yy_size_t的原因是因为flex生成的代码，会使用yy_size_t与其他类型的数字
做比较，导致编译报警
*/
#define YY_TYPEDEF_YY_SIZE_T
typedef int yy_size_t;

/* 参考生成的lex_sql.cpp代码，这个宏定义会放在每次运行yylex()最开始的地方 */
#define YY_USER_INIT                                         \
  yycolumn = 0;

/* 参考生成的lex_sql.cpp代码，这个宏定义会放在解析一个token之后，也可以在网上找到大量的参考资料 */
/* 我们在这里设置当前解析的token的位置信息，这样在yacc中就可以使用这些信息了 */
#define YY_USER_ACTION                                       \
do {                                                         \
  yylloc->first_line   = yylloc->last_line = yylineno;       \
  yylloc->first_column = yycolumn;                           \
  yylloc->last_column  = yylloc->first_column + yyleng - 1;  \
  yycolumn += yyleng;                                        \
}                                                            \
while (0);

}

%{
#include<string.h>
#include<stdio.h>

/**
 * flex 代码包含三个部分，使用 %% 分隔
 * 第一个部分的代码是C代码，flex会原样复制到目标文件中
 * 第二个部分是规则部分，使用正则表达式定义一系列规则
 * 第三个部分还是C代码，flex 会复制此段代码
 */

#include "sql/parser/parse_defs.h"
#include "common/log/log.h"
#include "yacc_sql.hpp"

#ifndef register
#define register 
#endif // register

extern int atoi();
extern double atof();

#define RETURN_TOKEN(token) LOG_DEBUG("%s", #token);return token
%}

/* Prevent the need for linking with -lfl */
%option noyywrap
%option nounput
%option noinput
%option bison-bridge
%option reentrant
/* 不区分大小写 */
%option case-insensitive
%option bison-locations

WHITE_SAPCE      [\ \t\b\f]
DIGIT            [0-9]+

ID               [A-Za-z_]+[A-Za-z0-9_]*
DOT              \.
QUOTE            [\'\"]
%x STR

/* 规则匹配的优先级：*/
/* 1. 匹配的规则长的优先 */
/* 2. 写在最前面的优先 */

/* yylval 就可以认为是 yacc 中 %union 定义的结构体(union 结构) */
%%

{WHITE_SAPCE}                                        // ignore whitespace
\n                                                   ;

{DIGIT}+                           yylval->number=atoi(yytext); RETURN_TOKEN(NUMBER);
{DIGIT}+{DOT}{DIGIT}+              yylval->floats=(float)(atof(yytext)); RETURN_TOKEN(FLOAT);

";"                                     RETURN_TOKEN(SEMICOLON);
{DOT}                                   RETURN_TOKEN(DOT);
EXIT                                    RETURN_TOKEN(EXIT);
HELP                                    RETURN_TOKEN(HELP);
DESC                                    RETURN_TOKEN(DESC);
CREATE                                  RETURN_TOKEN(CREATE);
DROP                                    RETURN_TOKEN(DROP);
TABLE                                   RETURN_TOKEN(TABLE);
TABLES                                  RETURN_TOKEN(TABLES);
INDEX                                   RETURN_TOKEN(INDEX);
UNIQUE                                  RETURN_TOKEN(UNIQUE);
ON                                      RETURN_TOKEN(ON);
SHOW                                    RETURN_TOKEN(SHOW);
SYNC                                    RETURN_TOKEN(SYNC);
SELECT                                  RETURN_TOKEN(SELECT);
CALC                                    RETURN_TOKEN(CALC);
FROM                                    RETURN_TOKEN(FROM);
WHERE                                   RETURN_TOKEN(WHERE);
AND                                     RETURN_TOKEN(AND);
OR                                      RETURN_TOKEN(OR);
INSERT                                  RETURN_TOKEN(INSERT);
INTO                                    RETURN_TOKEN(INTO);
VALUES                                  RETURN_TOKEN(VALUES);
DELETE                                  RETURN_TOKEN(DELETE);
UPDATE                                  RETURN_TOKEN(UPDATE);
SET                                     RETURN_TOKEN(SET);
BEGIN                                   RETURN_TOKEN(TRX_BEGIN);
COMMIT                                  RETURN_TOKEN(TRX_COMMIT);
ROLLBACK                                RETURN_TOKEN(TRX_ROLLBACK);
INT                                     RETURN_TOKEN(INT_T);
CHAR                                    RETURN_TOKEN(STRING_T);
FLOAT                                   RETURN_TOKEN(FLOAT_T);
DATE                                    RETURN_TOKEN(DATE_T);
{QUOTE}[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}{QUOTE} yylval->string=strdup(yytext);RETURN_TOKEN(DATE_STR);
INFILE                                  RETURN_TOKEN(INFILE);
EXPLAIN                                 RETURN_TOKEN(EXPLAIN);
"ORDER BY"                              RETURN_TOKEN(ORDER_BY);
MIN                                     yylval->string=strdup(yytext); RETURN_TOKEN(MIN);
MAX                                     yylval->string=strdup(yytext); RETURN_TOKEN(MAX);
AVG                                     yylval->string=strdup(yytext); RETURN_TOKEN(AVG);
COUNT                                   yylval->string=strdup(yytext); RETURN_TOKEN(COUNT);
SUM                                     yylval->string=strdup(yytext); RETURN_TOKEN(SUM);
ROUND                                   yylval->string=strdup(yytext); RETURN_TOKEN(ROUND);
"DATE_FORMAT"                           yylval->string=strdup(yytext); RETURN_TOKEN(DATE_FORMAT);
LENGTH                                  yylval->string=strdup(yytext); RETURN_TOKEN(LENGTH);
"GROUP BY"                              RETURN_TOKEN(GROUP_BY);
"HAVING"                                RETURN_TOKEN(HAVING);     
ASC                                     RETURN_TOKEN(ASC);
AS                                           RETURN_TOKEN(AS);
"not null"                              RETURN_TOKEN(SYM_NOT_NULL);
"is null"                               RETURN_TOKEN(SYM_IS_NULL);
"is not null"                           RETURN_TOKEN(SYM_IS_NOT_NULL);
"in"                                    RETURN_TOKEN(SYM_IN);
"not in"                                RETURN_TOKEN(SYM_NOT_IN);
"exists"                                RETURN_TOKEN(SYM_EXISTS);
"not exists"                            RETURN_TOKEN(SYM_NOT_EXISTS);
NULL                                    RETURN_TOKEN(SYM_NULL);
LIKE                                    RETURN_TOKEN(SYM_LIKE);
"not like"                              RETURN_TOKEN(SYM_NOT_LIKE);
TEXT                                    RETURN_TOKEN(TEXT_T);
"inner join"                            RETURN_TOKEN(SYM_INNER_JOIN);
VIEW                                    RETURN_TOKEN(SYM_VIEW);
{ID}                                    yylval->string=strdup(yytext); RETURN_TOKEN(ID);
"("                                     RETURN_TOKEN(LBRACE);
")"                                     RETURN_TOKEN(RBRACE);

","                                     RETURN_TOKEN(COMMA);
"="                                     RETURN_TOKEN(EQ);
"<="                                    RETURN_TOKEN(LE);
"<>"                                    RETURN_TOKEN(NE);
"!="                                    RETURN_TOKEN(NE);
"<"                                     RETURN_TOKEN(LT);
">="                                    RETURN_TOKEN(GE);
">"                                     RETURN_TOKEN(GT);

"+" |
"-" |
"*" |
"/"                                     { return yytext[0]; }
\"[^"]*\"                               yylval->string = strdup(yytext); RETURN_TOKEN(SSS);
'[^']*\'                                yylval->string = strdup(yytext); RETURN_TOKEN(SSS);

.                                       LOG_DEBUG("Unknown character [%c]",yytext[0]); return yytext[0];
%%

void scan_string(const char *str, yyscan_t scanner) {
  yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
}
