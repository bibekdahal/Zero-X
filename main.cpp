////////////////////////////////////////////////////////////
//
//   Zero-X Compiler
//
//          This program can parse 0x program and generate
//          a parse tree out of it
//
////////////////////////////////////////////////////////////


//Some includes
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <conio.h>

using namespace std;

//  These are some constants that define the type of a token
#define UNKNOWN     -1
#define EOL         0   //  End of line '\n'
#define NUMBER      1   //  123, 2, -45
#define STRING      2   //  "Hello World!"
#define SYMBOL      3   //  , + - | *
#define IDENTIFIER  4   //  The keywords and the variables' names: PRINT, A123, IF etc.

string Program="";      //  The complete program code

//  We scan from the beginning of the program i.e. pos=0
long unsigned pos=0;        //  Used to store the current position of scanning
long unsigned lines=0;      //  The total number of lines in the program
long unsigned lineStart=0;  //  The starting position, in the program, of the current line being scanned
                            //  -can be useful for displaying errors
long unsigned ln=0;         //  The index of current line being scanned


// Get next token while parsing
#define NextToken       tk++
// The current token being analysed while parsing
#define CToken           Tokens[tk]
// Throw out an error if we run out of tokens in the middle of parsing an statement
#define ErrOutOfTokens if (tk>=Tokens.size()-1 && tk!=(unsigned)-1) Error("Program ended unexpectedly", CToken.LineStart)

// Index of current token (which is being parsed) in the tokens list
long unsigned tk;

// The tree structure
struct Node
{
    string Value;           // Every node has a value
    string Attribs[3];      // and three attributes
    Node * Left, * Right;   // and may contain two children nodes
} * ParseTree;              // ParseTree is our root node

// To delete a node and all its children
void DeleteTree(Node*& pt)
{
    if (!pt) return;
    DeleteTree(pt->Left);
    DeleteTree(pt->Right);
    delete pt;
    pt=0;
}
// To create a node
void CreateTree(Node*& pt)
{
    pt = new Node();
    pt->Value="";
    pt->Attribs[0]=pt->Attribs[1]=pt->Attribs[2]="";
    pt->Left=pt->Right=0;
}


//  While displaying error (like invalid token), we might as well display
//  the line of program that generated the error.
//  We will need the starting pos of the current line (lineStart) for this
string GetLine(long unsigned Pos)
{
    string ret="";
    //  Get the text till the end of line
    while (Program[Pos]!='\n')
    {
        ret+=Program[Pos];
        Pos++;
    }
    return ret;
}


// We may need to frequently display an error
// So we create a function to do this
void Error(string err, long unsigned LineStart)
{
    cout << "Error:\n\t";
    cout << err << "\n\t\t";
    cout << GetLine(LineStart);
    cout << "\nLine Number:" << ln+1;
    DeleteTree(ParseTree);
    getch();
    exit(-1);
}

//The information about the token
struct _Token
{
    string Str;		    //  The value of the token
    int Type;			//  The type of the token

    ////  NEW CODE /////
    long unsigned LineStart;    //  The position at the program where the line containing
                                //  the token starts, useful for generating error while parsing
    ////  NEW CODE /////

} Token;
//  The variable Token is the token just scanned, that can be added to the tokens list

//  The list of all the tokens after scanning
vector<_Token> Tokens;

//  To skip the spaces and tabs as well as the comments that the parser won't need
void SkipWhiteSpace()
{
    while (isspace(Program[pos]))		//  as long as the current character is a space or a tab, skip it
    {
        if (Program[pos]=='\n')
            break;			            //  but do not skip the end of line '\n' character
        pos++;                          //  because that is not useless: EOL marks the end of
                                        //  programming statement in 0x
    }
    if (Program[pos]=='/' && Program[pos+1]=='/')
    {
        while (Program[pos]!='\n')		//  if the current and the next character are both
            pos++;			            //  ‘/’, then it is a comment: //COMMENT
    }
}

