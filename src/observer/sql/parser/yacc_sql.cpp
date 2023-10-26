/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "yacc_sql.y"


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


#line 118 "yacc_sql.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc_sql.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_CREATE = 4,                     /* CREATE  */
  YYSYMBOL_DROP = 5,                       /* DROP  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_TABLES = 7,                     /* TABLES  */
  YYSYMBOL_INDEX = 8,                      /* INDEX  */
  YYSYMBOL_CALC = 9,                       /* CALC  */
  YYSYMBOL_SELECT = 10,                    /* SELECT  */
  YYSYMBOL_DESC = 11,                      /* DESC  */
  YYSYMBOL_SHOW = 12,                      /* SHOW  */
  YYSYMBOL_SYNC = 13,                      /* SYNC  */
  YYSYMBOL_INSERT = 14,                    /* INSERT  */
  YYSYMBOL_DELETE = 15,                    /* DELETE  */
  YYSYMBOL_UPDATE = 16,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 17,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 18,                    /* RBRACE  */
  YYSYMBOL_COMMA = 19,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 20,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 21,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 22,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 23,                     /* INT_T  */
  YYSYMBOL_STRING_T = 24,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 25,                   /* FLOAT_T  */
  YYSYMBOL_HELP = 26,                      /* HELP  */
  YYSYMBOL_EXIT = 27,                      /* EXIT  */
  YYSYMBOL_DOT = 28,                       /* DOT  */
  YYSYMBOL_INTO = 29,                      /* INTO  */
  YYSYMBOL_VALUES = 30,                    /* VALUES  */
  YYSYMBOL_FROM = 31,                      /* FROM  */
  YYSYMBOL_WHERE = 32,                     /* WHERE  */
  YYSYMBOL_AND = 33,                       /* AND  */
  YYSYMBOL_SET = 34,                       /* SET  */
  YYSYMBOL_ON = 35,                        /* ON  */
  YYSYMBOL_LOAD = 36,                      /* LOAD  */
  YYSYMBOL_DATA = 37,                      /* DATA  */
  YYSYMBOL_INFILE = 38,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 39,                   /* EXPLAIN  */
  YYSYMBOL_EQ = 40,                        /* EQ  */
  YYSYMBOL_LT = 41,                        /* LT  */
  YYSYMBOL_GT = 42,                        /* GT  */
  YYSYMBOL_LE = 43,                        /* LE  */
  YYSYMBOL_GE = 44,                        /* GE  */
  YYSYMBOL_NE = 45,                        /* NE  */
  YYSYMBOL_DATE_T = 46,                    /* DATE_T  */
  YYSYMBOL_TEXT_T = 47,                    /* TEXT_T  */
  YYSYMBOL_ORDER_BY = 48,                  /* ORDER_BY  */
  YYSYMBOL_ASC = 49,                       /* ASC  */
  YYSYMBOL_SYM_NOT_NULL = 50,              /* SYM_NOT_NULL  */
  YYSYMBOL_SYM_NULL = 51,                  /* SYM_NULL  */
  YYSYMBOL_SYM_IS_NULL = 52,               /* SYM_IS_NULL  */
  YYSYMBOL_SYM_IS_NOT_NULL = 53,           /* SYM_IS_NOT_NULL  */
  YYSYMBOL_SYM_IN = 54,                    /* SYM_IN  */
  YYSYMBOL_SYM_NOT_IN = 55,                /* SYM_NOT_IN  */
  YYSYMBOL_SYM_EXISTS = 56,                /* SYM_EXISTS  */
  YYSYMBOL_SYM_NOT_EXISTS = 57,            /* SYM_NOT_EXISTS  */
  YYSYMBOL_MIN = 58,                       /* MIN  */
  YYSYMBOL_MAX = 59,                       /* MAX  */
  YYSYMBOL_AVG = 60,                       /* AVG  */
  YYSYMBOL_COUNT = 61,                     /* COUNT  */
  YYSYMBOL_SUM = 62,                       /* SUM  */
  YYSYMBOL_GROUP_BY = 63,                  /* GROUP_BY  */
  YYSYMBOL_HAVING = 64,                    /* HAVING  */
  YYSYMBOL_SYM_LIKE = 65,                  /* SYM_LIKE  */
  YYSYMBOL_SYM_NOT_LIKE = 66,              /* SYM_NOT_LIKE  */
  YYSYMBOL_ROUND = 67,                     /* ROUND  */
  YYSYMBOL_LENGTH = 68,                    /* LENGTH  */
  YYSYMBOL_DATE_FORMAT = 69,               /* DATE_FORMAT  */
  YYSYMBOL_DATE_STR = 70,                  /* DATE_STR  */
  YYSYMBOL_NUMBER = 71,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 72,                     /* FLOAT  */
  YYSYMBOL_ID = 73,                        /* ID  */
  YYSYMBOL_SSS = 74,                       /* SSS  */
  YYSYMBOL_75_ = 75,                       /* '+'  */
  YYSYMBOL_76_ = 76,                       /* '-'  */
  YYSYMBOL_77_ = 77,                       /* '*'  */
  YYSYMBOL_78_ = 78,                       /* '/'  */
  YYSYMBOL_UMINUS = 79,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 80,                  /* $accept  */
  YYSYMBOL_commands = 81,                  /* commands  */
  YYSYMBOL_command_wrapper = 82,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 83,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 84,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 85,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 86,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 87,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 88,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 89,           /* drop_table_stmt  */
  YYSYMBOL_show_tables_stmt = 90,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 91,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 92,         /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 93,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 94,         /* create_table_stmt  */
  YYSYMBOL_attr_def_list = 95,             /* attr_def_list  */
  YYSYMBOL_attr_def = 96,                  /* attr_def  */
  YYSYMBOL_basic_attr_def = 97,            /* basic_attr_def  */
  YYSYMBOL_number = 98,                    /* number  */
  YYSYMBOL_type = 99,                      /* type  */
  YYSYMBOL_insert_stmt = 100,              /* insert_stmt  */
  YYSYMBOL_value_rows = 101,               /* value_rows  */
  YYSYMBOL_value_row = 102,                /* value_row  */
  YYSYMBOL_value_list = 103,               /* value_list  */
  YYSYMBOL_value = 104,                    /* value  */
  YYSYMBOL_non_negative_value = 105,       /* non_negative_value  */
  YYSYMBOL_negative_value = 106,           /* negative_value  */
  YYSYMBOL_delete_stmt = 107,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 108,              /* update_stmt  */
  YYSYMBOL_set_value_list = 109,           /* set_value_list  */
  YYSYMBOL_set_value = 110,                /* set_value  */
  YYSYMBOL_select_stmt = 111,              /* select_stmt  */
  YYSYMBOL_group_by = 112,                 /* group_by  */
  YYSYMBOL_group_by_list = 113,            /* group_by_list  */
  YYSYMBOL_calc_stmt = 114,                /* calc_stmt  */
  YYSYMBOL_expression_list = 115,          /* expression_list  */
  YYSYMBOL_expression = 116,               /* expression  */
  YYSYMBOL_rel_attr = 117,                 /* rel_attr  */
  YYSYMBOL_rel_list = 118,                 /* rel_list  */
  YYSYMBOL_where = 119,                    /* where  */
  YYSYMBOL_expr_list = 120,                /* expr_list  */
  YYSYMBOL_expr = 121,                     /* expr  */
  YYSYMBOL_identifier = 122,               /* identifier  */
  YYSYMBOL_sub_query = 123,                /* sub_query  */
  YYSYMBOL_order_by = 124,                 /* order_by  */
  YYSYMBOL_order_by_list = 125,            /* order_by_list  */
  YYSYMBOL_order_by_attr = 126,            /* order_by_attr  */
  YYSYMBOL_comp_op = 127,                  /* comp_op  */
  YYSYMBOL_load_data_stmt = 128,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 129,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 130,        /* set_variable_stmt  */
  YYSYMBOL_agg_func = 131,                 /* agg_func  */
  YYSYMBOL_select_expr = 132,              /* select_expr  */
  YYSYMBOL_select_expr_list = 133,         /* select_expr_list  */
  YYSYMBOL_having = 134,                   /* having  */
  YYSYMBOL_having_list = 135,              /* having_list  */
  YYSYMBOL_function_node_const = 136,      /* function_node_const  */
  YYSYMBOL_function_node_field = 137,      /* function_node_field  */
  YYSYMBOL_function_node_all = 138,        /* function_node_all  */
  YYSYMBOL_function_node_const_list = 139, /* function_node_const_list  */
  YYSYMBOL_opt_semicolon = 140             /* opt_semicolon  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  81
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   341

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  61
/* YYNRULES -- Number of rules.  */
#define YYNRULES  161
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  309

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   330


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    77,    75,     2,    76,     2,    78,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      79
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   243,   243,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   274,   280,   285,   291,   297,   303,   309,
     316,   322,   330,   348,   358,   377,   380,   393,   398,   403,
     411,   419,   427,   438,   442,   443,   444,   445,   448,   458,
     463,   471,   485,   488,   503,   506,   512,   516,   520,   530,
     537,   544,   547,   553,   565,   582,   587,   596,   608,   648,
     665,   668,   676,   682,   690,   700,   705,   716,   719,   722,
     725,   728,   732,   735,   763,   768,   796,   799,   813,   816,
     822,   826,   833,   837,   841,   845,   849,   854,   861,   868,
     875,   882,   888,   895,   902,   907,   912,   917,   922,   926,
     930,   934,   941,   945,   952,   962,   965,   972,   975,   981,
     989,   995,  1002,  1011,  1012,  1013,  1014,  1015,  1016,  1020,
    1033,  1041,  1056,  1063,  1070,  1077,  1084,  1091,  1104,  1111,
    1116,  1121,  1129,  1134,  1143,  1146,  1152,  1156,  1163,  1170,
    1177,  1185,  1197,  1206,  1215,  1224,  1237,  1240,  1246,  1251,
    1257,  1258
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "CREATE",
  "DROP", "TABLE", "TABLES", "INDEX", "CALC", "SELECT", "DESC", "SHOW",
  "SYNC", "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE", "COMMA",
  "TRX_BEGIN", "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T",
  "FLOAT_T", "HELP", "EXIT", "DOT", "INTO", "VALUES", "FROM", "WHERE",
  "AND", "SET", "ON", "LOAD", "DATA", "INFILE", "EXPLAIN", "EQ", "LT",
  "GT", "LE", "GE", "NE", "DATE_T", "TEXT_T", "ORDER_BY", "ASC",
  "SYM_NOT_NULL", "SYM_NULL", "SYM_IS_NULL", "SYM_IS_NOT_NULL", "SYM_IN",
  "SYM_NOT_IN", "SYM_EXISTS", "SYM_NOT_EXISTS", "MIN", "MAX", "AVG",
  "COUNT", "SUM", "GROUP_BY", "HAVING", "SYM_LIKE", "SYM_NOT_LIKE",
  "ROUND", "LENGTH", "DATE_FORMAT", "DATE_STR", "NUMBER", "FLOAT", "ID",
  "SSS", "'+'", "'-'", "'*'", "'/'", "UMINUS", "$accept", "commands",
  "command_wrapper", "exit_stmt", "help_stmt", "sync_stmt", "begin_stmt",
  "commit_stmt", "rollback_stmt", "drop_table_stmt", "show_tables_stmt",
  "desc_table_stmt", "create_index_stmt", "drop_index_stmt",
  "create_table_stmt", "attr_def_list", "attr_def", "basic_attr_def",
  "number", "type", "insert_stmt", "value_rows", "value_row", "value_list",
  "value", "non_negative_value", "negative_value", "delete_stmt",
  "update_stmt", "set_value_list", "set_value", "select_stmt", "group_by",
  "group_by_list", "calc_stmt", "expression_list", "expression",
  "rel_attr", "rel_list", "where", "expr_list", "expr", "identifier",
  "sub_query", "order_by", "order_by_list", "order_by_attr", "comp_op",
  "load_data_stmt", "explain_stmt", "set_variable_stmt", "agg_func",
  "select_expr", "select_expr_list", "having", "having_list",
  "function_node_const", "function_node_field", "function_node_all",
  "function_node_const_list", "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-203)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-157)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     194,    35,    41,     9,   181,   -43,    31,  -203,     6,    20,
     -15,  -203,  -203,  -203,  -203,  -203,   -11,    55,   194,    65,
     125,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,
    -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,
    -203,    36,    62,    64,    70,     9,  -203,  -203,  -203,  -203,
    -203,     9,  -203,  -203,    63,   129,   156,   158,   163,   170,
     175,   177,   179,   191,  -203,  -203,  -203,   208,   198,     8,
    -203,  -203,  -203,  -203,  -203,   159,   161,   197,   195,   206,
    -203,  -203,  -203,  -203,   219,   212,  -203,   216,    -2,  -203,
       9,     9,     9,     9,     9,   182,   182,   182,   -37,   182,
      -1,     4,    22,   183,   181,   184,    58,   222,   227,   187,
     119,   188,   190,   192,   193,  -203,  -203,    81,    81,  -203,
    -203,   243,   246,   249,   250,   251,   252,    87,   150,   253,
     254,   255,   256,  -203,  -203,  -203,   257,   260,   261,   262,
     263,  -203,   264,    91,  -203,   233,   227,   265,   114,  -203,
    -203,  -203,   258,   199,   266,   151,   269,  -203,  -203,  -203,
    -203,  -203,  -203,  -203,  -203,   209,  -203,   217,  -203,  -203,
     215,   218,   210,   227,   221,   220,   223,   119,   271,  -203,
      42,   274,   278,   268,    91,    91,  -203,   267,   -21,  -203,
    -203,   119,  -203,   187,  -203,  -203,   292,  -203,  -203,  -203,
    -203,  -203,   282,   190,   283,  -203,  -203,   229,   285,   286,
     287,   288,   257,   244,   289,   264,  -203,    79,   291,   300,
     300,   238,   146,   146,    91,  -203,  -203,  -203,  -203,  -203,
    -203,  -203,  -203,   295,   296,   240,   241,    91,    91,    91,
      91,    91,  -203,  -203,   245,   248,   266,  -203,   257,  -203,
    -203,  -203,  -203,  -203,   182,   259,   119,   298,  -203,  -203,
    -203,   299,   302,  -203,   -21,   300,   300,  -203,  -203,   160,
     160,   152,   152,   106,  -203,  -203,   303,  -203,   304,  -203,
     305,    91,   277,   289,  -203,  -203,  -203,   308,   309,  -203,
    -203,   182,   -21,   297,   182,  -203,  -203,  -203,  -203,  -203,
      91,     1,  -203,   310,   -21,  -203,  -203,   182,  -203
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    25,     0,     0,
       0,    26,    27,    28,    24,    23,     0,     0,     0,     0,
     160,    22,    21,    14,    15,    16,    17,     9,    10,    11,
      12,    13,     8,     5,     7,     6,     4,     3,    18,    19,
      20,     0,     0,     0,     0,     0,    60,    59,    56,    57,
      58,     0,    83,    74,    75,     0,     0,     0,     0,     0,
       0,     0,     0,    84,   138,   140,   139,   142,     0,   158,
     157,   141,    69,    31,    30,     0,     0,     0,     0,     0,
     130,     1,   161,     2,     0,     0,    29,     0,     0,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    88,     0,
       0,     0,     0,     0,     0,    81,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,   143,   156,    86,     0,     0,     0,
     158,   159,     0,     0,    63,     0,    88,    65,     0,   131,
      54,    55,     0,     0,    35,    37,     0,    33,   134,   133,
     135,   137,   136,   132,   148,     0,   152,     0,   150,   154,
       0,     0,     0,    88,     0,     0,     0,     0,    48,    49,
       0,     0,     0,   112,     0,     0,    92,    89,    90,    93,
      94,     0,    64,     0,    61,    62,     0,    44,    45,    46,
      47,    42,    41,     0,     0,    38,    39,     0,     0,     0,
       0,     0,    86,    70,    52,     0,   114,     0,     0,     0,
       0,     0,    95,    96,     0,   123,   124,   125,   126,   127,
     128,   106,   107,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    67,    66,     0,     0,    35,    34,    86,   149,
     153,   151,   155,    87,     0,   144,     0,     0,    50,   108,
     109,     0,     0,   113,    91,     0,     0,   102,   103,    97,
      98,    99,   100,   101,   129,    43,     0,    36,     0,    71,
      72,     0,   115,    52,    51,   110,   111,     0,     0,    40,
      32,     0,   146,   145,   117,    68,    53,   104,   105,    73,
       0,   120,   116,   118,   147,   122,   121,   117,   119
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -203,  -203,   313,  -203,  -203,  -203,  -203,  -203,  -203,  -203,
    -203,  -203,  -203,  -203,  -203,    82,   130,  -203,  -203,  -203,
    -203,  -203,   117,    51,  -162,    -3,  -203,  -203,  -203,   142,
    -203,    28,  -203,    45,  -203,   247,    85,   -94,  -187,  -136,
    -203,  -171,  -203,  -202,  -203,    32,  -203,  -203,  -203,  -203,
    -203,     7,  -203,   234,  -203,  -203,   -20,  -203,  -203,   235,
    -203
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,   204,   154,   155,   276,   202,
      33,   178,   179,   257,   149,   186,   151,    34,    35,   146,
     147,   216,   255,   279,    37,    53,    54,    65,   173,   144,
     187,   188,   189,   218,   295,   302,   303,   241,    38,    39,
      40,   190,    67,    68,   282,   293,    69,    70,    71,    72,
      83
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      52,   121,   122,   123,   125,   126,   128,   130,   132,   217,
     192,    66,   305,   222,   223,   214,   115,   261,   262,   225,
     226,   227,   228,   229,   230,   253,    45,   106,    36,   242,
      73,   231,   232,   233,   234,    75,    63,   213,    74,  -156,
     124,    41,    52,    42,   235,   236,    36,    43,    52,    44,
     306,    76,     4,   264,   237,   238,   239,   240,    77,   180,
      46,   278,    78,   287,   288,    81,   269,   270,   271,   272,
     273,   127,    63,    91,    92,    93,    94,    63,   129,    47,
      48,    49,    90,    50,   135,    51,   140,    52,    52,    52,
      52,    52,    79,    46,   283,    63,   131,   259,   181,   182,
      55,    56,    57,    58,    59,   164,   165,   150,   180,    84,
     292,    66,    47,    48,    49,   183,    50,   184,   185,   225,
     226,   227,   228,   229,   230,   137,   138,   139,    82,   304,
      88,   231,   232,   233,   234,    85,    89,    86,    91,    92,
      93,    94,    46,    87,   235,   236,    95,   181,   182,    55,
      56,    57,    58,    59,   237,   238,   239,   240,    93,    94,
     280,    47,    48,    49,   183,    50,   184,   185,   166,   167,
      46,   235,   236,    96,   150,    97,   117,   118,   119,   120,
      98,   237,   238,   239,   240,   194,   195,    99,   150,    47,
      48,    49,   100,    50,   101,   148,   102,   280,     1,     2,
     301,   205,   206,     3,     4,     5,     6,     7,     8,     9,
      10,  -157,  -157,   301,    11,    12,    13,   235,   236,   103,
      14,    15,   197,   198,   199,   235,   236,   104,    16,   105,
      17,   109,   107,    18,   108,   110,   112,   239,   240,    55,
      56,    57,    58,    59,   111,   200,   201,   113,    60,    61,
      62,   114,   142,   150,    63,    63,   133,   136,    64,   143,
     145,   158,   152,   153,   159,   156,   157,   160,   161,   162,
     163,   168,   169,   191,   170,   171,   172,   174,   175,   176,
     208,   177,   106,   212,   193,   203,   207,   196,   209,   210,
     215,   219,   211,   127,   129,   220,   221,   131,   244,   245,
     224,   247,   248,   249,   250,   251,   252,   254,   256,   260,
       4,   263,   265,   266,   267,   268,   284,   285,   274,   275,
     286,   289,   290,   281,   291,   294,   297,   298,   277,   307,
     300,    80,   258,   246,   296,   243,   299,   116,   134,   308,
       0,   141
};

