/* A Bison parser, made by GNU Bison 3.8.2.  */

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

#ifndef YY_EX_HOME_USER_CLIONPROJECTS_REGEXTHESIS_EXTERNAL_BUILD_DEPS_GRAPHVIZ_BUILD_WASM_LIB_EXPR_EXPARSE_H_INCLUDED
# define YY_EX_HOME_USER_CLIONPROJECTS_REGEXTHESIS_EXTERNAL_BUILD_DEPS_GRAPHVIZ_BUILD_WASM_LIB_EXPR_EXPARSE_H_INCLUDED
/* Debug traces.  */
#ifndef EX_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define EX_DEBUG 1
#  else
#   define EX_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define EX_DEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined EX_DEBUG */
#if EX_DEBUG
extern int ex_debug;
#endif

/* Token kinds.  */
#ifndef EX_TOKENTYPE
# define EX_TOKENTYPE
  enum ex_tokentype
  {
    EX_EMPTY = -2,
    EX_EOF = 0,                    /* "end of file"  */
    EX_error = 256,                /* error  */
    EX_UNDEF = 257,                /* "invalid token"  */
    MINTOKEN = 258,                /* MINTOKEN  */
    INTEGER = 259,                 /* INTEGER  */
    UNSIGNED = 260,                /* UNSIGNED  */
    CHARACTER = 261,               /* CHARACTER  */
    FLOATING = 262,                /* FLOATING  */
    STRING = 263,                  /* STRING  */
    VOIDTYPE = 264,                /* VOIDTYPE  */
    ADDRESS = 265,                 /* ADDRESS  */
    ARRAY = 266,                   /* ARRAY  */
    BREAK = 267,                   /* BREAK  */
    CALL = 268,                    /* CALL  */
    CASE = 269,                    /* CASE  */
    CONSTANT = 270,                /* CONSTANT  */
    CONTINUE = 271,                /* CONTINUE  */
    DECLARE = 272,                 /* DECLARE  */
    DEFAULT = 273,                 /* DEFAULT  */
    DYNAMIC = 274,                 /* DYNAMIC  */
    ELSE = 275,                    /* ELSE  */
    EXIT = 276,                    /* EXIT  */
    FOR = 277,                     /* FOR  */
    FUNCTION = 278,                /* FUNCTION  */
    GSUB = 279,                    /* GSUB  */
    ITERATE = 280,                 /* ITERATE  */
    ITERATOR = 281,                /* ITERATOR  */
    ID = 282,                      /* ID  */
    IF = 283,                      /* IF  */
    LABEL = 284,                   /* LABEL  */
    MEMBER = 285,                  /* MEMBER  */
    NAME = 286,                    /* NAME  */
    POS = 287,                     /* POS  */
    PRAGMA = 288,                  /* PRAGMA  */
    PRE = 289,                     /* PRE  */
    PRINT = 290,                   /* PRINT  */
    PRINTF = 291,                  /* PRINTF  */
    PROCEDURE = 292,               /* PROCEDURE  */
    QUERY = 293,                   /* QUERY  */
    RAND = 294,                    /* RAND  */
    RETURN = 295,                  /* RETURN  */
    SCANF = 296,                   /* SCANF  */
    SPLIT = 297,                   /* SPLIT  */
    SPRINTF = 298,                 /* SPRINTF  */
    SRAND = 299,                   /* SRAND  */
    SSCANF = 300,                  /* SSCANF  */
    SUB = 301,                     /* SUB  */
    SUBSTR = 302,                  /* SUBSTR  */
    SWITCH = 303,                  /* SWITCH  */
    TOKENS = 304,                  /* TOKENS  */
    UNSET = 305,                   /* UNSET  */
    WHILE = 306,                   /* WHILE  */
    F2I = 307,                     /* F2I  */
    F2S = 308,                     /* F2S  */
    I2F = 309,                     /* I2F  */
    I2S = 310,                     /* I2S  */
    S2B = 311,                     /* S2B  */
    S2F = 312,                     /* S2F  */
    S2I = 313,                     /* S2I  */
    F2X = 314,                     /* F2X  */
    I2X = 315,                     /* I2X  */
    S2X = 316,                     /* S2X  */
    X2F = 317,                     /* X2F  */
    X2I = 318,                     /* X2I  */
    X2S = 319,                     /* X2S  */
    X2X = 320,                     /* X2X  */
    XPRINT = 321,                  /* XPRINT  */
    OR = 322,                      /* OR  */
    AND = 323,                     /* AND  */
    EQ = 324,                      /* EQ  */
    NE = 325,                      /* NE  */
    LE = 326,                      /* LE  */
    GE = 327,                      /* GE  */
    LSH = 328,                     /* LSH  */
    RSH = 329,                     /* RSH  */
    IN_OP = 330,                   /* IN_OP  */
    UNARY = 331,                   /* UNARY  */
    INC = 332,                     /* INC  */
    DEC = 333,                     /* DEC  */
    CAST = 334,                    /* CAST  */
    MAXTOKEN = 335                 /* MAXTOKEN  */
  };
  typedef enum ex_tokentype ex_token_kind_t;
