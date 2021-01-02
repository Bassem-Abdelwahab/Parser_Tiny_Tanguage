#include "parser.h"
#include <string.h>
#include <iostream>

#define TERMINATING     0
#define NON_TERMINATING 1


struct context_grammar
{
  unsigned char type;
  union 
  {
    tokens terminating;
    EBNF nonterminating;
  };
};


syntax_tree_node::syntax_tree_node() :children()
{
  parentNode=NULL;
  adjacentNode=NULL;
  title = "";
  token.tokenType=tokens::OTHER;
  token.tokenValue=NULL;
}

syntax_tree_node::syntax_tree_node(syntax_tree_node *parent, std::string nodeTitle, token_type nodeToken):children()
{
  parentNode=parent;
  adjacentNode=NULL;
  title=nodeTitle;
  token.tokenType=nodeToken.tokenType;
  token.tokenValue=nodeToken.tokenValue;
}

syntax_tree_node::~syntax_tree_node()
{
  delete adjacentNode;
  for(unsigned int i = 0; i<children.size();i++)
    {
      delete children[i];
    }
}

int syntax_tree_node::width_of_children()
{
  int result=0 , sizeOfChildren=children.size();
  for(int i=0 ; i < sizeOfChildren ; i++)
    {
      result+= children[i]->width_of_children();
    }
  result+=sizeOfChildren;
  return result;
}

int syntax_tree_node::height_of_children()
{
  int result=0 , sizeOfChildren=children.size();
  if(sizeOfChildren!=0)
    {
      result = 0;
      int temp=0;
      for(int i=0 ; i < sizeOfChildren ; i++)
      {
        temp= children[i]->height_of_children();
        if(temp+1>result)result=temp+1;
      }
    }
  if(adjacentNode != NULL && adjacentNode->height_of_children() > result) result = adjacentNode->height_of_children();
  return result;
}

syntax_tree_node* syntax_tree_node::get_parentNode()
{
  return parentNode;
}

syntax_tree_node* syntax_tree_node::get_adjacentNode()
{
  return adjacentNode;
}

std::vector<syntax_tree_node*> syntax_tree_node::get_children()
{
  return children;
}

std::string syntax_tree_node::get_title()
{
  return title;
}

token_type syntax_tree_node::get_token()
{
  return token;
}

void syntax_tree_node::set_parentNode(syntax_tree_node* input)
{
  parentNode=input;
}

void syntax_tree_node::set_adjacentNode(syntax_tree_node* input)
{
  adjacentNode = input;
  input->set_parentNode(this);
}

void syntax_tree_node::add_child(syntax_tree_node* input)
{
  children.push_back(input);
  input->set_parentNode(this);
}

void syntax_tree_node::set_title(std::string input)
{
  title = input;
}

void syntax_tree_node::set_token(token_type input)
{
  token.tokenType=input.tokenType;
  token.tokenValue=input.tokenValue;
}

parser::parser() :listOfTokens()
{
  root=NULL;
}

parser::~parser()
{
  delete root;
}

grammar_return_type parser::match(tokens input)
{
  std::string tempError;
  grammar_return_type result;
  if(listOfTokens.empty())
  {
    result.evaluated = false;
    tempError = "Expecting ";
    tempError.append(tokenName[input]);
    tempError.append(" before end of tokens");
    result.error_message = tempError.c_str();
    result.childNode=NULL;
  }
  else if(listOfTokens.front().tokenType != input) 
  {
    result.evaluated = false;
    tempError = "Expecting ";
    tempError.append(tokenName[input]);
    tempError.append(" instead of ");
    tempError.append(tokenName[listOfTokens.front().tokenType]);
//    tempError.append(". Number of tokens left is ");
//    tempError.append(std::to_string(listOfTokens.size()));
    result.error_message = tempError.c_str();
    result.childNode=NULL;
  }
  else
  {
    result.evaluated = true;
    tempError = "Successful";
    result.error_message = tempError.c_str();
    //result.childNode=new syntax_tree_node(NULL,"",listOfTokens.front());
    listOfTokens.pop();
  }
  return result;
}

grammar_return_type parser::match(EBNF input)
{
  grammar_return_type result;
  switch (input)
  {
    case EBNF::PROGRAM:
      result=program();
      break;
    case EBNF::STMT_SEQUENCE:
      result=stmt_sequence();
      break;
    case EBNF::STATEMENT:
      result=statement();
      break;
    case EBNF::IF_STMT:
      result=if_stmt();
      break;
    case EBNF::REPEAT_STMT:
      result=repeat_stmt();
      break;
    case EBNF::ASSIGN_STMT:
      result=assign_stmt();
      break;
    case EBNF::READ_STMT:
      result=read_stmt();
      break;
    case EBNF::WRITE_STMT:
      result=write_stmt();
      break;
    case EBNF::EXP:
      result=exp();
      break;
    case EBNF::COMPARISON_OP:
      result=comparison_op();
      break;
    case EBNF::SIMPLE_EXP:
      result=simple_exp();
      break;
    case EBNF::ADDOP:
      result=addop();
      break;
    case EBNF::TERM:
      result=term();
      break;
    case EBNF::MULOP:
      result=mulop();
      break;
    case EBNF::FACTOR:
      result=factor();
      break;
  }
  return result;
}

