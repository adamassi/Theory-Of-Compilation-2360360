#include "tokens.hpp"
#include "output.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include "tokens.hpp"
using namespace std;
//note to self: -10 is for indicating not an escape
	
    // unkown string  => -1 
    // unclosed string => -2
    // error unkown sequence escape => -3

// helper functions
bool find_escape(char escp)
{
    if (escp == 'n') {
        return  true;
    } else if (escp == '0') {
        return  true;
    } else if (escp == 'r') {
        return  true;
    } else if (escp == 't') {
        return  true;
    } else if (escp == '\\') {
        return true;
    } else if (escp == '"') {
        return true;
    } else if (escp == 'x') {
        return true;
    } 
        return false;

}
bool is_escape(char c)
{
    if(find_escape(c) == false)
    {
        return false;
    }
    return true;

}


// this is according to the requirment that the number should be 0x00->0x7F
// hex_x is the first num hex_y is the second num \xhex_x hex_y
// hex_x is the first num hex_y is the second num \xhex_x
// return true if hex_x,hex_y are hexadicimal value of a char in ascii
//			else return false
bool is_character(char hex_x, char hex_y)
{
    if(hex_x >='0' && hex_x <='7')
    {
        if((hex_y >= '0' && hex_y <= '9') || (hex_y >= 'A' && hex_y <= 'F')){
            return 1;
        }
    }

    return 0;
}

void printErr()
{
    std::string ptr = (string) yytext;
        int size = ptr.size();
        for(int i=1; i < size; i++)
        {
        if(ptr[i]=='\n' || ptr[i]=='\r' || (ptr[i]=='\\' && i==size-2))
            {
                output::errorUnclosedString();
            }
            if(ptr[i]=='\\')
            {
                if(!is_escape(ptr[i+1]))
                {
                    output::errorUndefinedEscape(ptr.substr(i+1,1).c_str());
                }
                else if(ptr[i+1]=='x')
                {
                    //&& ptr[i+3]!=' ' && ptr[i+3]!='\t'
                    if(i+3 < size-1 && ptr[i+2]!=' ' && ptr[i+2]!='\t'  ) // at least we have \xdd"
                    {
                            if(!is_character(ptr[i+2],ptr[i+3]) )
                            {
                              output::errorUndefinedEscape(ptr.substr(i+1,3).c_str());
                            }
                            else i += 3;


                    }
                    else if(i+2 <  size-1)
                    {
                        output::errorUndefinedEscape(ptr.substr(i+1,2).c_str());
                    }
                    else
                    {
                        output::errorUndefinedEscape(ptr.substr(i+1,1).c_str());
                    }

                }
                else{
                    i+=1;
                }
            }
        }
      output::errorUnclosedString();
}


    char hex_convert_to_ascii(char h_x, char h_y)
    {
        std::string full = std::string(1, h_x) + h_y;
        return (char)(int)strtol(full.c_str(), nullptr, 16);
    }





int main() {
    enum tokentype token;
    // read tokens until the end of file is reached
    while ((token = static_cast<tokentype>(yylex()))) {
      
        // indicates unknown characater
        if (token == -1) {
            output::errorUnknownChar(yytext[0]);
        } else if (token == -2||token == -3) {
            //std::cout << "Error " << yytext << std::endl;
            printErr();
            //exit(0);
        } else if (token == COMMENT) {
            output::printToken(yylineno, token, "");
        } else if (token == STRING) {
            string output = "";
            string txt = (string)yytext;
            int size = txt.size();
            for(int i=1; i< size-1;)
            {
                if(txt[i] != '\\')
                {
                    output += txt[i];
                    i++;
                }
                else{
                    if(txt[i+1] == 't')
                    {
                        output += '\t';
                        i= i+2;
                    }
                    else if(txt[i+1] == 'r')
                    {
                        output += '\r';
                        i= i+2;
                    }
                    else if(txt[i+1] == 'n'){
                        output += '\n';
                        i= i+2;/////
                    }
                    else if(txt[i+1] == '\\'){
                        output += '\\';
                        i= i+2;
                    }
                    else if(txt[i+1] == '\"')
                    {
                        output += '\"';
                        i= i+2;
                    }
                    // reached end line
                    else if(txt[i+1] == '0')
                    {
                        i= i+2;
                        //cout<< yylineno <<" " << "STRING" <<" " << output << endl;
                        break;
                        
                    }
                    else if(txt[i+1] == 'x')
                    {
                        
                        char hex_num = hex_convert_to_ascii(txt[i+2],txt[i+3]);
                        // in case we reached end of line and need to print now
                        if(hex_num < 31 || hex_num > 127)
                        {
                            output::errorUndefinedEscape(txt.substr(i+1,3).c_str());
                        }
                        else
                        if(hex_num == '\0'){
                            //cout<< yylineno <<" " << "STRING" <<" " << output << endl;
                            //printErr()
                            break;

                        }
                        output+=hex_num;
                        i+=4;
                    }



                }

            }

            
            output::printToken(yylineno, STRING, output.c_str());
        }

        // this is not a string or error or any other token, its just a regular token 
        // default behaviour
        else {
            
            output::printToken(yylineno, token, yytext);    
        }
	}
    return 0;
}