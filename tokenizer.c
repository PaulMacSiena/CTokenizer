/*
  tokenizer.c 
  Adapted from front.c by Dr. Jim Teresco at Siena College
  Many additions made to implement ability to tokenize "Little C" by
  Paul Macfarlane and Cheryl McClean 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/* define the longest allowable lexeme length */
#define MAX_LEXEME_LENGTH 100

/* Many programs that do low-level work (like this lexical analyzer)
   use global variables used to avoid passing around lots of
   parameters - think of them like the instance variables of a class.
   We could encapsulate them in a struct or at least declare them as
   static to restrict them to file scope to better restrict
   access.  */

int charClass;  // will hold one of the character class constants defined below
char lexeme[MAX_LEXEME_LENGTH];  // last lexeme processed
char nextChar;  // most recently read char from input
int lexLen;     // how long is current lexeme?
int nextToken;  // most recently processed token
FILE *in_fp;    // FILE pointer to input file

/* Function prototypes -- very simple since they communicate through globals */
void addChar();
void getChar();
void getNonBlank();
int lex();

/* Character classes */
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

/* Token codes */
#define INT_LIT 10
#define IDENT 11
#define FLOAT_LIT 12
#define NEG_INT_LIT 13
#define NEG_FLOAT_LIT 14
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define EQUAL_OP 25
#define LESSTHAN_OP 26
#define GREATERTHAN_OP 27
#define LTEQ_OP 28
#define GTEQ_OP 29
#define NOTEQ_OP 30
#define AND_OP 31
#define OR_OP 32 
#define MOD_OP 33 
#define LEFT_PAREN 40
#define RIGHT_PAREN 41
#define SCOLON_TERM 42 
#define LEFT_BRACKET 43 
#define RIGHT_BRACKET 44 
#define OPEN_BRACKET 45
#define CLOSE_BRACKET 46
#define COLON 47
#define IF_KEY 50 
#define ELSE_KEY 51
#define FOR_KEY 52
#define FLOAT_KEY 53
#define COMMA 60
#define QUOTE 61
#define PERIOD 62
#define WHILE_KEY 63
#define VOID_KEY 90
#define MAIN_KEY 91 
/* main driver */
int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    exit(1);
  }

  /* open the input data file and process contents */
  if ((in_fp = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: cannot open input file %s for reading\n", 
	    argv[0], argv[1]);
    exit(1);
  }

  /* we have our input file open, we now proceed with lexical analysis */
  /* initialize things by getting the first char */
  getChar();
  do {
    lex(); /* process (and in this case, print info about) next lexeme */
  } while (nextToken != EOF);

  fclose(in_fp);
  return 0;
}

/* function to return a printable representation of the defined
   token codes above, given the numeric code */
/* for output purposes only, not part of the lexer */
char *tokenCode(int code) {

  switch (code) {

  case INT_LIT:
    return "INT_LIT";

  case IDENT:
    return "IDENT";

  case FLOAT_LIT:
    return "FLOAT_LIT";
  
  case NEG_INT_LIT:
    return "NEG_INT_LIT";

  case NEG_FLOAT_LIT:
    return "NEG_FLOAT_LIT";

  case ASSIGN_OP:
    return "ASSIGN_OP";

  case ADD_OP:
    return "ADD_OP";

  case SUB_OP:
    return "SUB_OP";

  case MULT_OP:
    return "MULT_OP";

  case DIV_OP:
    return "DIV_OP";

  case EQUAL_OP:
    return "EQUAL_OP";

  case LESSTHAN_OP:
    return "LESSTHAN_OP";

  case GREATERTHAN_OP:
    return "GREATERTHAN_OP";

  case LTEQ_OP:
    return "LTEQ_OP";

  case GTEQ_OP:
    return "GTEQ_OP";
  
  case NOTEQ_OP:
    return "NOTEQ_OP";
 
  case AND_OP:
    return "AND_OP";
   
  case OR_OP:
    return "OR_OP";
  
  case MOD_OP:
    return "MOD_OP";

  case LEFT_PAREN:
    return "LEFT_PAREN";

  case RIGHT_PAREN:
    return "RIGHT_PAREN";
    
  case SCOLON_TERM:
    return "SCOLON_TERM";
  
  case LEFT_BRACKET:
    return "LEFT_BRACKET";

  case RIGHT_BRACKET:
    return "RIGHT_BRACKET";

  case IF_KEY:
    return "IF_KEY";
    
  case ELSE_KEY:
    return "ELSE_KEY";

  case FOR_KEY:
    return "FOR_KEY";

  case WHILE_KEY:
    return "WHILE_KEY";
 
  case VOID_KEY:
    return "VOID_KEY";

  case MAIN_KEY:
    return "MAIN_KEY";

  case FLOAT_KEY:
    return "FLOAT_KEY";

  case COMMA:
    return "COMMA";

  case OPEN_BRACKET:
    return "OPEN_BRACKET";
 
  case CLOSE_BRACKET:
    return "CLOSE_BRACKET";

  case QUOTE:
    return "QOUTE";

  case COLON:
    return "COLON";

  case PERIOD:
    return "PERIOD";

  default:
    return "UNKNOWN";
  }
}