#endif
/* Token kinds.  */
#define EX_EMPTY -2
#define EX_EOF 0
#define EX_error 256
#define EX_UNDEF 257
#define MINTOKEN 258
#define INTEGER 259
#define UNSIGNED 260
#define CHARACTER 261
#define FLOATING 262
#define STRING 263
#define VOIDTYPE 264
#define ADDRESS 265
#define ARRAY 266
#define BREAK 267
#define CALL 268
#define CASE 269
#define CONSTANT 270
#define CONTINUE 271
#define DECLARE 272
#define DEFAULT 273
#define DYNAMIC 274
#define ELSE 275
#define EXIT 276
#define FOR 277
#define FUNCTION 278
#define GSUB 279
#define ITERATE 280
#define ITERATOR 281
#define ID 282
#define IF 283
#define LABEL 284
#define MEMBER 285
#define NAME 286
#define POS 287
#define PRAGMA 288
#define PRE 289
#define PRINT 290
#define PRINTF 291
#define PROCEDURE 292
#define QUERY 293
#define RAND 294
#define RETURN 295
#define SCANF 296
#define SPLIT 297
#define SPRINTF 298
#define SRAND 299
#define SSCANF 300
#define SUB 301
#define SUBSTR 302
#define SWITCH 303
#define TOKENS 304
#define UNSET 305
#define WHILE 306
#define F2I 307
#define F2S 308
#define I2F 309
#define I2S 310
#define S2B 311
#define S2F 312
#define S2I 313
#define F2X 314
#define I2X 315
#define S2X 316
#define X2F 317
#define X2I 318
#define X2S 319
#define X2X 320
#define XPRINT 321
#define OR 322
#define AND 323
#define EQ 324
#define NE 325
#define LE 326
#define GE 327
#define LSH 328
#define RSH 329
#define IN_OP 330
#define UNARY 331
#define INC 332
#define DEC 333
#define CAST 334
#define MAXTOKEN 335

/* Value type.  */
#if ! defined EX_STYPE && ! defined EX_STYPE_IS_DECLARED
union EX_STYPE
{
#line 43 "/home/user/CLionProjects/RegexThesis/external/build-deps/graphviz/lib/expr/exparse.y"

	struct Exnode_s*expr;
	double		floating;
	struct Exref_s*	reference;
	struct Exid_s*	id;
	long long integer;
	int		op;
	char*		string;

#line 245 "/home/user/CLionProjects/RegexThesis/external/build-deps/graphviz/build-wasm/lib/expr/exparse.h"

};
typedef union EX_STYPE EX_STYPE;
# define EX_STYPE_IS_TRIVIAL 1
# define EX_STYPE_IS_DECLARED 1
#endif


extern EX_STYPE ex_lval;


int ex_parse (void);


#endif /* !YY_EX_HOME_USER_CLIONPROJECTS_REGEXTHESIS_EXTERNAL_BUILD_DEPS_GRAPHVIZ_BUILD_WASM_LIB_EXPR_EXPARSE_H_INCLUDED  */
