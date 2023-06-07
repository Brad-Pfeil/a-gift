#include "CompilerParser.h"


/**
 * Constructor for the CompilerParser
 * @param tokens A linked list of tokens to be parsed
 */
CompilerParser::CompilerParser(std::list<Token*> tokens) {
    while(tokens.front() != nullptr) {
        tokensIn.push_back(tokens.front());
        tokens.pop_front();
    }
}

/**
 * Generates a parse tree for a single program
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileProgram() {

    ParseTree* result = new ParseTree("class", "");

    result->addChild(mustBe("keyword", "class"));
    result->addChild(mustBe("identifier", "Main"));
    result->addChild(mustBe("symbol", "{"));
    result->addChild(mustBe("symbol", "}"));

    return result;
}

/**
 * Generates a parse tree for a single class
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileClass() {

    ParseTree* result = new ParseTree("class", "");

    result->addChild(mustBe("keyword", "class"));
    result->addChild(mustBe("identifier", "Main"));
    result->addChild(mustBe("symbol", "{"));

    //check if there are any classVarDecs
    if(have("keyword", "static") || have("keyword", "field")) {
        result->addChild(compileClassVarDec());
    }

    //check if there are any subroutines
    if(have("keyword", "constructor") || have("keyword", "function") || have("keyword", "method")) {
        result->addChild(compileSubroutine());
    }

    result->addChild(mustBe("symbol", "}"));

    return result;
}

/**
 * Generates a parse tree for a static variable declaration or field declaration
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileClassVarDec() {

    ParseTree* result = new ParseTree("classVarDec", "");

    result->addChild(new Token("keyword", current()->getValue()));
    next();
    result->addChild(new Token("keyword", current()->getValue()));
    next();

    //check if there are more variables
    while(true) {
        if(current()->getType() == "identifier") {
            result->addChild(current());
            next();
        } else if (have("symbol", ",")) {
            result->addChild(current());
            next();
        } else if (have("symbol", ";")) {
            result->addChild(current());
            next();
            break;
        } else {
            throw ParseException();
        }
    }

    return result;
}

/**
 * Generates a parse tree for a method, function, or constructor
 * @return a ParseTree
 */

ParseTree* CompilerParser::compileSubroutine() {

    ParseTree* result = new ParseTree("subroutine", "");

    result->addChild(new Token("keyword", current()->getValue()));
    next();

    auto value = current()->getValue();
    if(value == "void") {
        result->addChild(new Token("keyword", current()->getValue()));
        next();
    } else if(value == "int" || value == "char" || value == "boolean") {
        result->addChild(new Token("keyword", current()->getValue()));
        next();
    } else if(current()->getType() == "identifier") {
        result->addChild(new Token("identifier", current()->getValue()));
        next();
    } else {
        throw ParseException();
    }

    result->addChild(new Token("identifier", current()->getValue()));
    next();

    result->addChild(mustBe("symbol", "("));
    result->addChild(compileParameterList());
    result->addChild(mustBe("symbol", ")"));
    result->addChild(compileSubroutineBody());

    return result;
}

/**
 * Generates a parse tree for a subroutine's parameters
 * @return a ParseTree
 */

ParseTree* CompilerParser::compileParameterList() {

    ParseTree* result = new ParseTree("parameterList", "");

    //check if there are any parameters
    if(have("symbol", ")")) {
        return result;
    }

    //add the first parameter
    result->addChild(new Token("keyword", current()->getValue()));
    next();
    result->addChild(new Token("identifier", current()->getValue()));
    next();

    //check if there are more parameters
    while(have("symbol", ",")) {
        result->addChild(mustBe("symbol", ","));
        result->addChild(new Token("keyword", current()->getValue()));
        next();
        result->addChild(new Token("identifier", current()->getValue()));
        next();
    }

    return result;
}

/**
 * Generates a parse tree for a subroutine's body
 * @return a ParseTree
 */

ParseTree* CompilerParser::compileSubroutineBody() {

    ParseTree* result = new ParseTree("subroutineBody", "");

    result->addChild(mustBe("symbol", "{"));

    //check if there are any varDecs
    if(have("keyword", "var")) {
        result->addChild(compileVarDec());
    }

    result->addChild(compileStatements());

    result->addChild(mustBe("symbol", "}"));

    return result;
}