static const yytype_int16 yycheck[] =
{
       3,    95,    96,    97,    98,    99,   100,   101,   102,   180,
     146,     4,    11,   184,   185,   177,    18,   219,   220,    40,
      41,    42,    43,    44,    45,   212,    17,    19,     0,   191,
      73,    52,    53,    54,    55,    29,    73,   173,     7,    31,
      77,     6,    45,     8,    65,    66,    18,     6,    51,     8,
      49,    31,    10,   224,    75,    76,    77,    78,    73,    17,
      51,   248,    73,   265,   266,     0,   237,   238,   239,   240,
     241,    72,    73,    75,    76,    77,    78,    73,    74,    70,
      71,    72,    19,    74,   104,    76,   106,    90,    91,    92,
      93,    94,    37,    51,   256,    73,    74,    18,    56,    57,
      58,    59,    60,    61,    62,    18,    19,   110,    17,    73,
     281,   104,    70,    71,    72,    73,    74,    75,    76,    40,
      41,    42,    43,    44,    45,    67,    68,    69,     3,   300,
      45,    52,    53,    54,    55,    73,    51,    73,    75,    76,
      77,    78,    51,    73,    65,    66,    17,    56,    57,    58,
      59,    60,    61,    62,    75,    76,    77,    78,    77,    78,
     254,    70,    71,    72,    73,    74,    75,    76,    18,    19,
      51,    65,    66,    17,   177,    17,    91,    92,    93,    94,
      17,    75,    76,    77,    78,    71,    72,    17,   191,    70,
      71,    72,    17,    74,    17,    76,    17,   291,     4,     5,
     294,    50,    51,     9,    10,    11,    12,    13,    14,    15,
      16,    65,    66,   307,    20,    21,    22,    65,    66,    28,
      26,    27,    23,    24,    25,    65,    66,    19,    34,    31,
      36,    34,    73,    39,    73,    40,    17,    77,    78,    58,
      59,    60,    61,    62,    38,    46,    47,    35,    67,    68,
      69,    35,    30,   256,    73,    73,    73,    73,    77,    32,
      73,    18,    74,    73,    18,    73,    73,    18,    18,    18,
      18,    18,    18,    40,    19,    19,    19,    17,    17,    17,
      71,    17,    19,    73,    19,    19,    17,    29,    71,    74,
      19,    17,    74,    72,    74,    17,    28,    74,     6,    17,
      33,    18,    73,    18,    18,    18,    18,    63,    19,    18,
      10,    73,    17,    17,    74,    74,    18,    18,    73,    71,
      18,    18,    18,    64,    19,    48,    18,    18,   246,    19,
      33,    18,   215,   203,   283,   193,   291,    90,   104,   307,
      -1,   106
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,     5,     9,    10,    11,    12,    13,    14,    15,
      16,    20,    21,    22,    26,    27,    34,    36,    39,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,   100,   107,   108,   111,   114,   128,   129,
     130,     6,     8,     6,     8,    17,    51,    70,    71,    72,
      74,    76,   105,   115,   116,    58,    59,    60,    61,    62,
      67,    68,    69,    73,    77,   117,   131,   132,   133,   136,
     137,   138,   139,    73,     7,    29,    31,    73,    73,    37,
      82,     0,     3,   140,    73,    73,    73,    73,   116,   116,
      19,    75,    76,    77,    78,    17,    17,    17,    17,    17,
      17,    17,    17,    28,    19,    31,    19,    73,    73,    34,
      40,    38,    17,    35,    35,    18,   115,   116,   116,   116,
     116,   117,   117,   117,    77,   117,   117,    72,   117,    74,
     117,    74,   117,    73,   133,   136,    73,    67,    68,    69,
     136,   139,    30,    32,   119,    73,   109,   110,    76,   104,
     105,   106,    74,    73,    96,    97,    73,    73,    18,    18,
      18,    18,    18,    18,    18,    19,    18,    19,    18,    18,
      19,    19,    19,   118,    17,    17,    17,    17,   101,   102,
      17,    56,    57,    73,    75,    76,   105,   120,   121,   122,
     131,    40,   119,    19,    71,    72,    29,    23,    24,    25,
      46,    47,    99,    19,    95,    50,    51,    17,    71,    71,
      74,    74,    73,   119,   104,    19,   111,   121,   123,    17,
      17,    28,   121,   121,    33,    40,    41,    42,    43,    44,
      45,    52,    53,    54,    55,    65,    66,    75,    76,    77,
      78,   127,   104,   109,     6,    17,    96,    18,    73,    18,
      18,    18,    18,   118,    63,   112,    19,   103,   102,    18,
      18,   123,   123,    73,   121,    17,    17,    74,    74,   121,
     121,   121,   121,   121,    73,    71,    98,    95,   118,   113,
     117,    64,   134,   104,    18,    18,    18,   123,   123,    18,
      18,    19,   121,   135,    48,   124,   103,    18,    18,   113,
      33,   117,   125,   126,   121,    11,    49,    19,   125
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    80,    81,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    95,    96,    96,    96,
      97,    97,    97,    98,    99,    99,    99,    99,   100,   101,
     101,   102,   103,   103,   104,   104,   105,   105,   105,   105,
     105,   106,   106,   107,   108,   109,   109,   110,   111,   111,
     112,   112,   113,   113,   114,   115,   115,   116,   116,   116,
     116,   116,   116,   116,   117,   117,   118,   118,   119,   119,
     120,   120,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   122,   122,   123,   124,   124,   125,   125,   125,
     126,   126,   126,   127,   127,   127,   127,   127,   127,   128,
     129,   130,   131,   131,   131,   131,   131,   131,   132,   132,
     132,   132,   133,   133,   134,   134,   135,   135,   136,   136,
     136,   136,   137,   137,   137,   137,   138,   138,   139,   139,
     140,   140
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     2,     9,     5,     7,     0,     3,     1,     2,     2,
       5,     2,     2,     1,     1,     1,     1,     1,     5,     1,
       3,     4,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     4,     5,     1,     3,     3,     9,     2,
       0,     2,     1,     3,     2,     1,     3,     3,     3,     3,
       3,     3,     2,     1,     1,     3,     0,     3,     0,     2,
       1,     3,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     5,     5,     2,     2,     3,     3,
       4,     4,     1,     3,     1,     0,     2,     0,     1,     3,
       1,     2,     2,     1,     1,     1,     1,     1,     1,     7,
       2,     4,     4,     4,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     3,     0,     2,     1,     3,     4,     6,
       4,     6,     4,     6,     4,     6,     1,     1,     1,     3,
       0,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, sql_string, sql_result, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, sql_string, sql_result, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), sql_string, sql_result, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, sql_string, sql_result, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* commands: command_wrapper opt_semicolon  */
