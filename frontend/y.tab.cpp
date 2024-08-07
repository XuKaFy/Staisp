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
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

  #include "ast.hpp"
  #include <memory>
  #include <cstring>
  #include <stdarg.h>

  #include "../include/convert_ast_to_ir.h"

  using namespace std;
  using namespace Ast;
  vector<pNode> root; /* the top level root node of our final AST */

  extern int yylineno;
  extern int yylex();
  extern void yyerror(const char *s);
  extern void setFileName(const char *name);
  char filename[256];

#line 90 "y.tab.cpp"

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

#include "y.tab.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT = 3,                        /* INT  */
  YYSYMBOL_FLOAT = 4,                      /* FLOAT  */
  YYSYMBOL_ID = 5,                         /* ID  */
  YYSYMBOL_GTE = 6,                        /* GTE  */
  YYSYMBOL_LTE = 7,                        /* LTE  */
  YYSYMBOL_GT = 8,                         /* GT  */
  YYSYMBOL_LT = 9,                         /* LT  */
  YYSYMBOL_EQ = 10,                        /* EQ  */
  YYSYMBOL_NEQ = 11,                       /* NEQ  */
  YYSYMBOL_INTTYPE = 12,                   /* INTTYPE  */
  YYSYMBOL_FLOATTYPE = 13,                 /* FLOATTYPE  */
  YYSYMBOL_VOID = 14,                      /* VOID  */
  YYSYMBOL_CONST = 15,                     /* CONST  */
  YYSYMBOL_RETURN = 16,                    /* RETURN  */
  YYSYMBOL_IF = 17,                        /* IF  */
  YYSYMBOL_ELSE = 18,                      /* ELSE  */
  YYSYMBOL_WHILE = 19,                     /* WHILE  */
  YYSYMBOL_BREAK = 20,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 21,                  /* CONTINUE  */
  YYSYMBOL_LP = 22,                        /* LP  */
  YYSYMBOL_RP = 23,                        /* RP  */
  YYSYMBOL_LB = 24,                        /* LB  */
  YYSYMBOL_RB = 25,                        /* RB  */
  YYSYMBOL_LC = 26,                        /* LC  */
  YYSYMBOL_RC = 27,                        /* RC  */
  YYSYMBOL_COMMA = 28,                     /* COMMA  */
  YYSYMBOL_SEMICOLON = 29,                 /* SEMICOLON  */
  YYSYMBOL_NOT = 30,                       /* NOT  */
  YYSYMBOL_ASSIGN = 31,                    /* ASSIGN  */
  YYSYMBOL_MINUS = 32,                     /* MINUS  */
  YYSYMBOL_ADD = 33,                       /* ADD  */
  YYSYMBOL_MUL = 34,                       /* MUL  */
  YYSYMBOL_DIV = 35,                       /* DIV  */
  YYSYMBOL_MOD = 36,                       /* MOD  */
  YYSYMBOL_AND = 37,                       /* AND  */
  YYSYMBOL_OR = 38,                        /* OR  */
  YYSYMBOL_LOWER_THEN_ELSE = 39,           /* LOWER_THEN_ELSE  */
  YYSYMBOL_YYACCEPT = 40,                  /* $accept  */
  YYSYMBOL_Program = 41,                   /* Program  */
  YYSYMBOL_CompUnit = 42,                  /* CompUnit  */
  YYSYMBOL_DeclDef = 43,                   /* DeclDef  */
  YYSYMBOL_Decl = 44,                      /* Decl  */
  YYSYMBOL_BType = 45,                     /* BType  */
  YYSYMBOL_DefList = 46,                   /* DefList  */
  YYSYMBOL_Def = 47,                       /* Def  */
  YYSYMBOL_ArrayIndexes = 48,              /* ArrayIndexes  */
  YYSYMBOL_InitVal = 49,                   /* InitVal  */
  YYSYMBOL_InitValList = 50,               /* InitValList  */
  YYSYMBOL_FuncDef = 51,                   /* FuncDef  */
  YYSYMBOL_FuncFParamList = 52,            /* FuncFParamList  */
  YYSYMBOL_FuncFParam = 53,                /* FuncFParam  */
  YYSYMBOL_Block = 54,                     /* Block  */
  YYSYMBOL_BlockItemList = 55,             /* BlockItemList  */
  YYSYMBOL_BlockItem = 56,                 /* BlockItem  */
  YYSYMBOL_Stmt = 57,                      /* Stmt  */
  YYSYMBOL_SelectStmt = 58,                /* SelectStmt  */
  YYSYMBOL_IterationStmt = 59,             /* IterationStmt  */
  YYSYMBOL_ReturnStmt = 60,                /* ReturnStmt  */
  YYSYMBOL_Exp = 61,                       /* Exp  */
  YYSYMBOL_Cond = 62,                      /* Cond  */
  YYSYMBOL_LVal = 63,                      /* LVal  */
  YYSYMBOL_PrimaryExp = 64,                /* PrimaryExp  */
  YYSYMBOL_Number = 65,                    /* Number  */
  YYSYMBOL_UnaryExp = 66,                  /* UnaryExp  */
  YYSYMBOL_Call = 67,                      /* Call  */
  YYSYMBOL_UnaryOp = 68,                   /* UnaryOp  */
  YYSYMBOL_Args = 69,                      /* Args  */
  YYSYMBOL_MulExp = 70,                    /* MulExp  */
  YYSYMBOL_AddExp = 71,                    /* AddExp  */
  YYSYMBOL_RelExp = 72,                    /* RelExp  */
  YYSYMBOL_EqExp = 73,                     /* EqExp  */
  YYSYMBOL_LAndExp = 74,                   /* LAndExp  */
  YYSYMBOL_LOrExp = 75                     /* LOrExp  */
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
typedef yytype_uint8 yy_state_t;

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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   230

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  90
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  160

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   294


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   108,   108,   114,   119,   125,   128,   135,   142,   152,
     155,   161,   165,   172,   180,   186,   193,   201,   205,   213,
     216,   219,   226,   230,   237,   241,   244,   248,   254,   259,
     267,   271,   275,   285,   288,   295,   298,   305,   308,   314,
     317,   320,   323,   326,   329,   332,   335,   338,   343,   346,
     351,   356,   359,   364,   369,   374,   378,   385,   388,   391,
     397,   400,   405,   408,   411,   417,   427,   434,   437,   440,
     445,   449,   455,   458,   461,   464,   469,   472,   475,   480,
     483,   486,   489,   492,   497,   500,   503,   508,   511,   516,
     519
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INT", "FLOAT", "ID",
  "GTE", "LTE", "GT", "LT", "EQ", "NEQ", "INTTYPE", "FLOATTYPE", "VOID",
  "CONST", "RETURN", "IF", "ELSE", "WHILE", "BREAK", "CONTINUE", "LP",
  "RP", "LB", "RB", "LC", "RC", "COMMA", "SEMICOLON", "NOT", "ASSIGN",
  "MINUS", "ADD", "MUL", "DIV", "MOD", "AND", "OR", "LOWER_THEN_ELSE",
  "$accept", "Program", "CompUnit", "DeclDef", "Decl", "BType", "DefList",
  "Def", "ArrayIndexes", "InitVal", "InitValList", "FuncDef",
  "FuncFParamList", "FuncFParam", "Block", "BlockItemList", "BlockItem",
  "Stmt", "SelectStmt", "IterationStmt", "ReturnStmt", "Exp", "Cond",
  "LVal", "PrimaryExp", "Number", "UnaryExp", "Call", "UnaryOp", "Args",
  "MulExp", "AddExp", "RelExp", "EqExp", "LAndExp", "LOrExp", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-124)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      72,  -124,  -124,    14,    55,    30,    72,  -124,  -124,    37,
    -124,    16,    75,  -124,  -124,     2,    65,  -124,    -1,    -6,
      88,     4,   197,   164,     5,    75,  -124,    63,    90,    35,
    -124,  -124,    63,    41,  -124,  -124,    33,   197,  -124,  -124,
    -124,    78,  -124,  -124,  -124,  -124,  -124,   197,    38,    45,
      49,  -124,  -124,   197,   164,  -124,    93,  -124,    83,    63,
      55,  -124,    63,   176,    87,    95,  -124,  -124,   197,   197,
     197,   197,   197,  -124,  -124,    73,    96,  -124,   188,   102,
     108,   106,   109,  -124,  -124,  -124,    75,  -124,   124,  -124,
    -124,  -124,  -124,  -124,   113,   121,   130,  -124,  -124,  -124,
    -124,  -124,    42,  -124,  -124,  -124,  -124,    38,    38,  -124,
     164,  -124,  -124,   133,   197,   197,  -124,  -124,  -124,  -124,
    -124,   197,   151,  -124,   197,  -124,  -124,   155,    45,   125,
     148,   147,   149,   162,   159,    87,  -124,   144,   197,   197,
     197,   197,   197,   197,   197,   197,   144,  -124,   171,    45,
      45,    45,    45,   125,   125,   148,   147,  -124,   144,  -124
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     9,    10,     0,     0,     0,     2,     4,     5,     0,
       6,     0,     0,     1,     3,    16,     0,    11,     0,    16,
       0,     0,     0,     0,    15,     0,     8,     0,     0,     0,
      28,     7,     0,     0,    60,    61,    55,     0,    69,    68,
      67,     0,    58,    62,    59,    72,    63,     0,    76,    53,
       0,    14,    19,     0,     0,    12,     0,    27,    30,     0,
       0,    25,     0,     0,    56,     0,    17,    64,     0,     0,
       0,     0,     0,    20,    23,     0,     0,    13,     0,     0,
       0,     0,     0,    33,    39,    37,     0,    44,     0,    35,
      38,    46,    47,    45,     0,    58,     0,    26,    29,    24,
      65,    70,     0,    57,    73,    74,    75,    78,    77,    21,
       0,    18,    52,     0,     0,     0,    43,    42,    34,    36,
      41,     0,    31,    66,     0,    22,    51,     0,    79,    84,
      87,    89,    54,     0,     0,    32,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    40,    48,    80,
      81,    82,    83,    85,    86,    88,    90,    50,     0,    49
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -124,  -124,  -124,   189,   -51,     3,   191,   179,   -32,   -44,
    -124,  -124,   184,   152,   -19,  -124,   119,  -123,  -124,  -124,
    -124,   -22,    98,   -54,  -124,  -124,    -8,  -124,  -124,  -124,
     100,   -94,    40,    67,    69,  -124
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     5,     6,     7,     8,    28,    16,    17,    24,    51,
      75,    10,    29,    30,    87,    88,    89,    90,    91,    92,
      93,    94,   127,    42,    43,    44,    45,    46,    47,   102,
      48,    49,   129,   130,   131,   132
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      41,    52,    95,     9,    64,    85,    74,    12,    57,     9,
      77,     1,     2,    61,   148,    65,     1,     2,    22,    11,
     128,   128,    27,   157,    21,    23,    22,    32,    52,    53,
      13,    76,    52,    23,    95,   159,    54,    85,    18,    67,
      97,   101,    15,    99,   149,   150,   151,   152,   128,   128,
     128,   128,    34,    35,    36,    63,   113,    22,    59,    86,
     104,   105,   106,    60,    62,   123,   125,     1,     2,    60,
     124,    37,    68,    69,    70,    50,    73,    71,    72,    38,
      19,    39,    40,    95,     1,     2,     3,     4,    52,    56,
     135,    86,    95,    25,    26,    58,    34,    35,    36,   134,
     109,   110,   136,    66,    95,     1,     2,    96,     4,    78,
      79,    53,    80,    81,    82,    37,    25,    31,   103,    56,
      83,   111,    84,    38,   114,    39,    40,    34,    35,    36,
     115,   138,   139,   140,   141,   116,     1,     2,   117,     4,
      78,    79,   120,    80,    81,    82,    37,    34,    35,    36,
      56,   118,   121,    84,    38,   122,    39,    40,   142,   143,
      78,    79,   126,    80,    81,    82,    37,    34,    35,    36,
      56,   107,   108,    84,    38,    22,    39,    40,   137,    34,
      35,    36,   153,   154,   144,   146,    37,   145,   147,   158,
      50,    34,    35,    36,    38,    14,    39,    40,    37,   100,
      34,    35,    36,    20,    55,    33,    38,   119,    39,    40,
      37,   155,    98,   133,   156,     0,     0,   112,    38,    37,
      39,    40,     0,     0,     0,     0,     0,    38,     0,    39,
      40
};

