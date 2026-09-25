// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.


// Take the name prefix into account.
#define yylex   tlyylex



#include "parsetl.hh"


// Unqualified %code blocks.
#line 195 "parsetl.yy"

/* parsetl.hh and parsedecl.hh include each other recursively.
   We mut ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include <spot/parsetl/parsedecl.hh>
using namespace spot;

#define missing_right_op_msg(op, str)		\
  error_list.emplace_back(op,			\
    "missing right operand for \"" str "\"");

#define missing_right_op(res, op, str)		\
  do						\
    {						\
      missing_right_op_msg(op, str);		\
      res = fnode::ff();		\
    }						\
  while (0);

// right is missing, so complain and use left.
#define missing_right_binop(res, left, op, str)	\
  do						\
    {						\
      missing_right_op_msg(op, str);		\
      res = left;				\
    }						\
  while (0);

  static const fnode*
  sere_ensure_bool(const fnode* f, const spot::location& loc,
                   const char* oper, spot::parse_error_list& error_list)
  {
    if (f->is_boolean())
      return f;
    f->destroy();
    std::string s;
    s.reserve(80);
    s = "not a Boolean expression: in a SERE ";
    s += oper;
    s += " can only be applied to a Boolean expression";
    error_list.emplace_back(loc, s);
    return nullptr;
  }

  static const fnode*
  error_false_block(const spot::location& loc,
                    spot::parse_error_list& error_list)
  {
    error_list.emplace_back(loc, "treating this block as false");
    return fnode::ff();
  }

  static const fnode*
  parse_ap(const std::string& str,
           const spot::location& location,
           spot::environment& env,
           spot::parse_error_list& error_list)
  {
    auto res = env.require(str);
    if (!res)
      {
        std::string s;
        s.reserve(64);
        s = "unknown atomic proposition `";
        s += str;
        s += "' in ";
        s += env.name();
        error_list.emplace_back(location, s);
      }
    return res.to_node_();
  }

  enum parser_type { parser_ltl, parser_bool, parser_sere };

  static const fnode*
  try_recursive_parse(const std::string& str,
		      const spot::location& location,
		      spot::environment& env,
		      bool debug,
		      parser_type type,
		      spot::parse_error_list& error_list)
    {
      // We want to parse a U (b U c) as two until operators applied
      // to the atomic propositions a, b, and c.  We also want to
      // parse a U (b == c) as one until operator applied to the
      // atomic propositions "a" and "b == c".  The only problem is
      // that we do not know anything about "==" or in general about
      // the syntax of atomic proposition of our users.
      //
      // To support that, the lexer will return "b U c" and "b == c"
      // as PAR_BLOCK tokens.  We then try to parse such tokens
      // recursively.  If, as in the case of "b U c", the block is
      // successfully parsed as a formula, we return this formula.
      // Otherwise, we convert the string into an atomic proposition
      // (it's up to the environment to check the syntax of this
      // proposition, and maybe reject it).

      if (str.empty())
	{
	  error_list.emplace_back(location, "unexpected empty block");
	  return fnode::ff();
	}

      spot::parsed_formula pf;
      switch (type)
	{
	case parser_sere:
	  pf = spot::parse_infix_sere(str, env, debug, true);
	  break;
	case parser_bool:
	  pf = spot::parse_infix_boolean(str, env, debug, true);
	  break;
	case parser_ltl:
	  pf = spot::parse_infix_psl(str, env, debug, true);
	  break;
	}

      if (pf.errors.empty())
	return pf.f.to_node_();
      return parse_ap(str, location, env, error_list);
    }


#line 172 "parsetl.cc"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if TLYYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !TLYYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !TLYYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace tlyy {
#line 264 "parsetl.cc"

  /// Build a parser object.
  parser::parser (spot::parse_error_list &error_list_yyarg, spot::environment &parse_environment_yyarg, spot::formula &result_yyarg)
#if TLYYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      error_list (error_list_yyarg),
      parse_environment (parse_environment_yyarg),
      result (result_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

  // basic_symbol.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        value.copy< minmax_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        value.copy< pnode > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_aplist: // aplist
        value.copy< std::vector<const spot::fnode*> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        value.copy< unsigned > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }




  template <typename Base>
  parser::symbol_kind_type
  parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        value.move< minmax_t > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        value.move< pnode > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        value.move< std::string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_aplist: // aplist
        value.move< std::vector<const spot::fnode*> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        value.move< unsigned > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
  }

  // by_kind.
  parser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  parser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  parser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  parser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  parser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  parser::symbol_kind_type
  parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  parser::symbol_kind_type
  parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }



  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_kind_type
  parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        value.YY_MOVE_OR_COPY< minmax_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        value.YY_MOVE_OR_COPY< pnode > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_aplist: // aplist
        value.YY_MOVE_OR_COPY< std::vector<const spot::fnode*> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        value.YY_MOVE_OR_COPY< unsigned > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        value.move< minmax_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        value.move< pnode > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_aplist: // aplist
        value.move< std::vector<const spot::fnode*> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        value.move< unsigned > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        value.copy< minmax_t > (that.value);
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        value.copy< pnode > (that.value);
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_aplist: // aplist
        value.copy< std::vector<const spot::fnode*> > (that.value);
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        value.copy< unsigned > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        value.move< minmax_t > (that.value);
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        value.move< pnode > (that.value);
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_aplist: // aplist
        value.move< std::vector<const spot::fnode*> > (that.value);
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        value.move< unsigned > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if TLYYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        switch (yykind)
    {
      case symbol_kind::S_PAR_BLOCK: // "(...) block"
#line 417 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < std::string > (); }
#line 747 "parsetl.cc"
        break;

      case symbol_kind::S_BRA_BLOCK: // "{...} block"
#line 417 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < std::string > (); }
#line 753 "parsetl.cc"
        break;

      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
#line 417 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < std::string > (); }
#line 759 "parsetl.cc"
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
#line 420 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < unsigned > (); }
#line 765 "parsetl.cc"
        break;

      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
#line 417 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < std::string > (); }
#line 771 "parsetl.cc"
        break;

      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
#line 420 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < unsigned > (); }
#line 777 "parsetl.cc"
        break;

      case symbol_kind::S_sqbkt_num: // sqbkt_num
#line 420 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < unsigned > (); }
#line 783 "parsetl.cc"
        break;

      case symbol_kind::S_sqbracketargs: // sqbracketargs
#line 421 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < minmax_t > ().min << ".." << yysym.value.template as < minmax_t > ().max; }
#line 789 "parsetl.cc"
        break;

      case symbol_kind::S_gotoargs: // gotoargs
#line 421 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < minmax_t > ().min << ".." << yysym.value.template as < minmax_t > ().max; }
#line 795 "parsetl.cc"
        break;

      case symbol_kind::S_starargs: // starargs
#line 421 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < minmax_t > ().min << ".." << yysym.value.template as < minmax_t > ().max; }
#line 801 "parsetl.cc"
        break;

      case symbol_kind::S_fstarargs: // fstarargs
#line 421 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < minmax_t > ().min << ".." << yysym.value.template as < minmax_t > ().max; }
#line 807 "parsetl.cc"
        break;

      case symbol_kind::S_equalargs: // equalargs
#line 421 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < minmax_t > ().min << ".." << yysym.value.template as < minmax_t > ().max; }
#line 813 "parsetl.cc"
        break;

      case symbol_kind::S_delayargs: // delayargs
#line 421 "parsetl.yy"
                 { debug_stream() << yysym.value.template as < minmax_t > ().min << ".." << yysym.value.template as < minmax_t > ().max; }
#line 819 "parsetl.cc"
        break;

      case symbol_kind::S_atomprop: // atomprop
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 825 "parsetl.cc"
        break;

      case symbol_kind::S_booleanatom: // booleanatom
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 831 "parsetl.cc"
        break;

      case symbol_kind::S_sere: // sere
#line 419 "parsetl.yy"
                 { print_sere(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 837 "parsetl.cc"
        break;

      case symbol_kind::S_bracedsere: // bracedsere
#line 419 "parsetl.yy"
                 { print_sere(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 843 "parsetl.cc"
        break;

      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 849 "parsetl.cc"
        break;

      case symbol_kind::S_boolformula: // boolformula
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 855 "parsetl.cc"
        break;

      case symbol_kind::S_aplist: // aplist
#line 422 "parsetl.yy"
                 {
  debug_stream() << "[ ";
  for (unsigned i = 0; i < yysym.value.template as < std::vector<const spot::fnode*> > ().size(); ++i)
    debug_stream() << yysym.value.template as < std::vector<const spot::fnode*> > ()[i]->ap_name() << ' ';
  debug_stream() << ']';
}
#line 866 "parsetl.cc"
        break;

      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 872 "parsetl.cc"
        break;

      case symbol_kind::S_quantifiedformula: // quantifiedformula
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 878 "parsetl.cc"
        break;

      case symbol_kind::S_subformula: // subformula
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 884 "parsetl.cc"
        break;

      case symbol_kind::S_lbtformula: // lbtformula
#line 418 "parsetl.yy"
                 { print_psl(debug_stream(), yysym.value.template as < pnode > ().tmp()); }
#line 890 "parsetl.cc"
        break;

      default:
        break;
    }
        yyo << ')';
      }
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if TLYYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // TLYYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location, error_list));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_sqbracketargs: // sqbracketargs
      case symbol_kind::S_gotoargs: // gotoargs
      case symbol_kind::S_starargs: // starargs
      case symbol_kind::S_fstarargs: // fstarargs
      case symbol_kind::S_equalargs: // equalargs
      case symbol_kind::S_delayargs: // delayargs
        yylhs.value.emplace< minmax_t > ();
        break;

      case symbol_kind::S_atomprop: // atomprop
      case symbol_kind::S_booleanatom: // booleanatom
      case symbol_kind::S_sere: // sere
      case symbol_kind::S_bracedsere: // bracedsere
      case symbol_kind::S_parenthesedsubformula: // parenthesedsubformula
      case symbol_kind::S_boolformula: // boolformula
      case symbol_kind::S_maybequantifiedformula: // maybequantifiedformula
      case symbol_kind::S_quantifiedformula: // quantifiedformula
      case symbol_kind::S_subformula: // subformula
      case symbol_kind::S_lbtformula: // lbtformula
        yylhs.value.emplace< pnode > ();
        break;

      case symbol_kind::S_PAR_BLOCK: // "(...) block"
      case symbol_kind::S_BRA_BLOCK: // "{...} block"
      case symbol_kind::S_BRA_BANG_BLOCK: // "{...}! block"
      case symbol_kind::S_ATOMIC_PROP: // "atomic proposition"
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_aplist: // aplist
        yylhs.value.emplace< std::vector<const spot::fnode*> > ();
        break;

      case symbol_kind::S_OP_SQBKT_NUM: // "number for square bracket operator"
      case symbol_kind::S_OP_DELAY_N: // "SVA delay operator"
      case symbol_kind::S_sqbkt_num: // sqbkt_num
        yylhs.value.emplace< unsigned > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // result: "LTL start marker" subformula "end of formula"
#line 435 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1184 "parsetl.cc"
    break;

  case 3: // result: "LTL start marker" enderror
#line 440 "parsetl.yy"
              {
		result = nullptr;
		YYABORT;
	      }
#line 1193 "parsetl.cc"
    break;

  case 4: // result: "LTL start marker" subformula enderror
#line 445 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1202 "parsetl.cc"
    break;

  case 5: // result: "LTL start marker" quantifiedformula "end of formula"
#line 450 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1211 "parsetl.cc"
    break;

  case 6: // result: "LTL start marker" quantifiedformula enderror
#line 455 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1220 "parsetl.cc"
    break;

  case 7: // result: "LTL start marker" emptyinput
#line 460 "parsetl.yy"
              { YYABORT; }
#line 1226 "parsetl.cc"
    break;

  case 8: // result: "BOOLEAN start marker" boolformula "end of formula"
#line 462 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1235 "parsetl.cc"
    break;

  case 9: // result: "BOOLEAN start marker" enderror
#line 467 "parsetl.yy"
              {
		result = nullptr;
		YYABORT;
	      }
#line 1244 "parsetl.cc"
    break;

  case 10: // result: "BOOLEAN start marker" boolformula enderror
#line 472 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1253 "parsetl.cc"
    break;

  case 11: // result: "BOOLEAN start marker" emptyinput
#line 477 "parsetl.yy"
              { YYABORT; }
#line 1259 "parsetl.cc"
    break;

  case 12: // result: "SERE start marker" sere "end of formula"
#line 479 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1268 "parsetl.cc"
    break;

  case 13: // result: "SERE start marker" enderror
#line 484 "parsetl.yy"
              {
		result = nullptr;
		YYABORT;
	      }
#line 1277 "parsetl.cc"
    break;

  case 14: // result: "SERE start marker" sere enderror
#line 489 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1286 "parsetl.cc"
    break;

  case 15: // result: "SERE start marker" emptyinput
#line 494 "parsetl.yy"
              { YYABORT; }
#line 1292 "parsetl.cc"
    break;

  case 16: // result: "LBT start marker" lbtformula "end of formula"
#line 496 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1301 "parsetl.cc"
    break;

  case 17: // result: "LBT start marker" enderror
#line 501 "parsetl.yy"
              {
		result = nullptr;
		YYABORT;
	      }
#line 1310 "parsetl.cc"
    break;

  case 18: // result: "LBT start marker" lbtformula enderror
#line 506 "parsetl.yy"
              {
		result = formula(YY_MOVE (yystack_[1].value.as < pnode > ()));
		YYACCEPT;
	      }
#line 1319 "parsetl.cc"
    break;

  case 19: // result: "LBT start marker" emptyinput
#line 511 "parsetl.yy"
              { YYABORT; }
#line 1325 "parsetl.cc"
    break;

  case 20: // emptyinput: "end of formula"
#line 514 "parsetl.yy"
              {
		error_list.emplace_back(yylhs.location, "empty input");
		result = nullptr;
	      }
#line 1334 "parsetl.cc"
    break;

  case 21: // enderror: error "end of formula"
#line 520 "parsetl.yy"
              {
		error_list.emplace_back(yystack_[1].location, "ignoring trailing garbage");
	      }
#line 1342 "parsetl.cc"
    break;

  case 28: // sqbkt_num: "number for square bracket operator"
#line 532 "parsetl.yy"
         {
           auto n = YY_MOVE (yystack_[0].value.as < unsigned > ());
           if (n >= fnode::unbounded())
             {
               auto max = fnode::unbounded() - 1;
               std::ostringstream s;
               s << n << " exceeds maximum supported repetition ("
                 << max << ")";
               error_list.emplace_back(yystack_[0].location, s.str());
               yylhs.value.as < unsigned > () = max;
             }
           else
             {
               yylhs.value.as < unsigned > () = n;
             }
         }
#line 1363 "parsetl.cc"
    break;

  case 29: // sqbracketargs: sqbkt_num "separator for square bracket operator" sqbkt_num "closing bracket"
#line 551 "parsetl.yy"
              { yylhs.value.as < minmax_t > ().min = YY_MOVE (yystack_[3].value.as < unsigned > ()); yylhs.value.as < minmax_t > ().max = YY_MOVE (yystack_[1].value.as < unsigned > ()); }
#line 1369 "parsetl.cc"
    break;

  case 30: // sqbracketargs: sqbkt_num OP_SQBKT_SEP_unbounded "closing bracket"
#line 553 "parsetl.yy"
              { yylhs.value.as < minmax_t > ().min = YY_MOVE (yystack_[2].value.as < unsigned > ()); yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1375 "parsetl.cc"
    break;

  case 31: // sqbracketargs: "separator for square bracket operator" sqbkt_num "closing bracket"
#line 555 "parsetl.yy"
              { yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = YY_MOVE (yystack_[1].value.as < unsigned > ()); }
#line 1381 "parsetl.cc"
    break;

  case 32: // sqbracketargs: OP_SQBKT_SEP_opt "closing bracket"
#line 557 "parsetl.yy"
              { yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1387 "parsetl.cc"
    break;

  case 33: // sqbracketargs: sqbkt_num "closing bracket"
#line 559 "parsetl.yy"
              { yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = YY_MOVE (yystack_[1].value.as < unsigned > ()); }
#line 1393 "parsetl.cc"
    break;

  case 34: // gotoargs: "opening bracket for goto operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing bracket"
#line 563 "parsetl.yy"
           { yylhs.value.as < minmax_t > ().min = YY_MOVE (yystack_[3].value.as < unsigned > ()); yylhs.value.as < minmax_t > ().max = YY_MOVE (yystack_[1].value.as < unsigned > ()); }
#line 1399 "parsetl.cc"
    break;

  case 35: // gotoargs: "opening bracket for goto operator" sqbkt_num OP_SQBKT_SEP_unbounded "closing bracket"
#line 565 "parsetl.yy"
           { yylhs.value.as < minmax_t > ().min = YY_MOVE (yystack_[2].value.as < unsigned > ()); yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1405 "parsetl.cc"
    break;

  case 36: // gotoargs: "opening bracket for goto operator" "separator for square bracket operator" sqbkt_num "closing bracket"
#line 567 "parsetl.yy"
           { yylhs.value.as < minmax_t > ().min = 1U; yylhs.value.as < minmax_t > ().max = YY_MOVE (yystack_[1].value.as < unsigned > ()); }
#line 1411 "parsetl.cc"
    break;

  case 37: // gotoargs: "opening bracket for goto operator" OP_SQBKT_SEP_unbounded "closing bracket"
#line 569 "parsetl.yy"
           { yylhs.value.as < minmax_t > ().min = 1U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1417 "parsetl.cc"
    break;

  case 38: // gotoargs: "opening bracket for goto operator" "closing bracket"
#line 571 "parsetl.yy"
           { yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 1U; }
#line 1423 "parsetl.cc"
    break;

  case 39: // gotoargs: "opening bracket for goto operator" sqbkt_num "closing bracket"
#line 573 "parsetl.yy"
           { yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = YY_MOVE (yystack_[1].value.as < unsigned > ()); }
#line 1429 "parsetl.cc"
    break;

  case 40: // gotoargs: "opening bracket for goto operator" error "closing bracket"
#line 575 "parsetl.yy"
           { error_list.emplace_back(yylhs.location, "treating this goto block as [->]");
             yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 1U; }
#line 1436 "parsetl.cc"
    break;

  case 41: // gotoargs: "opening bracket for goto operator" error_opt "end of formula"
#line 578 "parsetl.yy"
           { error_list.
	       emplace_back(yylhs.location, "missing closing bracket for goto operator");
	     yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 0U; }
#line 1444 "parsetl.cc"
    break;

  case 44: // starargs: kleen_star
#line 585 "parsetl.yy"
            { yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1450 "parsetl.cc"
    break;

  case 45: // starargs: "plus operator"
#line 587 "parsetl.yy"
            { yylhs.value.as < minmax_t > ().min = 1U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1456 "parsetl.cc"
    break;

  case 46: // starargs: "opening bracket for star operator" sqbracketargs
#line 589 "parsetl.yy"
            { yylhs.value.as < minmax_t > () = YY_MOVE (yystack_[0].value.as < minmax_t > ()); }
#line 1462 "parsetl.cc"
    break;

  case 47: // starargs: "opening bracket for star operator" error "closing bracket"
#line 591 "parsetl.yy"
            { error_list.emplace_back(yylhs.location, "treating this star block as [*]");
              yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1469 "parsetl.cc"
    break;

  case 48: // starargs: "opening bracket for star operator" error_opt "end of formula"
#line 594 "parsetl.yy"
            { error_list.emplace_back(yylhs.location, "missing closing bracket for star");
	      yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 0U; }
#line 1476 "parsetl.cc"
    break;

  case 49: // fstarargs: "bracket fusion-star operator"
#line 598 "parsetl.yy"
            { yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1482 "parsetl.cc"
    break;

  case 50: // fstarargs: "fusion-plus operator"
#line 600 "parsetl.yy"
            { yylhs.value.as < minmax_t > ().min = 1U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1488 "parsetl.cc"
    break;

  case 51: // fstarargs: "opening bracket for fusion-star operator" sqbracketargs
#line 602 "parsetl.yy"
            { yylhs.value.as < minmax_t > () = YY_MOVE (yystack_[0].value.as < minmax_t > ()); }
#line 1494 "parsetl.cc"
    break;

  case 52: // fstarargs: "opening bracket for fusion-star operator" error "closing bracket"
#line 604 "parsetl.yy"
            { error_list.emplace_back
		(yylhs.location, "treating this fusion-star block as [:*]");
              yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1502 "parsetl.cc"
    break;

  case 53: // fstarargs: "opening bracket for fusion-star operator" error_opt "end of formula"
#line 608 "parsetl.yy"
            { error_list.emplace_back
		(yylhs.location, "missing closing bracket for fusion-star");
	      yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 0U; }
#line 1510 "parsetl.cc"
    break;

  case 54: // equalargs: "opening bracket for equal operator" sqbracketargs
#line 613 "parsetl.yy"
            { yylhs.value.as < minmax_t > () = YY_MOVE (yystack_[0].value.as < minmax_t > ()); }
#line 1516 "parsetl.cc"
    break;

  case 55: // equalargs: "opening bracket for equal operator" error "closing bracket"
#line 615 "parsetl.yy"
            { error_list.emplace_back(yylhs.location, "treating this equal block as [=]");
              yylhs.value.as < minmax_t > ().min = 0U; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1523 "parsetl.cc"
    break;

  case 56: // equalargs: "opening bracket for equal operator" error_opt "end of formula"
#line 618 "parsetl.yy"
            { error_list.
		emplace_back(yylhs.location, "missing closing bracket for equal operator");
	      yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 0U; }
#line 1531 "parsetl.cc"
    break;

  case 57: // delayargs: "opening bracket for SVA delay operator" sqbracketargs
#line 623 "parsetl.yy"
            { yylhs.value.as < minmax_t > () = YY_MOVE (yystack_[0].value.as < minmax_t > ()); }
#line 1537 "parsetl.cc"
    break;

  case 58: // delayargs: "opening bracket for SVA delay operator" error "closing bracket"
#line 625 "parsetl.yy"
            { error_list.emplace_back(yylhs.location, "treating this delay block as ##1");
              yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 1U; }
#line 1544 "parsetl.cc"
    break;

  case 59: // delayargs: "opening bracket for SVA delay operator" error_opt "end of formula"
#line 628 "parsetl.yy"
            { error_list.
		emplace_back(yylhs.location, "missing closing bracket for ##[");
	      yylhs.value.as < minmax_t > ().min = yylhs.value.as < minmax_t > ().max = 1U; }
#line 1552 "parsetl.cc"
    break;

  case 60: // delayargs: "##[+] operator"
#line 632 "parsetl.yy"
          { yylhs.value.as < minmax_t > ().min = 1; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1558 "parsetl.cc"
    break;

  case 61: // delayargs: "##[*] operator"
#line 634 "parsetl.yy"
          { yylhs.value.as < minmax_t > ().min = 0; yylhs.value.as < minmax_t > ().max = fnode::unbounded(); }
#line 1564 "parsetl.cc"
    break;

  case 62: // atomprop: "atomic proposition"
#line 637 "parsetl.yy"
          {
            auto* f = parse_ap(YY_MOVE (yystack_[0].value.as < std::string > ()), yystack_[0].location, parse_environment, error_list);
            if (!f)
              YYERROR;
            yylhs.value.as < pnode > () = f;
          }
#line 1575 "parsetl.cc"
    break;

  case 63: // booleanatom: atomprop
#line 644 "parsetl.yy"
             { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 1581 "parsetl.cc"
    break;

  case 64: // booleanatom: atomprop "positive suffix"
#line 645 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ()); }
#line 1587 "parsetl.cc"
    break;

  case 65: // booleanatom: atomprop "negative suffix"
#line 647 "parsetl.yy"
              {
		yylhs.value.as < pnode > () = fnode::unop(op::Not, YY_MOVE (yystack_[1].value.as < pnode > ()));
	      }
#line 1595 "parsetl.cc"
    break;

  case 66: // booleanatom: "constant true"
#line 651 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::tt(); }
#line 1601 "parsetl.cc"
    break;

  case 67: // booleanatom: "constant false"
#line 653 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::ff(); }
#line 1607 "parsetl.cc"
    break;

  case 68: // sere: booleanatom
#line 655 "parsetl.yy"
      { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 1613 "parsetl.cc"
    break;

  case 69: // sere: "not operator" sere
#line 657 "parsetl.yy"
              {
		if (auto f = sere_ensure_bool(YY_MOVE (yystack_[0].value.as < pnode > ()), yystack_[0].location, "`!'", error_list))
		  {
		    yylhs.value.as < pnode > () = fnode::unop(op::Not, f);
		  }
		else
		  {
		    yylhs.value.as < pnode > () = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1628 "parsetl.cc"
    break;

  case 70: // sere: bracedsere
#line 667 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 1634 "parsetl.cc"
    break;

  case 71: // sere: "(...) block"
#line 669 "parsetl.yy"
              {
		yylhs.value.as < pnode > () =
		  try_recursive_parse(YY_MOVE (yystack_[0].value.as < std::string > ()), yystack_[0].location, parse_environment,
				      debug_level(), parser_sere, error_list);
		if (!yylhs.value.as < pnode > ())
		  YYERROR;
	      }
#line 1646 "parsetl.cc"
    break;

  case 72: // sere: "opening parenthesis" sere "closing parenthesis"
#line 677 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ()); }
#line 1652 "parsetl.cc"
    break;

  case 73: // sere: "opening parenthesis" error "closing parenthesis"
#line 679 "parsetl.yy"
              { error_list.
		  emplace_back(yylhs.location,
			       "treating this parenthetical block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 1662 "parsetl.cc"
    break;

  case 74: // sere: "opening parenthesis" sere "end of formula"
#line 685 "parsetl.yy"
              { error_list.emplace_back(yystack_[2].location + yystack_[1].location, "missing closing parenthesis");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ());
	      }
#line 1670 "parsetl.cc"
    break;

  case 75: // sere: "opening parenthesis" error "end of formula"
#line 689 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
                    "missing closing parenthesis, "
		    "treating this parenthetical block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 1680 "parsetl.cc"
    break;

  case 76: // sere: sere "and operator" sere
#line 695 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::AndRat, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 1686 "parsetl.cc"
    break;

  case 77: // sere: sere "and operator" error
#line 697 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location,
				    "length-matching and operator"); }
#line 1693 "parsetl.cc"
    break;

  case 78: // sere: sere "short and operator" sere
#line 700 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::AndNLM, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 1699 "parsetl.cc"
    break;

  case 79: // sere: sere "short and operator" error
#line 702 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location,
                                    "non-length-matching and operator"); }
#line 1706 "parsetl.cc"
    break;

  case 80: // sere: sere "or operator" sere
#line 705 "parsetl.yy"
            { yylhs.value.as < pnode > () = pnode(op::OrRat, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 1712 "parsetl.cc"
    break;

  case 81: // sere: sere "or operator" error
#line 707 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "or operator"); }
#line 1718 "parsetl.cc"
    break;

  case 82: // sere: sere "concat operator" sere
#line 709 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::Concat, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 1724 "parsetl.cc"
    break;

  case 83: // sere: sere "concat operator" error
#line 711 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "concat operator"); }
#line 1730 "parsetl.cc"
    break;

  case 84: // sere: sere ":" sere
#line 713 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::Fusion, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 1736 "parsetl.cc"
    break;

  case 85: // sere: sere ":" error
#line 715 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "fusion operator"); }
#line 1742 "parsetl.cc"
    break;

  case 86: // sere: "SVA delay operator" sere
#line 717 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[1].value.as < unsigned > ()); yylhs.value.as < pnode > () = formula::sugar_delay(formula(YY_MOVE (yystack_[0].value.as < pnode > ())), n, n).to_node_(); }
#line 1748 "parsetl.cc"
    break;

  case 87: // sere: "SVA delay operator" error
#line 719 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), fnode::tt(), yystack_[1].location, "SVA delay operator"); }
#line 1754 "parsetl.cc"
    break;

  case 88: // sere: sere "SVA delay operator" sere
#line 721 "parsetl.yy"
            { unsigned n = YY_MOVE (yystack_[1].value.as < unsigned > ());
              yylhs.value.as < pnode > () = formula::sugar_delay(formula(YY_MOVE (yystack_[2].value.as < pnode > ())), formula(YY_MOVE (yystack_[0].value.as < pnode > ())),
                                        n, n).to_node_(); }
#line 1762 "parsetl.cc"
    break;

  case 89: // sere: sere "SVA delay operator" error
#line 725 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "SVA delay operator"); }
#line 1768 "parsetl.cc"
    break;

  case 90: // sere: delayargs sere
#line 727 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[1].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[1].location, "reversed range");
		    std::swap(max, min);
		  }
                yylhs.value.as < pnode > () = formula::sugar_delay(formula(YY_MOVE (yystack_[0].value.as < pnode > ())),
                                          min, max).to_node_();
              }
#line 1783 "parsetl.cc"
    break;

  case 91: // sere: delayargs error
#line 738 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), fnode::tt(), yystack_[1].location, "SVA delay operator"); }
#line 1789 "parsetl.cc"
    break;

  case 92: // sere: sere delayargs sere
#line 740 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[1].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[2].location, "reversed range");
		    std::swap(max, min);
		  }
                yylhs.value.as < pnode > () = formula::sugar_delay(formula(YY_MOVE (yystack_[2].value.as < pnode > ())), formula(YY_MOVE (yystack_[0].value.as < pnode > ())),
                                          min, max).to_node_();
              }
#line 1804 "parsetl.cc"
    break;

  case 93: // sere: sere delayargs error
#line 751 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "SVA delay operator"); }
#line 1810 "parsetl.cc"
    break;

  case 94: // sere: starargs
#line 753 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[0].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap(max, min);
		  }
		yylhs.value.as < pnode > () = fnode::bunop(op::Star, fnode::tt(), min, max);
	      }
#line 1824 "parsetl.cc"
    break;

  case 95: // sere: sere starargs
#line 763 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[0].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap(max, min);
		  }
		yylhs.value.as < pnode > () = fnode::bunop(op::Star, YY_MOVE (yystack_[1].value.as < pnode > ()), min, max);
	      }
#line 1838 "parsetl.cc"
    break;

  case 96: // sere: sere fstarargs
#line 773 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[0].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap(max, min);
		  }
		yylhs.value.as < pnode > () = fnode::bunop(op::FStar, YY_MOVE (yystack_[1].value.as < pnode > ()), min, max);
	      }
#line 1852 "parsetl.cc"
    break;

  case 97: // sere: sere equalargs
#line 783 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[0].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap(max, min);
		  }
		if (auto f = sere_ensure_bool(YY_MOVE (yystack_[1].value.as < pnode > ()), yystack_[1].location, "[=...]", error_list))
		  {
		    yylhs.value.as < pnode > () = formula::sugar_equal(formula(f),
					      min, max).to_node_();
		  }
		else
		  {
		    yylhs.value.as < pnode > () = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1874 "parsetl.cc"
    break;

  case 98: // sere: sere gotoargs
#line 801 "parsetl.yy"
              {
                auto [min, max] = YY_MOVE (yystack_[0].value.as < minmax_t > ());
		if (max < min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap(max, min);
		  }
		if (auto f = sere_ensure_bool(YY_MOVE (yystack_[1].value.as < pnode > ()), yystack_[1].location, "[->...]", error_list))
		  {
		    yylhs.value.as < pnode > () = formula::sugar_goto(formula(f), min, max).to_node_();
		  }
		else
		  {
		    yylhs.value.as < pnode > () = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1895 "parsetl.cc"
    break;

  case 99: // sere: sere "xor operator" sere
#line 818 "parsetl.yy"
              {
                auto left = sere_ensure_bool(YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[2].location, "`^'", error_list);
                auto right = sere_ensure_bool(YY_MOVE (yystack_[0].value.as < pnode > ()), yystack_[0].location, "`^'", error_list);
		if (left && right)
		  {
		    yylhs.value.as < pnode > () = fnode::binop(op::Xor, left, right);
		  }
		else
		  {
                    if (left)
                      left->destroy();
                    else if (right)
                      right->destroy();
		    yylhs.value.as < pnode > () = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1916 "parsetl.cc"
    break;

  case 100: // sere: sere "xor operator" error
#line 835 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "xor operator"); }
#line 1922 "parsetl.cc"
    break;

  case 101: // sere: sere "implication operator" sere
#line 837 "parsetl.yy"
              {
                auto left = sere_ensure_bool(YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[2].location, "`->'", error_list);
		if (left)
		  {
		    yylhs.value.as < pnode > () = fnode::binop(op::Implies, left, YY_MOVE (yystack_[0].value.as < pnode > ()));
		  }
		else
		  {
		    yylhs.value.as < pnode > () = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1938 "parsetl.cc"
    break;

  case 102: // sere: sere "implication operator" error
#line 849 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "implication operator"); }
#line 1944 "parsetl.cc"
    break;

  case 103: // sere: sere "equivalent operator" sere
#line 851 "parsetl.yy"
              {
                auto left = sere_ensure_bool(YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[2].location, "`<->'", error_list);
                auto right = sere_ensure_bool(YY_MOVE (yystack_[0].value.as < pnode > ()), yystack_[0].location, "`<->'", error_list);
                if (left && right)
		  {
		    yylhs.value.as < pnode > () = fnode::binop(op::Equiv, left, right);
		  }
		else
		  {
                    if (left)
                      left->destroy();
                    else if (right)
                      right->destroy();
		    yylhs.value.as < pnode > () = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1965 "parsetl.cc"
    break;

  case 104: // sere: sere "equivalent operator" error
#line 868 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "equivalent operator"); }
#line 1971 "parsetl.cc"
    break;

  case 105: // sere: "first_match" "opening parenthesis" sere "closing parenthesis"
#line 870 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::first_match, YY_MOVE (yystack_[1].value.as < pnode > ())); }
#line 1977 "parsetl.cc"
    break;

  case 106: // bracedsere: "opening brace" sere "closing brace"
#line 873 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ()); }
#line 1983 "parsetl.cc"
    break;

  case 107: // bracedsere: "opening brace" sere error "closing brace"
#line 875 "parsetl.yy"
              { error_list.emplace_back(yystack_[1].location, "ignoring this");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[2].value.as < pnode > ());
	      }
#line 1991 "parsetl.cc"
    break;

  case 108: // bracedsere: "opening brace" error "closing brace"
#line 879 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
					"treating this brace block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 2000 "parsetl.cc"
    break;

  case 109: // bracedsere: "opening brace" sere "end of formula"
#line 884 "parsetl.yy"
              { error_list.emplace_back(yystack_[2].location + yystack_[1].location,
					"missing closing brace");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ());
	      }
#line 2009 "parsetl.cc"
    break;

  case 110: // bracedsere: "opening brace" sere error "end of formula"
#line 889 "parsetl.yy"
              { error_list. emplace_back(yystack_[1].location,
                  "ignoring trailing garbage and missing closing brace");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[2].value.as < pnode > ());
	      }
#line 2018 "parsetl.cc"
    break;

  case 111: // bracedsere: "opening brace" error "end of formula"
#line 894 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
                    "missing closing brace, "
		    "treating this brace block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 2028 "parsetl.cc"
    break;

  case 112: // bracedsere: "{...} block"
#line 900 "parsetl.yy"
              {
		yylhs.value.as < pnode > () = try_recursive_parse(YY_MOVE (yystack_[0].value.as < std::string > ()), yystack_[0].location, parse_environment,
					 debug_level(),
                                         parser_sere, error_list);
		if (!yylhs.value.as < pnode > ())
		  YYERROR;
	      }
#line 2040 "parsetl.cc"
    break;

  case 113: // parenthesedsubformula: "(...) block"
#line 909 "parsetl.yy"
              {
		yylhs.value.as < pnode > () = try_recursive_parse(YY_MOVE (yystack_[0].value.as < std::string > ()), yystack_[0].location, parse_environment,
					 debug_level(), parser_ltl, error_list);
		if (!yylhs.value.as < pnode > ())
		  YYERROR;
	      }
#line 2051 "parsetl.cc"
    break;

  case 114: // parenthesedsubformula: "opening parenthesis" subformula "closing parenthesis"
#line 916 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ()); }
#line 2057 "parsetl.cc"
    break;

  case 115: // parenthesedsubformula: "opening parenthesis" subformula error "closing parenthesis"
#line 918 "parsetl.yy"
              { error_list.emplace_back(yystack_[1].location, "ignoring this");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[2].value.as < pnode > ());
	      }
#line 2065 "parsetl.cc"
    break;

  case 116: // parenthesedsubformula: "opening parenthesis" error "closing parenthesis"
#line 922 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
		 "treating this parenthetical block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 2074 "parsetl.cc"
    break;

  case 117: // parenthesedsubformula: "opening parenthesis" subformula "end of formula"
#line 927 "parsetl.yy"
              { error_list.emplace_back(yystack_[2].location + yystack_[1].location, "missing closing parenthesis");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ());
	      }
#line 2082 "parsetl.cc"
    break;

  case 118: // parenthesedsubformula: "opening parenthesis" subformula error "end of formula"
#line 931 "parsetl.yy"
              { error_list.emplace_back(yystack_[1].location,
                "ignoring trailing garbage and missing closing parenthesis");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[2].value.as < pnode > ());
	      }
#line 2091 "parsetl.cc"
    break;

  case 119: // parenthesedsubformula: "opening parenthesis" error "end of formula"
#line 936 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
                    "missing closing parenthesis, "
		    "treating this parenthetical block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 2101 "parsetl.cc"
    break;

  case 120: // boolformula: booleanatom
#line 943 "parsetl.yy"
             { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 2107 "parsetl.cc"
    break;

  case 121: // boolformula: "(...) block"
#line 945 "parsetl.yy"
              {
		yylhs.value.as < pnode > () = try_recursive_parse(YY_MOVE (yystack_[0].value.as < std::string > ()), yystack_[0].location, parse_environment,
					 debug_level(),
                                         parser_bool, error_list);
		if (!yylhs.value.as < pnode > ())
		  YYERROR;
	      }
#line 2119 "parsetl.cc"
    break;

  case 122: // boolformula: "opening parenthesis" boolformula "closing parenthesis"
#line 953 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ()); }
#line 2125 "parsetl.cc"
    break;

  case 123: // boolformula: "opening parenthesis" boolformula error "closing parenthesis"
#line 955 "parsetl.yy"
              { error_list.emplace_back(yystack_[1].location, "ignoring this");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[2].value.as < pnode > ());
	      }
#line 2133 "parsetl.cc"
    break;

  case 124: // boolformula: "opening parenthesis" error "closing parenthesis"
#line 959 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
		 "treating this parenthetical block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 2142 "parsetl.cc"
    break;

  case 125: // boolformula: "opening parenthesis" boolformula "end of formula"
#line 964 "parsetl.yy"
              { error_list.emplace_back(yystack_[2].location + yystack_[1].location,
					"missing closing parenthesis");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[1].value.as < pnode > ());
	      }
#line 2151 "parsetl.cc"
    break;

  case 126: // boolformula: "opening parenthesis" boolformula error "end of formula"
#line 969 "parsetl.yy"
              { error_list.emplace_back(yystack_[1].location,
                "ignoring trailing garbage and missing closing parenthesis");
		yylhs.value.as < pnode > () = YY_MOVE (yystack_[2].value.as < pnode > ());
	      }
#line 2160 "parsetl.cc"
    break;

  case 127: // boolformula: "opening parenthesis" error "end of formula"
#line 974 "parsetl.yy"
              { error_list.emplace_back(yylhs.location,
                    "missing closing parenthesis, "
		    "treating this parenthetical block as false");
		yylhs.value.as < pnode > () = fnode::ff();
	      }
#line 2170 "parsetl.cc"
    break;

  case 128: // boolformula: boolformula "and operator" boolformula
#line 980 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2176 "parsetl.cc"
    break;

  case 129: // boolformula: boolformula "and operator" error
#line 982 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "and operator"); }
#line 2182 "parsetl.cc"
    break;

  case 130: // boolformula: boolformula "short and operator" boolformula
#line 984 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2188 "parsetl.cc"
    break;

  case 131: // boolformula: boolformula "short and operator" error
#line 986 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "and operator"); }
#line 2194 "parsetl.cc"
    break;

  case 132: // boolformula: boolformula "star operator" boolformula
#line 988 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2200 "parsetl.cc"
    break;

  case 133: // boolformula: boolformula "star operator" error
#line 990 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "and operator"); }
#line 2206 "parsetl.cc"
    break;

  case 134: // boolformula: boolformula "or operator" boolformula
#line 992 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::Or, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2212 "parsetl.cc"
    break;

  case 135: // boolformula: boolformula "or operator" error
#line 994 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "or operator"); }
#line 2218 "parsetl.cc"
    break;

  case 136: // boolformula: boolformula "xor operator" boolformula
#line 996 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Xor, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2224 "parsetl.cc"
    break;

  case 137: // boolformula: boolformula "xor operator" error
#line 998 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "xor operator"); }
#line 2230 "parsetl.cc"
    break;

  case 138: // boolformula: boolformula "implication operator" boolformula
#line 1000 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Implies, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2236 "parsetl.cc"
    break;

  case 139: // boolformula: boolformula "implication operator" error
#line 1002 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "implication operator"); }
#line 2242 "parsetl.cc"
    break;

  case 140: // boolformula: boolformula "equivalent operator" boolformula
#line 1004 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Equiv, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2248 "parsetl.cc"
    break;

  case 141: // boolformula: boolformula "equivalent operator" error
#line 1006 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "equivalent operator"); }
#line 2254 "parsetl.cc"
    break;

  case 142: // boolformula: "not operator" boolformula
#line 1008 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::Not, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2260 "parsetl.cc"
    break;

  case 143: // boolformula: "not operator" error
#line 1010 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location, "not operator"); }
#line 2266 "parsetl.cc"
    break;

  case 144: // aplist: atomprop
#line 1013 "parsetl.yy"
         { yylhs.value.as < std::vector<const spot::fnode*> > () = std::vector<const spot::fnode*>{ YY_MOVE (yystack_[0].value.as < pnode > ()) }; }
#line 2272 "parsetl.cc"
    break;

  case 145: // aplist: aplist atomprop
#line 1015 "parsetl.yy"
         { yylhs.value.as < std::vector<const spot::fnode*> > () = YY_MOVE (yystack_[1].value.as < std::vector<const spot::fnode*> > ()); yylhs.value.as < std::vector<const spot::fnode*> > ().push_back(YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2278 "parsetl.cc"
    break;

  case 146: // aplist: aplist "," atomprop
#line 1017 "parsetl.yy"
         { yylhs.value.as < std::vector<const spot::fnode*> > () = YY_MOVE (yystack_[2].value.as < std::vector<const spot::fnode*> > ()); yylhs.value.as < std::vector<const spot::fnode*> > ().push_back(YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2284 "parsetl.cc"
    break;

  case 147: // maybequantifiedformula: subformula
#line 1019 "parsetl.yy"
                        { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 2290 "parsetl.cc"
    break;

  case 148: // maybequantifiedformula: quantifiedformula
#line 1019 "parsetl.yy"
                                     { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 2296 "parsetl.cc"
    break;

  case 151: // quantifiedformula: "exists operator" aplist ":" maybequantifiedformula
#line 1025 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::quantify(op::exists, YY_MOVE (yystack_[2].value.as < std::vector<const spot::fnode*> > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2302 "parsetl.cc"
    break;

  case 152: // quantifiedformula: "forall operator" aplist ":" maybequantifiedformula
#line 1027 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::quantify(op::forall, YY_MOVE (yystack_[2].value.as < std::vector<const spot::fnode*> > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2308 "parsetl.cc"
    break;

  case 153: // quantifiedformula: exists_or_forall error ":" maybequantifiedformula
#line 1029 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ());
                error_list.emplace_back(yystack_[3].location + yystack_[1].location, "ignoring quantification");
              }
#line 2316 "parsetl.cc"
    break;

  case 154: // quantifiedformula: exists_or_forall error_opt "end of formula"
#line 1033 "parsetl.yy"
              {
                yylhs.value.as < pnode > () = fnode::ff();
                error_list.emplace_back(yylhs.location, "syntax error in quantification");
              }
#line 2325 "parsetl.cc"
    break;

  case 155: // subformula: booleanatom
#line 1038 "parsetl.yy"
            { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 2331 "parsetl.cc"
    break;

  case 156: // subformula: parenthesedsubformula
#line 1039 "parsetl.yy"
              { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 2337 "parsetl.cc"
    break;

  case 157: // subformula: subformula "and operator" subformula
#line 1041 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2343 "parsetl.cc"
    break;

  case 158: // subformula: subformula "and operator" error
#line 1043 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "and operator"); }
#line 2349 "parsetl.cc"
    break;

  case 159: // subformula: subformula "short and operator" subformula
#line 1045 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2355 "parsetl.cc"
    break;

  case 160: // subformula: subformula "short and operator" error
#line 1047 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "and operator"); }
#line 2361 "parsetl.cc"
    break;

  case 161: // subformula: subformula "star operator" subformula
#line 1049 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2367 "parsetl.cc"
    break;

  case 162: // subformula: subformula "star operator" error
#line 1051 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "and operator"); }
#line 2373 "parsetl.cc"
    break;

  case 163: // subformula: subformula "or operator" subformula
#line 1053 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::Or, YY_MOVE (yystack_[2].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2379 "parsetl.cc"
    break;

  case 164: // subformula: subformula "or operator" error
#line 1055 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "or operator"); }
#line 2385 "parsetl.cc"
    break;

  case 165: // subformula: subformula "xor operator" subformula
#line 1057 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Xor, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2391 "parsetl.cc"
    break;

  case 166: // subformula: subformula "xor operator" error
#line 1059 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "xor operator"); }
#line 2397 "parsetl.cc"
    break;

  case 167: // subformula: subformula "implication operator" subformula
#line 1061 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Implies, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2403 "parsetl.cc"
    break;

  case 168: // subformula: subformula "implication operator" error
#line 1063 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "implication operator"); }
#line 2409 "parsetl.cc"
    break;

  case 169: // subformula: subformula "equivalent operator" subformula
#line 1065 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Equiv, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2415 "parsetl.cc"
    break;

  case 170: // subformula: subformula "equivalent operator" error
#line 1067 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "equivalent operator"); }
#line 2421 "parsetl.cc"
    break;

  case 171: // subformula: subformula "until operator" subformula
#line 1069 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::U, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2427 "parsetl.cc"
    break;

  case 172: // subformula: subformula "until operator" error
#line 1071 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "until operator"); }
#line 2433 "parsetl.cc"
    break;

  case 173: // subformula: subformula "release operator" subformula
#line 1073 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::R, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2439 "parsetl.cc"
    break;

  case 174: // subformula: subformula "release operator" error
#line 1075 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "release operator"); }
#line 2445 "parsetl.cc"
    break;

  case 175: // subformula: subformula "weak until operator" subformula
#line 1077 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::W, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2451 "parsetl.cc"
    break;

  case 176: // subformula: subformula "weak until operator" error
#line 1079 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "weak until operator"); }
#line 2457 "parsetl.cc"
    break;

  case 177: // subformula: subformula "strong release operator" subformula
#line 1081 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::M, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2463 "parsetl.cc"
    break;

  case 178: // subformula: subformula "strong release operator" error
#line 1083 "parsetl.yy"
              { missing_right_binop(yylhs.value.as < pnode > (), YY_MOVE (yystack_[2].value.as < pnode > ()), yystack_[1].location, "strong release operator"); }
#line 2469 "parsetl.cc"
    break;

  case 179: // subformula: "sometimes operator" subformula
#line 1085 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::F, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2475 "parsetl.cc"
    break;

  case 180: // subformula: "sometimes operator" error
#line 1087 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location, "sometimes operator"); }
#line 2481 "parsetl.cc"
    break;

  case 181: // subformula: "F[.] operator" sqbkt_num "closing bracket" subformula
#line 1089 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[2].value.as < unsigned > ());
                yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::Or, n, n, YY_MOVE (yystack_[0].value.as < pnode > ()));
                error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "F[n:m] expects two parameters");
              }
#line 2491 "parsetl.cc"
    break;

  case 182: // subformula: "F[.] operator" sqbkt_num "closing !]" subformula
#line 1096 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[2].value.as < unsigned > ());
                yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X, op::Or, n, n, YY_MOVE (yystack_[0].value.as < pnode > ()));
                error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "F[n:m!] expects two parameters");
              }
#line 2501 "parsetl.cc"
    break;

  case 183: // subformula: "F[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing bracket" subformula
#line 1103 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::Or, YY_MOVE (yystack_[4].value.as < unsigned > ()), YY_MOVE (yystack_[2].value.as < unsigned > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2507 "parsetl.cc"
    break;

  case 184: // subformula: "F[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing !]" subformula
#line 1106 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X,
                                              op::Or, YY_MOVE (yystack_[4].value.as < unsigned > ()), YY_MOVE (yystack_[2].value.as < unsigned > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2514 "parsetl.cc"
    break;

  case 185: // subformula: "F[.] operator" sqbkt_num OP_SQBKT_SEP_unbounded "closing bracket" subformula
#line 1110 "parsetl.yy"
            { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::Or, YY_MOVE (yystack_[3].value.as < unsigned > ()),
                                            fnode::unbounded(), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2521 "parsetl.cc"
    break;

  case 186: // subformula: "F[.] operator" sqbkt_num OP_SQBKT_SEP_unbounded "closing !]" subformula
#line 1114 "parsetl.yy"
            { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X, op::Or, YY_MOVE (yystack_[3].value.as < unsigned > ()),
                                            fnode::unbounded(), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2528 "parsetl.cc"
    break;

  case 187: // subformula: "F[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing bracket" error
#line 1118 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[5].location + yystack_[1].location, "F[.] operator"); }
#line 2534 "parsetl.cc"
    break;

  case 188: // subformula: "F[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing !]" error
#line 1121 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[5].location + yystack_[1].location, "F[.!] operator"); }
#line 2540 "parsetl.cc"
    break;

  case 189: // subformula: "F[.] operator" error_opt "end of formula"
#line 1123 "parsetl.yy"
              { error_list.emplace_back(yylhs.location, "missing closing bracket for F[.]");
                yylhs.value.as < pnode > () = fnode::ff(); }
#line 2547 "parsetl.cc"
    break;

  case 190: // subformula: "F[.] operator" error "closing bracket" subformula
#line 1126 "parsetl.yy"
              { error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "treating this F[.] as a simple F");
                yylhs.value.as < pnode > () = fnode::unop(op::F, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2555 "parsetl.cc"
    break;

  case 191: // subformula: "F[.] operator" error "closing !]" subformula
#line 1130 "parsetl.yy"
              { error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "treating this F[.!] as a simple F");
                yylhs.value.as < pnode > () = fnode::unop(op::F, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2563 "parsetl.cc"
    break;

  case 192: // subformula: "always operator" subformula
#line 1134 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::G, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2569 "parsetl.cc"
    break;

  case 193: // subformula: "always operator" error
#line 1136 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location, "always operator"); }
#line 2575 "parsetl.cc"
    break;

  case 194: // subformula: "G[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing bracket" subformula
#line 1139 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::And, YY_MOVE (yystack_[4].value.as < unsigned > ()), YY_MOVE (yystack_[2].value.as < unsigned > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2581 "parsetl.cc"
    break;

  case 195: // subformula: "G[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing !]" subformula
#line 1142 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X, op::And,
                                              YY_MOVE (yystack_[4].value.as < unsigned > ()), YY_MOVE (yystack_[2].value.as < unsigned > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2588 "parsetl.cc"
    break;

  case 196: // subformula: "G[.] operator" sqbkt_num OP_SQBKT_SEP_unbounded "closing bracket" subformula
#line 1146 "parsetl.yy"
            { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::And, YY_MOVE (yystack_[3].value.as < unsigned > ()),
                                            fnode::unbounded(), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2595 "parsetl.cc"
    break;

  case 197: // subformula: "G[.] operator" sqbkt_num OP_SQBKT_SEP_unbounded "closing !]" subformula
#line 1150 "parsetl.yy"
            { yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X, op::And, YY_MOVE (yystack_[3].value.as < unsigned > ()),
                                            fnode::unbounded(), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2602 "parsetl.cc"
    break;

  case 198: // subformula: "G[.] operator" sqbkt_num "closing bracket" subformula
#line 1153 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[2].value.as < unsigned > ());
                yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::And, n, n, YY_MOVE (yystack_[0].value.as < pnode > ()));
                error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "G[n:m] expects two parameters");
              }
#line 2612 "parsetl.cc"
    break;

  case 199: // subformula: "G[.] operator" sqbkt_num "closing !]" subformula
#line 1160 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[2].value.as < unsigned > ());
                yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X, op::And,
                                              n, n, YY_MOVE (yystack_[0].value.as < pnode > ()));
                error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "G[n:m!] expects two parameters");
              }
#line 2623 "parsetl.cc"
    break;

  case 200: // subformula: "G[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing bracket" error
#line 1168 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[5].location + yystack_[1].location, "G[.] operator"); }
#line 2629 "parsetl.cc"
    break;

  case 201: // subformula: "G[.] operator" sqbkt_num "separator for square bracket operator" sqbkt_num "closing !]" error
#line 1171 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[5].location + yystack_[1].location, "G[.!] operator"); }
#line 2635 "parsetl.cc"
    break;

  case 202: // subformula: "G[.] operator" error_opt "end of formula"
#line 1173 "parsetl.yy"
              { error_list.emplace_back(yylhs.location, "missing closing bracket for G[.]");
                yylhs.value.as < pnode > () = fnode::ff(); }
#line 2642 "parsetl.cc"
    break;

  case 203: // subformula: "G[.] operator" error "closing bracket" subformula
#line 1176 "parsetl.yy"
              { error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "treating this G[.] as a simple G");
                yylhs.value.as < pnode > () = fnode::unop(op::F, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2650 "parsetl.cc"
    break;

  case 204: // subformula: "G[.] operator" error "closing !]" subformula
#line 1180 "parsetl.yy"
              { error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "treating this G[.!] as a simple G");
                yylhs.value.as < pnode > () = fnode::unop(op::F, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2658 "parsetl.cc"
    break;

  case 205: // subformula: "next operator" subformula
#line 1184 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::X, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2664 "parsetl.cc"
    break;

  case 206: // subformula: "next operator" error
#line 1186 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location, "next operator"); }
#line 2670 "parsetl.cc"
    break;

  case 207: // subformula: "strong next operator" subformula
#line 1188 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::strong_X, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2676 "parsetl.cc"
    break;

  case 208: // subformula: "strong next operator" error
#line 1190 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location, "strong next operator"); }
#line 2682 "parsetl.cc"
    break;

  case 209: // subformula: "X[.] operator" sqbkt_num "closing bracket" subformula
#line 1192 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[2].value.as < unsigned > ());
                yylhs.value.as < pnode > () = fnode::nested_unop_range(op::X, op::Or, n, n, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2689 "parsetl.cc"
    break;

  case 210: // subformula: "X[.] operator" sqbkt_num "closing bracket" error
#line 1195 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[3].location + yystack_[1].location, "X[.] operator"); }
#line 2695 "parsetl.cc"
    break;

  case 211: // subformula: "X[.] operator" error "closing bracket" subformula
#line 1197 "parsetl.yy"
              { error_list.emplace_back(yylhs.location, "treating this X[.] as a simple X");
                yylhs.value.as < pnode > () = fnode::unop(op::X, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2702 "parsetl.cc"
    break;

  case 212: // subformula: "X[.] operator" "closing !]" subformula
#line 1200 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::strong_X, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2708 "parsetl.cc"
    break;

  case 213: // subformula: "X[.] operator" sqbkt_num "closing !]" subformula
#line 1203 "parsetl.yy"
              { unsigned n = YY_MOVE (yystack_[2].value.as < unsigned > ());
                yylhs.value.as < pnode > () = fnode::nested_unop_range(op::strong_X,
                                              op::Or, n, n, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2716 "parsetl.cc"
    break;

  case 214: // subformula: "X[.] operator" error "closing !]" subformula
#line 1207 "parsetl.yy"
              { error_list.emplace_back(yylhs.location, "treating this X[.!] as a simple X[!]");
                yylhs.value.as < pnode > () = fnode::unop(op::strong_X, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2723 "parsetl.cc"
    break;

  case 215: // subformula: "X[.] operator" error_opt "end of formula"
#line 1210 "parsetl.yy"
              { error_list.emplace_back(yylhs.location, "missing closing bracket for X[.]");
                yylhs.value.as < pnode > () = fnode::ff(); }
#line 2730 "parsetl.cc"
    break;

  case 216: // subformula: "not operator" subformula
#line 1213 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::Not, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2736 "parsetl.cc"
    break;

  case 217: // subformula: "not operator" error
#line 1215 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location, "not operator"); }
#line 2742 "parsetl.cc"
    break;

  case 218: // subformula: bracedsere
#line 1217 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::Closure, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2748 "parsetl.cc"
    break;

  case 219: // subformula: bracedsere "universal concat operator" subformula
#line 1219 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::UConcat, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2754 "parsetl.cc"
    break;

  case 220: // subformula: bracedsere parenthesedsubformula
#line 1221 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::UConcat, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2760 "parsetl.cc"
    break;

  case 221: // subformula: bracedsere "universal concat operator" error
#line 1223 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location,
                                 "universal overlapping concat operator"); }
#line 2767 "parsetl.cc"
    break;

  case 222: // subformula: bracedsere "existential concat operator" subformula
#line 1226 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[2].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::EConcat, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2773 "parsetl.cc"
    break;

  case 223: // subformula: bracedsere "existential concat operator" error
#line 1228 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location,
                                 "existential overlapping concat operator");
	      }
#line 2781 "parsetl.cc"
    break;

  case 224: // subformula: bracedsere "universal non-overlapping concat operator" subformula
#line 1233 "parsetl.yy"
              {
                const fnode* left = pnode(op::Concat, YY_MOVE (yystack_[2].value.as < pnode > ()), fnode::tt());
                yylhs.value.as < pnode > () = fnode::binop(op::UConcat, left, YY_MOVE (yystack_[0].value.as < pnode > ()));
              }
#line 2790 "parsetl.cc"
    break;

  case 225: // subformula: bracedsere "universal non-overlapping concat operator" error
#line 1238 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location,
                                 "universal non-overlapping concat operator");
	      }
#line 2798 "parsetl.cc"
    break;

  case 226: // subformula: bracedsere "existential non-overlapping concat operator" subformula
#line 1243 "parsetl.yy"
              {
                const fnode* left = pnode(op::Concat, YY_MOVE (yystack_[2].value.as < pnode > ()), fnode::tt());
                yylhs.value.as < pnode > () = fnode::binop(op::EConcat, left, YY_MOVE (yystack_[0].value.as < pnode > ()));
              }
#line 2807 "parsetl.cc"
    break;

  case 227: // subformula: bracedsere "existential non-overlapping concat operator" error
#line 1248 "parsetl.yy"
              { missing_right_op(yylhs.value.as < pnode > (), yystack_[1].location,
                                 "existential non-overlapping concat operator");
	      }
#line 2815 "parsetl.cc"
    break;

  case 228: // subformula: "opening brace" sere "closing brace-bang"
#line 1253 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::binop(op::EConcat, YY_MOVE (yystack_[1].value.as < pnode > ()), fnode::tt()); }
#line 2821 "parsetl.cc"
    break;

  case 229: // subformula: "{...}! block"
#line 1255 "parsetl.yy"
              {
		yylhs.value.as < pnode > () = try_recursive_parse(YY_MOVE (yystack_[0].value.as < std::string > ()), yystack_[0].location, parse_environment,
					 debug_level(),
                                         parser_sere, error_list);
		if (!yylhs.value.as < pnode > ())
		  YYERROR;
		yylhs.value.as < pnode > () = fnode::binop(op::EConcat, yylhs.value.as < pnode > (), fnode::tt());
	      }
#line 2834 "parsetl.cc"
    break;

  case 230: // lbtformula: atomprop
#line 1264 "parsetl.yy"
            { yylhs.value.as < pnode > () = YY_MOVE (yystack_[0].value.as < pnode > ()); }
#line 2840 "parsetl.cc"
    break;

  case 231: // lbtformula: '!' lbtformula
#line 1266 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::Not, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2846 "parsetl.cc"
    break;

  case 232: // lbtformula: '&' lbtformula lbtformula
#line 1268 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::And, YY_MOVE (yystack_[1].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2852 "parsetl.cc"
    break;

  case 233: // lbtformula: '|' lbtformula lbtformula
#line 1270 "parsetl.yy"
              { yylhs.value.as < pnode > () = pnode(op::Or, YY_MOVE (yystack_[1].value.as < pnode > ()), YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2858 "parsetl.cc"
    break;

  case 234: // lbtformula: '^' lbtformula lbtformula
#line 1272 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Xor, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2864 "parsetl.cc"
    break;

  case 235: // lbtformula: 'i' lbtformula lbtformula
#line 1274 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Implies, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2870 "parsetl.cc"
    break;

  case 236: // lbtformula: 'e' lbtformula lbtformula
#line 1276 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::Equiv, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2876 "parsetl.cc"
    break;

  case 237: // lbtformula: 'X' lbtformula
#line 1278 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::X, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2882 "parsetl.cc"
    break;

  case 238: // lbtformula: 'F' lbtformula
#line 1280 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::F, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2888 "parsetl.cc"
    break;

  case 239: // lbtformula: 'G' lbtformula
#line 1282 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::unop(op::G, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2894 "parsetl.cc"
    break;

  case 240: // lbtformula: 'U' lbtformula lbtformula
#line 1284 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::U, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2900 "parsetl.cc"
    break;

  case 241: // lbtformula: 'V' lbtformula lbtformula
#line 1286 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::R, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2906 "parsetl.cc"
    break;

  case 242: // lbtformula: 'R' lbtformula lbtformula
#line 1288 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::R, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2912 "parsetl.cc"
    break;

  case 243: // lbtformula: 'W' lbtformula lbtformula
#line 1290 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::W, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2918 "parsetl.cc"
    break;

  case 244: // lbtformula: 'M' lbtformula lbtformula
#line 1292 "parsetl.yy"
              { const fnode* left = YY_MOVE (yystack_[1].value.as < pnode > ()); yylhs.value.as < pnode > () = fnode::binop(op::M, left, YY_MOVE (yystack_[0].value.as < pnode > ())); }
#line 2924 "parsetl.cc"
    break;

  case 245: // lbtformula: 't'
#line 1294 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::tt(); }
#line 2930 "parsetl.cc"
    break;

  case 246: // lbtformula: 'f'
#line 1296 "parsetl.yy"
              { yylhs.value.as < pnode > () = fnode::ff(); }
#line 2936 "parsetl.cc"
    break;


#line 2940 "parsetl.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
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
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // parser::context.
  parser::context::context (const parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short parser::yypact_ninf_ = -220;

  const signed char parser::yytable_ninf_ = -27;

  const short
  parser::yypact_[] =
  {
     336,  1226,   337,   685,   480,    22,   -51,  1305,  -220,  -220,
    -220,   290,  1358,  1398,  1433,  1468,   -39,   -39,  1503,    50,
       9,   120,  -220,  -220,  -220,  -220,  -220,  -220,    11,  -220,
     148,  -220,     6,     7,   187,  2374,  2374,  2374,  2374,  2374,
    2374,  2374,  2374,  2374,  2374,  2374,  2374,  2374,  2374,  -220,
    -220,  -220,  -220,  -220,    14,   722,  -220,   290,  1233,  -220,
    -220,  -220,     3,    24,   759,   121,  -220,  -220,  -220,  -220,
    -220,  -220,   796,  -220,   625,  -220,   615,  -220,   620,  -220,
    -220,  -220,   692,  -220,  -220,    16,   510,    10,   540,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,   116,   117,
    -220,  -220,    60,  2308,  -220,   -21,    67,   173,   -16,    55,
     203,   -11,    96,  -220,  -220,  1538,  1573,  1608,  1643,  -220,
       8,    43,  -220,  -220,  1678,  1713,  1748,  1783,  1818,  1853,
    1888,  1923,  1958,  1993,  2028,  -220,  -220,  -220,  2374,  2374,
    2374,  2374,  2374,  -220,  -220,  -220,  2374,  2374,  2374,  2374,
    2374,  -220,  -220,    18,  1285,   594,  -220,    15,   218,  -220,
      95,   115,    -4,  -220,  1233,  -220,  2394,   134,   130,  -220,
    -220,  2394,   833,   870,   907,   944,   981,  1018,  -220,  -220,
     168,   270,   276,  1055,  1092,  -220,  1129,  -220,  -220,  -220,
    -220,  -220,  1166,    34,    17,  -220,  -220,  1153,  1209,  2210,
    2215,  2232,  2239,  2244,  -220,  -220,  -220,  -220,    35,  -220,
    -220,  -220,  -220,    51,  -220,  -220,  -220,   -39,  2297,  -220,
    2297,  2308,  2308,  -220,  -220,  2063,  2308,  2308,  2308,  -220,
    2308,  2308,   218,   301,  2308,  2308,  -220,  2308,  2308,   218,
     306,  -220,  2442,  -220,  2442,  -220,  2442,  -220,  2442,  2297,
    -220,  -220,   526,  -220,   682,  -220,   400,  -220,   400,  -220,
    2442,  -220,  2442,  -220,   362,  -220,   362,  -220,   362,  -220,
     362,  -220,   362,  -220,  -220,  -220,  -220,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,   167,
    -220,  -220,  -220,   218,   171,  1338,  -220,  -220,  -220,  2384,
    -220,  2242,  -220,   549,  -220,   549,  -220,  2394,  -220,  2394,
     177,   175,  -220,   193,   180,  -220,   197,  -220,   218,   212,
     201,   107,  -220,  2342,  -220,  2353,  -220,  2394,  -220,  2394,
    -220,  -220,    38,  -220,  -220,  -220,   160,  -220,   228,  -220,
     195,  -220,   195,  -220,   236,  -220,   236,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  2442,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,   319,  2308,  2308,
    -220,  -220,  -220,  -220,   331,  2308,  2308,  -220,  -220,   229,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,   231,  -220,  -220,
    -220,   218,   235,  -220,  -220,  2098,  2133,  -220,  -220,  2168,
    2203,  -220,  -220,  -220,  -220,   253,  -220,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220
  };

  const unsigned char
  parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,     0,   113,   112,
     229,     0,     0,     0,     0,     0,   149,   150,     0,     0,
       0,     0,    62,    66,    67,    20,     7,     3,    63,   155,
     218,   156,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   245,
     246,    19,    17,   230,     0,     0,    71,     0,     0,    42,
      43,    45,     0,     0,     0,     0,    60,    61,    15,    13,
      44,    94,     0,    68,     0,    70,     0,   121,     0,    11,
       9,   120,     0,     1,    21,     0,     0,     0,     0,   180,
     179,   193,   192,   206,   205,   208,   207,   144,     0,     0,
     217,   216,    27,     0,    28,     0,     0,    27,     0,     0,
      27,     0,     0,    65,    64,     0,     0,     0,     0,   220,
      27,     0,     5,     6,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     4,   231,     0,     0,
       0,     0,     0,   237,   238,   239,     0,     0,     0,     0,
       0,    16,    18,     0,     0,     0,    69,    27,    22,    25,
       0,     0,     0,    46,     0,    87,    86,    27,     0,    57,
      91,    90,     0,     0,     0,     0,     0,     0,    49,    50,
       0,     0,     0,     0,     0,    12,     0,    14,    98,    95,
      96,    97,     0,     0,     0,   143,   142,     0,     0,     0,
       0,     0,     0,     0,     8,    10,   116,   119,     0,   114,
     117,   108,   111,     0,   106,   228,   109,     0,     0,   145,
       0,     0,     0,   212,   215,     0,     0,     0,     0,   189,
       0,     0,    22,     0,     0,     0,   202,     0,     0,    22,
       0,   221,   219,   223,   222,   225,   224,   227,   226,     0,
     154,   164,   163,   166,   165,   158,   157,   160,   159,   168,
     167,   170,   169,   172,   171,   174,   173,   176,   175,   178,
     177,   162,   161,   232,   233,   234,   235,   236,   240,   241,
     242,   243,   244,    73,    75,    72,    74,    47,    23,     0,
      32,    48,    33,    22,     0,     0,    58,    59,    81,    80,
     100,    99,    77,    76,    79,    78,   102,   101,   104,   103,
      27,     0,    51,    27,     0,    54,    27,    38,    22,     0,
       0,     0,    83,    82,    85,    84,    89,    88,    93,    92,
     124,   127,     0,   122,   125,   135,   134,   137,   136,   129,
     128,   131,   130,   139,   138,   141,   140,   133,   132,   115,
     118,   107,   110,   146,   151,   148,   147,   152,   211,   214,
     210,   209,   213,   190,   191,   181,   182,     0,     0,     0,
     203,   204,   198,   199,     0,     0,     0,   153,    31,     0,
      30,   105,    52,    53,    55,    56,    40,     0,    37,    41,
      39,    22,     0,   123,   126,     0,     0,   185,   186,     0,
       0,   196,   197,    29,    36,     0,    35,   187,   183,   188,
     184,   200,   194,   201,   195,    34
  };

  const short
  parser::yypgoto_[] =
  {
    -220,  -220,   292,    25,  -107,  -220,    88,     0,   -62,  -220,
    -220,   -20,  -220,  -220,   -17,    45,   256,   324,   186,   233,
     -38,   284,  -219,  -220,   272,    -1,   425
  };

  const short
  parser::yydefgoto_[] =
  {
       0,     5,    26,    27,   159,   160,   105,   162,   163,   188,
      70,    71,   190,   191,    72,    28,    29,    74,    30,    31,
      82,    98,   354,    32,   355,   356,    54
  };

  const short
  parser::yytable_[] =
  {
      34,   357,   233,   169,   157,   240,    86,   120,     6,    84,
     107,    90,    92,    94,    96,     6,    22,   101,   332,   106,
     109,   112,    83,   211,   206,   333,   283,    52,    69,    80,
     377,   164,   197,   198,   199,   200,   201,   202,   194,   224,
     196,   292,   330,   349,   229,   293,   393,    53,   -24,   236,
     104,   102,   158,   203,   189,   294,   104,   192,   123,   136,
     287,    97,    97,   -26,   351,   249,   -26,   122,   189,   -26,
     212,   192,   113,   114,   151,   319,   207,   334,   284,   152,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,   331,   350,   103,   104,   394,   187,
     230,   231,   223,   250,   232,   221,   222,   205,   108,   111,
     -26,   352,   225,   226,   242,   244,   246,   248,   312,   315,
     121,   110,   167,   252,   254,   256,   258,   260,   262,   264,
     266,   268,   270,   272,   189,   189,   189,   192,   192,   192,
     290,   237,   238,   219,   219,   239,   189,   217,   217,   192,
     161,   189,   390,   168,   192,     7,   391,     8,   289,   336,
     338,   340,   342,   344,   346,   348,   -24,   104,   104,   310,
     158,    22,    22,   218,   220,   291,   198,   199,   200,   296,
     -26,   -26,   321,    53,    53,    53,    53,    53,     6,    75,
     297,    53,    53,    53,    53,    53,   203,    75,   115,   116,
     117,   118,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   378,   -24,   392,   104,   380,   158,   227,   228,
     358,   359,   382,   134,   361,   362,   363,   364,   -26,   365,
     366,   203,   367,   370,   371,   383,   372,   373,   384,   374,
     385,    75,   386,    75,    75,   199,   200,   135,   234,   235,
      75,   197,   198,   199,   200,   201,   202,   388,    75,    73,
      81,   389,   353,   119,   203,   104,   288,    73,   311,   314,
     320,   313,   203,    33,   403,   189,   404,   316,   192,   189,
     406,   189,   192,   189,   192,   189,   192,   189,   192,   189,
     192,    87,   192,   379,    51,    68,    79,    55,   415,    56,
       9,    99,    57,   189,     0,   189,   192,   189,   192,   189,
     192,    73,   192,    73,    73,   -24,     0,   104,   387,   158,
      73,   317,    58,   104,     0,   318,    59,    60,    73,    61,
     -26,    62,    81,     0,    81,    88,   -26,     0,     6,     1,
       2,     3,     4,     0,    63,    22,   368,   369,    23,    24,
      75,   375,   376,    64,    65,    66,    67,     0,    75,    75,
      75,    75,    75,    75,   395,   396,     0,   397,   398,    75,
      75,     0,    75,     0,   401,   402,   399,   400,    75,   154,
       0,   155,   156,   130,   131,   132,   133,     0,   166,     0,
       0,   405,    22,     0,   408,   410,   171,    25,   412,   414,
       0,     0,     0,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      73,   130,   131,   132,   133,     0,     0,     0,    73,    73,
      73,    73,    73,    73,     0,     0,   134,     0,     0,    73,
      73,     0,    73,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,    81,    81,    81,    81,    81,    81,    81,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,     0,     0,     0,     0,     0,     0,
       0,     6,     0,     0,     0,     0,     0,    76,   295,    77,
       0,     0,     0,     0,     0,     0,   299,   301,   303,   305,
     307,   309,     0,     0,     0,     0,     0,   323,   325,     0,
     327,   208,    78,     0,     0,     0,   329,     0,   209,     0,
       0,     0,     0,     0,     0,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    22,     0,     0,    23,    24,
      25,   213,   125,   126,   127,     0,   134,   130,   131,   132,
     133,     0,     0,   214,   215,   172,   173,   174,   175,   176,
     177,     0,   134,   273,   274,   275,   276,   277,     0,     0,
     210,   278,   279,   280,   281,   282,    59,    60,   178,    61,
     179,    62,   180,   181,   182,    59,    60,   178,    61,   179,
      62,   180,   181,   182,     0,   213,   183,   184,     0,     0,
     216,     0,     0,   186,    65,    66,    67,   214,     0,   172,
     173,   174,   175,   176,   177,     0,   193,     0,     0,     0,
       0,   195,    76,     0,    77,     0,     6,    76,     0,    77,
      59,    60,   178,    61,   179,    62,   180,   181,   182,     0,
     172,   173,   174,   175,   176,   177,     0,    78,     0,     0,
     183,   184,    78,     0,   216,     0,     0,   186,    65,    66,
      67,    59,    60,   178,    61,   179,    62,   180,   181,   182,
      22,     0,     0,    23,    24,    22,     0,     0,    23,    24,
       0,   183,   184,     0,     0,   185,     6,     0,   186,    65,
      66,    67,    55,     6,    56,     9,     0,    57,     0,   126,
     127,     0,     0,   130,   131,   132,   133,   197,   198,   199,
     200,   201,   202,     0,     0,     0,     0,    58,   134,     0,
       0,    59,    60,   153,    61,     0,    62,     0,   203,    55,
       0,    56,     9,     0,    57,     0,     0,     0,     0,    63,
      22,     0,     0,    23,    24,    25,     0,     0,    64,    65,
      66,    67,   204,     0,    58,     0,     0,     0,    59,    60,
     165,    61,     0,    62,     0,     0,    55,     0,    56,     9,
       0,    57,     0,     0,     0,     0,    63,    22,     0,     0,
      23,    24,     0,     0,     0,    64,    65,    66,    67,     0,
       0,    58,     0,     0,     0,    59,    60,   170,    61,     0,
      62,     0,     0,    55,     0,    56,     9,     0,    57,     0,
       0,     0,     0,    63,    22,     0,     0,    23,    24,     0,
       0,     0,    64,    65,    66,    67,     0,     0,    58,     0,
       0,     0,    59,    60,   298,    61,     0,    62,     0,     0,
      55,     0,    56,     9,     0,    57,     0,     0,     0,     0,
      63,    22,     0,     0,    23,    24,     0,     0,     0,    64,
      65,    66,    67,     0,     0,    58,     0,     0,     0,    59,
      60,   300,    61,     0,    62,     0,     0,    55,     0,    56,
       9,     0,    57,     0,     0,     0,     0,    63,    22,     0,
       0,    23,    24,     0,     0,     0,    64,    65,    66,    67,
       0,     0,    58,     0,     0,     0,    59,    60,   302,    61,
       0,    62,     0,     0,    55,     0,    56,     9,     0,    57,
       0,     0,     0,     0,    63,    22,     0,     0,    23,    24,
       0,     0,     0,    64,    65,    66,    67,     0,     0,    58,
       0,     0,     0,    59,    60,   304,    61,     0,    62,     0,
       0,    55,     0,    56,     9,     0,    57,     0,     0,     0,
       0,    63,    22,     0,     0,    23,    24,     0,     0,     0,
      64,    65,    66,    67,     0,     0,    58,     0,     0,     0,
      59,    60,   306,    61,     0,    62,     0,     0,    55,     0,
      56,     9,     0,    57,     0,     0,     0,     0,    63,    22,
       0,     0,    23,    24,     0,     0,     0,    64,    65,    66,
      67,     0,     0,    58,     0,     0,     0,    59,    60,   308,
      61,     0,    62,     0,     0,    55,     0,    56,     9,     0,
      57,     0,     0,     0,     0,    63,    22,     0,     0,    23,
      24,     0,     0,     0,    64,    65,    66,    67,     0,     0,
      58,     0,     0,     0,    59,    60,   322,    61,     0,    62,
       0,     0,    55,     0,    56,     9,     0,    57,     0,     0,
       0,     0,    63,    22,     0,     0,    23,    24,     0,     0,
       0,    64,    65,    66,    67,     0,     0,    58,     0,     0,
       0,    59,    60,   324,    61,     0,    62,     0,     0,    55,
       0,    56,     9,     0,    57,     0,     0,     0,     0,    63,
      22,     0,     0,    23,    24,     0,     0,     0,    64,    65,
      66,    67,     0,     0,    58,     0,     0,     0,    59,    60,
     326,    61,     0,    62,     0,     0,    55,     0,    56,     9,
       0,    57,     0,     0,     0,     0,    63,    22,     0,     0,
      23,    24,     0,     0,   335,    64,    65,    66,    67,     0,
      76,    58,    77,     0,     0,    59,    60,   328,    61,     0,
      62,     0,     0,    55,     0,    56,     9,     0,    57,     0,
       0,     0,     0,    63,    22,    78,     0,    23,    24,     0,
       0,     0,    64,    65,    66,    67,     0,     0,    58,     0,
       0,     0,    59,    60,     0,    61,     0,    62,    22,     0,
     337,    23,    24,     0,     0,     0,    76,     0,    77,     0,
      63,    22,     0,     0,    23,    24,     0,     6,     0,    64,
      65,    66,    67,     7,     0,     8,     9,    10,    11,     0,
      55,    78,    56,     9,     0,    57,     0,     0,     0,     0,
       0,    12,    13,    14,    15,    16,    17,     0,    18,    19,
      20,    21,     0,     0,    22,    58,     0,    23,    24,    59,
      60,     0,    61,     0,    62,     0,     0,     0,     0,     0,
       0,    22,     0,     0,    23,    24,    25,    63,    22,     0,
       0,    23,    24,   285,     0,     0,    64,    65,    66,    67,
     172,   173,   174,   175,   176,   177,    85,     0,     0,     0,
       0,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,    59,    60,   178,    61,   179,    62,   180,   181,   182,
      12,    13,    14,    15,     0,     0,     0,    18,    19,    20,
      21,   183,   184,     0,     0,   286,   381,     0,   186,    65,
      66,    67,     0,   172,   173,   174,   175,   176,   177,    89,
      22,     0,     0,    23,    24,     7,     0,     8,     9,    10,
      11,     0,     0,     0,    59,    60,   178,    61,   179,    62,
     180,   181,   182,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   183,   184,     0,     0,     0,    91,
       0,   186,    65,    66,    67,     7,     0,     8,     9,    10,
      11,     0,     0,    22,     0,     0,    23,    24,     0,     0,
       0,     0,     0,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,    93,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,    95,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   100,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   241,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   243,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   245,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   247,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   251,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   253,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   255,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   257,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   259,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   261,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   263,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   265,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   267,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   269,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   271,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   360,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   407,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   409,     0,     0,     0,     0,     0,
       7,     0,     8,     9,    10,    11,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    12,    13,
      14,    15,     0,     0,     0,    18,    19,    20,    21,   411,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,    23,    24,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,   413,     0,     0,     0,     0,     0,
       7,   339,     8,     9,    10,    11,   341,    76,     0,    77,
       0,     0,    76,    22,    77,     0,    23,    24,    12,    13,
      14,    15,     0,   343,     0,    18,    19,    20,    21,    76,
     345,    77,    78,     0,     0,   347,    76,    78,    77,     0,
       0,    76,     0,    77,     0,     0,     0,     0,    22,   174,
     175,    23,    24,     0,    78,    22,     0,     0,    23,    24,
      22,    78,     0,    23,    24,     0,    78,     0,    59,    60,
     178,    61,   179,    62,   180,   181,   182,    22,     0,     0,
      23,    24,     0,     0,    22,     0,     0,    23,    24,    22,
       0,     0,    23,    24,     7,     0,     8,     9,    10,    11,
       0,     0,     0,     0,     0,     7,     0,     8,     9,    10,
      11,     0,    12,    13,    14,    15,    16,    17,     0,    18,
      19,    20,    21,    12,    13,    14,    15,     0,     0,     0,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,    23,    24,   172,   173,   174,
     175,   176,   177,    22,     0,     0,    23,    24,   172,   173,
     174,   175,   176,   177,     0,     0,     0,     0,    59,    60,
     178,    61,   179,    62,   180,   181,   182,     0,     0,    59,
      60,   178,    61,   179,    62,   180,   181,   182,     0,   184,
     173,   174,   175,     0,     0,   186,    65,    66,    67,   172,
     173,   174,   175,   176,   177,     0,   186,    65,    66,    67,
      59,    60,   178,    61,   179,    62,   180,   181,   182,    22,
      59,    60,   178,    61,   179,    62,   180,   181,   182,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   134
  };

  const short
  parser::yycheck_[] =
  {
       1,   220,   109,    65,     1,   112,     7,     1,     1,    60,
       1,    12,    13,    14,    15,     1,    55,    18,     1,    19,
      20,    21,     0,    13,     8,     8,     8,     2,     3,     4,
     249,     7,    15,    16,    17,    18,    19,    20,    76,    60,
      78,    45,     8,     8,    60,    49,     8,     2,    45,    60,
      47,     1,    49,    36,    74,   162,    47,    74,    33,    34,
      45,    16,    17,    60,    13,    57,    60,    60,    88,    60,
      60,    88,    61,    62,    60,   182,    60,    60,    60,    54,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    60,    60,    46,    47,    60,    74,
      45,    46,   103,    60,    49,    45,    46,    82,    20,    21,
      60,    60,    45,    46,   115,   116,   117,   118,   180,   181,
      32,     1,     1,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   154,   155,   156,   154,   155,   156,
      45,    45,    46,    98,    99,    49,   166,    31,    31,   166,
      62,   171,    45,    65,   171,     7,    49,     9,   158,   197,
     198,   199,   200,   201,   202,   203,    45,    47,    47,     1,
      49,    55,    55,    57,    57,    60,    16,    17,    18,    45,
      60,    60,   182,   138,   139,   140,   141,   142,     1,     3,
      60,   146,   147,   148,   149,   150,    36,    11,    50,    51,
      52,    53,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    45,    45,   321,    47,    45,    49,    45,    46,
     221,   222,    45,    36,   225,   226,   227,   228,    60,   230,
     231,    36,   232,   234,   235,    60,   237,   238,    45,   239,
      60,    55,    45,    57,    58,    17,    18,    60,    45,    46,
      64,    15,    16,    17,    18,    19,    20,    45,    72,     3,
       4,    60,   217,    30,    36,    47,    48,    11,   180,   181,
     182,     1,    36,     1,    45,   295,    45,     1,   295,   299,
      45,   301,   299,   303,   301,   305,   303,   307,   305,   309,
     307,     1,   309,   293,     2,     3,     4,     7,    45,     9,
      10,    17,    12,   323,    -1,   325,   323,   327,   325,   329,
     327,    55,   329,    57,    58,    45,    -1,    47,   318,    49,
      64,    45,    32,    47,    -1,    49,    36,    37,    72,    39,
      60,    41,    76,    -1,    78,    11,    60,    -1,     1,     3,
       4,     5,     6,    -1,    54,    55,    45,    46,    58,    59,
     164,    45,    46,    63,    64,    65,    66,    -1,   172,   173,
     174,   175,   176,   177,    45,    46,    -1,   368,   369,   183,
     184,    -1,   186,    -1,   375,   376,    45,    46,   192,    55,
      -1,    57,    58,    21,    22,    23,    24,    -1,    64,    -1,
      -1,   391,    55,    -1,   395,   396,    72,    60,   399,   400,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
     164,    21,    22,    23,    24,    -1,    -1,    -1,   172,   173,
     174,   175,   176,   177,    -1,    -1,    36,    -1,    -1,   183,
     184,    -1,   186,    -1,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,    -1,   197,   198,   199,   200,   201,   202,   203,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    -1,    -1,    -1,    -1,     7,   164,     9,
      -1,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,    -1,   183,   184,    -1,
     186,     1,    32,    -1,    -1,    -1,   192,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    55,    -1,    -1,    58,    59,
      60,     1,    16,    17,    18,    -1,    36,    21,    22,    23,
      24,    -1,    -1,    13,    14,    15,    16,    17,    18,    19,
      20,    -1,    36,   138,   139,   140,   141,   142,    -1,    -1,
      60,   146,   147,   148,   149,   150,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    -1,     1,    56,    57,    -1,    -1,
      60,    -1,    -1,    63,    64,    65,    66,    13,    -1,    15,
      16,    17,    18,    19,    20,    -1,     1,    -1,    -1,    -1,
      -1,     1,     7,    -1,     9,    -1,     1,     7,    -1,     9,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      15,    16,    17,    18,    19,    20,    -1,    32,    -1,    -1,
      56,    57,    32,    -1,    60,    -1,    -1,    63,    64,    65,
      66,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      55,    -1,    -1,    58,    59,    55,    -1,    -1,    58,    59,
      -1,    56,    57,    -1,    -1,    60,     1,    -1,    63,    64,
      65,    66,     7,     1,     9,    10,    -1,    12,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    15,    16,    17,
      18,    19,    20,    -1,    -1,    -1,    -1,    32,    36,    -1,
      -1,    36,    37,     1,    39,    -1,    41,    -1,    36,     7,
      -1,     9,    10,    -1,    12,    -1,    -1,    -1,    -1,    54,
      55,    -1,    -1,    58,    59,    60,    -1,    -1,    63,    64,
      65,    66,    60,    -1,    32,    -1,    -1,    -1,    36,    37,
       1,    39,    -1,    41,    -1,    -1,     7,    -1,     9,    10,
      -1,    12,    -1,    -1,    -1,    -1,    54,    55,    -1,    -1,
      58,    59,    -1,    -1,    -1,    63,    64,    65,    66,    -1,
      -1,    32,    -1,    -1,    -1,    36,    37,     1,    39,    -1,
      41,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      -1,    -1,    -1,    54,    55,    -1,    -1,    58,    59,    -1,
      -1,    -1,    63,    64,    65,    66,    -1,    -1,    32,    -1,
      -1,    -1,    36,    37,     1,    39,    -1,    41,    -1,    -1,
       7,    -1,     9,    10,    -1,    12,    -1,    -1,    -1,    -1,
      54,    55,    -1,    -1,    58,    59,    -1,    -1,    -1,    63,
      64,    65,    66,    -1,    -1,    32,    -1,    -1,    -1,    36,
      37,     1,    39,    -1,    41,    -1,    -1,     7,    -1,     9,
      10,    -1,    12,    -1,    -1,    -1,    -1,    54,    55,    -1,
      -1,    58,    59,    -1,    -1,    -1,    63,    64,    65,    66,
      -1,    -1,    32,    -1,    -1,    -1,    36,    37,     1,    39,
      -1,    41,    -1,    -1,     7,    -1,     9,    10,    -1,    12,
      -1,    -1,    -1,    -1,    54,    55,    -1,    -1,    58,    59,
      -1,    -1,    -1,    63,    64,    65,    66,    -1,    -1,    32,
      -1,    -1,    -1,    36,    37,     1,    39,    -1,    41,    -1,
      -1,     7,    -1,     9,    10,    -1,    12,    -1,    -1,    -1,
      -1,    54,    55,    -1,    -1,    58,    59,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    -1,    32,    -1,    -1,    -1,
      36,    37,     1,    39,    -1,    41,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    -1,    -1,    -1,    54,    55,
      -1,    -1,    58,    59,    -1,    -1,    -1,    63,    64,    65,
      66,    -1,    -1,    32,    -1,    -1,    -1,    36,    37,     1,
      39,    -1,    41,    -1,    -1,     7,    -1,     9,    10,    -1,
      12,    -1,    -1,    -1,    -1,    54,    55,    -1,    -1,    58,
      59,    -1,    -1,    -1,    63,    64,    65,    66,    -1,    -1,
      32,    -1,    -1,    -1,    36,    37,     1,    39,    -1,    41,
      -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,    -1,
      -1,    -1,    54,    55,    -1,    -1,    58,    59,    -1,    -1,
      -1,    63,    64,    65,    66,    -1,    -1,    32,    -1,    -1,
      -1,    36,    37,     1,    39,    -1,    41,    -1,    -1,     7,
      -1,     9,    10,    -1,    12,    -1,    -1,    -1,    -1,    54,
      55,    -1,    -1,    58,    59,    -1,    -1,    -1,    63,    64,
      65,    66,    -1,    -1,    32,    -1,    -1,    -1,    36,    37,
       1,    39,    -1,    41,    -1,    -1,     7,    -1,     9,    10,
      -1,    12,    -1,    -1,    -1,    -1,    54,    55,    -1,    -1,
      58,    59,    -1,    -1,     1,    63,    64,    65,    66,    -1,
       7,    32,     9,    -1,    -1,    36,    37,     1,    39,    -1,
      41,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      -1,    -1,    -1,    54,    55,    32,    -1,    58,    59,    -1,
      -1,    -1,    63,    64,    65,    66,    -1,    -1,    32,    -1,
      -1,    -1,    36,    37,    -1,    39,    -1,    41,    55,    -1,
       1,    58,    59,    -1,    -1,    -1,     7,    -1,     9,    -1,
      54,    55,    -1,    -1,    58,    59,    -1,     1,    -1,    63,
      64,    65,    66,     7,    -1,     9,    10,    11,    12,    -1,
       7,    32,     9,    10,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    25,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    -1,    -1,    55,    32,    -1,    58,    59,    36,
      37,    -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    58,    59,    60,    54,    55,    -1,
      -1,    58,    59,     8,    -1,    -1,    63,    64,    65,    66,
      15,    16,    17,    18,    19,    20,     1,    -1,    -1,    -1,
      -1,    -1,     7,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      25,    26,    27,    28,    -1,    -1,    -1,    32,    33,    34,
      35,    56,    57,    -1,    -1,    60,     8,    -1,    63,    64,
      65,    66,    -1,    15,    16,    17,    18,    19,    20,     1,
      55,    -1,    -1,    58,    59,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,    56,    57,    -1,    -1,    -1,     1,
      -1,    63,    64,    65,    66,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    55,    -1,    -1,    58,    59,    -1,    -1,
      -1,    -1,    -1,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    59,    25,    26,
      27,    28,    -1,    -1,    -1,    32,    33,    34,    35,     1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    59,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,     1,    -1,    -1,    -1,    -1,    -1,
       7,     1,     9,    10,    11,    12,     1,     7,    -1,     9,
      -1,    -1,     7,    55,     9,    -1,    58,    59,    25,    26,
      27,    28,    -1,     1,    -1,    32,    33,    34,    35,     7,
       1,     9,    32,    -1,    -1,     1,     7,    32,     9,    -1,
      -1,     7,    -1,     9,    -1,    -1,    -1,    -1,    55,    17,
      18,    58,    59,    -1,    32,    55,    -1,    -1,    58,    59,
      55,    32,    -1,    58,    59,    -1,    32,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    55,    -1,    -1,
      58,    59,    -1,    -1,    55,    -1,    -1,    58,    59,    55,
      -1,    -1,    58,    59,     7,    -1,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,     7,    -1,     9,    10,    11,
      12,    -1,    25,    26,    27,    28,    29,    30,    -1,    32,
      33,    34,    35,    25,    26,    27,    28,    -1,    -1,    -1,
      32,    33,    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    58,    59,    15,    16,    17,
      18,    19,    20,    55,    -1,    -1,    58,    59,    15,    16,
      17,    18,    19,    20,    -1,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    -1,    57,
      16,    17,    18,    -1,    -1,    63,    64,    65,    66,    15,
      16,    17,    18,    19,    20,    -1,    63,    64,    65,    66,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    55,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36
  };

  const signed char
  parser::yystos_[] =
  {
       0,     3,     4,     5,     6,    84,     1,     7,     9,    10,
      11,    12,    25,    26,    27,    28,    29,    30,    32,    33,
      34,    35,    55,    58,    59,    60,    85,    86,    98,    99,
     101,   102,   106,   107,   108,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    85,    86,    98,   109,     7,     9,    12,    32,    36,
      37,    39,    41,    54,    63,    64,    65,    66,    85,    86,
      93,    94,    97,    99,   100,   101,     7,     9,    32,    85,
      86,    99,   103,     0,    60,     1,   108,     1,   100,     1,
     108,     1,   108,     1,   108,     1,   108,    98,   104,   104,
       1,   108,     1,    46,    47,    89,    90,     1,    89,    90,
       1,    89,    90,    61,    62,    50,    51,    52,    53,   102,
       1,    89,    60,    86,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    36,    60,    86,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,    60,    86,     1,   100,   100,   100,     1,    49,    87,
      88,    89,    90,    91,     7,     1,   100,     1,    89,    91,
       1,   100,    15,    16,    17,    18,    19,    20,    38,    40,
      42,    43,    44,    56,    57,    60,    63,    86,    92,    94,
      95,    96,    97,     1,   103,     1,   103,    15,    16,    17,
      18,    19,    20,    36,    60,    86,     8,    60,     1,     8,
      60,    13,    60,     1,    13,    14,    60,    31,    57,    98,
      57,    45,    46,   108,    60,    45,    46,    45,    46,    60,
      45,    46,    49,    87,    45,    46,    60,    45,    46,    49,
      87,     1,   108,     1,   108,     1,   108,     1,   108,    57,
      60,     1,   108,     1,   108,     1,   108,     1,   108,     1,
     108,     1,   108,     1,   108,     1,   108,     1,   108,     1,
     108,     1,   108,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,     8,    60,     8,    60,    45,    48,    90,
      45,    60,    45,    49,    87,   100,    45,    60,     1,   100,
       1,   100,     1,   100,     1,   100,     1,   100,     1,   100,
       1,    89,    91,     1,    89,    91,     1,    45,    49,    87,
      89,    90,     1,   100,     1,   100,     1,   100,     1,   100,
       8,    60,     1,     8,    60,     1,   103,     1,   103,     1,
     103,     1,   103,     1,   103,     1,   103,     1,   103,     8,
      60,    13,    60,    98,   105,   107,   108,   105,   108,   108,
       1,   108,   108,   108,   108,   108,   108,    90,    45,    46,
     108,   108,   108,   108,    90,    45,    46,   105,    45,    90,
      45,     8,    45,    60,    45,    60,    45,    90,    45,    60,
      45,    49,    87,     8,    60,    45,    46,   108,   108,    45,
      46,   108,   108,    45,    45,    90,    45,     1,   108,     1,
     108,     1,   108,     1,   108,    45
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    83,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      85,    86,    87,    87,    88,    88,    89,    89,    90,    91,
      91,    91,    91,    91,    92,    92,    92,    92,    92,    92,
      92,    92,    93,    93,    94,    94,    94,    94,    94,    95,
      95,    95,    95,    95,    96,    96,    96,    97,    97,    97,
      97,    97,    98,    99,    99,    99,    99,    99,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   101,   101,   101,   101,
     101,   101,   101,   102,   102,   102,   102,   102,   102,   102,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   104,   104,   104,   105,   105,   106,
     106,   107,   107,   107,   107,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     3,     2,     3,     3,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       1,     2,     1,     2,     0,     1,     0,     1,     1,     4,
       3,     3,     2,     2,     5,     4,     4,     3,     2,     3,
       3,     3,     1,     1,     1,     1,     2,     3,     3,     1,
       1,     2,     3,     3,     2,     3,     3,     2,     3,     3,
       1,     1,     1,     1,     2,     2,     1,     1,     1,     2,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     3,     3,
       2,     2,     3,     3,     1,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     4,     3,     4,     3,     3,
       4,     3,     1,     1,     3,     4,     3,     3,     4,     3,
       1,     1,     3,     4,     3,     3,     4,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     1,     2,     3,     1,     1,     1,
       1,     4,     4,     4,     3,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     4,     4,     6,     6,     5,     5,     6,     6,     3,
       4,     4,     2,     2,     6,     6,     5,     5,     4,     4,
       6,     6,     3,     4,     4,     2,     2,     2,     2,     4,
       4,     4,     3,     4,     4,     3,     2,     2,     1,     3,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     2,     3,     3,     3,     3,     3,     2,     2,     2,
       3,     3,     3,     3,     3,     1,     1
  };


#if TLYYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "\"LTL start marker\"",
  "\"LBT start marker\"", "\"SERE start marker\"",
  "\"BOOLEAN start marker\"", "\"opening parenthesis\"",
  "\"closing parenthesis\"", "\"(...) block\"", "\"{...} block\"",
  "\"{...}! block\"", "\"opening brace\"", "\"closing brace\"",
  "\"closing brace-bang\"", "\"or operator\"", "\"xor operator\"",
  "\"and operator\"", "\"short and operator\"", "\"implication operator\"",
  "\"equivalent operator\"", "\"until operator\"", "\"release operator\"",
  "\"weak until operator\"", "\"strong release operator\"",
  "\"sometimes operator\"", "\"always operator\"", "\"next operator\"",
  "\"strong next operator\"", "\"exists operator\"", "\"forall operator\"",
  "\",\"", "\"not operator\"", "\"X[.] operator\"", "\"F[.] operator\"",
  "\"G[.] operator\"", "\"star operator\"", "\"bracket star operator\"",
  "\"bracket fusion-star operator\"", "\"plus operator\"",
  "\"fusion-plus operator\"", "\"opening bracket for star operator\"",
  "\"opening bracket for fusion-star operator\"",
  "\"opening bracket for equal operator\"",
  "\"opening bracket for goto operator\"", "\"closing bracket\"",
  "\"closing !]\"", "\"number for square bracket operator\"",
  "\"unbounded mark\"", "\"separator for square bracket operator\"",
  "\"universal concat operator\"", "\"existential concat operator\"",
  "\"universal non-overlapping concat operator\"",
  "\"existential non-overlapping concat operator\"", "\"first_match\"",
  "\"atomic proposition\"", "\"concat operator\"", "\":\"",
  "\"constant true\"", "\"constant false\"", "\"end of formula\"",
  "\"negative suffix\"", "\"positive suffix\"", "\"SVA delay operator\"",
  "\"opening bracket for SVA delay operator\"", "\"##[+] operator\"",
  "\"##[*] operator\"", "'!'", "'&'", "'|'", "'^'", "'i'", "'e'", "'X'",
  "'F'", "'G'", "'U'", "'V'", "'R'", "'W'", "'M'", "'t'", "'f'", "$accept",
  "result", "emptyinput", "enderror", "OP_SQBKT_SEP_unbounded",
  "OP_SQBKT_SEP_opt", "error_opt", "sqbkt_num", "sqbracketargs",
  "gotoargs", "kleen_star", "starargs", "fstarargs", "equalargs",
  "delayargs", "atomprop", "booleanatom", "sere", "bracedsere",
  "parenthesedsubformula", "boolformula", "aplist",
  "maybequantifiedformula", "exists_or_forall", "quantifiedformula",
  "subformula", "lbtformula", YY_NULLPTR
  };
#endif


#if TLYYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   434,   434,   439,   444,   449,   454,   459,   461,   466,
     471,   476,   478,   483,   488,   493,   495,   500,   505,   510,
     513,   519,   525,   525,   526,   527,   528,   529,   531,   550,
     552,   554,   556,   558,   562,   564,   566,   568,   570,   572,
     574,   577,   582,   582,   584,   586,   588,   590,   593,   597,
     599,   601,   603,   607,   612,   614,   617,   622,   624,   627,
     631,   633,   636,   644,   645,   646,   650,   652,   655,   656,
     667,   668,   676,   678,   684,   688,   694,   696,   699,   701,
     704,   706,   708,   710,   712,   714,   716,   718,   720,   724,
     726,   737,   739,   750,   752,   762,   772,   782,   800,   817,
     834,   836,   848,   850,   867,   869,   872,   874,   878,   883,
     888,   893,   899,   908,   915,   917,   921,   926,   930,   935,
     943,   944,   952,   954,   958,   963,   968,   973,   979,   981,
     983,   985,   987,   989,   991,   993,   995,   997,   999,  1001,
    1003,  1005,  1007,  1009,  1012,  1014,  1016,  1019,  1019,  1022,
    1022,  1024,  1026,  1028,  1032,  1038,  1039,  1040,  1042,  1044,
    1046,  1048,  1050,  1052,  1054,  1056,  1058,  1060,  1062,  1064,
    1066,  1068,  1070,  1072,  1074,  1076,  1078,  1080,  1082,  1084,
    1086,  1088,  1094,  1101,  1104,  1108,  1112,  1116,  1119,  1122,
    1125,  1129,  1133,  1135,  1137,  1140,  1144,  1148,  1152,  1158,
    1166,  1169,  1172,  1175,  1179,  1183,  1185,  1187,  1189,  1191,
    1194,  1196,  1199,  1201,  1206,  1209,  1212,  1214,  1216,  1218,
    1220,  1222,  1225,  1227,  1231,  1237,  1241,  1247,  1251,  1254,
    1264,  1265,  1267,  1269,  1271,  1273,  1275,  1277,  1279,  1281,
    1283,  1285,  1287,  1289,  1291,  1293,  1295
  };

  void
  parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // TLYYDEBUG

  parser::symbol_kind_type
  parser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    67,     2,     2,     2,     2,    68,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      74,    75,     2,     2,     2,     2,     2,    80,     2,     2,
       2,     2,    78,     2,     2,    76,    77,    79,    73,     2,
       2,     2,     2,     2,    70,     2,     2,     2,     2,     2,
       2,    72,    82,     2,     2,    71,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    81,     2,     2,     2,
       2,     2,     2,     2,    69,     2,     2,     2,     2,     2,
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
      65,    66
    };
    // Last valid token kind.
    const int code_max = 321;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

} // tlyy
#line 4179 "parsetl.cc"

#line 1299 "parsetl.yy"


void
tlyy::parser::error(const location_type& location, const std::string& message)
{
  error_list.emplace_back(location, message);
}

namespace spot
{
  parsed_formula
  parse_infix_psl(const std::string& ltl_string,
		  environment& env,
		  bool debug, bool lenient)
  {
    parsed_formula result(ltl_string);
    flex_set_buffer(ltl_string,
		    tlyy::parser::token::START_LTL,
		    lenient);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  parsed_formula
  parse_infix_boolean(const std::string& ltl_string,
		      environment& env,
		      bool debug, bool lenient)
  {
    parsed_formula result(ltl_string);
    flex_set_buffer(ltl_string,
		    tlyy::parser::token::START_BOOL,
		    lenient);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  parsed_formula
  parse_prefix_ltl(const std::string& ltl_string,
		   environment& env,
		   bool debug)
  {
    parsed_formula result(ltl_string);
    flex_set_buffer(ltl_string,
		    tlyy::parser::token::START_LBT,
		    false);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  parsed_formula
  parse_infix_sere(const std::string& sere_string,
		   environment& env,
		   bool debug,
		   bool lenient)
  {
    parsed_formula result(sere_string);
    flex_set_buffer(sere_string,
		    tlyy::parser::token::START_SERE,
		    lenient);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  formula
  parse_formula(const std::string& ltl_string, environment& env)
  {
    parsed_formula pf = parse_infix_psl(ltl_string, env);
    std::ostringstream s;
    if (pf.format_errors(s))
      {
	parsed_formula pg = parse_prefix_ltl(ltl_string, env);
	if (pg.errors.empty())
	  return pg.f;
	else
	  throw parse_error(s.str());
      }
    return pf.f;
  }
}

// Local Variables:
// mode: c++
// End:
