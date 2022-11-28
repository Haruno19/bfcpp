/*
    bfcpp is a Brainf*ck language compiler and interpreter written in C++!
    version 1.0.0

    bibliography and documentation for Brainf*ck: https://en.wikipedia.org/wiki/Brainfuck
*/

#include <iostream>
#include <fstream>
#include <sstream> 

/****** CORE ******/
int error(int _type, int _arg);
int help();
int version();
std::string opensrc(char* _path);
void debug_print();

/***** PARSING ****/
void parse(std::string _src);
void parseInst(std::string _src);
void parseWhile(std::string _src);

char array[30000] = {0}, *ptr;   //BF main data structures 
int i; //index to the current character in the source file
bool debug = false;

/*** COMPILING ****/
void compile(std::string _src, char* _o_file);
std::string compileInst(std::string _src);
bool C = false;


int main(int argc, char* argv[])
{
    if(argc == 1) return error(0,0);    //no arguments means no input file 
    if(argv[1][1] == 'h') return help();    //-h argument 
    if(argv[1][1] == 'v') return version();    //-h argument 

    std::string src = opensrc(argv[1]); //read the source file in std::string
    if(src == "")  return error(1,0);   //if it didn't open, return error 1

    if(argc>2)  //if there are optional arguments after the input file
        switch(argv[2][1])
        {
            case 'C':   
                C = true;   //don't delete the C source file
            case 'c':       //compiler mode
                if(argc==4) compile(src, argv[3]);  //uses the output file name if specified
                else compile(src, NULL);    
                break;

            case 'd':
                debug = true;   //enables debug mode
            case 'i':
            default:
                parse(src);
        } 
    else
        parse(src);     //parsing is the default mode 
    

    return 0;
}

/****** CORE ******/

int error(int type, int arg)
{
    switch(type)
    {
        case 0:
            std::cout << "No input file selected.";
            break;
        case 1:
            std::cout << "I/O Error\n";
            break;
        case 2:
            std::cout << "Synatx error on character " << arg << ".\n";
            debug_print();
            exit(2);
    }

    return type;
}

int version()
{
    std::cout   << "- bfcpp version 1.0.0 by github.com/Haruno19\n"
                << "gcc version:\n";
                
    system("gcc --version");

    return 0;
}

int help()
{
    std::cout   << "bfcpp is a Brainf*ck language compiler and interpreter written in C++! \n\n"
                << "  Usage: bfcpp input_file [arguments] [output_file]\n\n"
                << "The available arguments are:\n"
                << "  -h : prints this help message.\n"
                << "  -v : prints bfcpp and gcc version.\n"
                << "  -i : interpreter mode, in this mode bfcpp executes directly your Brainf*ck code.\n"
                << "  -d : debug mode, operates in intepreter mode and outputs the pointer value at the end of execution and saves the array into a log file.\n"
                << "  -c : compilator mode, in this mode bfcpp creates an executable file for your Brainf*ck code.\n"
                << "         [note: gcc is required for this mode]\n"
                << "         [if an output_file name isn't specified, './a.out' will be used]\n"
                << "  -C : compilatore mode + C output, behaves exactly as the compilator mode, but also exports the C source file.\n\n"
                << "Without arguments, bfcpp operates in interpreter mode.\n\n"
                << "Main array size is set to 30000.\n\n";
    return 0;
}

std::string opensrc(char* path)
{  
    std::ifstream f(path);
    std::stringstream buf;
    //reads and returns string from "path" file  
    if(!f.is_open()) return "";
    buf << f.rdbuf();
    f.close();

    return buf.str();
}

void debug_print()
{
    std::cout   << "\n\n-------------------------------------" 
                << "\nPointer value: " << std::to_string(*ptr) 
                << "\nParsing index: " << i;

    std::ofstream output("array.log");
    for(int k=0;k<30000;k++)
    {
        if(k%50==0) output << "\n";
        output << std::to_string(array[k]) << ", ";
    }
    std::cout << "\nArray printed in array.log\n\n";
    output.close();
}


/***** PARSING ****/

void parse(std::string src)
{
    ptr = array;    //the main pointer initially points at the start of the array

    for(i=0; i<src.size(); i++) //for each character in the source file
       parseInst(src);  //it parses its instruction

    if(debug) debug_print();
}

void parseInst(std::string src)
{
    switch(src[i])  //instruction parsing, see bibliography
    {
        case '>':
            ++ptr;
            break;

        case '<':
            --ptr;
            break;

        case '+':
            ++*ptr;
            break;

        case '-':
            --*ptr;
            break;

        case '.':
            putchar(*ptr);
            break;

        case ',':
            *ptr = getchar();
            break;

        case '[':
            parseWhile(src);    //if there's the start of a while loop, its parser function is called
            break;

        case ']':   //the "end of loop" symbol is managed by the while parser function
        case '\n':
            break;

        default:
            error(2,i);
            return;
    }
}

void parseWhile(std::string src)
{
    int index0 = ++i;   //it saves the position of the frist instruction inside of the while
    
    while(*ptr) //checks the while condition, see bibliography
    {
        i = index0; //brings the current index back to the start of the while
        while(src[i] != ']')    //while there are still instructions inside of the loop
        {
            parseInst(src); //parse those instructions
            i++;
        }
        //when a loop ends (aka ']' character is found, it checks the while condition again, and proceeds accordingly)
        
        /*  
            this makes it easier to manager nested loops, taking advantage of the built-in stack implementation of function 
            calls in C++, rather than creating a more complex stack-like data structure to store indexes.
        */
    }

}


/*** COMPILING ****/

void compile(std::string src, char* o_file)
{
    std::string path="./source.c";
    std::string command = "gcc " + path;
    if(o_file) { command += " -o "; command+=  o_file; }
    std::ofstream output(path);
    std::string c_src = compileInst(src);   //converts src into C code
    output << c_src;    //prints "./source.c" file
    output.close();

    system(command.c_str());    //compiles the C source files invoking gcc
    if(!C) { command = "rm " + path; system(command.c_str()); } //if C mode isn't enabled it deletes the file
}

std::string compileInst(std::string src)
{
    std::string c_src= "/* C code auto-generated by bfcpp */\n\n#include <stdio.h>\n\nint main()\n{\n\tchar array[30000] = {0}; char *ptr = array;\n\n";
    int indent_lvl = 1;
    for(i=0; i<src.size(); i++)
    {
        for(int l=0;l<indent_lvl;l++)   //prints tabs to make the C code look good :)
            c_src+="\t";
        switch(src[i])  //instruction trasnlation, see bibliography
        {
            case '>':
                c_src+="++ptr;";
                break;

            case '<':
                c_src+="--ptr;";
                break;

            case '+':
                c_src+="++*ptr;";
                break;

            case '-':
                c_src+="--*ptr;";
                break;

            case '.':
                c_src+="putchar(*ptr);";
                break;

            case ',':
                c_src+="*ptr = getchar();";
                break;

            case '[':
                c_src+="while(*ptr) {";
                indent_lvl++;
                break;

            case ']':
                c_src+="}";
                indent_lvl--;
                break;
                
            case '\n':
                break;

            default:
                error(2,i);
                return "";
        }
        c_src+="\n";
    }

    return (c_src + "\treturn 0;\n}");
}