static const yytype_int16 yycheck[] =
{
      22,    23,    56,     0,    36,    56,    50,     4,    27,     6,
      54,    12,    13,    32,   137,    37,    12,    13,    24,     5,
     114,   115,    23,   146,    22,    31,    24,    23,    50,    24,
       0,    53,    54,    31,    88,   158,    31,    88,    22,    47,
      59,    63,     5,    62,   138,   139,   140,   141,   142,   143,
     144,   145,     3,     4,     5,    22,    78,    24,    23,    56,
      68,    69,    70,    28,    23,    23,   110,    12,    13,    28,
      28,    22,    34,    35,    36,    26,    27,    32,    33,    30,
       5,    32,    33,   137,    12,    13,    14,    15,   110,    26,
     122,    88,   146,    28,    29,     5,     3,     4,     5,   121,
      27,    28,   124,    25,   158,    12,    13,    24,    15,    16,
      17,    24,    19,    20,    21,    22,    28,    29,    23,    26,
      27,    25,    29,    30,    22,    32,    33,     3,     4,     5,
      22,     6,     7,     8,     9,    29,    12,    13,    29,    15,
      16,    17,    29,    19,    20,    21,    22,     3,     4,     5,
      26,    27,    31,    29,    30,    25,    32,    33,    10,    11,
      16,    17,    29,    19,    20,    21,    22,     3,     4,     5,
      26,    71,    72,    29,    30,    24,    32,    33,    23,     3,
       4,     5,   142,   143,    37,    23,    22,    38,    29,    18,
      26,     3,     4,     5,    30,     6,    32,    33,    22,    23,
       3,     4,     5,    12,    25,    21,    30,    88,    32,    33,
      22,   144,    60,   115,   145,    -1,    -1,    29,    30,    22,
      32,    33,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    12,    13,    14,    15,    41,    42,    43,    44,    45,
      51,     5,    45,     0,    43,     5,    46,    47,    22,     5,
      46,    22,    24,    31,    48,    28,    29,    23,    45,    52,
      53,    29,    23,    52,     3,     4,     5,    22,    30,    32,
      33,    61,    63,    64,    65,    66,    67,    68,    70,    71,
      26,    49,    61,    24,    31,    47,    26,    54,     5,    23,
      28,    54,    23,    22,    48,    61,    25,    66,    34,    35,
      36,    32,    33,    27,    49,    50,    61,    49,    16,    17,
      19,    20,    21,    27,    29,    44,    45,    54,    55,    56,
      57,    58,    59,    60,    61,    63,    24,    54,    53,    54,
      23,    61,    69,    23,    66,    66,    66,    70,    70,    27,
      28,    25,    29,    61,    22,    22,    29,    29,    27,    56,
      29,    31,    25,    23,    28,    49,    29,    62,    71,    72,
      73,    74,    75,    62,    61,    48,    61,    23,     6,     7,
       8,     9,    10,    11,    37,    38,    23,    29,    57,    71,
      71,    71,    71,    72,    72,    73,    74,    57,    18,    57
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    40,    41,    42,    42,    43,    43,    44,    44,    45,
      45,    46,    46,    47,    47,    47,    47,    48,    48,    49,
      49,    49,    50,    50,    51,    51,    51,    51,    52,    52,
      53,    53,    53,    54,    54,    55,    55,    56,    56,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    58,    58,
      59,    60,    60,    61,    62,    63,    63,    64,    64,    64,
      65,    65,    66,    66,    66,    67,    67,    68,    68,    68,
      69,    69,    70,    70,    70,    70,    71,    71,    71,    72,
      72,    72,    72,    72,    73,    73,    73,    74,    74,    75,
      75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     4,     3,     1,
       1,     1,     3,     4,     3,     2,     1,     3,     4,     1,
       2,     3,     3,     1,     6,     5,     6,     5,     1,     3,
       2,     4,     5,     2,     3,     1,     2,     1,     1,     1,
       4,     2,     2,     2,     1,     1,     1,     1,     5,     7,
       5,     3,     2,     1,     1,     1,     2,     3,     1,     1,
       1,     1,     1,     1,     2,     3,     4,     1,     1,     1,
       1,     3,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     1,
       3
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
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
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
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
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

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

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

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
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
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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
      yychar = yylex ();
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Program: CompUnit  */
#line 108 "parser.y"
           {
    root = *(yyvsp[0].args);
  }
#line 1277 "y.tab.cpp"
    break;

  case 3: /* CompUnit: CompUnit DeclDef  */
#line 114 "parser.y"
                   {
    (yyval.args) = (yyvsp[-1].args);
    (yyval.args)->insert((yyval.args)->end(), (yyvsp[0].args)->begin(), (yyvsp[0].args)->end());
    delete (yyvsp[0].args);
  }
#line 1287 "y.tab.cpp"
    break;

  case 4: /* CompUnit: DeclDef  */
#line 119 "parser.y"
          {
    (yyval.args) = (yyvsp[0].args);
  }
#line 1295 "y.tab.cpp"
    break;

  case 5: /* DeclDef: Decl  */
#line 125 "parser.y"
       {
    (yyval.args) = (yyvsp[0].args);
  }
#line 1303 "y.tab.cpp"
    break;

  case 6: /* DeclDef: FuncDef  */
#line 128 "parser.y"
          {
    (yyval.args) = new Vector<pNode>();
    (yyval.args)->push_back(pNode((yyvsp[0].funcDef)));
  }
#line 1312 "y.tab.cpp"
    break;

  case 7: /* Decl: CONST BType DefList SEMICOLON  */
#line 135 "parser.y"
                                {
    (yyval.args) = new Vector<pNode>();
    for (auto&& def : *(yyvsp[-1].defList)) {
      (yyval.args)->push_back(def->create(true, (yyvsp[-2].ty)));
    }
    delete (yyvsp[-1].defList);
  }
#line 1324 "y.tab.cpp"
    break;

  case 8: /* Decl: BType DefList SEMICOLON  */
#line 142 "parser.y"
                          {
    (yyval.args) = new Vector<pNode>();
    for (auto&& def : *(yyvsp[-1].defList)) {
      (yyval.args)->push_back(def->create(false, (yyvsp[-2].ty)));
    }
    delete (yyvsp[-1].defList);
  }
#line 1336 "y.tab.cpp"
    break;

  case 9: /* BType: INTTYPE  */
#line 152 "parser.y"
          {
    (yyval.ty) = TYPE_INT;
  }
#line 1344 "y.tab.cpp"
    break;

  case 10: /* BType: FLOATTYPE  */
#line 155 "parser.y"
            {
    (yyval.ty) = TYPE_FLOAT;
  }
#line 1352 "y.tab.cpp"
    break;

  case 11: /* DefList: Def  */
#line 161 "parser.y"
      {
    (yyval.defList) = new Vector<DefAST*>();
    (yyval.defList)->push_back((yyvsp[0].def));
  }
#line 1361 "y.tab.cpp"
    break;

  case 12: /* DefList: DefList COMMA Def  */
#line 165 "parser.y"
                    {
    (yyval.defList) = (yyvsp[-2].defList);
    (yyval.defList)->push_back((yyvsp[0].def));
  }
#line 1370 "y.tab.cpp"
    break;

  case 13: /* Def: ID ArrayIndexes ASSIGN InitVal  */
#line 172 "parser.y"
                                 { // int a[10] = {{0, 0}, {1, 1}}
    (yyval.def) = new DefAST();
    (yyval.def)->id = *(yyvsp[-3].token);
    delete (yyvsp[-3].token); 
    (yyval.def)->indexes = *(yyvsp[-2].args);
    delete (yyvsp[-2].args);
    (yyval.def)->initVal = pNode((yyvsp[0].initVal));
  }
#line 1383 "y.tab.cpp"
    break;

  case 14: /* Def: ID ASSIGN InitVal  */
#line 180 "parser.y"
                    {
    (yyval.def) = new DefAST();
    (yyval.def)->id = *(yyvsp[-2].token);
    delete (yyvsp[-2].token); 
    (yyval.def)->initVal = pNode((yyvsp[0].initVal));
  }
#line 1394 "y.tab.cpp"
    break;

  case 15: /* Def: ID ArrayIndexes  */
#line 186 "parser.y"
                  {
    (yyval.def) = new DefAST();
    (yyval.def)->id = *(yyvsp[-1].token);
    delete (yyvsp[-1].token); 
    (yyval.def)->indexes = *(yyvsp[0].args);
    delete (yyvsp[0].args);
  }
#line 1406 "y.tab.cpp"
    break;

  case 16: /* Def: ID  */
#line 193 "parser.y"
     {
    (yyval.def) = new DefAST();
    (yyval.def)->id = *(yyvsp[0].token);
    delete (yyvsp[0].token); 
  }
#line 1416 "y.tab.cpp"
    break;

  case 17: /* ArrayIndexes: LB Exp RB  */
#line 201 "parser.y"
            {
    (yyval.args) = new Vector<pNode>();
    (yyval.args)->push_back(pNode((yyvsp[-1].exp)));
  }
#line 1425 "y.tab.cpp"
    break;

  case 18: /* ArrayIndexes: ArrayIndexes LB Exp RB  */
#line 205 "parser.y"
                         {
    (yyval.args) = (yyvsp[-3].args);
    (yyval.args)->push_back(pNode((yyvsp[-1].exp)));
  }
#line 1434 "y.tab.cpp"
    break;

  case 19: /* InitVal: Exp  */
#line 213 "parser.y"
      {
    (yyval.initVal) = (yyvsp[0].exp);
  }
#line 1442 "y.tab.cpp"
    break;

  case 20: /* InitVal: LC RC  */
#line 216 "parser.y"
        { // { }
    (yyval.initVal) = new ArrayDefNode(NULL, {});
  }
#line 1450 "y.tab.cpp"
    break;

  case 21: /* InitVal: LC InitValList RC  */
#line 219 "parser.y"
                    { // {1, 2, 3}
    (yyval.initVal) = new ArrayDefNode(NULL, *(yyvsp[-1].args));
    delete (yyvsp[-1].args);
  }
#line 1459 "y.tab.cpp"
    break;

  case 22: /* InitValList: InitValList COMMA InitVal  */
#line 226 "parser.y"
                            {
    (yyval.args) = (yyvsp[-2].args);
    (yyval.args)->push_back(pNode((yyvsp[0].initVal)));
  }
#line 1468 "y.tab.cpp"
    break;

  case 23: /* InitValList: InitVal  */
#line 230 "parser.y"
          {
    (yyval.args) = new Vector<pNode>();
    (yyval.args)->push_back(pNode((yyvsp[0].initVal)));
  }
#line 1477 "y.tab.cpp"
    break;

  case 24: /* FuncDef: BType ID LP FuncFParamList RP Block  */
#line 237 "parser.y"
                                      {
    (yyval.funcDef) = new FuncDefNode(NULL, toTypedSym((yyvsp[-4].token), (yyvsp[-5].ty)), *(yyvsp[-2].FuncFParamList), pNode((yyvsp[0].block)));
    delete (yyvsp[-2].FuncFParamList);
  }
#line 1486 "y.tab.cpp"
    break;

  case 25: /* FuncDef: BType ID LP RP Block  */
#line 241 "parser.y"
                       {
    (yyval.funcDef) = new FuncDefNode(NULL, toTypedSym((yyvsp[-3].token), (yyvsp[-4].ty)), {}, pNode((yyvsp[0].block)));
  }
#line 1494 "y.tab.cpp"
    break;

  case 26: /* FuncDef: VOID ID LP FuncFParamList RP Block  */
#line 244 "parser.y"
                                     {
    (yyval.funcDef) = new FuncDefNode(NULL, toTypedSym((yyvsp[-4].token), TYPE_VOID), *(yyvsp[-2].FuncFParamList), pNode((yyvsp[0].block)));
    delete (yyvsp[-2].FuncFParamList);
  }
#line 1503 "y.tab.cpp"
    break;

  case 27: /* FuncDef: VOID ID LP RP Block  */
#line 248 "parser.y"
                      {
    (yyval.funcDef) = new FuncDefNode(NULL, toTypedSym((yyvsp[-3].token), TYPE_VOID), {}, pNode((yyvsp[0].block)));
  }
#line 1511 "y.tab.cpp"
    break;

  case 28: /* FuncFParamList: FuncFParam  */
#line 254 "parser.y"
             {
    (yyval.FuncFParamList) = new Vector<TypedNodeSym>();
    (yyval.FuncFParamList)->push_back(*(yyvsp[0].funcFParam));
    delete (yyvsp[0].funcFParam);
  }
#line 1521 "y.tab.cpp"
    break;

  case 29: /* FuncFParamList: FuncFParamList COMMA FuncFParam  */
#line 259 "parser.y"
                                  {
    (yyval.FuncFParamList) = (yyvsp[-2].FuncFParamList);
    (yyval.FuncFParamList)->push_back(*(yyvsp[0].funcFParam));
    delete (yyvsp[0].funcFParam);
  }
#line 1531 "y.tab.cpp"
    break;

  case 30: /* FuncFParam: BType ID  */
#line 267 "parser.y"
           {
    (yyval.funcFParam) = new TypedNodeSym(*(yyvsp[0].token), new_basic_type_node(NULL, toPTYPE((yyvsp[-1].ty))));
    delete (yyvsp[0].token);
  }
#line 1540 "y.tab.cpp"
    break;

  case 31: /* FuncFParam: BType ID LB RB  */
#line 271 "parser.y"
                 {
    (yyval.funcFParam) = new TypedNodeSym(*(yyvsp[-2].token), new_pointer_type_node(NULL, new_basic_type_node(NULL, toPTYPE((yyvsp[-3].ty)))));
    delete (yyvsp[-2].token);
  }
#line 1549 "y.tab.cpp"
    break;

  case 32: /* FuncFParam: BType ID LB RB ArrayIndexes  */
#line 275 "parser.y"
                              {
    auto innerTy = Ast::new_basic_type_node(NULL, toPTYPE((yyvsp[-4].ty)));
    for(auto i = std::rbegin(*(yyvsp[0].args)); i!=std::rend(*(yyvsp[0].args)); ++i) { 
      innerTy = Ast::new_array_type_node(NULL, innerTy, *i);
    }
    (yyval.funcFParam) = new TypedNodeSym(*(yyvsp[-3].token), new_pointer_type_node(NULL, innerTy));
  }
#line 1561 "y.tab.cpp"
    break;

  case 33: /* Block: LC RC  */
#line 285 "parser.y"
        {
    (yyval.block) = new BlockNode(NULL, {});
  }
#line 1569 "y.tab.cpp"
    break;

  case 34: /* Block: LC BlockItemList RC  */
#line 288 "parser.y"
                      {
    (yyval.block) = new BlockNode(NULL, AstProg((yyvsp[-1].args)->begin(), (yyvsp[-1].args)->end()));
    delete (yyvsp[-1].args);
  }
#line 1578 "y.tab.cpp"
    break;

  case 35: /* BlockItemList: BlockItem  */
#line 295 "parser.y"
            {
    (yyval.args) = (yyvsp[0].args);
  }
#line 1586 "y.tab.cpp"
    break;

  case 36: /* BlockItemList: BlockItemList BlockItem  */
#line 298 "parser.y"
                          {
    (yyval.args) = (yyvsp[-1].args);
    (yyval.args)->insert((yyval.args)->end(), (yyvsp[0].args)->begin(), (yyvsp[0].args)->end());
    delete (yyvsp[0].args);
  }
#line 1596 "y.tab.cpp"
    break;

  case 37: /* BlockItem: Decl  */
#line 305 "parser.y"
       {
    (yyval.args) = (yyvsp[0].args);
  }
#line 1604 "y.tab.cpp"
    break;

  case 38: /* BlockItem: Stmt  */
#line 308 "parser.y"
       {
    (yyval.args) = new Vector<pNode>();
    (yyval.args)->push_back(pNode((yyvsp[0].stmt)));
  }
#line 1613 "y.tab.cpp"
    break;

  case 39: /* Stmt: SEMICOLON  */
#line 314 "parser.y"
            {
    (yyval.stmt) = new BlockNode(NULL, {});
  }
#line 1621 "y.tab.cpp"
    break;

  case 40: /* Stmt: LVal ASSIGN Exp SEMICOLON  */
#line 317 "parser.y"
                            {
    (yyval.stmt) = new AssignNode(NULL, pNode((yyvsp[-3].lVal)), pNode((yyvsp[-1].exp)));
  }
#line 1629 "y.tab.cpp"
    break;

  case 41: /* Stmt: Exp SEMICOLON  */
#line 320 "parser.y"
                {
    (yyval.stmt) = (yyvsp[-1].exp);
  }
#line 1637 "y.tab.cpp"
    break;

  case 42: /* Stmt: CONTINUE SEMICOLON  */
#line 323 "parser.y"
                     {
    (yyval.stmt) = new ContinueNode(NULL);
  }
#line 1645 "y.tab.cpp"
    break;

  case 43: /* Stmt: BREAK SEMICOLON  */
#line 326 "parser.y"
                  {
    (yyval.stmt) = new BreakNode(NULL);
  }
#line 1653 "y.tab.cpp"
    break;

  case 44: /* Stmt: Block  */
#line 329 "parser.y"
        {
    (yyval.stmt) = (yyvsp[0].block);
  }
#line 1661 "y.tab.cpp"
    break;

  case 45: /* Stmt: ReturnStmt  */
#line 332 "parser.y"
             {
    (yyval.stmt) = (yyvsp[0].returnStmt);
  }
#line 1669 "y.tab.cpp"
    break;

  case 46: /* Stmt: SelectStmt  */
#line 335 "parser.y"
             {
    (yyval.stmt) = (yyvsp[0].selectStmt);
  }
#line 1677 "y.tab.cpp"
    break;

  case 47: /* Stmt: IterationStmt  */
#line 338 "parser.y"
                {
    (yyval.stmt) = (yyvsp[0].iterationStmt);
  }
#line 1685 "y.tab.cpp"
    break;

  case 48: /* SelectStmt: IF LP Cond RP Stmt  */
#line 343 "parser.y"
                                           {
    (yyval.selectStmt) = new IfNode(NULL, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].stmt)));
  }
