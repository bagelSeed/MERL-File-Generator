#include "../kind/kind.h"
#include "../lexer/lexer.h"
#include <vector>
#include <string>
#include <string>
#include <iostream>
#include <ctype.h>
#include <map>
#include <fstream>
// Use only the neeeded aspects of each namespace
using std::string;
using std::map;
using std::vector;
using std::endl;
using std::cerr;
using std::cout;
using std::cin;
using std::getline;
using ASM::Token;
using ASM::Lexer;

std::ofstream merl("file.merl");
void printBinary(int num);
int getRegisterNum(string reg, int toks);

int main(int argc, char* argv[]){
  // Nested vector representing lines of Tokens
  // Needs to be used here to cleanup in the case
  // of an exception
  vector< vector<Token*> > tokLines;

  try{

    map<string, int> symbolTable;
    vector< map<string, string> > labelTable;
    vector< map<int,int> > branchTable;
    vector< int > printMerlCom;

    // Create a MIPS recognizer to tokenize
    // the input lines
    Lexer lexer;
    // Tokenize each line of the input
    string line;
    while(getline(cin,line)){
      tokLines.push_back(lexer.scan(line));
    }
//    std::cout<<tokLines[0][0]<<endl;

    // Iterate over the lines of tokens and print them
    // to standard error
    int labelLine = 0;
    int cookie = 268435458;
    int merlLen = 12;
    int relocNum = 0;
    int codeLen = 0;
    string DWORD = "DOTWORD";


    for(int i = 0; i < tokLines.size(); i++){

        bool isDotWord = false;

        bool isJr = false;
        bool isJalr = false;

        int toks = 0;
        int regNumOp=0;
        bool isDone = false;

        bool add = false;
        bool sub = false;
        bool slt = false;
        bool sltu = false;

        bool beq = false;
        bool bne = false;

        bool lis = false;
        bool mflo = false;
        bool mfhi = false;

        bool mult = false;
        bool multu = false;
        bool div = false;
        bool divu = false;

        bool lw = false;
        bool sw = false;


        for(int j = 0; j < tokLines[i].size(); j++){
            if(j == 0){
                merlLen+=4;
            }
            if(lw || sw){
                string operation = lw? "lw" : "sw";

                if(isDone){
                    throw string("ERROR! INVALID INPUT AFTER " + operation + " OPERATION!");
                }

                if(toks == 0 || toks == 4){
                    if(tokLines[i][j]->getKind() == ASM::REGISTER){
                        int tempT = toks == 0? 4 : 2;
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), tempT);
                    } else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " !");
                    }
                } else if(toks == 1){
                    if(tokLines[i][j]->getKind() != ASM::COMMA){
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! PLEASE CHECK COMMAS!");
                    }
                } else if(toks == 2) {

                    if(tokLines[i][j]->getKind() == ASM::INT || tokLines[i][j]->getKind() == ASM::HEXINT){
                        if(tokLines[i][j]->getKind() == ASM::INT){
                            if(tokLines[i][j]->toInt()>32767 || tokLines[i][j]->toInt() < -32768){
                                throw string("ERROR! INVALID INPUT FOR " + operation + " ! CONSTANT VALUE OVERFLOW!");
                            }
                        } else if(tokLines[i][j]->getKind() == ASM::HEXINT){
                            if(tokLines[i][j]->toInt()>65535 || tokLines[i][j]->toInt()<0){
                                throw string("ERROR! INVALID INPUT FOR " + operation + " ! CONSTANT VALUE OVERFLOW!");
                            }
                        }

                        regNumOp+=tokLines[i][j]->toInt()>=0?tokLines[i][j]->toInt():65536+tokLines[i][j]->toInt();

                        if(lw){
                            regNumOp+=2348810240;
                        } else {
                            regNumOp+=2885681152;
                        }

                    } else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! CHECK CONSTANT VALUE!");
                    }
                } else if(toks == 3){
                    if(tokLines[i][j]->getKind() != ASM::LPAREN){
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! CHECK PARENTHESES!");
                    }
                } else if(toks == 5){
                    if(tokLines[i][j]->getKind() != ASM::RPAREN){
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! CHECK PARENTHESES!");
                    }
                    if(j == tokLines[i].size()-1){
                        map< string, string >temp;
                        temp["INTEGER"] = "INTEGER";
                        labelTable.push_back(temp);
                        printMerlCom.push_back(regNumOp);

                        //printBinary(regNumOp);
                    } else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! EXTRA SYMBOLS AFTER BRANCH!");
                    }
                    isDone=true;
                    labelLine++;
                }
                toks++;

            } else if(mult || multu || div || divu){

                string operation = mult? "mult" : multu? "mult" : div? "div" : "divu";

                if(isDone){
                    throw string("ERROR! INVALID INPUT FOR " + operation +" ! EXPECTED END OF LINE, TOO MANY ARGUMENTS!");
                }

                if(toks == 0 || toks == 2){
                    if(toks == 0 && tokLines[i][j]->getKind() == ASM::REGISTER){
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks+2);
                    } else if(toks == 2 && tokLines[i][j]->getKind() == ASM::REGISTER){

                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks+2);
                        if(mult){
                            regNumOp+=24;
                        } else if(multu){
                            regNumOp+=25;
                        } else if(div){
                            regNumOp+=26;
                        } else {
                            regNumOp+=27;
                        }

                        map< string, string >temp;
                        temp["INTEGER"] = "INTEGER";
                        labelTable.push_back(temp);
                        printMerlCom.push_back(regNumOp);


                        //printBinary(regNumOp);
                        labelLine++;
                        isDone = true;
                    }else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " !");
                    }
                } else if(toks == 1){
                    if(tokLines[i][j]->getKind() != ASM::COMMA){
                        throw string("ERROR! INVALID INPUT FOR " + operation + " !");
                    }
                }
                toks++;

            } else if(lis || mflo || mfhi){

                string operation = lis? "lis" : mflo? "mflo" : "mfhi";

                if(isDone){
                    throw string("ERROR! INVALID INPUT FOR " + operation +" ! EXPECTED END OF LINE, TOO MANY ARGUMENTS!");
                }

                if(tokLines[i][j]->getKind() == ASM::REGISTER){

                    if(lis){
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks) + 20;
                    } else if(mflo){
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks) + 18;
                    } else {
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks) + 16;
                    }

                } else {
                    throw string("ERROR! INVALID INPUT FOR " + operation + " !");
                }

                map< string, string >temp;
                temp["INTEGER"] = "INTEGER";
                labelTable.push_back(temp);
                printMerlCom.push_back(regNumOp);


                //printBinary(regNumOp);
                isDone = true;
                labelLine++;

            } else if(beq || bne){
                string operation = beq? "beq" : "bne";

                if(isDone){
                    throw string("ERROR! INVALID INPUT AFTER " + operation + " OPERATION!");
                }

                if(toks == 0 || toks == 2){
                    if(tokLines[i][j]->getKind() == ASM::REGISTER){
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks+2);
                    } else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " !");
                    }
                } else if(toks == 1 || toks == 3){
                    if(tokLines[i][j]->getKind() != ASM::COMMA){
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! PLEASE CHECK COMMAS!");
                    }
                } else if(toks == 4) {
                    if(tokLines[i][j]->getKind() == ASM::INT || tokLines[i][j]->getKind() == ASM::HEXINT){
                        if(tokLines[i][j]->getKind() == ASM::INT){
                            if(tokLines[i][j]->toInt()>32767 || tokLines[i][j]->toInt() < -32768){
                                throw string("ERROR! INVALID INPUT FOR " + operation + " ! CONSTANT VALUE OVERFLOW!");
                            }
                        } else if(tokLines[i][j]->getKind() == ASM::HEXINT){
                            if(tokLines[i][j]->toInt()>65535){
                                throw string("ERROR! INVALID INPUT FOR " + operation + " ! CONSTANT VALUE OVERFLOW!");
                            }
                        }

                        regNumOp+=tokLines[i][j]->toInt()>=0?tokLines[i][j]->toInt():65536+tokLines[i][j]->toInt();

                        if(j == tokLines[i].size()-1){
                            if(operation == "beq"){
                                regNumOp+=268435456;
                            } else {
                                regNumOp+=335544320;
                            }
                            map< string, string >temp;
                            temp["INTEGER"] = "INTEGER";
                            labelTable.push_back(temp);
                            printMerlCom.push_back(regNumOp);
                            //printBinary(regNumOp);
                        } else {
                            throw string("ERROR! INVALID INPUT FOR " + operation + " ! EXTRA SYMBOLS AFTER BRANCH!");
                        }

                    } else if(toks == 4 && tokLines[i][j]->getKind() == ASM::ID){
                        string id = tokLines[i][j]->getLexeme();
                        map<string,string>temp;
                        map<int,int>temp2;
                        temp[id]=operation;
                        temp2[labelLine*4] = regNumOp;
                        labelTable.push_back(temp);
                        branchTable.push_back(temp2);
                        //relocNum++;
                    } else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! CHECK CONSTANT VALUE!");
                    }
                    isDone = true;
                    labelLine++;
                }
                toks++;

            } else if(add || sub || slt || sltu){
                string operation = add? "add" : sub? "sub" : slt? "slt" : "sltu";
                if(isDone){
                    throw string("ERROR! INVALID INPUT AFTER " + operation + " OPERATION!");
                }
                if(toks == 0 || toks == 2 || toks == 4){
                    if(tokLines[i][j]->getKind() == ASM::REGISTER){
                        regNumOp += getRegisterNum(tokLines[i][j]->getLexeme(), toks);
                    } else {
                        throw string("ERROR! INVALID INPUT FOR " + operation + " !");
                    }
                } else {
                    if(tokLines[i][j]->getKind() != ASM::COMMA){
                        throw string("ERROR! INVALID INPUT FOR " + operation + " ! PLEASE CHECK COMMAS!");
                    }
                }
                toks++;
                if(toks == 5 && j == tokLines[i].size()-1){
                    if(operation == "add"){
                        regNumOp+=32;
                    } else if(operation == "sub"){
                        regNumOp+=34;
                    } else if(operation == "slt"){
                        regNumOp+=42;
                    } else {
                        regNumOp+=43;
                    }
                    map< string, string >temp;
                    temp["INTEGER"] = "INTEGER";
                    labelTable.push_back(temp);
                    printMerlCom.push_back(regNumOp);
//                    printBinary(regNumOp);
                    labelLine++;
                    isDone = true;
                }

            } else if(isJr || isJalr){

                if(isDone){
                    throw string("ERROR! INVALID INPUT AFTER JUMP OPERATION!");
                }

                if(tokLines[i][j]->getKind() == ASM::REGISTER){

                    string reg = tokLines[i][j]->getLexeme();
                    regNumOp += getRegisterNum(reg, 2);

                    if(isJr){
                        regNumOp+=8;
                    } else {
                        regNumOp+=9;
                    }

                    map< string, string >temp;
                    temp["INTEGER"] = "INTEGER";
                    labelTable.push_back(temp);
                    printMerlCom.push_back(regNumOp);

                    //printBinary(regNumOp);

                } else {
                    throw string("ERROR! INVALID INPUT AFTER JUMP OPERATION! MUST BE REGISTER!");
                }
                labelLine++;
                isDone = true;

            }else if(isDotWord){

                if(isDone){
                    throw string("ERROR! INVALID INPUT AFTER DOTWORD OPERATION!");
                }

                if (tokLines[i][j]->getKind() == ASM::INT || tokLines[i][j]->getKind() == ASM::HEXINT){
                    map< string, string >temp;
                    temp["INTEGER"] = "INTEGER";
                    labelTable.push_back(temp);
                    printMerlCom.push_back(tokLines[i][j]->toInt());
//                    printBinary(tokLines[i][j]->toInt());
                } else if(tokLines[i][j]->getKind() == ASM::ID){
                    string id = tokLines[i][j]->getLexeme();
                    map<string,string>temp;
                    temp[id]=DWORD;
                    labelTable.push_back(temp);
                    relocNum++;
                } else {
                    throw string("ERROR! INVALID INPUT AFTER DOTWORD!");
                }
                labelLine++;
                isDone=true;

            } else if(tokLines[i][j]->getKind() == ASM::DOTWORD){

                if(j == tokLines[i].size()-1) {
                    throw string("ERROR! INVALID INPUT FOR DOTWORD! NO ARGUMENT AFTER OPERATION!");
                }
                isDotWord=true;

            } else if(tokLines[i][j]->getKind() == ASM::LABEL){

                if(tokLines[i].size()==1){
                    merlLen-=4;
                }

                string label = tokLines[i][j]->getLexeme();
                label.erase(label.length()-1);
                map<string,int>::iterator it = symbolTable.find(label);

                if(it != symbolTable.end()){
                    throw string("ERROR! REDEFINING LABEL NAME!");
                }
                symbolTable[label] = 4*labelLine;

            } else if(tokLines[i][j]->getLexeme() == "jr" || tokLines[i][j]->getLexeme() == "jalr"){

                if(tokLines[i][j]->getLexeme() == "jr"){
                    isJr=true;
                }else{
                    isJalr=true;
                }

            } else if(tokLines[i][j]->getLexeme() == "add" || tokLines[i][j]->getLexeme() == "sub"||
                      tokLines[i][j]->getLexeme() == "slt" || tokLines[i][j]->getLexeme() == "sltu"){

                if(tokLines[i][j]->getLexeme() == "add"){
                    add=true;
                } else if(tokLines[i][j]->getLexeme() == "sub"){
                    sub=true;
                } else if(tokLines[i][j]->getLexeme() == "slt"){
                    slt=true;
                } else if(tokLines[i][j]->getLexeme() == "sltu"){
                    sltu=true;
                }

                if(tokLines[i].size() - j < 5){
                    string operation = add? "add" : sub? "sub" : slt? "slt" : "sltu";
                    throw string("ERROR! "+operation+" REQUIRES 3 REGISTERS AND 2 COMMAS SEPARATING THE REGISTERS");
                }

            }  else if(tokLines[i][j]->getLexeme() == "beq" || tokLines[i][j]->getLexeme() == "bne"){

                if(tokLines[i][j]->getLexeme() == "beq"){
                    beq=true;
                } else {
                    bne=true;
                }

                if(tokLines[i].size() - j < 5){
                    string operation = beq? "beq" : "bne";
                    throw string("ERROR! "+operation+" REQUIRES 2 REGISTERS AND 2 COMMAS SEPARATING THE REGISTERS AND A CONSTANT VALUE!");
                }

            } else if(tokLines[i][j]->getLexeme() == "lis" || tokLines[i][j]->getLexeme() == "mflo" ||
                      tokLines[i][j]->getLexeme() == "mfhi"){

                if(tokLines[i][j]->getLexeme() == "lis"){
                    lis=true;
                } else if(tokLines[i][j]->getLexeme() == "mflo"){
                    mflo=true;
                } else {
                    mfhi=true;
                }

                if(j == tokLines[i].size()-1){
                    string operation = lis? "lis" : mflo? "mflo" : "mfhi";
                    throw string("ERROR! " + operation + " REQUIRES 1 REGISTER VALUE! NEW LINE CHARACTER DETECTED!");
                }

            } else if(tokLines[i][j]->getLexeme() == "mult" || tokLines[i][j]->getLexeme() == "multu"||
                      tokLines[i][j]->getLexeme() == "div" || tokLines[i][j]->getLexeme() == "divu"){

                if(tokLines[i][j]->getLexeme() == "mult"){
                    mult=true;
                } else if(tokLines[i][j]->getLexeme() == "multu"){
                    multu=true;
                } else if(tokLines[i][j]->getLexeme() == "div"){
                    div=true;
                } else if(tokLines[i][j]->getLexeme() == "divu"){
                    divu=true;
                }

                if(tokLines[i].size() - j < 4){
                    string operation = mult? "mult" : multu? "multu" : div? "div" : "divu";
                    throw string("ERROR! "+operation+" REQUIRES 1 REGISTER AND 1 COMMA SEPARATING THE REGISTERS");
                }

            } else if(tokLines[i][j]->getLexeme() == "lw" || tokLines[i][j]->getLexeme() == "sw"){

                if(tokLines[i][j]->getLexeme() == "lw"){
                    lw=true;
                } else {
                    sw=true;
                }

                if(tokLines[i].size() - j < 7){
                    string operation = lw? "lw" : "sw";
                    throw string("ERROR! "+operation+" REQUIRES 2 REGISTERS AND 2 COMMAS SEPARATING THE REGISTERS AND A CONSTANT VALUE!");
                }

            } else {
                throw string("ERROR! INVALID MIPS INPUT!");
            }
        }
    }

    codeLen = merlLen;
    //cout<<"HERE IS: "<<relocNum<<endl;
    merlLen+=8*relocNum;
    printBinary(cookie);
    printBinary(merlLen);
    printBinary(codeLen);

    int branchNum = 0;
    int merlNum = 0;
    for(int i = 0; i < labelTable.size(); i++){
        if(labelTable[i].begin()->first == "INTEGER" && labelTable[i].begin()->second == "INTEGER"){
            //why did i check twice? well...g...gg...idk tbh
            printBinary(printMerlCom[merlNum]);
            merlNum++;
        } else {
            map<string,int>::iterator it = symbolTable.find(labelTable[i].begin()->first);
            if(it == symbolTable.end()){
                throw string("ERROR! LABEL" + labelTable[i].begin()->first + " IS NOT DEFINED WITHIN MERL FILE!");
            }

            if(labelTable[i].begin()->second == DWORD){
                printBinary(it->second + 12);
            } else if(labelTable[i].begin()->second == "beq" || labelTable[i].begin()->second == "bne"){
                int num = (it->second - branchTable[branchNum].begin()->first - 4)/4;
                if(num > 32767 || num < -32768){
                    throw string("ERROR! BRANCH OVERFLOW!");
                }
                int regNumOp=num>=0?num+branchTable[branchNum].begin()->second:65536+num+branchTable[branchNum].begin()->second;

                if(labelTable[i].begin()->second == "beq"){
                    regNumOp+=268435456;
                } else {
                    regNumOp+=335544320;
                }
                printBinary(regNumOp);
                branchNum++;
            }
        }
    }
    for(int i = 0; i < labelTable.size(); i++){
        if(labelTable[i].begin()->second == DWORD){
            printBinary(1);
            printBinary(12+i*4);
        }
    }

  } catch(const string& msg){
    // If an exception occurs print the message and end the program
    cerr << msg << endl;
  }

  // Delete the Tokens that have been made
  vector<vector<Token*> >::iterator it;
  for(it = tokLines.begin(); it != tokLines.end(); ++it){
    vector<Token*>::iterator it2;
    for(it2 = it->begin(); it2 != it->end(); ++it2){
      delete *it2;
    }
  }
}

void printBinary(int num){
    unsigned int out = num;
    unsigned char c;
    c = out >> 24;
    merl<<c;
    c = out >> 16;
    merl<<c;
    c = out >> 8;
    merl<<c;
    c = out;
    merl<<c;
}

int getRegisterNum(string reg, int toks){
    int placement = toks==0? 2 : toks==2? 5 : 4;
    int regNum = 0;
    regNum = reg[reg.length()-1] - '0';

    if(reg.length()!=2){
        regNum += (reg[reg.length()-2] - '0')*10;
        if(regNum>31 || reg.length()>3){
            throw string("ERROR! CONSTANT OUT OF RANGE FOR REGISTER!");
        }
    }

    int multi = 1;
    for(int i = 0; i < placement; i++){
        multi *= 16;
    }

    if(placement == 2){
        multi*=8;
    } else if(placement == 5){
        multi*=2;
    }

    regNum *= multi;
    return regNum;
}