// To get the identifier type of token
int GetIdentifier()
{
    do
    {
        Token.Str += toupper(Program[pos]);     //  The toupper is to make sure every character is uppercase
        pos++;                                  //  so that our compiler is not case sensitive
    }while (isalnum(Program[pos]) || Program[pos]=='_');  // Alphabet, number or undescore can be included
    Token.Type = IDENTIFIER;
    return IDENTIFIER;
}

//  To get the number type of token
int GetNumber()
{
    do
    {
        Token.Str += Program[pos];
        pos++;
    }while (isdigit(Program[pos]));  //  Scan till got a digit
    Token.Type = NUMBER;
    return NUMBER;
}

//  To get the string type of token
int GetString()
{
    pos++;
    do
    {
        if (Program[pos]=='\n')     // Of course, we can't end a line in the middle of a string
            Error("Expected ending quotation mark '\"'...\n\t\tin line:\n\t",lineStart);
        Token.Str += Program[pos];
        pos++;
    }while (Program[pos]!='"');  // Scan till we get the ending quotation mark
    pos++;
    Token.Type = STRING;
    return STRING;
}

//  To get the symbol type of token
int GetSymbol()
{
    Token.Str += Program[pos]; //  Normally a single character is all that is a token
            //  But in case of <=, >= or != we have two characters as token
    if ((Program[pos]=='<'||Program[pos]=='>'||Program[pos]=='!')&&Program[pos+1]=='=')
    {
        pos++;
        Token.Str += Program[pos];
    }
    pos++;
    Token.Type = SYMBOL;
    return SYMBOL;
}


//  To get te next token once the current one is scanned
int GetNextToken()
{
    //  By default...
    Token.Str = "";
    Token.Type = UNKNOWN;

    //  Start by skipping all the spaces and useless comments
    SkipWhiteSpace();

    ////  NEW CODE /////
    Token.LineStart = lineStart;
    ////  NEW CODE /////

    // If the first character is...
    //  ...an end of line then EOL
    if (Program[pos]=='\n')
        {Token.Type = EOL; return EOL;}
    //  ...an alphabet then its an identifier
    if (isalpha(Program[pos]))
        return GetIdentifier();
    //  ...a digit then its a number
    if (isdigit(Program[pos]))
        return GetNumber();
    //  ...a '"' mark then a string is starting
    if (Program[pos]=='"')
        return GetString();
    //  ...a symbol then its a symbol
    if (ispunct(Program[pos]))
        return GetSymbol();

    //  Non of the above, throw an error
    Error("Unrecognized Token...\n\t\tin line:\n\t",lineStart);
    return UNKNOWN;
}

//  Prepare a list of all the tokens in the program
void PrepareTokensList()
{
    //  Start from the first character
    pos=ln=lineStart=0;
    //  Scan next token till we pass through the end of file
    while (ln<lines) //  i.e. as long as currentLineIndex < TotalLines
    {
        GetNextToken(); //  Get the token
        if (Token.Type==EOL){ // An EOL means we have to update some variables
            lineStart=pos+1;
            ln++;
            pos++;
        }
        Tokens.push_back(Token);  // And add the the token to the list
    }
}


// The data types available in 0x
#define INT 0
#define STR 1

// Information about a variable
struct VarInfo
{
    string Name;
    int Type;
};
vector <VarInfo> Vars; // List of all variables created

// Check if a variable exists in the list
bool CheckVar(string var)
{
    for (unsigned i=0;i<Vars.size();i++)
    {
        if (Vars[i].Name==var)return true;
    }
    return false;
}
// Get the type of a variable existing in the list
int GetType(string var)
{
    for (unsigned i=0;i<Vars.size();i++)
    {
        if (Vars[i].Name==var)return Vars[i].Type;
    }
    return -1;
}
// Check for invalidity of a variable being created
bool InvalidVar(string var)
{
    // Following keywords can't be used as variable name
    if (var=="INT" || var=="STRING" || var=="PRINT" || var=="INPUT")
        return true;
    // Also an already existing variable name can't be reused
    if (CheckVar(var)) return true;
    // It's ok otherwise
    return false;
}

// The prototype comes before the definition here; it parses an expression and generates a parse tree at the node 'nd'
void ParseExpression(Node*&nd, int Type);