/*
  keyWordLookup - a function to look up potential keywords
 */
int keywordLookup(char* word){
  if (strcmp(word, "if") == 0){
    nextToken = IF_KEY;
  }
  else if (strcmp(word, "else") == 0){
    nextToken = ELSE_KEY;
  }
  else if (strcmp(word, "for") == 0){
    nextToken = FOR_KEY;
  }
  else if (strcmp(word, "while") == 0){
    nextToken = WHILE_KEY;
  }
  else if (strcmp(word, "main") == 0){
    nextToken = MAIN_KEY;
  }
  else if (strcmp(word, "void") == 0){
    nextToken = VOID_KEY;
  }
  else if (strcmp(word, "float") == 0){
    nextToken = FLOAT_KEY;
  }
  else {
    nextToken = UNKNOWN;
  }
  return nextToken;
}

/* lookup - a function to look up operators and parentheses -
   note that this both returns one of the token constants defined
   above and sets the nextToken global variable.
 */
int lookup(char ch) {

  switch (ch) {
  case '>':
    addChar();
    getChar();
    if (nextChar == '='){
      nextToken = GTEQ_OP;
      addChar();
    }
    else{
      nextToken = GREATERTHAN_OP;
    }
    break;
  case '<':
    addChar();
    getChar();
    nextToken = LESSTHAN_OP;
    if (nextChar == '='){
      nextToken= LTEQ_OP;
      addChar();
    }
    else{
      nextToken = LESSTHAN_OP;
    }
    break;
  case '(':
    addChar();
    nextToken = LEFT_PAREN;
    break;
  case ')':
    addChar();
    nextToken = RIGHT_PAREN;
    break;
  case '+':
    addChar();
    nextToken = ADD_OP;
    break;
  case '-':
    addChar();
    nextToken = SUB_OP;
    break;
  case '*':
    addChar();
    nextToken = MULT_OP;
    break;
  case '/':
    addChar();
    nextToken = DIV_OP;
    break;
  case '=':
    addChar();
    getChar();
    if (nextChar=='='){
      nextToken = EQUAL_OP;
      addChar();
    }
    else {
      nextToken = ASSIGN_OP;
    }
    break;
  case '!':
    addChar();
    getChar();
    if (nextChar=='='){
      nextToken = NOTEQ_OP;
      addChar();
    }
    else {
      nextToken = UNKNOWN;
    }
    break;
  case '&':
    addChar();
    getChar();
    if (nextChar == '&'){
      nextToken = AND_OP;
      addChar();
    }
    else {
      nextToken= UNKNOWN;
    }      
    break;
  case '|':
    addChar();
    getChar();
    if (nextChar == '|'){
      nextToken = OR_OP;
      addChar();
    }
    else {
      nextToken= UNKNOWN;                                                                                                                                                         
    }
    break;
  case '%':
    addChar();
    nextToken = MOD_OP;
    break;
  case ';':
    addChar();
    nextToken = SCOLON_TERM;
    break;
  case '[':
    addChar();
    nextToken = LEFT_BRACKET;
    break;
  case ']':
    addChar();
    nextToken = RIGHT_BRACKET;
    break;
  case ',':
    addChar();
    nextToken = COMMA;
    break;
  case '{':
    addChar();
    nextToken = OPEN_BRACKET;
    break;
  case '}':
    addChar();
    nextToken = CLOSE_BRACKET;
    break;
  case '"':
    addChar();
    nextToken = QUOTE;
    break;
  case ':':
    addChar();
    nextToken = COLON;
    break;
  case '.':
    addChar();
    nextToken = PERIOD;
    break;
  default:
    addChar();
    nextToken = EOF;
    break;
  }
  return nextToken;
}

/* addChar - a function to add next char to lexeme being processed */
void addChar(){

  if (lexLen <= (MAX_LEXEME_LENGTH-2)) {
    lexeme[lexLen++] = nextChar;
    lexeme[lexLen] = '\0';
  } 
  else {
    fprintf(stderr, "Error - lexeme too long: %s \n", lexeme);
  }
}

/* getChar - a function get the next char of input and determine
   its character class 
   sets global variables nextChar and charClass.
*/
void getChar(){

  if ((nextChar = getc(in_fp)) != EOF) {
    if (isalpha(nextChar))
      charClass = LETTER;
    else if (isdigit(nextChar))
      charClass = DIGIT;
    else 
      charClass = UNKNOWN;
  } 
  else {
    charClass = EOF;
  }
}

/* getNonBlank - a function to call getChar until it returns a
   non-whitespace character
   sets global variables nextChar and charClass via its calls to getChar.
*/
void getNonBlank(){

  while (isspace(nextChar))
    getChar();
}

/* lex - a simple lexical analyzer for arithmetic expressions.
   One call parses one lexeme.  And here, prints out some info about it.
 */
int lex(){

  lexLen = 0;
  getNonBlank();
  switch (charClass) {
  case LETTER: //take care of keywords
    /* parse identifiers (would also match keywords if we had any) */
    /* identifiers here start with a letter then can be followed by
       any number of letters and numbers */
    addChar();  // build lexeme
    getChar();  // move to next char of input, see if still part of an IDENT
    while (charClass == LETTER || charClass == DIGIT){
      addChar(); // continue to build lexeme
      getChar(); // and on to next char
    }
    keywordLookup(lexeme);
    if (nextToken == 99){ //if != 99, if is a keyword
      nextToken = IDENT;
    }
    break;
    
  case DIGIT:
    /* parse integer literals - starts with a digit, continue until we
       encounter something else */
    addChar();
    getChar();
    while (charClass == DIGIT ){
      addChar();
      getChar();
    }
    if (nextChar == '.'){
      nextToken = FLOAT_LIT;
      addChar();
      getChar();
      while (charClass == DIGIT){ 
	addChar();
	getChar();
      }
    }
    else{
      nextToken = INT_LIT;
    }
    break;

  case UNKNOWN:
    /* Parentheses and operators -- all are single character in our
       simple language.   So they become tokens in their own right, if
       recognized by lookup, become EOF otherwise.
    */
    
    //take char of negative int LITS
    if (nextChar == '-'){
      addChar();
      getChar();
      while (charClass == DIGIT ){
	addChar();
	getChar();
      }
      if (nextChar == '.'){
	nextToken = NEG_FLOAT_LIT;
	addChar();
	getChar();
	while (charClass == DIGIT){
	  addChar();
	  getChar();
	}
      }
      else{
	nextToken = NEG_INT_LIT;
      }
      break;
    }
    if (nextChar =='.'){
      addChar();
      getChar();
      while (charClass == DIGIT){ 
        addChar();
        getChar();
	nextToken = FLOAT_LIT;
      }
      break;
    }
    lookup(nextChar);
    getChar();
    break;


  case EOF:
    /* EOF -- return a special EOF token */
    nextToken = EOF;
    lexeme[0] = 'E';
    lexeme[1] = 'O';
    lexeme[2] = 'F';
    lexeme[3] = '\0';
    break;

  } /* end of switch */

  printf("Next token is: %2d (%12s), next lexeme is %s\n",
	 nextToken, tokenCode(nextToken), lexeme);
  return nextToken;
} /* end lex */