#line 244 "yacc_sql.y"
  {
    std::unique_ptr<ParsedSqlNode> sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 1885 "yacc_sql.cpp"
    break;

  case 23: /* exit_stmt: EXIT  */
#line 274 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 1894 "yacc_sql.cpp"
    break;

  case 24: /* help_stmt: HELP  */
#line 280 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 1902 "yacc_sql.cpp"
    break;

  case 25: /* sync_stmt: SYNC  */
#line 285 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 1910 "yacc_sql.cpp"
    break;

  case 26: /* begin_stmt: TRX_BEGIN  */
#line 291 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 1918 "yacc_sql.cpp"
    break;

  case 27: /* commit_stmt: TRX_COMMIT  */
#line 297 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 1926 "yacc_sql.cpp"
    break;

  case 28: /* rollback_stmt: TRX_ROLLBACK  */
#line 303 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 1934 "yacc_sql.cpp"
    break;

  case 29: /* drop_table_stmt: DROP TABLE ID  */
#line 309 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1944 "yacc_sql.cpp"
    break;

  case 30: /* show_tables_stmt: SHOW TABLES  */
#line 316 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 1952 "yacc_sql.cpp"
    break;

  case 31: /* desc_table_stmt: DESC ID  */
#line 322 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1962 "yacc_sql.cpp"
    break;

  case 32: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE ID rel_list RBRACE  */
#line 331 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-6].string);
      create_index.relation_name = (yyvsp[-4].string);
      if ((yyvsp[-1].relation_list) != nullptr) {
        create_index.attribute_names.swap(*(yyvsp[-1].relation_list));
        delete (yyvsp[-1].relation_list);
      }
      (yyval.sql_node)->create_index.attribute_names.push_back((yyvsp[-2].string));
      free((yyvsp[-6].string));
      free((yyvsp[-4].string));
      free((yyvsp[-2].string));
    }