// To parse a factor in an expression
// In EBNF:
//          <factor> ::= <IDENTIFIER> | <CONSTANT> | ( ‘(’, <expression>, ‘)’ );
// We might be expecting a string or a numeric factor
// So we need to know the Type
// The 'nd' is the tree node where the factor is to be held
void ParseFactor(Node*& nd, int Type)
{
    ErrOutOfTokens;
    bool Neg = false;
    if (CToken.Str=="-" && Type == INT)
    {
        Neg = true;
        NextToken;
    }
    // We start by checking for bracketed expression
    if (CToken.Str=="(")
    {
        NextToken;                  // Get the next token after the opening bracket
        ParseExpression(nd,Type);   // Parse the epxression starting at that token at node 'nd'
        if (CToken.Str!=")") Error("Expected closing bracket ')'",CToken.LineStart);    // We need to close bracket
        NextToken;  // Make next token ready for parsing
        if (Neg)
        nd->Attribs[2]="NEGATE";
        return;     // Done with this factor, so return
    }

    // If not bracketed expression, it is either an identifier or a constant
    CreateTree(nd);             // Start by creating the node
    nd->Value = CToken.Str;     // and then store the value (variable's name or constant) in the node

    // Here goes the semantic part of analysis:

    // For a variable, check if it exists
    if (CToken.Type==IDENTIFIER && !CheckVar(CToken.Str))
        Error("Undeclared variable name used in expression",CToken.LineStart);

    // Also check for invalid factor type
    // If factor is not identifier then check if the token is number when expected type is INT
    // or string when expected type is STR
    if ((CToken.Type!=IDENTIFIER &&
            ((Type==INT && CToken.Type!=NUMBER)
            || (Type==STR && CToken.Type!=STRING)
            ))
    // Otherwise if it is an identifier then check if the type of the variable matches with the expecting type
        || (CToken.Type==IDENTIFIER && GetType(CToken.Str)!=Type))
                // Throw out an error in case of failure while checking
                    Error("Invalid factor type used in an expression",CToken.LineStart);

    // Add some extra information to aid in code generation
    // The type: whether constant or variable
    if (CToken.Type==IDENTIFIER) nd->Attribs[0]="VARIABLE";
    else nd->Attribs[0]="VALUE";

    if (Neg)
        nd->Attribs[2]="NEGATE";

    // Make next token ready for parsing
    NextToken;
}

// To parse a term in the epression
// In EBNF:
//              <term> ::= <factor>, { ‘*’|’/’, <factor> };
void ParseTerm(Node*& nd)
{
    ErrOutOfTokens;
    // <term> ::= <factor> ...
    // Parse the first factor
    ParseFactor(nd,INT);
    // Then the {...} - the repetition block part
    // For each '*' or '/' parse another factor
    while (CToken.Str=="*" || CToken.Str=="/")
    {
        // Remember we have either an already parsed factor
        // or a previously parsed '*' or '/' operator  in our node 'nd'
        // But now we need to make that as a child node

                //  ( See an example in the accompanying presentation to find why we need
                //    the last of the operators that appear in the expression, and not the first,
                //    as the parent node )

        // and the new '*' or '/' operator as its parent node

        Node tmp;   // Temporary node to store the current 'nd' node
        tmp = *nd;  // Copy 'nd' node to tmp
        delete nd;  // Delete 'nd' data
        CreateTree(nd); // Create new node at 'nd'
        CreateTree(nd->Left);   // Also create a left child for tmp
        *(nd->Left)=tmp;        // Copy tmp to left child
        nd->Value=CToken.Str;   // Set operator's value to 'nd'
        NextToken;              // Ready for next token parsing
        ParseFactor(nd->Right,INT); //Parse another factor on the right
    }
}
// To parse the expression
// In EBNF:
//              <expression> ::= <term>, { ‘+’|’-’, <term> };
void ParseExpression(Node*& nd, int Type)
{
    ErrOutOfTokens;
    // A numeric expression may be made up of terms
    // but a string expression is made up of string factors only :
    //      we can't multiply or divide strings
    if (Type==INT)
        ParseTerm(nd);
    else
        ParseFactor(nd,STR);

    // Code is similar to ParseTerm function
    // So no explanation here
    while (CToken.Str=="+" || (Type==INT && CToken.Str=="-"))
    {
        Node tmp;
        tmp = *nd;
        delete nd;
        CreateTree(nd);
        CreateTree(nd->Left);
        *(nd->Left)=tmp;
        nd->Value=CToken.Str;
        NextToken;
        if (Type==INT)
            ParseTerm(nd->Right);
        else
            ParseFactor(nd->Right,STR);
    }
}

