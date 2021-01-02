#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include <string>
#include <queue>
#include <vector>

enum EBNF {PROGRAM, STMT_SEQUENCE, STATEMENT, IF_STMT, REPEAT_STMT, ASSIGN_STMT, READ_STMT, WRITE_STMT, EXP, COMPARISON_OP, SIMPLE_EXP, ADDOP, TERM, MULOP, FACTOR};

class syntax_tree_node
{
private:
  syntax_tree_node *parentNode;
  syntax_tree_node *adjacentNode;
  std::vector<syntax_tree_node*> children;
  std::string title;
  token_type token;
public:
  syntax_tree_node();
  syntax_tree_node(syntax_tree_node *parent,std::string nodeTitle,token_type nodeToken);
  ~syntax_tree_node();
  int width_of_children();
  int height_of_children();
  syntax_tree_node* get_parentNode();
  syntax_tree_node* get_adjacentNode();
  std::vector<syntax_tree_node*> get_children();
  std::string get_title();
  token_type get_token();
  void set_parentNode(syntax_tree_node* input);
  void set_adjacentNode(syntax_tree_node* input);
  void add_child(syntax_tree_node* input);
  void set_title(std::string input);
  void set_token(token_type input);
};

struct parser_return_type
{
  bool evaluated;
  std::string error_message;
};

struct grammar_return_type
{
  bool evaluated;
  syntax_tree_node* childNode;
  std::string error_message;
};

class parser
{
private:
  syntax_tree_node *root;
  std::queue<token_type> listOfTokens;
  
  grammar_return_type program       ();// program       -> stmt_sequence
  grammar_return_type stmt_sequence ();// stmt_sequence -> statement{;statement}
  grammar_return_type statement     ();// statement     -> if_stmt|repeat_stmt|assing_stmt|read_stmt|write_stmt
  grammar_return_type if_stmt       ();// if_stmt       -> if exp then stmt_sequence [else stmt_sequence] end
  grammar_return_type repeat_stmt   ();// repeat_stmt   -> repeat stmt_sequence until exp
  grammar_return_type assign_stmt   ();// assign_stmt   -> identifier := exp
  grammar_return_type read_stmt     ();// read_stmt     -> read identifier
  grammar_return_type write_stmt    ();// write_stmt    -> write exp
  grammar_return_type exp           ();// exp           -> simple_exp [comparison_op simple_exp]
  grammar_return_type comparison_op ();// comparison_op -> <|=
  grammar_return_type simple_exp    ();// simple_exp    -> term {addop term}
  grammar_return_type addop         ();// addop         -> +|-    
  grammar_return_type term          ();// term          -> factor {mulop factor}
  grammar_return_type mulop         ();// mulop         -> *|/
  grammar_return_type factor        ();// factor        -> (exp)|number|identifier
  
  grammar_return_type match(tokens input);
  grammar_return_type match(EBNF input);
public:
  parser();
  ~parser();
  parser_return_type parse_tokens(std::queue<token_type> &listOfTokens);
  unsigned int get_number_of_tokens();
  syntax_tree_node* get_parse_tree_root();
};



#endif // PARSER_H