#line 1981 "yacc_sql.cpp"
    break;

  case 33: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 349 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].string);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 1993 "yacc_sql.cpp"
    break;

  case 34: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE  */
#line 359 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-4].string);
      free((yyvsp[-4].string));

      std::vector<AttrInfoSqlNode> *src_attrs = (yyvsp[-1].attr_infos);

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
      }
      create_table.attr_infos.emplace_back(*(yyvsp[-2].attr_info));
      std::reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete (yyvsp[-2].attr_info);
    }
#line 2013 "yacc_sql.cpp"
    break;

  case 35: /* attr_def_list: %empty  */
#line 377 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2021 "yacc_sql.cpp"
    break;

  case 36: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 381 "yacc_sql.y"
    {
      if ((yyvsp[0].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[0].attr_infos);
      } else {
        (yyval.attr_infos) = new std::vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->emplace_back(*(yyvsp[-1].attr_info));
      delete (yyvsp[-1].attr_info);
    }
#line 2035 "yacc_sql.cpp"
    break;

  case 37: /* attr_def: basic_attr_def  */
#line 394 "yacc_sql.y"
    {
      (yyvsp[0].attr_info)->nullable = false;
      (yyval.attr_info) = (yyvsp[0].attr_info);
    }
#line 2044 "yacc_sql.cpp"
    break;

  case 38: /* attr_def: basic_attr_def SYM_NOT_NULL  */
#line 399 "yacc_sql.y"
    {
      (yyvsp[-1].attr_info)->nullable = false;
      (yyval.attr_info) = (yyvsp[-1].attr_info);
    }
#line 2053 "yacc_sql.cpp"
    break;

  case 39: /* attr_def: basic_attr_def SYM_NULL  */
#line 404 "yacc_sql.y"
    {
      (yyvsp[-1].attr_info)->nullable = true;
      (yyval.attr_info) = (yyvsp[-1].attr_info);
    }
#line 2062 "yacc_sql.cpp"
    break;

  case 40: /* basic_attr_def: ID type LBRACE number RBRACE  */
#line 412 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].string);
      (yyval.attr_info)->length = (yyvsp[-1].number);
      free((yyvsp[-4].string));
    }
#line 2074 "yacc_sql.cpp"
    break;

  case 41: /* basic_attr_def: ID type  */
#line 420 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].string);
      (yyval.attr_info)->length = 4;
      free((yyvsp[-1].string));
    }
#line 2086 "yacc_sql.cpp"
    break;

  case 42: /* basic_attr_def: ID TEXT_T  */
#line 428 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = TEXTS;
      (yyval.attr_info)->name = (yyvsp[-1].string);
      (yyval.attr_info)->length = 32;
      free((yyvsp[-1].string));
    }
#line 2098 "yacc_sql.cpp"
    break;

  case 43: /* number: NUMBER  */
#line 438 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2104 "yacc_sql.cpp"
    break;

  case 44: /* type: INT_T  */
#line 442 "yacc_sql.y"
               { (yyval.number)=INTS; }
#line 2110 "yacc_sql.cpp"
    break;

  case 45: /* type: STRING_T  */
#line 443 "yacc_sql.y"
               { (yyval.number)=CHARS; }
#line 2116 "yacc_sql.cpp"
    break;

  case 46: /* type: FLOAT_T  */
#line 444 "yacc_sql.y"
               { (yyval.number)=FLOATS; }
#line 2122 "yacc_sql.cpp"
    break;

  case 47: /* type: DATE_T  */
#line 445 "yacc_sql.y"
               { (yyval.number)=DATES; }
#line 2128 "yacc_sql.cpp"
    break;

  case 48: /* insert_stmt: INSERT INTO ID VALUES value_rows  */
#line 449 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_INSERT);
      (yyval.sql_node)->insertion.relation_name = (yyvsp[-2].string);
      (yyval.sql_node)->insertion.value_rows.swap(*(yyvsp[0].value_rows));
      free((yyvsp[-2].string));
    }
#line 2139 "yacc_sql.cpp"
    break;

  case 49: /* value_rows: value_row  */
#line 459 "yacc_sql.y"
    {
      (yyval.value_rows) = new std::vector<std::vector<Value>>();
      (yyval.value_rows)->emplace_back(*(yyvsp[0].value_row));
    }
#line 2148 "yacc_sql.cpp"
    break;

  case 50: /* value_rows: value_rows COMMA value_row  */
#line 464 "yacc_sql.y"
    {
      (yyval.value_rows) = (yyvsp[-2].value_rows);
      (yyval.value_rows)->emplace_back(*(yyvsp[0].value_row));
    }
#line 2157 "yacc_sql.cpp"
    break;

  case 51: /* value_row: LBRACE value value_list RBRACE  */
#line 472 "yacc_sql.y"
    {
      (yyval.value_row) = new std::vector<Value>();
      if((yyvsp[-1].value_list) != nullptr){
        (yyval.value_row)->swap(*(yyvsp[-1].value_list));
        delete (yyvsp[-1].value_list);
      }
      (yyval.value_row)->emplace_back(*(yyvsp[-2].value));
      std::reverse((yyval.value_row)->begin(), (yyval.value_row)->end());
    }
#line 2171 "yacc_sql.cpp"
    break;

  case 52: /* value_list: %empty  */
#line 485 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2179 "yacc_sql.cpp"
    break;

  case 53: /* value_list: COMMA value value_list  */
#line 488 "yacc_sql.y"
                              { 
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new std::vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2193 "yacc_sql.cpp"
    break;

  case 54: /* value: non_negative_value  */
#line 503 "yacc_sql.y"
                       {
      (yyval.value) = (yyvsp[0].value);
    }
#line 2201 "yacc_sql.cpp"
    break;

  case 55: /* value: negative_value  */
#line 506 "yacc_sql.y"
                     {
      (yyval.value) = (yyvsp[0].value);
    }
#line 2209 "yacc_sql.cpp"
    break;

  case 56: /* non_negative_value: NUMBER  */
#line 512 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2218 "yacc_sql.cpp"
    break;

  case 57: /* non_negative_value: FLOAT  */
#line 516 "yacc_sql.y"
           {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2227 "yacc_sql.cpp"
    break;

  case 58: /* non_negative_value: SSS  */
#line 520 "yacc_sql.y"
         {
      // 这里为什么要-2
      // A: 注意这里有双引号
      char *tmp = common::substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
      if(strlen(tmp) > 65535){
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("string too long"));
      }
      (yyval.value) = new Value(tmp);
      free(tmp);
    }
#line 2242 "yacc_sql.cpp"
    break;

  case 59: /* non_negative_value: DATE_STR  */
#line 530 "yacc_sql.y"
             {
      int time;
      if(!CheckTimeRange((yyvsp[0].string), time)){
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("date invalid"));
      }
      (yyval.value) = new Value(time, AttrType::DATES);
    }
#line 2254 "yacc_sql.cpp"
    break;

  case 60: /* non_negative_value: SYM_NULL  */
#line 537 "yacc_sql.y"
              {
      (yyval.value) = new Value();
      (yyval.value)->set_type(AttrType::NULL_TYPE);
    }
#line 2263 "yacc_sql.cpp"
    break;

  case 61: /* negative_value: '-' NUMBER  */
#line 544 "yacc_sql.y"
               {
      (yyval.value) = new Value((int)-(yyvsp[0].number));
    }
#line 2271 "yacc_sql.cpp"
    break;

  case 62: /* negative_value: '-' FLOAT  */
#line 547 "yacc_sql.y"
               {
      (yyval.value) = new Value((float)-(yyvsp[0].floats));
    }
#line 2279 "yacc_sql.cpp"
    break;

  case 63: /* delete_stmt: DELETE FROM ID where  */
#line 554 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].string);
      if ((yyvsp[0].expr_node_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].expr_node_list));
        delete (yyvsp[0].expr_node_list);
      }
      free((yyvsp[-1].string));
    }
#line 2293 "yacc_sql.cpp"
    break;

  case 64: /* update_stmt: UPDATE ID SET set_value_list where  */
#line 566 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-3].string);
      if ((yyvsp[-1].set_value_list) != nullptr) {
        (yyval.sql_node)->update.set_value_list.swap(*(yyvsp[-1].set_value_list));
        delete (yyvsp[-1].set_value_list);
      }
      if ((yyvsp[0].expr_node_list) != nullptr){
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].expr_node_list));
        delete (yyvsp[0].expr_node_list);
      }
      delete (yyvsp[-3].string);
      
    }
#line 2312 "yacc_sql.cpp"
    break;

  case 65: /* set_value_list: set_value  */
#line 582 "yacc_sql.y"
            {
    (yyval.set_value_list) = new std::vector<SetValueSqlNode>;
    (yyval.set_value_list)->emplace_back(*(yyvsp[0].set_value));
    delete (yyvsp[0].set_value);
  }
#line 2322 "yacc_sql.cpp"
    break;

  case 66: /* set_value_list: set_value COMMA set_value_list  */
#line 588 "yacc_sql.y"
  {
    (yyval.set_value_list) = (yyvsp[0].set_value_list);
    (yyval.set_value_list)->emplace_back(*(yyvsp[-2].set_value));
    delete (yyvsp[-2].set_value);
  }
#line 2332 "yacc_sql.cpp"
    break;

  case 67: /* set_value: ID EQ value  */
#line 597 "yacc_sql.y"
    {
      (yyval.set_value) = new SetValueSqlNode;
      (yyval.set_value)->attr_name = (yyvsp[-2].string);
      (yyval.set_value)->value = *(yyvsp[0].value);
      delete (yyvsp[-2].string);
      delete (yyvsp[0].value);
    }
#line 2344 "yacc_sql.cpp"
    break;

  case 68: /* select_stmt: SELECT select_expr_list FROM ID rel_list where group_by having order_by  */
#line 609 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-7].select_expr_list) != nullptr) {
        (yyval.sql_node)->selection.select_exprs.swap(*(yyvsp[-7].select_expr_list));
        // 在这里直接把expr_list翻转过来
        std::reverse((yyval.sql_node)->selection.select_exprs.begin(), (yyval.sql_node)->selection.select_exprs.end());
        delete (yyvsp[-7].select_expr_list);
      }
      if ((yyvsp[-4].relation_list) != nullptr) {
        (yyval.sql_node)->selection.relations.swap(*(yyvsp[-4].relation_list));
        delete (yyvsp[-4].relation_list);
      }
      (yyval.sql_node)->selection.relations.push_back((yyvsp[-5].string));
      std::reverse((yyval.sql_node)->selection.relations.begin(), (yyval.sql_node)->selection.relations.end());

      if ((yyvsp[-3].expr_node_list) != nullptr) {
        (yyval.sql_node)->selection.conditions.swap(*(yyvsp[-3].expr_node_list));
        delete (yyvsp[-3].expr_node_list);
      }

      if ((yyvsp[-2].rel_attr_list) != nullptr) {
        (yyval.sql_node)->selection.group_by_attrs.swap(*(yyvsp[-2].rel_attr_list));
        std::reverse((yyval.sql_node)->selection.group_by_attrs.begin(), (yyval.sql_node)->selection.group_by_attrs.end());
        delete (yyvsp[-2].rel_attr_list);
      }

      if((yyvsp[-1].expr_node_list) != nullptr){
        (yyval.sql_node)->selection.having_attrs.swap(*(yyvsp[-1].expr_node_list));
        delete (yyvsp[-1].expr_node_list);
      }

      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.sql_node)->selection.order_by_attrs.swap(*(yyvsp[0].order_by_list));
        std::reverse((yyval.sql_node)->selection.order_by_attrs.begin(), (yyval.sql_node)->selection.order_by_attrs.end());
        delete (yyvsp[0].order_by_list);
      }

      free((yyvsp[-5].string));
    }
#line 2388 "yacc_sql.cpp"
    break;

  case 69: /* select_stmt: SELECT function_node_const_list  */
#line 648 "yacc_sql.y"
                                     {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[0].function_node_const_list) != nullptr) {
        std::reverse((yyvsp[0].function_node_const_list)->begin(), (yyvsp[0].function_node_const_list)->end());
        for(auto &node: *(yyvsp[0].function_node_const_list)){
          node.function_kernel->set_has_table(false);
          // 有unique ptr
          SelectExprSqlNode sesn = std::move(node);
          (yyval.sql_node)->selection.select_exprs.emplace_back(std::move(sesn));
        }
        delete (yyvsp[0].function_node_const_list);
      }
    }
#line 2406 "yacc_sql.cpp"
    break;

  case 70: /* group_by: %empty  */
#line 665 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = nullptr;
    }
#line 2414 "yacc_sql.cpp"
    break;

  case 71: /* group_by: GROUP_BY group_by_list  */
#line 668 "yacc_sql.y"
                             {
      (yyval.rel_attr_list) = (yyvsp[0].rel_attr_list);  
    }
#line 2422 "yacc_sql.cpp"
    break;

  case 72: /* group_by_list: rel_attr  */
#line 677 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      (yyval.rel_attr_list)->emplace_back(*(yyvsp[0].rel_attr));
      delete (yyvsp[0].rel_attr);
    }
#line 2432 "yacc_sql.cpp"
    break;

  case 73: /* group_by_list: rel_attr COMMA group_by_list  */
#line 683 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = (yyvsp[0].rel_attr_list);
      (yyval.rel_attr_list)->emplace_back(*(yyvsp[-2].rel_attr));
      delete (yyvsp[-2].rel_attr);
    }
#line 2442 "yacc_sql.cpp"
    break;

  case 74: /* calc_stmt: CALC expression_list  */
#line 691 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      std::reverse((yyvsp[0].expression_list)->begin(), (yyvsp[0].expression_list)->end());
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2453 "yacc_sql.cpp"
    break;

  case 75: /* expression_list: expression  */
#line 701 "yacc_sql.y"
    {
      (yyval.expression_list) = new std::vector<Expression*>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2462 "yacc_sql.cpp"
    break;

  case 76: /* expression_list: expression COMMA expression_list  */
#line 706 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new std::vector<Expression *>;
      }
      (yyval.expression_list)->emplace_back((yyvsp[-2].expression));
    }
#line 2475 "yacc_sql.cpp"
    break;

  case 77: /* expression: expression '+' expression  */
#line 716 "yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2483 "yacc_sql.cpp"
    break;

  case 78: /* expression: expression '-' expression  */
#line 719 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2491 "yacc_sql.cpp"
    break;

  case 79: /* expression: expression '*' expression  */
#line 722 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2499 "yacc_sql.cpp"
    break;

  case 80: /* expression: expression '/' expression  */
#line 725 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2507 "yacc_sql.cpp"
    break;

  case 81: /* expression: LBRACE expression RBRACE  */
