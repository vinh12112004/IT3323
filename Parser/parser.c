/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 2.0
 */

#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    // printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  // assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  // assert("Program parsed!");
}

void compileBlock(void) {
  // assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  } 
  else compileBlock2();
  // assert("Block parsed!");
}

void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileConstDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileConstDecl();
  }
}

void compileConstDecl(void) {
  eat(TK_IDENT);
  eat(SB_EQ);
  compileConstant();
  eat(SB_SEMICOLON);
}

void compileTypeDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileTypeDecl();
  }
}

void compileTypeDecl(void) {
  eat(TK_IDENT);
  eat(SB_EQ);
  compileType();
  eat(SB_SEMICOLON);
}

void compileVarDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileVarDecl();
  }
}

void compileVarDecl(void) {
  eat(TK_IDENT);
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
  }
  if (lookAhead->tokenType == SB_COLON){
    eat(SB_COLON);
    compileType();
  }
  else {
    eat(SB_EQ);
    compileConstant();
  }
  
  eat(SB_SEMICOLON);
}

void compileSubDecls(void) {
  
  while (lookAhead->tokenType == KW_PROCEDURE ||
         lookAhead->tokenType == KW_FUNCTION) {
    // assert("Parsing subtoutines ....");
    if (lookAhead->tokenType == KW_PROCEDURE)
      compileProcDecl();
    else
      compileFuncDecl();
    // assert("Subtoutine parsed ....");
  }
}

void compileFuncDecl(void) {
  // assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  if (lookAhead->tokenType == SB_LPAR)
    compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();       // function body
  eat(SB_SEMICOLON);    // end of function declaration
  // assert("Function parsed ....");
}

void compileProcDecl(void) {
  // assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  if (lookAhead->tokenType == SB_LPAR)
    compileParams();
  eat(SB_SEMICOLON);
  compileBlock();       // procedure body
  eat(SB_SEMICOLON);    // end of procedure declaration
  // assert("Procedure parsed ....");
}