grammar_return_type parser::program() 
{ //program -> stmt_sequence
  return stmt_sequence();
}

grammar_return_type parser::stmt_sequence()
{ //stmt_sequence -> statement{;statment}
  
  //initialization 
  grammar_return_type temp , result;
  syntax_tree_node *currentNode;
  currentNode = NULL;
  result.childNode = NULL;

  //match satatement
  result = statement();
  if(!result.evaluated) return result;
  currentNode =result.childNode;

  //match {;statement}
  while(!listOfTokens.empty() && listOfTokens.front().tokenType == tokens::SEMICOLON)
  {
    match(tokens::SEMICOLON); // No chance of error because check in while condition
    
    //match satatement
    temp = statement();
    result.evaluated = temp.evaluated;
    result.error_message=temp.error_message;
    currentNode->set_adjacentNode(temp.childNode);
    currentNode = temp.childNode;
    if(!result.evaluated) return result;
  }
  return result;  //if all went well return successfull
}

grammar_return_type parser::statement()
{ // statement -> if_stmt|repeat_stmt|assign_stmt|read_stmt|write_stmt
  grammar_return_type result;
  if(!listOfTokens.empty())
  {
    token_type temp = listOfTokens.front();
    switch (temp.tokenType)
      {
      case tokens::IF :
        result =if_stmt();
        break;
      case tokens::REPEAT :
        result =repeat_stmt();
        break;
      case tokens::ID :
        result =assign_stmt();
        break;
      case tokens::READ :
        result =read_stmt();
        break;
      case tokens::WRITE :
        result =write_stmt();
        break;
      default:
        result.evaluated = false;
        result.error_message = "unexpected Statment type";
        result.childNode=NULL;
      }
  }
  else
    {
      result.evaluated = false;
      result.error_message = "Expected Statment before end of tokens";
      result.childNode=NULL;
    }
  return result;
}

grammar_return_type parser::if_stmt()
{ //if_stmt -> if exp then stmt_sequence [else stmt_sequence] end
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;

  // if
  tempToken = listOfTokens.front();
  temp = match(tokens::IF);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"if", tempToken);

  // exp
  temp = exp();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  result.childNode->add_child(temp.childNode);
  if(!result.evaluated) return result;

  // then
  temp = match(tokens::THEN);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  
  // stmt_sequence
  temp = stmt_sequence();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  result.childNode->add_child(temp.childNode);
  if(!result.evaluated) return result;

  // [else statement]
  if(listOfTokens.front().tokenType == tokens::ELSE)
  {
    match(tokens::ELSE);

    // stmt_sequence
    temp = stmt_sequence();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    result.childNode->add_child(temp.childNode);
    if(!result.evaluated) return result;
  }

  // end
  temp = match(tokens::END);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;

  return result;
}

grammar_return_type parser::repeat_stmt()
{ //repeat_stmt -> repeat stmt_sequence until exp

  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;

  // repeat
  tempToken = listOfTokens.front();
  temp = match(tokens::REPEAT);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"repeat", tempToken);

  // stmt_sequence
  temp = stmt_sequence();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  result.childNode->add_child(temp.childNode);
  if(!result.evaluated) return result;

  // until
  temp = match(tokens::UNTIL);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  
  // exp
  temp = exp();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  result.childNode->add_child(temp.childNode);

  return result;
}

grammar_return_type parser::assign_stmt()
{ //assign_stmt -> identifier := exp
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;
  
  // identifier
  tempToken = listOfTokens.front();
  temp = match(tokens::ID);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"assign", tempToken);

  // :=
  temp = match(tokens::ASSIGN);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;

  // exp
  temp = exp();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  result.childNode->add_child(temp.childNode);

  return result;
}

grammar_return_type parser::read_stmt()
{ //read_stmt -> read identifier
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;
  
  // read
  tempToken = listOfTokens.front();
  temp = match(tokens::READ);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL , "read", tempToken);

  // identifier
  tempToken = listOfTokens.front();
  temp = match(tokens::ID);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode->set_token(tempToken);

  return result;
}

grammar_return_type parser::write_stmt()
  {//write_stmt -> write exp
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;
  
  // write
  tempToken = listOfTokens.front();
  temp = match(tokens::WRITE);
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"write", tempToken);

  // exp
  temp = exp();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  result.childNode->add_child(temp.childNode);

  return result;
}

grammar_return_type parser::exp()
{ //exp -> simple_exp [comparison_op simple_exp]

  //initialization
  grammar_return_type temp , result;
  result.childNode = NULL;

  // simple_exp
  temp = simple_exp();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode =temp.childNode;

  //[comparison_op simple_exp]
  if(!listOfTokens.empty()&&
    (listOfTokens.front().tokenType == tokens::LESSTHAN||
     listOfTokens.front().tokenType == tokens::EQUAL))
  {
    //comparison_op
    temp = comparison_op();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    if(!result.evaluated) return result;
    temp.childNode->add_child(result.childNode);
    result.childNode = temp.childNode;

    // simple_exp
    temp = simple_exp();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    if(!result.evaluated) return result;
    result.childNode->add_child(temp.childNode);
  }

  return result;
}