#line 728 "yacc_sql.y"
                               {
      (yyval.expression) = (yyvsp[-1].expression);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2516 "yacc_sql.cpp"
    break;

  case 82: /* expression: '-' expression  */
#line 732 "yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2524 "yacc_sql.cpp"
    break;

  case 83: /* expression: non_negative_value  */
#line 735 "yacc_sql.y"
                         {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2534 "yacc_sql.cpp"
    break;

  case 84: /* rel_attr: ID  */
#line 763 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 2544 "yacc_sql.cpp"
    break;

  case 85: /* rel_attr: ID DOT ID  */
#line 768 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].string);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2556 "yacc_sql.cpp"
    break;

  case 86: /* rel_list: %empty  */
#line 796 "yacc_sql.y"
    {
      (yyval.relation_list) = nullptr;
    }
#line 2564 "yacc_sql.cpp"
    break;

  case 87: /* rel_list: COMMA ID rel_list  */
#line 799 "yacc_sql.y"
                        {
      if ((yyvsp[0].relation_list) != nullptr) {
        (yyval.relation_list) = (yyvsp[0].relation_list);
      } else {
        (yyval.relation_list) = new std::vector<std::string>;
      }

      (yyval.relation_list)->push_back((yyvsp[-1].string));
      free((yyvsp[-1].string));
    }
#line 2579 "yacc_sql.cpp"
    break;

  case 88: /* where: %empty  */
#line 813 "yacc_sql.y"
    {
      (yyval.expr_node_list) = nullptr;
    }
#line 2587 "yacc_sql.cpp"
    break;

  case 89: /* where: WHERE expr_list  */
#line 816 "yacc_sql.y"
                      {
      (yyval.expr_node_list) = (yyvsp[0].expr_node_list);
    }
#line 2595 "yacc_sql.cpp"
    break;

  case 90: /* expr_list: expr  */
#line 822 "yacc_sql.y"
         {
      (yyval.expr_node_list) = new std::vector<std::unique_ptr<ExprSqlNode>>;
      (yyval.expr_node_list)->emplace_back((yyvsp[0].expr_node));
    }
#line 2604 "yacc_sql.cpp"
    break;

  case 91: /* expr_list: expr_list AND expr  */
#line 826 "yacc_sql.y"
                         {
      (yyval.expr_node_list) = (yyvsp[-2].expr_node_list);
      (yyval.expr_node_list)->emplace_back((yyvsp[0].expr_node));
    }
#line 2613 "yacc_sql.cpp"
    break;

  case 92: /* expr: non_negative_value  */
#line 834 "yacc_sql.y"
    {
      (yyval.expr_node) = new ValueExprSqlNode(token_name(sql_string, &(yyloc)), *(yyvsp[0].value));
    }
#line 2621 "yacc_sql.cpp"
    break;

  case 93: /* expr: identifier  */
#line 838 "yacc_sql.y"
    {
      (yyval.expr_node) = (yyvsp[0].expr_node);
    }
#line 2629 "yacc_sql.cpp"
    break;

  case 94: /* expr: agg_func  */
#line 842 "yacc_sql.y"
    {
      (yyval.expr_node) = new AggIdentifierExprSqlNode((yyvsp[0].agg_func)->name);
    }
#line 2637 "yacc_sql.cpp"
    break;

  case 95: /* expr: '+' expr  */
#line 846 "yacc_sql.y"
    {
      (yyval.expr_node) = (yyvsp[0].expr_node);
    }
#line 2645 "yacc_sql.cpp"
    break;

  case 96: /* expr: '-' expr  */
#line 850 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> sub_expr((yyvsp[0].expr_node));
      (yyval.expr_node) = new NegativeArithExprSqlNode(token_name(sql_string, &(yyloc)), std::move(sub_expr));
    }
#line 2654 "yacc_sql.cpp"
    break;

  case 97: /* expr: expr '+' expr  */
#line 855 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      std::unique_ptr<ExprSqlNode> right((yyvsp[0].expr_node));
      (yyval.expr_node) = new BinaryArithExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Add);
    }
#line 2665 "yacc_sql.cpp"
    break;

  case 98: /* expr: expr '-' expr  */
#line 862 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      std::unique_ptr<ExprSqlNode> right((yyvsp[0].expr_node));
      (yyval.expr_node) = new BinaryArithExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Sub);
    }
#line 2676 "yacc_sql.cpp"
    break;

  case 99: /* expr: expr '*' expr  */
#line 869 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      std::unique_ptr<ExprSqlNode> right((yyvsp[0].expr_node));
      (yyval.expr_node) = new BinaryArithExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Mul);
    }
#line 2687 "yacc_sql.cpp"
    break;

  case 100: /* expr: expr '/' expr  */
#line 876 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      std::unique_ptr<ExprSqlNode> right((yyvsp[0].expr_node));
      (yyval.expr_node) = new BinaryArithExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), std::move(right), 
        BinaryArithExprSqlNode::BinaryArithType::Div);
    }
#line 2698 "yacc_sql.cpp"
    break;

  case 101: /* expr: expr comp_op expr  */
#line 883 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      std::unique_ptr<ExprSqlNode> right((yyvsp[0].expr_node));
      (yyval.expr_node) = new CompareExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), std::move(right), (yyvsp[-1].comp));
    }
#line 2708 "yacc_sql.cpp"
    break;

  case 102: /* expr: expr SYM_LIKE SSS  */
#line 889 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      char *tmp = common::substr((yyvsp[0].string), 1, strlen((yyvsp[0].string)) - 2);
      (yyval.expr_node) = new LikeExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), tmp, false);
      free(tmp);
    }
#line 2719 "yacc_sql.cpp"
    break;

  case 103: /* expr: expr SYM_NOT_LIKE SSS  */
#line 896 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> left((yyvsp[-2].expr_node));
      char *tmp = common::substr((yyvsp[0].string), 1, strlen((yyvsp[0].string)) - 2);
      (yyval.expr_node) = new LikeExprSqlNode(token_name(sql_string, &(yyloc)), std::move(left), tmp, true);
      free(tmp);
    }
#line 2730 "yacc_sql.cpp"
    break;

  case 104: /* expr: expr SYM_IN LBRACE sub_query RBRACE  */
#line 903 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> child((yyvsp[-4].expr_node));
      (yyval.expr_node) = new QuantifiedCompSubqueryExprSqlNode(token_name(sql_string, &(yyloc)), std::move(child), (yyvsp[-1].sql_node), IN);
    }
#line 2739 "yacc_sql.cpp"
    break;

  case 105: /* expr: expr SYM_NOT_IN LBRACE sub_query RBRACE  */
#line 908 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> child((yyvsp[-4].expr_node));
      (yyval.expr_node) = new QuantifiedCompSubqueryExprSqlNode(token_name(sql_string, &(yyloc)), std::move(child), (yyvsp[-1].sql_node), NOT_IN);
    }
#line 2748 "yacc_sql.cpp"
    break;

  case 106: /* expr: expr SYM_IS_NULL  */
#line 913 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> child((yyvsp[-1].expr_node));
      (yyval.expr_node) = new PredNullExprSqlNode(token_name(sql_string, &(yyloc)), std::move(child), IS_NULL);
    }
#line 2757 "yacc_sql.cpp"
    break;

  case 107: /* expr: expr SYM_IS_NOT_NULL  */
#line 918 "yacc_sql.y"
    {
      std::unique_ptr<ExprSqlNode> child((yyvsp[-1].expr_node));
      (yyval.expr_node) = new PredNullExprSqlNode(token_name(sql_string, &(yyloc)), std::move(child), IS_NOT_NULL);
    }
#line 2766 "yacc_sql.cpp"
    break;

  case 108: /* expr: LBRACE expr RBRACE  */
#line 923 "yacc_sql.y"
    {
      (yyval.expr_node) = (yyvsp[-1].expr_node);
    }
#line 2774 "yacc_sql.cpp"
    break;

  case 109: /* expr: LBRACE sub_query RBRACE  */
#line 927 "yacc_sql.y"
    {
      (yyval.expr_node) = new ScalarSubqueryExprSqlNode(token_name(sql_string, &(yyloc)), (yyvsp[-1].sql_node));
    }
#line 2782 "yacc_sql.cpp"
    break;

  case 110: /* expr: SYM_EXISTS LBRACE sub_query RBRACE  */
#line 931 "yacc_sql.y"
    {
      (yyval.expr_node) = new ExistentialSubqueryExprSqlNode(token_name(sql_string, &(yyloc)), (yyvsp[-1].sql_node), EXISTS);
    }
#line 2790 "yacc_sql.cpp"
    break;

  case 111: /* expr: SYM_NOT_EXISTS LBRACE sub_query RBRACE  */
#line 935 "yacc_sql.y"
    {
      (yyval.expr_node) = new ExistentialSubqueryExprSqlNode(token_name(sql_string, &(yyloc)), (yyvsp[-1].sql_node), NOT_EXISTS);
    }
#line 2798 "yacc_sql.cpp"
    break;

  case 112: /* identifier: ID  */