// To declare a variable
// In EBNF:
//              <DECLARE> ::= ( 'INT' | 'STR' ), <IDENTIFIER>;
void DeclareVar(Node* nd)
{
    // The node's name is DECLARE and its attributes are the data type and variable name
    nd->Value = "DECLARE";
    nd->Attribs[1] = CToken.Str;
    NextToken;
    ErrOutOfTokens;
    nd->Attribs[0] = CToken.Str;

    //Semantic part
    // Check for valid variable name
    if (CToken.Type!=IDENTIFIER || InvalidVar(CToken.Str))
        Error("Invalid variable name used in declaration",CToken.LineStart);
    // Add the variable to the variable list for future reference
    VarInfo var;
    var.Name = CToken.Str;
    if (nd->Attribs[1]=="INT") var.Type = INT;
    else var.Type = STR;
    Vars.push_back(var);
    // Prepare next token for parsing
    NextToken;
}

// To assign expression to a varibale
// In EBNF:
//              <ASSIGN> ::= <IDENTIFIER>, '=', <EXPRESSION>;
void Assign(Node* nd)
{
    // Create node with name ASSIGN
    nd->Value = "ASSIGN";
    // The left child is the variable name
    CreateTree(nd->Left);
    nd->Left->Value = CToken.Str;
    // Skip through '=' sign to get to the starting of the expression
    NextToken;NextToken;
    // The right child is the expression
    //      Note: we are also passing the type of the assignment variable for parsing correct type of expression
    ParseExpression(nd->Right, GetType(nd->Left->Value));

    // Semantic part
    // Check for validity of variable name
    if (!CheckVar(nd->Left->Value))
        Error("Undeclared variable name used in assignment",CToken.LineStart);
    // Set node's attribute to the type of variable to aid in code generation
    if (GetType(nd->Left->Value)==INT) nd->Attribs[0]="INT";
    else nd->Attribs[0]="STRING";

    // Note: we do not need to do NextToken as usual; this is because
    // we are already a token ahead of the expression after ParseExpression is called
}

// <PRINT> ::= 'PRINT', ( <IDENTIFIER> | <CONSTANT> );
void Print(Node* nd)
{
    // Blah, blah, blah...
    nd->Value = "PRINT";
    NextToken;
    ErrOutOfTokens;
    nd->Attribs[0] = CToken.Str;

    // Semantic part
    // Check for validity: srgument should be varable or constant
    if (CToken.Type!=IDENTIFIER && CToken.Type!=STRING && CToken.Type!=NUMBER)
    {
        if (CToken.Str=="-")
        {
            NextToken;
            if (CToken.Type!=NUMBER)
                Error("Invalid argument for PRINT statement",CToken.LineStart);
            else nd->Attribs[0]="-"+CToken.Str;
        }
        else
            Error("Invalid argument for PRINT statement",CToken.LineStart);
    }
    // and in case of variable, it should exist
    if (CToken.Type==IDENTIFIER && !CheckVar(CToken.Str))
        Error("Undeclared variable name used as argument in PRINT",CToken.LineStart);
    // Store some attributes for aid in code generation: the data type and variable/constant type
    if (CToken.Type==NUMBER || GetType(CToken.Str)==INT)
        nd->Attribs[1] = "INT";
    else
        nd->Attribs[1] = "STRING";

    if (CToken.Type==IDENTIFIER) nd->Attribs[2]="VARIABLE";
    else nd->Attribs[2]="VALUE";
    // Prepare next token for parsing
    NextToken;
}

// <INPUT> ::= 'INPUT', <IDENTIFIER>
// Similar to above, only no constant allowed
void Input(Node* nd)
{
    nd->Value = "INPUT";
    NextToken;
    ErrOutOfTokens;
    nd->Attribs[0] = CToken.Str;
    //Semantic part
    if (CToken.Type!=IDENTIFIER)
        Error("Invalid argument for INPUT statement",CToken.LineStart);
    if (!CheckVar(CToken.Str))
        Error("Undeclared variable name used as argument in INPUT",CToken.LineStart);
    if (GetType(CToken.Str)==INT)
        nd->Attribs[1] = "INT";
    else
        nd->Attribs[1] = "STRING";
    NextToken;
}

// Parse a new statement
void ParseStatement(Node* nd)
{
    // Call appropriate parsing function according to first token in the statement
    if (CToken.Str=="INT" || CToken.Str=="STRING")
        DeclareVar(nd);
    else if (CToken.Str=="PRINT")
        Print(nd);
    else if (CToken.Str=="INPUT")
        Input(nd);
    else if (CToken.Type==IDENTIFIER && Tokens[tk+1].Str=="=")
        Assign(nd);
    /////////////// NEW CODE ///////////////////////
    else if (CToken.Str=="NEWLINE")
    {
        nd->Value = "NEWLINE";
        NextToken;
    }
    /////////////// NEW CODE ///////////////////////
    else
        Error("INVALID STATEMENT", CToken.LineStart);
}

// Parse the whole program
void ParseProgram()
{
    // Start at the first token
    tk=0;
    // Create the root node
    CreateTree(ParseTree);
    // Set active node to the root node
    Node* nd = ParseTree;
    for (ln=0;ln<lines;ln++)    // For each line
    {
        if (CToken.Type!=EOL){  // Ignore if EOL at the starting of the line i.e. empty statement
            if (ln==lines-1)ParseStatement(nd); // Parse at active node in case of last statement
            else
            {
                nd->Value="STATEMENTS"; // If not the last statement
                CreateTree(nd->Left);   // Create a 'STATEMENTS' node at the active node
                CreateTree(nd->Right);  // and parse at its left child node while
                ParseStatement(nd->Left);   // making the right child node as the new active node
                nd = nd->Right;
            }
            // Notice how we make sure NextToken is called at the end of almost every parsing function
            // So we are one token ahead of what has been already parsed
            // At the end of every statement there is an EOL
            // But if there is no EOL then it is a syntax error
            if (CToken.Type!=EOL) Error("End Of Line Expected!",CToken.LineStart);
        }
        // Now go to next token i.e. go to the starting of next statement
        NextToken;
    }
}

// Display the value and attributes of a node and all its children
// I am not explaining this one, try to figure it by yourself
void DisplayTree(Node* node, string Inner="-")
{
    if (!node)return;
    cout << "\n"+Inner+"> "+node->Value;
    string Attrib="";
    for (int i=0;i<3;i++)
    {
        if (node->Attribs[i]!="")
            Attrib+=" "+node->Attribs[i];
    }
    if (Attrib!="")
        cout << "  (Attributes:"+Attrib+")";
    DisplayTree(node->Left, Inner+"-");
    DisplayTree(node->Right, Inner+"-");
}


//////////// New Code //////////////////
string Data="";
string Code="";
string Final="";
void AddData(string st)
{
    Data+="\n"+st;
}
void AddCode(string st)
{
    Code+="\n"+st;
}
void AddEnd(string st)
{
    Final+="\n"+st;
}
unsigned long tmp=0;
string GetTmp()
{
    char buffer[30];
    sprintf(buffer, "tmp%lu",tmp);
    tmp++;
    return buffer;
}
void CodeNum(Node*nd);
void OprStart(Node*nd)
{
    CodeNum(nd->Left);
    CodeNum(nd->Right);
    AddCode("pop ebx");
    AddCode("pop eax");
}
void CodeNum(Node*nd)
{
    if (nd->Value=="+")
    {
        OprStart(nd);
        AddCode("add eax, ebx");
        AddCode("push eax");
    }
    else if (nd->Value=="-")
    {
        OprStart(nd);
        AddCode("sub eax, ebx");
        AddCode("push eax");
    }
    else if (nd->Value=="*")
    {
        OprStart(nd);
        AddCode("imul eax, ebx");
        AddCode("push eax");
    }
    else if (nd->Value=="/")
    {
        OprStart(nd);
        AddCode("cdq");
        AddCode("idiv ebx");
        AddCode("push eax");
    }
    else
    {
        string varName;
        if (nd->Attribs[0]=="VALUE")
            AddCode("push dword " + nd->Value);
        else
            AddCode("push dword["+ nd->Value+"ASM]");
    }
    if (nd->Attribs[2]=="NEGATE")
    {
        AddCode("pop eax");
        AddCode("neg eax");
        AddCode("push eax");
    }
}

void CodeStr(Node*nd)
{
    if (nd->Value=="+")
    {
        CodeStr(nd->Left);
        CodeStr(nd->Right);
    }
    else
    {
        string varName;
        if (nd->Attribs[0]=="VALUE")
            varName = nd->Attribs[1];
        else
            varName="dword["+nd->Value+"ASM]";

        AddCode("push "+ varName);
        AddCode("push dword[tmpStr]");
        AddCode("call strcat");
        AddCode("add esp, 8");
    }
}
void StrLen(Node*& nd)
{
    if (nd->Value=="+")
    {
        StrLen(nd->Left);
        StrLen(nd->Right);
        AddCode("pop eax");
        AddCode("pop ebx");
        AddCode("add eax,ebx");
        AddCode("dec eax");
        AddCode("push eax");
    }
    else
    {
        string varName;
        if (nd->Attribs[0]=="VALUE")
        {
            varName = GetTmp();
            AddData(varName+"  db  '" + nd->Value+"',0");
            nd->Attribs[1]=varName;
        }
        else
            varName="dword["+nd->Value+"ASM]";

        AddCode("push "+ varName);
        AddCode("call strlen");
        AddCode("add esp, 4");
        AddCode("push eax");
    }
}
void CodeAssign(Node* nd)
{
    string varName=nd->Left->Value+"ASM";
    if (nd->Attribs[0]=="INT")
    {
        CodeNum(nd->Right);
        AddCode("pop dword["+varName+"]");
    }
    else
    {
        AddCode("push dword 1");
        StrLen(nd->Right);
        AddCode("call calloc");
        AddCode("add esp, 8");
        AddCode("mov dword[tmpStr],eax");
        CodeStr(nd->Right);
        AddCode("push dword["+varName+"]");
        AddCode("call free");
        AddCode("add esp, 4");
        AddCode("mov eax,dword[tmpStr]");
        AddCode("mov dword["+varName+"],eax");
    }
}

void CodeDeclare(Node* nd)
{
    string varName = nd->Attribs[0]+"ASM";
    AddData(varName+"  dd  0");
    if (nd->Attribs[1]=="STRING")
    {
        AddCode("push dword 1");
        AddCode("push dword 1");
        AddCode("call calloc");
        AddCode("add esp, 4");
        AddCode("mov dword["+varName+"], eax");
        AddEnd("push dword["+varName+"]");
        AddEnd("call free");
        AddEnd("add esp, 4");
    }
}
void CodePrint(Node* nd)
{
    if (nd->Attribs[1]=="STRING")
    {
        if (nd->Attribs[2]=="VALUE")
        {
            string var=GetTmp();
            AddData(var+"  db  '" + nd->Attribs[0]+"',0");
            AddCode("push "+ var);
            AddCode("call printf");
            AddCode("add esp, 4");
            tmp++;
        }
        else
        {
            AddCode("push dword["+nd->Attribs[0]+"ASM]");
            AddCode("call printf");
            AddCode("add esp, 4");
        }
    }
    else
    {
        string varName;
        if (nd->Attribs[2]=="VALUE")
            AddCode("push dword "+ nd->Attribs[0]);
        else
            AddCode("push dword["+ nd->Attribs[0]+"ASM]");
        AddCode("push IntFmt");
        AddCode("call printf");
        AddCode("add esp, 8");
    }
}
void CodeInput(Node* nd)
{
    if (nd->Attribs[1]=="STRING")
    {
        AddCode("push dword["+nd->Attribs[0]+"ASM]");
        AddCode("call free");
        AddCode("add esp, 4");
        AddCode("push dword 1");
        AddCode("push dword 50");
        AddCode("call calloc");

        AddCode("mov dword["+nd->Attribs[0]+"ASM],eax");

        AddCode("push dword["+nd->Attribs[0]+"ASM]");
        AddCode("push StrFmt");
        AddCode("call scanf");
        AddCode("add esp, 8");
    }
    else
    {
        AddCode("push "+ nd->Attribs[0]+"ASM");
        AddCode("push IntFmt");
        AddCode("call scanf");
        AddCode("add esp, 8");
    }
}

void CodeGen(Node* nd)
{
    if (nd->Value=="STATEMENTS")
    {
        CodeGen(nd->Left);
        CodeGen(nd->Right);
        return;
    }
    else if (nd->Value=="DECLARE")
        CodeDeclare(nd);
    else if (nd->Value=="PRINT")
        CodePrint(nd);
    else if (nd->Value=="INPUT")
        CodeInput(nd);
    else if (nd->Value=="ASSIGN")
        CodeAssign(nd);
    else if (nd->Value=="NEWLINE")
    {
        AddCode("push nlStr");
        AddCode("call printf");
        AddCode("add esp, 4");
    }
    AddCode("");
}

string StripOffExtension(string FileName){
  int Length = (int)FileName.length();
  for (int i = Length; i > 0; i--){
    if (FileName[i] == '\\' || FileName[i] == '/'){
      return FileName;
    }
    if (FileName[i] == '.'){
      return FileName.substr(0, i);
    }
  }
  return FileName;
}
//////////// New Code //////////////////


//  the MAIN function
int main(int argc, char **argv)
{
    string FileName;
    //  if argument supplied while opening the program, use it as filename
    //  e.g.:  if opened from command prompt by entering:  Zero-X.exe Test.txt
    //  else ask for a filename
    if (argc != 2){
        cout << "Usage: Zero-X <filename>\nEnter Filename: ";
        getline(cin,FileName);
        cout << "\r\n";
    }
    else
        FileName = argv[1];
    //  Read the program code from the file
    //  First open the file and check if its opened
    ifstream file(FileName.c_str());
    if (!file.is_open())
    {
        cout << "Error:\n\tCan't open file \"" + FileName + "\"\n";
        getch();
        return -1;
    }
    //  Next read each line in file and store it, also update the TotalLines variable
    string tmp="";
    while (getline(file, tmp))
    {
        Program = Program + tmp + "\n";
        lines += 1;
    }
    //  Close the file
    file.close();

    //  If empty file, error
    if (Program.length() == 0)
    {
        cout << "Error:\n\tEmpty File\n";
        getch();
        return -1;
    }

    //  Prepare the list of the token
    PrepareTokensList();

    // After the tokens list is prepared, parse the program, display the ParseTree and delete the tree
    ParseProgram();

    CodeGen(ParseTree);
    string fname=StripOffExtension(FileName);
    string AsmFName=fname+".asm";
    ofstream AsmFile (AsmFName.c_str());
    if (AsmFile.is_open())
    {
        AsmFile << "extern calloc\nextern free\nextern printf\nextern scanf\nextern exit"
                "\nextern strcpy\nextern strcat\nextern strlen\n";
        AsmFile << "\n\n\nsection .data\n IntFmt  db  '%d',0\n StrFmt  db  '%s',0\n"
                " tmpStr dd 0\n nlStr db 10,0\n"<< Data;
        AsmFile << "\n\nsection .text\n global START\nSTART:\n" << Code;
        AsmFile << "\n" << Final << "\npush 0\ncall exit\nadd esp, 4";
        AsmFile.close();

        // Assuming you are on Windows, you will need NASM and GoLink to assemble and link the compiled files.
        string command="NAsm -f win32 \""+AsmFName+"\"";
        system(command.c_str());
        command="GoLink /console /ni \""+fname+".obj\" msvcrt.dll";
        system(command.c_str());
    }
    DeleteTree(ParseTree);
    cout << "Zero-X: Compilation complete !\n\tOutput file: "+fname+".exe\n\n\tPress any key...";
    //  Wait for input
    getch();
    //  The end of the program
    return 0;
}