grammar_return_type parser::comparison_op()
{ // comparison_op -> <|=
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;

  tempToken = listOfTokens.front();
  if(!listOfTokens.empty()&& listOfTokens.front().tokenType == tokens::LESSTHAN)
  {
    temp = match(tokens::LESSTHAN);
  }
  else
  {
    temp = match(tokens::EQUAL);
  }
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"op", tempToken);

  return result;
}

grammar_return_type parser::simple_exp()
{ //simple_exp -> term{addop term}

  //initialization
  grammar_return_type temp , result;
  result.childNode = NULL;

  // term
  temp = term();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode =temp.childNode;

  //{addop term}
  while(!listOfTokens.empty()&&
       (listOfTokens.front().tokenType == tokens::PLUS||
        listOfTokens.front().tokenType == tokens::MINUS))
  {
    //addop
    temp = addop();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    if(!result.evaluated) return result;
    temp.childNode->add_child(result.childNode);
    result.childNode = temp.childNode;

    // term
    temp = term();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    if(!result.evaluated) return result;
    result.childNode->add_child(temp.childNode);
  }

  return result;
}

grammar_return_type parser::addop()
{ // addop -> +|-
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;

  tempToken = listOfTokens.front();
  if(!listOfTokens.empty()&& listOfTokens.front().tokenType == tokens::PLUS)
  {
    temp = match(tokens::PLUS);
  }
  else
  {
    temp = match(tokens::MINUS);
  }
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"op", tempToken);

  return result;
}

grammar_return_type parser::term()
{ //term -> factor{mulop factor}

  //initialization
  grammar_return_type temp , result;
  result.childNode = NULL;

  // factor
  temp = factor();
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode =temp.childNode;

  //{mulop factor}
  while(!listOfTokens.empty()&&
       (listOfTokens.front().tokenType == tokens::MULTIPLY||
        listOfTokens.front().tokenType == tokens::DIVIDE))
  {
    //mulop
    temp = mulop();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    if(!result.evaluated) return result;
    temp.childNode->add_child(result.childNode);
    result.childNode = temp.childNode;

    // factor
    temp = factor();
    result.evaluated = temp.evaluated;
    result.error_message = temp.error_message;
    if(!result.evaluated) return result;
    result.childNode->add_child(temp.childNode);
  }

  return result;
}

grammar_return_type parser::mulop()
{ // mulop -> *|/
  
  //initialization
  grammar_return_type temp , result;
  token_type tempToken;
  result.childNode = NULL;

  tempToken = listOfTokens.front();
  if(!listOfTokens.empty()&& listOfTokens.front().tokenType == tokens::MULTIPLY)
  {
    temp = match(tokens::MULTIPLY);
  }
  else
  {
    temp = match(tokens::DIVIDE);
  }
  result.evaluated = temp.evaluated;
  result.error_message = temp.error_message;
  if(!result.evaluated) return result;
  result.childNode = new syntax_tree_node(NULL ,"op", tempToken);

  return result;
}

grammar_return_type parser::factor()
{ // factor -> (exp)|number|identifier
  grammar_return_type temp ,result;
  if(!listOfTokens.empty())
  {
    token_type tempToken = listOfTokens.front();
    switch (tempToken.tokenType)
      {
      case tokens::OPENBRACKET :
        // (
        match(tokens::OPENBRACKET);

        // exp
        result = exp();
        if(!result.evaluated) return result;

        // )
        temp = match(tokens::CLOSEBRACKET);
        result.evaluated = temp.evaluated;
        result.error_message = temp.error_message;

        break;
      case tokens::NUM :
        result =match(tokens::NUM);
        result.childNode = new syntax_tree_node(NULL ,"const", tempToken);
        break;
      case tokens::ID :
        result =match(tokens::ID);
        result.childNode = new syntax_tree_node(NULL ,"id", tempToken);
        break;
      default:
        result.evaluated = false;
        result.error_message = "Expecting a number or variable";
        result.childNode=NULL;
      }
  }
  else
    {
      result.evaluated = false;
      result.error_message = "Expected number or variable before end of tokens";
      result.childNode=NULL;
    }
  return result;
}

parser_return_type parser::parse_tokens(std::queue<token_type> &listOfTokens)
{
  this->listOfTokens = listOfTokens;
  // std::cout<<this->listOfTokens.size() <<std::endl;
  grammar_return_type temp = program();
  if(root != NULL)
    {
      delete root;
    }
  root = temp.childNode;
  return {temp.evaluated,temp.error_message};
}

unsigned int parser::get_number_of_tokens()
{
  return listOfTokens.size();
}

syntax_tree_node* parser::get_parse_tree_root()
{
  return root;
}