/**
 * Generates a parse tree for a subroutine variable declaration
 * @return a ParseTree
 */

ParseTree* CompilerParser::compileVarDec() {
    
    ParseTree* result = new ParseTree("varDec", "");

    result->addChild(mustBe("keyword", "var"));
    result->addChild(new Token("keyword", current()->getValue()));
    next();
    result->addChild(new Token("identifier", current()->getValue()));
    next();

    //check if there are more variables
    while(have("symbol", ",")) {
        result->addChild(mustBe("symbol", ","));
        result->addChild(new Token("identifier", current()->getValue()));
        next();
    }

    result->addChild(mustBe("symbol", ";"));

    return result;
}

/**
 * Generates a parse tree for a series of statements
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileStatements() {

    ParseTree* result = new ParseTree("statements", "");

    //check if there are any statements
    if(!have("keyword", "let") && !have("keyword", "if") && !have("keyword", "while") && !have("keyword", "do") && !have("keyword", "return")) {
        return result;
    }

    //add the first statement
    if(have("keyword", "let")) {
        result->addChild(compileLet());
    } else if(have("keyword", "if")) {
        result->addChild(compileIf());
    } else if(have("keyword", "while")) {
        result->addChild(compileWhile());
    } else if(have("keyword", "do")) {
        result->addChild(compileDo());
    } else if(have("keyword", "return")) {
        result->addChild(compileReturn());
    }

    //check if there are more statements
    while(have("keyword", "let") || have("keyword", "if") || have("keyword", "while") || have("keyword", "do") || have("keyword", "return")) {
        if(have("keyword", "let")) {
            result->addChild(compileLet());
        } else if(have("keyword", "if")) {
            result->addChild(compileIf());
        } else if(have("keyword", "while")) {
            result->addChild(compileWhile());
        } else if(have("keyword", "do")) {
            result->addChild(compileDo());
        } else if(have("keyword", "return")) {
            result->addChild(compileReturn());
        }
    }

    return result;
}

/**
 * Generates a parse tree for a let statement
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileLet() {

    ParseTree* result = new ParseTree("letStatement", "");

    result->addChild(mustBe("keyword", "let"));
    result->addChild(new Token("identifier", current()->getValue()));
    next();

    //check if there is an array index
    if(have("symbol", "[")) {
        result->addChild(mustBe("symbol", "["));
        result->addChild(compileExpression());
        result->addChild(mustBe("symbol", "]"));
    }

    result->addChild(mustBe("symbol", "="));
    result->addChild(compileExpression());
    result->addChild(mustBe("symbol", ";"));

    return result;

}

/**
 * Generates a parse tree for an if statement
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileIf() {

    ParseTree* result = new ParseTree("ifStatement", "");

    result->addChild(mustBe("keyword", "if"));
    result->addChild(mustBe("symbol", "("));
    result->addChild(compileExpression());
    result->addChild(mustBe("symbol", ")"));
    result->addChild(mustBe("symbol", "{"));
    result->addChild(compileStatements());
    result->addChild(mustBe("symbol", "}"));

    //check if there is an else statement
    if(have("keyword", "else")) {
        result->addChild(mustBe("keyword", "else"));
        result->addChild(mustBe("symbol", "{"));
        result->addChild(compileStatements());
        result->addChild(mustBe("symbol", "}"));
    }

    return result;
}

/**
 * Generates a parse tree for a while statement
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileWhile() {

    ParseTree* result = new ParseTree("whileStatement", "");

    result->addChild(mustBe("keyword", "while"));
    result->addChild(mustBe("symbol", "("));
    result->addChild(compileExpression());
    result->addChild(mustBe("symbol", ")"));
    result->addChild(mustBe("symbol", "{"));
    result->addChild(compileStatements());
    result->addChild(mustBe("symbol", "}"));

    return result;
}

/**
 * Generates a parse tree for a do statement
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileDo() {

    ParseTree* result = new ParseTree("doStatement", "");

    result->addChild(mustBe("keyword", "do"));
    result->addChild(compileExpression());
    result->addChild(mustBe("symbol", ";"));

    return result;
}

/**
 * Generates a parse tree for a return statement
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileReturn() {

    ParseTree* result = new ParseTree("returnStatement", "");

    result->addChild(mustBe("keyword", "return"));

    //check if there is an expression
    if(!have("symbol", ";")) {
        result->addChild(compileExpression());
    } else {
        result->addChild(new Token("symbol", ";"));
    }

    return result;
}

/**
 * Generates a parse tree for an expression
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileExpression() {

    ParseTree* result = new ParseTree("expression", "");

    //check if the expression is just a term
    if(have("keyword", "skip")) {
        result->addChild(new Token("keyword", current()->getValue()));
        next();
        return result;
    } else if(have("keyword", "integerConstant") || have("keyword", "stringConstant") || have("keyword", "keyword") || have("keyword", "identifier")) {
        result->addChild(compileTerm());
        return result;
    } else if(have("symbol", "(") || have("symbol", "-") || have("symbol", "~")) {
        result->addChild(compileTerm());
        return result;
    } else if(have("symbol", ";")) {
        return result;
    }

    result->addChild(compileTerm());

    //check if there are more terms
    while(have("symbol", "+") || have("symbol", "-") || have("symbol", "*") || have("symbol", "/") || have("symbol", "&") || have("symbol", "|") || have("symbol", "<") || have("symbol", ">") || have("symbol", "=")) {
        result->addChild(new Token("symbol", current()->getValue()));
        next();
        result->addChild(compileTerm());
    }

    return result;
}

/**
 * Generates a parse tree for an expression term
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileTerm() {
    ParseTree* result = new ParseTree("term", "");

    //check if the term is an integer constant
    if(have("integerConstant", "")) {
        result->addChild(new Token("integerConstant", current()->getValue()));
        next();
        return result;
    } else if(have("stringConstant", "")) {
        result->addChild(new Token("stringConstant", current()->getValue()));
        next();
        return result;
    } else if(have("keyword", "true") || have("keyword", "false") || have("keyword", "null") || have("keyword", "this")) {
        result->addChild(new Token("keyword", current()->getValue()));
        next();
        return result;
    } else if(have("identifier", "")) {
        result->addChild(new Token("identifier", current()->getValue()));
        next();
        return result;
    } else if(have("symbol", "(")) {
        result->addChild(mustBe("symbol", "("));
        result->addChild(compileExpression());
        result->addChild(mustBe("symbol", ")"));
        return result;
    } else if(have("symbol", "-") || have("symbol", "~")) {
        result->addChild(new Token("symbol", current()->getValue()));
        next();
        result->addChild(compileTerm());
        return result;
    }

    return NULL;
}

/**
 * Generates a parse tree for an expression list
 * @return a ParseTree
 */