void compileUnsignedConstant(void) {
  switch (lookAhead->tokenType) {
    case TK_NUMBER:
      eat(TK_NUMBER);
      break;
    case TK_IDENT:
      eat(TK_IDENT);
      break;
    case TK_CHAR:
      eat(TK_CHAR);
      break;
    default:
      error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileConstant(void) {
  switch (lookAhead->tokenType) {
    case SB_PLUS:
      eat(SB_PLUS);
      compileUnsignedConstant();
      break;
    case SB_MINUS:
      eat(SB_MINUS);
      compileUnsignedConstant();
      break;
    case TK_NUMBER:
    case TK_IDENT:
    case TK_CHAR:
      compileUnsignedConstant();
      break;
    default:
      error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileConstant2(void) {
  // TODO
}

void compileType(void) {
  switch (lookAhead->tokenType) {
    case KW_INTEGER:
    case KW_CHAR:
      compileBasicType();
      break;

    case TK_IDENT:
      eat(TK_IDENT);
      break;

    case KW_ARRAY:
      eat(KW_ARRAY);
      eat(SB_LSEL);        // (.
      eat(TK_NUMBER);      // kích thước mảng
      eat(SB_RSEL);        // .)
      eat(KW_OF);          // OF
      compileType();       // kiểu phần tử
      break;

    default:
      error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileBasicType(void) {
  if (lookAhead->tokenType == KW_INTEGER)
    eat(KW_INTEGER);
  else if (lookAhead->tokenType == KW_CHAR)
    eat(KW_CHAR);
  else
    error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    compileParams2();
    eat(SB_RPAR);
  }
}

void compileParams2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileParam();
  }
}

void compileParam(void) {
  eat(TK_IDENT);
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
  }
  eat(SB_COLON);
  compileType();
}

void compileStatements(void) {
  compileStatement();
  compileStatements2();
}

void compileStatements2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
  // Kiểm tra nếu có statement tiếp theo mà thiếu dấu ;
  switch (lookAhead->tokenType) {
    case KW_END:
    case KW_ELSE:
    case SB_PERIOD:
      // Đây là FOLLOW tokens hợp lệ, không làm gì
      break;
    case TK_IDENT:
    case KW_CALL:
    case KW_BEGIN:
    case KW_IF:
    case KW_WHILE:
    case KW_FOR:
      // Có statement mới nhưng thiếu dấu ;
      missingToken(SB_SEMICOLON, lookAhead->lineNo, lookAhead->colNo);
      compileStatement();
      compileStatements2();
      break;
    default:
      // Token khác, để parser xử lý ở mức cao hơn
      break;
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileAssignSt(void) {
  // assert("Parsing an assign statement ....");
  eat(TK_IDENT);
  compileIndexes();          // optional array indexing like A(. i .)
  eat(SB_ASSIGN);            // :=  (SB_ASSIGN)
  compileExpression();
  // assert("Assign statement parsed ....");
}

void compileCallSt(void) {
  // assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  if (lookAhead->tokenType == SB_LPAR) {
    compileArguments();
  }
  // assert("Call statement parsed ....");
}

void compileGroupSt(void) {
  // assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  // assert("Group statement parsed ....");
}

void compileIfSt(void) {
  // assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
  // assert("If statement parsed ....");
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  // assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  // assert("While statement parsed ....");
}

void compileForSt(void) {
  // assert("Parsing a for statement ....");
  eat(KW_FOR);
  if (lookAhead->tokenType != SB_LPAR){
    eat(TK_IDENT);
    eat(SB_ASSIGN);         // :=
    compileExpression();
    if (lookAhead->tokenType == KW_TO){
      eat(KW_TO);
    }
    // else if (lookAhead->tokenType == KW_DOWNTO)
    //   eat(KW_DOWNTO);
    
    else
      error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    compileExpression();
    eat(KW_DO);
    compileStatement();
  }
  else {
    eat(SB_LPAR);
    eat(TK_IDENT);
    eat(SB_ASSIGN);         // :=
    compileExpression();
    if (lookAhead->tokenType == SB_SEMICOLON){
      eat(SB_SEMICOLON);
      compileCondition();
    }
    else
      error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    if(lookAhead->tokenType == SB_SEMICOLON){
      eat(SB_SEMICOLON);
      eat(TK_IDENT);
      eat(SB_ASSIGN);         // :=
      compileExpression();
    }
    else
      error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    eat(SB_RPAR);
    compileStatement();
  }
  
  // assert("For statement parsed ....");
}

void compileArguments(void) {
  eat(SB_LPAR);
  compileExpression();
  compileArguments2();
  eat(SB_RPAR);
}

void compileArguments2(void) {
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
  }
}

void compileCondition(void) {
  compileExpression();
  switch (lookAhead->tokenType) {
    case SB_EQ: case SB_NEQ:
    case SB_LE: case SB_LT:
    case SB_GE: case SB_GT:
      eat(lookAhead->tokenType);
      break;
    default:
      error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }
  compileExpression();
}

void compileCondition2(void) {
  // TODO
}

void compileExpression(void) {
  // assert("Parsing an expression");
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS) {
    eat(lookAhead->tokenType);   // optional leading + or -
  }
  compileTerm();
  compileExpression2();
  // assert("Expression parsed");
}

void compileExpression2(void) {
  while (lookAhead->tokenType == SB_PLUS ||
         lookAhead->tokenType == SB_MINUS) {
    eat(lookAhead->tokenType);
    compileTerm();
  }
}


void compileExpression3(void) {
  // TODO
}

void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  while (lookAhead->tokenType == SB_TIMES ||
         lookAhead->tokenType == SB_SLASH) {
    eat(lookAhead->tokenType);
    compileFactor();
  }
}

void compileFactor(void) {
  switch (lookAhead->tokenType) {
    case TK_NUMBER:
      eat(TK_NUMBER);
      break;
    case TK_CHAR:
      eat(TK_CHAR);
      break;
    case TK_IDENT:
      eat(TK_IDENT);
      if (lookAhead->tokenType == SB_LSEL) {     // (.
        compileIndexes(); //varible 
      } else if (lookAhead->tokenType == SB_LPAR) { // function call in expression
        compileArguments();
      }
      break;
    case SB_LPAR:
      eat(SB_LPAR);
      compileExpression();
      eat(SB_RPAR);
      break;
    default:
      error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {   // SB_LSEL stands for (.
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);                            // .)
  }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