#line 942 "yacc_sql.y"
    {
      (yyval.expr_node) = new IdentifierExprSqlNode(token_name(sql_string, &(yyloc)), std::string(), (yyvsp[0].string));
    }
#line 2806 "yacc_sql.cpp"
    break;

  case 113: /* identifier: ID DOT ID  */
#line 946 "yacc_sql.y"
    {
      (yyval.expr_node) = new IdentifierExprSqlNode(token_name(sql_string, &(yyloc)), (yyvsp[-2].string), (yyvsp[0].string));
    }
#line 2814 "yacc_sql.cpp"
    break;

  case 114: /* sub_query: select_stmt  */
#line 953 "yacc_sql.y"
    {
      (yyval.sql_node) = (yyvsp[0].sql_node);
    }
#line 2822 "yacc_sql.cpp"
    break;

  case 115: /* order_by: %empty  */
#line 962 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 2830 "yacc_sql.cpp"
    break;

  case 116: /* order_by: ORDER_BY order_by_list  */
#line 965 "yacc_sql.y"
                             {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);  
    }
#line 2838 "yacc_sql.cpp"
    break;

  case 117: /* order_by_list: %empty  */
#line 972 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 2846 "yacc_sql.cpp"
    break;

  case 118: /* order_by_list: order_by_attr  */
#line 976 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderByAttrSqlNode>;
      (yyval.order_by_list)->emplace_back(*(yyvsp[0].order_by_attr));
      delete (yyvsp[0].order_by_attr);
    }
#line 2856 "yacc_sql.cpp"
    break;

  case 119: /* order_by_list: order_by_attr COMMA order_by_list  */
#line 982 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
      (yyval.order_by_list)->emplace_back(*(yyvsp[-2].order_by_attr));
      delete (yyvsp[-2].order_by_attr);
    }
#line 2866 "yacc_sql.cpp"
    break;

  case 120: /* order_by_attr: rel_attr  */
#line 989 "yacc_sql.y"
            {
      (yyval.order_by_attr) = new OrderByAttrSqlNode;
      (yyval.order_by_attr)->attr = *(yyvsp[0].rel_attr);
      (yyval.order_by_attr)->is_asc = true;
      delete (yyvsp[0].rel_attr);
    }
#line 2877 "yacc_sql.cpp"
    break;

  case 121: /* order_by_attr: rel_attr ASC  */
#line 996 "yacc_sql.y"
    {
      (yyval.order_by_attr) = new OrderByAttrSqlNode;
      (yyval.order_by_attr)->attr = *(yyvsp[-1].rel_attr);
      (yyval.order_by_attr)->is_asc = true;
      delete (yyvsp[-1].rel_attr);
    }
#line 2888 "yacc_sql.cpp"
    break;

  case 122: /* order_by_attr: rel_attr DESC  */
#line 1003 "yacc_sql.y"
    {
      (yyval.order_by_attr) = new OrderByAttrSqlNode;
      (yyval.order_by_attr)->attr = *(yyvsp[-1].rel_attr);
      (yyval.order_by_attr)->is_asc = false;
      delete (yyvsp[-1].rel_attr);
    }
#line 2899 "yacc_sql.cpp"
    break;

  case 123: /* comp_op: EQ  */
#line 1011 "yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 2905 "yacc_sql.cpp"
    break;

  case 124: /* comp_op: LT  */
#line 1012 "yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 2911 "yacc_sql.cpp"
    break;

  case 125: /* comp_op: GT  */
#line 1013 "yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 2917 "yacc_sql.cpp"
    break;

  case 126: /* comp_op: LE  */
#line 1014 "yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 2923 "yacc_sql.cpp"
    break;

  case 127: /* comp_op: GE  */
#line 1015 "yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 2929 "yacc_sql.cpp"
    break;

  case 128: /* comp_op: NE  */
#line 1016 "yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 2935 "yacc_sql.cpp"
    break;

  case 129: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1021 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].string), 1, strlen((yyvsp[-3].string)) - 2);
      
      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].string);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free((yyvsp[0].string));
      free(tmp_file_name);
    }
#line 2949 "yacc_sql.cpp"
    break;

  case 130: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1034 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 2958 "yacc_sql.cpp"
    break;

  case 131: /* set_variable_stmt: SET ID EQ value  */
#line 1042 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].string);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      free((yyvsp[-2].string));
      delete (yyvsp[0].value);
    }
#line 2970 "yacc_sql.cpp"
    break;

  case 132: /* agg_func: SUM LBRACE rel_attr RBRACE  */
#line 1056 "yacc_sql.y"
                              {
      (yyval.agg_func) = new AggregateFuncSqlNode();
      (yyval.agg_func)->agg_op = AggregateOp::AGG_SUM;
      (yyval.agg_func)->attr = *(yyvsp[-1].rel_attr);
      (yyval.agg_func)->name = token_name(sql_string, &(yyloc));
      delete (yyvsp[-1].rel_attr);
    }
#line 2982 "yacc_sql.cpp"
    break;

  case 133: /* agg_func: MAX LBRACE rel_attr RBRACE  */
#line 1063 "yacc_sql.y"
                                {
      (yyval.agg_func) = new AggregateFuncSqlNode();
      (yyval.agg_func)->agg_op = AggregateOp::AGG_MAX;
      (yyval.agg_func)->attr = *(yyvsp[-1].rel_attr);
      (yyval.agg_func)->name = token_name(sql_string, &(yyloc));
      delete (yyvsp[-1].rel_attr);
    }
#line 2994 "yacc_sql.cpp"
    break;

  case 134: /* agg_func: MIN LBRACE rel_attr RBRACE  */
#line 1070 "yacc_sql.y"
                                {
      (yyval.agg_func) = new AggregateFuncSqlNode();
      (yyval.agg_func)->agg_op = AggregateOp::AGG_MIN;
      (yyval.agg_func)->attr = *(yyvsp[-1].rel_attr);
      (yyval.agg_func)->name = token_name(sql_string, &(yyloc));
      delete (yyvsp[-1].rel_attr);
    }
#line 3006 "yacc_sql.cpp"
    break;

  case 135: /* agg_func: AVG LBRACE rel_attr RBRACE  */
#line 1077 "yacc_sql.y"
                                {
      (yyval.agg_func) = new AggregateFuncSqlNode();
      (yyval.agg_func)->agg_op = AggregateOp::AGG_AVG;
      (yyval.agg_func)->attr = *(yyvsp[-1].rel_attr);
      (yyval.agg_func)->name = token_name(sql_string, &(yyloc));
      delete (yyvsp[-1].rel_attr);
    }
#line 3018 "yacc_sql.cpp"
    break;

  case 136: /* agg_func: COUNT LBRACE rel_attr RBRACE  */
#line 1084 "yacc_sql.y"
                                  {
      (yyval.agg_func) = new AggregateFuncSqlNode();
      (yyval.agg_func)->agg_op = AggregateOp::AGG_COUNT;
      (yyval.agg_func)->attr = *(yyvsp[-1].rel_attr);
      (yyval.agg_func)->name = token_name(sql_string, &(yyloc));
      delete (yyvsp[-1].rel_attr);
    }
#line 3030 "yacc_sql.cpp"
    break;

  case 137: /* agg_func: COUNT LBRACE '*' RBRACE  */
#line 1091 "yacc_sql.y"
                             {
      (yyval.agg_func) = new AggregateFuncSqlNode();
      (yyval.agg_func)->agg_op = AggregateOp::AGG_COUNT;

      (yyval.agg_func)->attr = RelAttrSqlNode();
      (yyval.agg_func)->attr.relation_name  = "";
      (yyval.agg_func)->attr.attribute_name = "*";
      (yyval.agg_func)->name = token_name(sql_string, &(yyloc));
    }
#line 3044 "yacc_sql.cpp"
    break;

  case 138: /* select_expr: '*'  */
#line 1104 "yacc_sql.y"
       {
      (yyval.select_expr) = new SelectExprSqlNode;
      RelAttrSqlNode attr_node;
      attr_node.relation_name  = "";
      attr_node.attribute_name = "*";
      *(yyval.select_expr) = attr_node;
    }
#line 3056 "yacc_sql.cpp"
    break;

  case 139: /* select_expr: agg_func  */
#line 1111 "yacc_sql.y"
               {
      (yyval.select_expr) = new SelectExprSqlNode();
      *(yyval.select_expr) = *(yyvsp[0].agg_func);
      delete (yyvsp[0].agg_func);
    }
#line 3066 "yacc_sql.cpp"
    break;

  case 140: /* select_expr: rel_attr  */
#line 1116 "yacc_sql.y"
               {
      (yyval.select_expr) = new SelectExprSqlNode();
      *(yyval.select_expr) = *(yyvsp[0].rel_attr);
      delete (yyvsp[0].rel_attr);
    }
#line 3076 "yacc_sql.cpp"
    break;

  case 141: /* select_expr: function_node_all  */