ParseTree* CompilerParser::compileExpressionList() {

    ParseTree* result = new ParseTree("expressionList", "");

    //check if the expression list is empty
    if(have("symbol", ")")) {
        return result;
    } else {
        result->addChild(compileExpression());
    }

    //check if there are more expressions
    while(have("symbol", ",")) {
        result->addChild(mustBe("symbol", ","));
        result->addChild(compileExpression());
    }

    return result;
}

/**
 * Advance to the next token
 */
void CompilerParser::next(){
    if(tokensIn.front() != nullptr) {
        tokensIn.pop_front();
    }
    return;
}

/**
 * Return the current token
 * @return the Token
 */
Token* CompilerParser::current(){
    return tokensIn.front();
}

/**
 * Check if the current token matches the expected type and value.
 * @return true if a match, false otherwise
 */
bool CompilerParser::have(std::string expectedType, std::string expectedValue){

    if(tokensIn.size() == 0) {
        return false;
    } else if(tokensIn.front()->getType() == expectedType && tokensIn.front()->getValue() == expectedValue) {
        return true;
    } 
    return false;
}

/**
 * Check if the current token matches the expected type and value.
 * If so, advance to the next token, returning the current token, otherwise throw a ParseException.
 * @return the current token before advancing
 */
Token* CompilerParser::mustBe(std::string expectedType, std::string expectedValue){
    if(have(expectedType, expectedValue)) {
        Token* result = tokensIn.front();
        next();
        return result;
    }
    throw ParseException();
    return NULL;
}

/**
 * Definition of a ParseException
 * You can use this ParseException with `throw ParseException();`
 */
const char* ParseException::what() {
    return "An Exception occurred while parsing!";
}
