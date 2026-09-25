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

#ifndef YY_GML_HOME_USER_CLIONPROJECTS_REGEXTHESIS_EXTERNAL_BUILD_DEPS_GRAPHVIZ_BUILD_WASM_CMD_TOOLS_GMLPARSE_H_INCLUDED
# define YY_GML_HOME_USER_CLIONPROJECTS_REGEXTHESIS_EXTERNAL_BUILD_DEPS_GRAPHVIZ_BUILD_WASM_CMD_TOOLS_GMLPARSE_H_INCLUDED
/* Debug traces.  */
#ifndef GMLDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define GMLDEBUG 1
#  else
#   define GMLDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define GMLDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined GMLDEBUG */
#if GMLDEBUG
extern int gmldebug;
#endif

/* Token kinds.  */
#ifndef GMLTOKENTYPE
# define GMLTOKENTYPE
  enum gmltokentype
  {
    GMLEMPTY = -2,
    GMLEOF = 0,                    /* "end of file"  */
    GMLerror = 256,                /* error  */
    GMLUNDEF = 257,                /* "invalid token"  */
    GRAPH = 258,                   /* GRAPH  */
    NODE = 259,                    /* NODE  */
    EDGE = 260,                    /* EDGE  */
    DIRECTED = 261,                /* DIRECTED  */
    SOURCE = 262,                  /* SOURCE  */
    TARGET = 263,                  /* TARGET  */
    XVAL = 264,                    /* XVAL  */
    YVAL = 265,                    /* YVAL  */
    WVAL = 266,                    /* WVAL  */
    HVAL = 267,                    /* HVAL  */
    LABEL = 268,                   /* LABEL  */
    GRAPHICS = 269,                /* GRAPHICS  */
    LABELGRAPHICS = 270,           /* LABELGRAPHICS  */
    TYPE = 271,                    /* TYPE  */
    FILL = 272,                    /* FILL  */
    OUTLINE = 273,                 /* OUTLINE  */
    OUTLINESTYLE = 274,            /* OUTLINESTYLE  */
    OUTLINEWIDTH = 275,            /* OUTLINEWIDTH  */
    WIDTH = 276,                   /* WIDTH  */
    STYLE = 277,                   /* STYLE  */
    LINE = 278,                    /* LINE  */
    POINT = 279,                   /* POINT  */
    TEXT = 280,                    /* TEXT  */
    FONTSIZE = 281,                /* FONTSIZE  */
    FONTNAME = 282,                /* FONTNAME  */
    COLOR = 283,                   /* COLOR  */
    INTEGER = 284,                 /* INTEGER  */
    REAL = 285,                    /* REAL  */
    STRING = 286,                  /* STRING  */
    ID = 287,                      /* ID  */
    NAME = 288,                    /* NAME  */
    GML_LIST = 289                 /* GML_LIST  */
  };
  typedef enum gmltokentype gmltoken_kind_t;
#endif

/* Value type.  */
#if ! defined GMLSTYPE && ! defined GMLSTYPE_IS_DECLARED
union GMLSTYPE
{
#line 203 "/home/user/CLionProjects/RegexThesis/external/build-deps/graphviz/cmd/tools/gmlparse.y"

    int i;
    char *str;
    gmlnode* np;
    gmledge* ep;
    gmlattr* ap;
    attrs_t *list;

#line 115 "/home/user/CLionProjects/RegexThesis/external/build-deps/graphviz/build-wasm/cmd/tools/gmlparse.h"

};
typedef union GMLSTYPE GMLSTYPE;
# define GMLSTYPE_IS_TRIVIAL 1
# define GMLSTYPE_IS_DECLARED 1
#endif


extern GMLSTYPE gmllval;


int gmlparse (void);


#endif /* !YY_GML_HOME_USER_CLIONPROJECTS_REGEXTHESIS_EXTERNAL_BUILD_DEPS_GRAPHVIZ_BUILD_WASM_CMD_TOOLS_GMLPARSE_H_INCLUDED  */
