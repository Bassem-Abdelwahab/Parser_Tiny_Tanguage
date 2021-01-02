#include "scanner.h"

char tokenName[][TOKENS_SIZE] = {"PLUS", "MINUS", "MULT", "DIV", "EQUAL", "LESSTHAN",
                        "OPENBRACKET", "CLOSEBRACKET", "SEMICOLON",
                        "IF", "THEN", "ELSE", "END", "REPEAT", "UNTIL", "READ", "WRITE",
                        "ASSIGN", "NUMBER", "IDENTIFIER", "OTHER"};

char reserved[][RESERVED_SIZE] = {"if", "then", "else", "end", "repeat", "until", "read", "write"};

//enum class wordState{I, E, R, T, U, W,
//                     IF, EN, EL, RE, TH, UN, WR,
//                     END, ELS, REP, REA, THE, UNT, WRI,
//                     ELSE, REPE, READ, THEN, UNTI, WRIT,
//                     REPEA, UNTIL, WRITE,
//                     REPEAT, ID, OTHER};


                             // 0    1    2    3    4    5    6    7    8
char symbols[SYMBOLS_SIZE] = { '+', '-', '*', '/', '=', '<', '(', ')', ';'};

char whiteSpaces[] = { ' ', '\t', '\n', '\r', '\0', EOF};

token_type::~token_type()
{
//  delete[] tokenValue;
}

tokens search_token_by_name(std::string input)
{
  for(int i=0;i<TOKENS_SIZE ; i++)
    {
      if( strcmp(input.c_str(),tokenName[i])==0)return (tokens)i;
    }
  return tokens::OTHER;
}

bool is_digit(const char input)
{
  return (input>= '0' && input<= '9');
}

bool is_letter(const char input)
{
  return ((input>= 'a' && input<= 'z')||
          (input>= 'A' && input<= 'Z'));
}

bool is_special(const char input , int &location)
{
  bool result = false;
  location=SYMBOLS_SIZE;
  for(int i=0; i<SYMBOLS_SIZE; i++)
    {
      if(input == symbols[i])
        {
          result = true;
          location = i;
          i=SYMBOLS_SIZE;   //to break;
        }
    }
  return result;
}
bool is_white_space(const char input)
{
  bool result = false;
  int i=0;
  do
    {
      i++;
      if(input == whiteSpaces[i])
        {
          result = true;
          break;
        }
    }while(whiteSpaces[i]!= '\0');
  return result;
}
bool is_reserved(const char token[] , int &location)
{
  int checked ,i;
  bool flag=true;
  for(checked=0 ; checked<RESERVED_SIZE ;checked++)
  {
    for(i=0 ; flag && token[i]!='\0' && reserved[checked][i]!='\0' ;i++)
      {
        if(token[i]!=reserved[checked][i])
          {
            flag=false;
          }
      }
    if(flag && token[i]=='\0')break;  //true reserved word
    else flag=true;                   //reset flag for next check
  }
  location=checked;
  return (checked!=RESERVED_SIZE);
}

scanner::scanner()
{
  state= START;
  resulting_token= OTHER;
  nextChar= CONSUMED;
  reservedCandidate = false;
  for(token_value_index=0; token_value_index<MAX_ID_SIZE; token_value_index++) token_value[token_value_index]=0;
  token_value_index=0;
}

void scanner::consume_character(bool dontIgnore)
{
  if(dontIgnore)
  {
    token_value[token_value_index]=nextChar;
    token_value_index++;
    token_value[token_value_index]='\0';    //terminating string
  }
  nextChar=CONSUMED;
}

states scanner::evaluate_current_state()
{
  int location=0;
  switch(state)
    {
    case START:
        if(nextChar=='{')
          {
            state = INCOMMMENT;
          }
        else if(nextChar==':')
          {
            state = INASSIGN;
            consume_character(true);
          }
        else if(is_digit(nextChar))
          {
             state = INNUM;
             consume_character(true);
          }
        else if(is_letter(nextChar))
          {
            state = INID;
            consume_character(true);
            reservedCandidate=true;
          }
        else if(is_special(nextChar,location))
          {
            state = INSYMBOL;
            consume_character(true);
            resulting_token= (tokens)location;
          }
        else if(!is_white_space(nextChar))
          {
            state=DONE;
            consume_character(true);
            resulting_token = OTHER;
          }
        else
          {
            consume_character(false);
          }
    break;

    case INCOMMMENT:
        if(nextChar == '}') state= START;
    break;

    case INNUM:
        if(is_digit(nextChar))
          {
            consume_character(true);
          }
        else if(is_letter(nextChar))
          {
            state=DONE;
            resulting_token=OTHER;
          }
        else
          {
            state=DONE;
            resulting_token=NUM;
          }
    break;

    case INID:
      if(is_letter(nextChar))
      {
        consume_character(true);
      }
      else if(is_digit(nextChar))
      {
        reservedCandidate=false;
        consume_character(true);
      }
      else
      {
        state=DONE;
        if(reservedCandidate==true)
          {
            int condition = 0;
            if(is_reserved(token_value,condition))
              {
                resulting_token=(tokens)(SYMBOLS_SIZE+condition);
              }
            else resulting_token= ID;
          }
        else resulting_token=ID;
      }

    break;

    case INASSIGN:
        if(nextChar == '=')
          {
            consume_character(true);
            resulting_token=ASSIGN;
          }
        else
          {
            resulting_token=OTHER;
          }
        state = DONE;
    break;

    case DONE:
        state = START;
        resulting_token=OTHER;
        token_value_index=0;
    break;

    case INSYMBOL:
        state = DONE;
    break;

    default:
    break;
    }
  return state;
}

evaluation scanner::insert_next_char(char next)
//this function takes the next character and evaluates the current state based on it
{
  if(nextChar != CONSUMED)
  {
    evaluate_current_state();
  }
  nextChar = next;
  evaluate_current_state();
  evaluation result={(state==DONE), resulting_token ,token_value};
  if(result.tokenEvaluated)                    //DONE state
    {
      state = START;
      token_value_index=0;
    }
  return result;
}

states scanner::get_state(void)
{
  return state;
}