#line 1693 "y.tab.cpp"
    break;

  case 49: /* SelectStmt: IF LP Cond RP Stmt ELSE Stmt  */
#line 346 "parser.y"
                               {
    (yyval.selectStmt) = new IfNode(NULL, pNode((yyvsp[-4].exp)), pNode((yyvsp[-2].stmt)), pNode((yyvsp[0].stmt)));
  }
#line 1701 "y.tab.cpp"
    break;

  case 50: /* IterationStmt: WHILE LP Cond RP Stmt  */
#line 351 "parser.y"
                        {
    (yyval.iterationStmt) = new WhileNode(NULL, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].stmt)));
  }
#line 1709 "y.tab.cpp"
    break;

  case 51: /* ReturnStmt: RETURN Exp SEMICOLON  */
#line 356 "parser.y"
                       {
    (yyval.returnStmt) = new ReturnNode(NULL, pNode((yyvsp[-1].exp)));
  }
#line 1717 "y.tab.cpp"
    break;

  case 52: /* ReturnStmt: RETURN SEMICOLON  */
#line 359 "parser.y"
                   {
    (yyval.returnStmt) = new ReturnNode(NULL);
  }
#line 1725 "y.tab.cpp"
    break;

  case 53: /* Exp: AddExp  */
#line 364 "parser.y"
         {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1733 "y.tab.cpp"
    break;

  case 54: /* Cond: LOrExp  */
#line 369 "parser.y"
         {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1741 "y.tab.cpp"
    break;

  case 55: /* LVal: ID  */
#line 374 "parser.y"
     {
    (yyval.lVal) = new SymNode(NULL, *(yyvsp[0].token));
    delete (yyvsp[0].token);
  }
#line 1750 "y.tab.cpp"
    break;

  case 56: /* LVal: ID ArrayIndexes  */
#line 378 "parser.y"
                  {
    (yyval.lVal) = new ItemNode(NULL, new_sym_node(NULL, *(yyvsp[-1].token)), *(yyvsp[0].args));
    delete (yyvsp[-1].token);
    delete (yyvsp[0].args);
  }
#line 1760 "y.tab.cpp"
    break;

  case 57: /* PrimaryExp: LP Exp RP  */
#line 385 "parser.y"
            {
    (yyval.exp) = (yyvsp[-1].exp);
  }
#line 1768 "y.tab.cpp"
    break;

  case 58: /* PrimaryExp: LVal  */
#line 388 "parser.y"
       {
    (yyval.exp) = (yyvsp[0].lVal);
  }
#line 1776 "y.tab.cpp"
    break;

  case 59: /* PrimaryExp: Number  */
#line 391 "parser.y"
         {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1784 "y.tab.cpp"
    break;

  case 60: /* Number: INT  */
#line 397 "parser.y"
      {
    (yyval.exp) = new ImmNode(NULL, ImmValue((yyvsp[0].int_val)));
  }
#line 1792 "y.tab.cpp"
    break;

  case 61: /* Number: FLOAT  */
#line 400 "parser.y"
        {
    (yyval.exp) = new ImmNode(NULL, ImmValue((yyvsp[0].float_val)));
  }
#line 1800 "y.tab.cpp"
    break;

  case 62: /* UnaryExp: PrimaryExp  */
#line 405 "parser.y"
             {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1808 "y.tab.cpp"
    break;

  case 63: /* UnaryExp: Call  */
#line 408 "parser.y"
       {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1816 "y.tab.cpp"
    break;

  case 64: /* UnaryExp: UnaryOp UnaryExp  */
#line 411 "parser.y"
                   {
    (yyval.exp) = new UnaryNode(NULL, (yyvsp[-1].op), pNode((yyvsp[0].exp)));
  }
#line 1824 "y.tab.cpp"
    break;

  case 65: /* Call: ID LP RP  */
#line 417 "parser.y"
           {
    if (*(yyvsp[-2].token) == "starttime") {
    	(yyval.exp) = new CallNode(NULL, "_sysy_starttime", {pNode(new ImmNode(NULL, ImmValue(yylineno)))});
    } else if (*(yyvsp[-2].token) == "stoptime") {
    	(yyval.exp) = new CallNode(NULL, "_sysy_stoptime", {pNode(new ImmNode(NULL, ImmValue(yylineno)))});
    } else {
    	(yyval.exp) = new CallNode(NULL, *(yyvsp[-2].token), {});
    }
    delete (yyvsp[-2].token);
  }
#line 1839 "y.tab.cpp"
    break;

  case 66: /* Call: ID LP Args RP  */
#line 427 "parser.y"
                {
    (yyval.exp) = new CallNode(NULL, *(yyvsp[-3].token), *(yyvsp[-1].args));
    delete (yyvsp[-3].token);
    delete (yyvsp[-1].args);
  }
#line 1849 "y.tab.cpp"
    break;

  case 67: /* UnaryOp: ADD  */
#line 434 "parser.y"
      {
    (yyval.op) = OPR_POS;
  }
#line 1857 "y.tab.cpp"
    break;

  case 68: /* UnaryOp: MINUS  */
#line 437 "parser.y"
        {
    (yyval.op) = OPR_NEG;
  }
#line 1865 "y.tab.cpp"
    break;

  case 69: /* UnaryOp: NOT  */
#line 440 "parser.y"
      {
    (yyval.op) = OPR_NOT;
  }
#line 1873 "y.tab.cpp"
    break;

  case 70: /* Args: Exp  */
#line 445 "parser.y"
      {
    (yyval.args) = new Vector<pNode>();
    (yyval.args)->push_back(pNode((yyvsp[0].exp)));
  }
#line 1882 "y.tab.cpp"
    break;

  case 71: /* Args: Args COMMA Exp  */
#line 449 "parser.y"
                 {
    (yyval.args) = (yyvsp[-2].args);
    (yyval.args)->push_back(pNode((yyvsp[0].exp)));
  }
#line 1891 "y.tab.cpp"
    break;

  case 72: /* MulExp: UnaryExp  */
#line 455 "parser.y"
           {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1899 "y.tab.cpp"
    break;

  case 73: /* MulExp: MulExp MUL UnaryExp  */
#line 458 "parser.y"
                      {
    (yyval.exp) = new BinaryNode(NULL, OPR_MUL, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1907 "y.tab.cpp"
    break;

  case 74: /* MulExp: MulExp DIV UnaryExp  */
#line 461 "parser.y"
                      {
    (yyval.exp) = new BinaryNode(NULL, OPR_DIV, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1915 "y.tab.cpp"
    break;

  case 75: /* MulExp: MulExp MOD UnaryExp  */
#line 464 "parser.y"
                      {
    (yyval.exp) = new BinaryNode(NULL, OPR_REM, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1923 "y.tab.cpp"
    break;

  case 76: /* AddExp: MulExp  */
#line 469 "parser.y"
         {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1931 "y.tab.cpp"
    break;

  case 77: /* AddExp: AddExp ADD MulExp  */
#line 472 "parser.y"
                    {
    (yyval.exp) = new BinaryNode(NULL, OPR_ADD, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1939 "y.tab.cpp"
    break;

  case 78: /* AddExp: AddExp MINUS MulExp  */
#line 475 "parser.y"
                      {
    (yyval.exp) = new BinaryNode(NULL, OPR_SUB, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1947 "y.tab.cpp"
    break;

  case 79: /* RelExp: AddExp  */
#line 480 "parser.y"
         {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1955 "y.tab.cpp"
    break;

  case 80: /* RelExp: RelExp GTE AddExp  */
#line 483 "parser.y"
                    {
    (yyval.exp) = new BinaryNode(NULL, OPR_GE, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1963 "y.tab.cpp"
    break;

  case 81: /* RelExp: RelExp LTE AddExp  */
#line 486 "parser.y"
                    {
    (yyval.exp) = new BinaryNode(NULL, OPR_LE, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1971 "y.tab.cpp"
    break;

  case 82: /* RelExp: RelExp GT AddExp  */
#line 489 "parser.y"
                   {
    (yyval.exp) = new BinaryNode(NULL, OPR_GT, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1979 "y.tab.cpp"
    break;

  case 83: /* RelExp: RelExp LT AddExp  */
#line 492 "parser.y"
                   {
    (yyval.exp) = new BinaryNode(NULL, OPR_LT, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 1987 "y.tab.cpp"
    break;

  case 84: /* EqExp: RelExp  */
#line 497 "parser.y"
         {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 1995 "y.tab.cpp"
    break;

  case 85: /* EqExp: EqExp EQ RelExp  */
#line 500 "parser.y"
                  {
    (yyval.exp) = new BinaryNode(NULL, OPR_EQ, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 2003 "y.tab.cpp"
    break;

  case 86: /* EqExp: EqExp NEQ RelExp  */
#line 503 "parser.y"
                   {
    (yyval.exp) = new BinaryNode(NULL, OPR_NE, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 2011 "y.tab.cpp"
    break;

  case 87: /* LAndExp: EqExp  */
#line 508 "parser.y"
        {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 2019 "y.tab.cpp"
    break;

  case 88: /* LAndExp: LAndExp AND EqExp  */
#line 511 "parser.y"
                    {
    (yyval.exp) = new BinaryNode(NULL, OPR_AND, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 2027 "y.tab.cpp"
    break;

  case 89: /* LOrExp: LAndExp  */
#line 516 "parser.y"
          {
    (yyval.exp) = (yyvsp[0].exp);
  }
#line 2035 "y.tab.cpp"
    break;

  case 90: /* LOrExp: LOrExp OR LAndExp  */
#line 519 "parser.y"
                    {
    (yyval.exp) = new BinaryNode(NULL, OPR_OR, pNode((yyvsp[-2].exp)), pNode((yyvsp[0].exp)));
  }
#line 2043 "y.tab.cpp"
    break;


#line 2047 "y.tab.cpp"

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
      yyerror (YY_("syntax error"));
    }

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
                      yytoken, &yylval);
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


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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
  yyerror (YY_("memory exhausted"));
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
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 522 "parser.y"


void setFileName(const char *name) {
  strcpy(filename, name);
  freopen(filename, "r", stdin);
}

void yyerror(const char* fmt) {
  printf("%s:%d ", filename, yylineno);
  printf("%s\n", fmt);
}