#line 1121 "yacc_sql.y"
                       {
      (yyval.select_expr) = new SelectExprSqlNode();
      *(yyval.select_expr) = std::move(*(yyvsp[0].function_node));
      delete (yyvsp[0].function_node);
    }
#line 3086 "yacc_sql.cpp"
    break;

  case 142: /* select_expr_list: select_expr  */
#line 1129 "yacc_sql.y"
               {
      (yyval.select_expr_list) = new std::vector<SelectExprSqlNode>;
      (yyval.select_expr_list)->emplace_back(std::move(*(yyvsp[0].select_expr)));
      delete (yyvsp[0].select_expr);
    }
#line 3096 "yacc_sql.cpp"
    break;

  case 143: /* select_expr_list: select_expr COMMA select_expr_list  */
#line 1134 "yacc_sql.y"
                                        {
      (yyval.select_expr_list) = (yyvsp[0].select_expr_list);
      (yyval.select_expr_list)->emplace_back(std::move(*(yyvsp[-2].select_expr)));
      delete (yyvsp[-2].select_expr);
    }
#line 3106 "yacc_sql.cpp"
    break;

  case 144: /* having: %empty  */
#line 1143 "yacc_sql.y"
    {
      (yyval.expr_node_list) = nullptr;
    }
#line 3114 "yacc_sql.cpp"
    break;

  case 145: /* having: HAVING having_list  */
#line 1146 "yacc_sql.y"
                         {
      (yyval.expr_node_list) = (yyvsp[0].expr_node_list);  
    }
#line 3122 "yacc_sql.cpp"
    break;

  case 146: /* having_list: expr  */
#line 1152 "yacc_sql.y"
         {
      (yyval.expr_node_list) = new std::vector<std::unique_ptr<ExprSqlNode>>;
      (yyval.expr_node_list)->emplace_back((yyvsp[0].expr_node));
    }
#line 3131 "yacc_sql.cpp"
    break;

  case 147: /* having_list: having_list AND expr  */
#line 1156 "yacc_sql.y"
                           {
      (yyval.expr_node_list) = (yyvsp[-2].expr_node_list);
      (yyval.expr_node_list)->emplace_back((yyvsp[0].expr_node));
    }
#line 3140 "yacc_sql.cpp"
    break;

  case 148: /* function_node_const: ROUND LBRACE FLOAT RBRACE  */
#line 1163 "yacc_sql.y"
                             {
      // round(3.1415)
      (yyval.function_node) = new FunctionSqlNode();
      (yyval.function_node)->function_kernel = make_unique<RoundFunctionKernel>(true, true, 0, (yyvsp[-1].floats));
      (yyval.function_node)->is_const = true;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
    }
#line 3152 "yacc_sql.cpp"
    break;

  case 149: /* function_node_const: ROUND LBRACE FLOAT COMMA NUMBER RBRACE  */
#line 1170 "yacc_sql.y"
                                            {
      // round(3.1415, 2)
      (yyval.function_node) = new FunctionSqlNode();
      (yyval.function_node)->function_kernel = make_unique<RoundFunctionKernel>(true, false, (yyvsp[-1].number), (yyvsp[-3].floats));
      (yyval.function_node)->is_const = true;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
    }
#line 3164 "yacc_sql.cpp"
    break;

  case 150: /* function_node_const: LENGTH LBRACE SSS RBRACE  */
#line 1177 "yacc_sql.y"
                              {
      // length('str')
      (yyval.function_node) = new FunctionSqlNode();
      std::string s = std::string((yyvsp[-1].string)).substr(1, strlen((yyvsp[-1].string)) - 2);
      (yyval.function_node)->function_kernel = make_unique<LengthFunctionKernel>(true, s);
      (yyval.function_node)->is_const = true;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
    }
#line 3177 "yacc_sql.cpp"
    break;

  case 151: /* function_node_const: DATE_FORMAT LBRACE SSS COMMA SSS RBRACE  */
#line 1185 "yacc_sql.y"
                                             {
      // date_format("2017-06-15", "%y")
      (yyval.function_node) = new FunctionSqlNode();
      std::string s = std::string((yyvsp[-3].string)).substr(1, strlen((yyvsp[-3].string)) - 2);
      std::string format_without_quotation = std::string((yyvsp[-1].string)).substr(1, strlen((yyvsp[-1].string)) - 2);
      (yyval.function_node)->function_kernel = make_unique<FormatFunctionKernel>(true, format_without_quotation, s);
      (yyval.function_node)->is_const = true;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
    }
#line 3191 "yacc_sql.cpp"
    break;

  case 152: /* function_node_field: ROUND LBRACE rel_attr RBRACE  */
#line 1197 "yacc_sql.y"
                                {
      // round(score)
      (yyval.function_node) = new FunctionSqlNode();
      (yyval.function_node)->function_kernel = make_unique<RoundFunctionKernel>(false, true, 0, 0);
      (yyval.function_node)->is_const = false;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
      (yyval.function_node)->rel_attr = *(yyvsp[-1].rel_attr);
      delete (yyvsp[-1].rel_attr);
    }
#line 3205 "yacc_sql.cpp"
    break;

  case 153: /* function_node_field: ROUND LBRACE rel_attr COMMA NUMBER RBRACE  */
#line 1206 "yacc_sql.y"
                                               {
      // round(score, 2)
      (yyval.function_node) = new FunctionSqlNode();
      (yyval.function_node)->function_kernel = make_unique<RoundFunctionKernel>(false, false, (yyvsp[-1].number), 0);
      (yyval.function_node)->is_const = false;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
      (yyval.function_node)->rel_attr = *(yyvsp[-3].rel_attr);
      delete (yyvsp[-3].rel_attr);
    }
#line 3219 "yacc_sql.cpp"
    break;

  case 154: /* function_node_field: LENGTH LBRACE rel_attr RBRACE  */
#line 1215 "yacc_sql.y"
                                   {
      // length('str')
      (yyval.function_node) = new FunctionSqlNode();
      (yyval.function_node)->function_kernel = make_unique<LengthFunctionKernel>(false, "");
      (yyval.function_node)->is_const = false;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
      (yyval.function_node)->rel_attr = *(yyvsp[-1].rel_attr);
      delete (yyvsp[-1].rel_attr);
    }
#line 3233 "yacc_sql.cpp"
    break;

  case 155: /* function_node_field: DATE_FORMAT LBRACE rel_attr COMMA SSS RBRACE  */
#line 1224 "yacc_sql.y"
                                                  {
      // date_format(date_field, "%y")
      (yyval.function_node) = new FunctionSqlNode();
      std::string format_without_quotation = std::string((yyvsp[-1].string)).substr(1, strlen((yyvsp[-1].string)) - 2);
      (yyval.function_node)->function_kernel = make_unique<FormatFunctionKernel>(false, format_without_quotation, "");
      (yyval.function_node)->is_const = false;
      (yyval.function_node)->virtual_field_name = token_name(sql_string, &(yyloc));
      (yyval.function_node)->rel_attr = *(yyvsp[-3].rel_attr);
      delete (yyvsp[-3].rel_attr);
    }
#line 3248 "yacc_sql.cpp"
    break;

  case 156: /* function_node_all: function_node_const  */
#line 1237 "yacc_sql.y"
                       {
      (yyval.function_node) = (yyvsp[0].function_node);
    }
#line 3256 "yacc_sql.cpp"
    break;

  case 157: /* function_node_all: function_node_field  */
#line 1240 "yacc_sql.y"
                         {
      (yyval.function_node) = (yyvsp[0].function_node);
    }
#line 3264 "yacc_sql.cpp"
    break;

  case 158: /* function_node_const_list: function_node_const  */
#line 1246 "yacc_sql.y"
                       {
      (yyval.function_node_const_list) = new std::vector<FunctionSqlNode>;
      (yyval.function_node_const_list)->emplace_back(std::move(*(yyvsp[0].function_node)));
      delete (yyvsp[0].function_node);
    }
#line 3274 "yacc_sql.cpp"
    break;

  case 159: /* function_node_const_list: function_node_const COMMA function_node_const_list  */
#line 1251 "yacc_sql.y"
                                                        {
      (yyval.function_node_const_list) = (yyvsp[0].function_node_const_list);
      (yyval.function_node_const_list)->emplace_back(std::move(*(yyvsp[-2].function_node)));
      delete (yyvsp[-2].function_node);
    }
#line 3284 "yacc_sql.cpp"
    break;


#line 3288 "yacc_sql.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, sql_string, sql_result, scanner, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, sql_string, sql_result, scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1260 "yacc_sql.y"

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, ParsedSqlResult *sql_result) {
  yyscan_t scanner;
  yylex_init(&scanner);
  scan_string(s, scanner);
  int result = yyparse(s, sql_result, scanner);
  yylex_destroy(scanner);
  return result;
}
