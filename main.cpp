/*
    Author: Bull.Zhang
    Date: 2025.05.01
*/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "tcl.h"
#include "tk.h"
#include <iostream>
#include <tcl.h>
#include<iostream>
#include <string>
#include <mutex>
#include <pthread.h>
#include "time.h"
#include "unistd.h"
#include <ncurses.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <vector>
#include <string>
#include <algorithm>
#include "argparser.hpp"
// #include <conio.h> //windows os;


#define KEYCODE_R 0x43
#define KEYCODE_L 0x44
#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_HOME 0x48
#define KEYCODE_END  0x46
#define KEYCODE_Q 0x71
#define KEYCODE_DELETE 0x33
#define KEYCODE_INSERT 0x32
#define KEYCODE_BACKSPACE 0x7f

using namespace std;

Tcl_Interp* interp;
int kfd = 0;
struct termios cooked, termios_raw;

char* myCppcmd() {
    printf("\ntcl: cmd.\n");
    return "This is return string.";
}

char* myCppExitcmd() {
    printf("\ntcl: exit cmd.\n");
    return "exit";
}

char* myCppTestParacmd(int argc, const char **argv) {
    printf("\ntcl: argc:%d, argv[1]:%s.\n",argc,argv[1]);
    return "test_para";
}

int MyTclCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char* argv[]) {  
    if (argc != 1) {  
        return TCL_ERROR;  
    }  
    char* res = myCppcmd();
    Tcl_SetResult(interp, res, TCL_VOLATILE);  
    // TCL����ֲ��ｨ��ʹ��Tcl_AppendResult�滻Tcl_SetResult
    return TCL_OK;  
}  

int MyTclExitCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char* argv[]) {  
    if (argc != 1) {  
        return TCL_ERROR;  
    }  
    char* res = myCppExitcmd();
    Tcl_SetResult(interp, res, TCL_VOLATILE);  
    // TCL����ֲ��ｨ��ʹ��Tcl_AppendResult�滻Tcl_SetResult
    return TCL_OK;  
} 


int MyTclTestParaCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char* argv[]) {  
    if(argc<2)
    {
        return TCL_ERROR; 
    }
    char* res = myCppTestParacmd(argc,argv);
    Tcl_SetResult(interp, res, TCL_VOLATILE);  
    // TCL����ֲ��ｨ��ʹ��Tcl_AppendResult�滻Tcl_SetResult
    return TCL_OK;  
} 


int main(int argc, char **argv){
    //string input;
    std::string tcl_cmd_str="";
    char *tcl_result;

    #define READ_CHAR_MAX 1024
    char c[READ_CHAR_MAX];
    int read_char_len=0;


    printf("Tcl and Keyboard processing in C, Ver:0.0.3\n");


    // #define HISTORY_NUMBER 4
    std::vector<std::string> system_cmds={"ls","pwd"};
    std::vector<std::string> history;//={"ls","mv a.txt b.txt","mkdir bill","test"};
    int history_index=0;

    // get the console in termios_raw mode
    tcgetattr(kfd, &cooked); // �õ� termios �ṹ�屣�棬Ȼ�����������ն�
    // exit(0);
    memcpy(&termios_raw, &cooked, sizeof(struct termios));
    //termios_raw.c_lflag &=~ (ICANON);
    termios_raw.c_lflag &=~ (ICANON | ECHO);
    // Setting a new line, then end of file
    termios_raw.c_cc[VEOL] = 1;
    termios_raw.c_cc[VEOF] = 2;
    //termios_raw.c_cc[VMIN]=1;
    //termios_raw.c_cc[VTIME]=0;
    tcsetattr(kfd, TCSANOW, &termios_raw);
    /*
    termios_raw.c_lflag |= (ICANON | ECHO);
    tcsetattr(kfd, TCSANOW, &termios_raw);
    exit(0);
    */


    
  
    // puts("Reading from keyboard");
    // puts("---------------------------");
    // puts("Use arrow keys to move the robot.");
    // puts("otherwise the key values will be printed");
    ap::parser p(argc, argv);
    p.add("-f", "--firstname",  "File name");
    auto args = p.parse();
    if (!args.parsed_successfully()) 
    {
        std::cerr << "Unsuccessful parse\n";	
        return -1;
    }
    auto fileName = args["-f"];
    // printf("%s\n",fileName.c_str());

    interp = Tcl_CreateInterp();
    Tcl_CreateCommand(interp, "mycmd", MyTclCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "exit", MyTclExitCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "test_para", MyTclTestParaCmd, NULL, NULL);
    if(argc>=2)
    {
        // int tcl_code=Tcl_EvalFile(interp,"./test.tcl");
        int tcl_code=Tcl_EvalFile(interp,fileName.c_str());
        
        tcl_result=Tcl_GetStringResult(interp);
        if((tcl_code)!=TCL_OK)
        {
            printf("exe tcl file fail, file name:%s.\n",fileName.c_str());
            termios_raw.c_lflag |= (ICANON | ECHO);
            tcsetattr(kfd, TCSANOW, &termios_raw);
            //printf("exit.\n");
            Tcl_DeleteInterp(interp);
            fflush(stdout);
            exit(0);
        }
        printf("tcl result: %s\n",tcl_result);
        if((string)tcl_result=="exit")
        {
            termios_raw.c_lflag |= (ICANON | ECHO);
            tcsetattr(kfd, TCSANOW, &termios_raw);
            //printf("exit.\n");
            Tcl_DeleteInterp(interp);
            fflush(stdout);
            exit(0);
        }
    }

    #define KEYWORD_MAX_LENGTH 256
    int keyword_position_index=0;
    int keyword_index=0;
    char keyword[KEYWORD_MAX_LENGTH];
    int keyword_swap_buffer_index=0;
    char keyword_swap_buffer[KEYWORD_MAX_LENGTH];
    int insert_mode=0;
    memset(keyword,0,KEYWORD_MAX_LENGTH);
    // set_termios_raw_mode(STDIN_FILENO);
    printf("%%->");
    fflush(stdout);
    // while(true) 
    for(;;)
    {
        //cout << " % ";
        //getline(cin, input);
        //if(input=="exit") break;

        // get the next event from the keyboard
        memset(c,0,READ_CHAR_MAX);
        if(read(kfd, &c, READ_CHAR_MAX) < 0)
        {
            perror("read():");
            exit(-1);
        }
        read_char_len=strlen(c);
        //printf("len:%d, value: %c = 0x%02X = %d, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",read_char_len, c[0], c[0], c[0], c[1], c[2], c[3], c[4]);
        if(read_char_len>0)
        {
            if(c[0]==0x1b)
            {
                // control charactor;
                switch (read_char_len)
                {
                    case 3:
                    {
                        switch(c[2])
                        {
                            case KEYCODE_L:
                            {
                                //printf("\b");
                                //printf("\x1b[D");
                                if(keyword_index>0)
                                {
                                    if(keyword_position_index>0)
                                    {
                                        keyword_position_index--;
                                        printf("\033[D");
                                        fflush(stdout);
                                    }
                                }
                                //printf("\n%d,%d",keyword_position_index,keyword_index);
                                break;
                            }
                            case KEYCODE_R:
                            {
                                if(keyword_index>0)
                                {
                                    if(keyword_position_index<keyword_index)
                                    {
                                        keyword_position_index++;
                                        printf("\033[C");
                                        fflush(stdout);
                                    }
                                }
                                //printf("\n%d,%d",keyword_position_index,keyword_index);
                                break;
                            }
                            case KEYCODE_U:
                            {
                                if(history.size()>0)
                                {
                                    if(history_index<(history.size()-1))
                                    {
                                        history_index++;
                                    }
                                    //printf(" UP");
                                    printf("\r\033[K%%->%s",history[history_index].c_str());
                                    //printf("\033[A");
                                    fflush(stdout);
                                }
                                keyword_index=strlen(history[history_index].c_str());
                                keyword_position_index=keyword_index;
                                memset(keyword,0,KEYWORD_MAX_LENGTH);
                                memcpy(keyword,history[history_index].c_str(),keyword_index);
                                //keyword_index=0;
                                //memset(keyword,0,KEYWORD_MAX_LENGTH);
                                break;
                            }
                            case KEYCODE_D:
                            {
                                //printf(" DOWN");
                                if(history.size()>0)
                                {
                                    if(history_index>0)
                                    {
                                        history_index--;
                                    }
                                    printf("\r\033[K%%->%s",history[history_index].c_str());
                                    //printf("\033[B");
                                    fflush(stdout);
                                }
                                keyword_index=strlen(history[history_index].c_str());
                                keyword_position_index=keyword_index;
                                memset(keyword,0,KEYWORD_MAX_LENGTH);
                                memcpy(keyword,history[history_index].c_str(),keyword_index);
                                //keyword_index=0;
                                //memset(keyword,0,KEYWORD_MAX_LENGTH);
                                break;
                            }
                            case KEYCODE_HOME:
                            {
                                //printf("\r\033[K%%->%s",history[history_index].c_str());
                                if(keyword_index>0)
                                {
                                    int keyword_var=0;
                                    for(keyword_var=0;keyword_var<keyword_position_index;keyword_var++)
                                    {
                                        printf("\033[D");
                                    }
                                    keyword_position_index=0;
                                    fflush(stdout);
                                }
                                break;
                            }
                            case KEYCODE_END:
                            {
                                if(keyword_index>0)
                                {
                                    int keyword_var=0;
                                    for(keyword_var=0;keyword_var<keyword_index-keyword_position_index;keyword_var++)
                                    {
                                        printf("\033[C");
                                    }
                                    keyword_position_index=keyword_index;
                                    fflush(stdout);
                                }
                                break;
                            }
                            default:
                            {
                                
                                break;
                            }
                        }
                        break;
                    }
                    case 4:
                    {
                        switch(c[2])
                        {
                            case KEYCODE_DELETE:
                            {
                                //printf("\033[3~");
                                //printf("\ndelete\n");
                                if(keyword_index>0)
                                {
                                    if(keyword_position_index<keyword_index)
                                    {
                                        memcpy(keyword_swap_buffer,keyword,keyword_index);
                                        memset(keyword,0,KEYWORD_MAX_LENGTH);
                                        memcpy(&keyword[0],&keyword_swap_buffer[0],keyword_position_index);
                                        memcpy(&keyword[keyword_position_index],&keyword_swap_buffer[keyword_position_index+1],(keyword_index-keyword_position_index)-1);
                                        keyword_index--;
                                        //keyword_position_index=keyword_index;
                                        printf("\r\033[K%%->%s",keyword);
                                        int keyword_var=0;
                                        for(keyword_var=0;keyword_var<(keyword_index-keyword_position_index);keyword_var++)
                                        {
                                            printf("\033[D");
                                        }
                                    }
                                }
                                fflush(stdout);
                                break;
                            }
                            case KEYCODE_INSERT:
                            {
                                //printf("\033[3~");
                                //printf("\ninsert\n");
                                if(insert_mode)
                                {
                                    insert_mode=0;
                                }
                                else
                                {
                                    insert_mode=1;
                                }
                                //fflush(stdout);
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                        break;
                    }
                    case 5:
                    {
                        break;
                    }
                    default:
                    {
                        break;
                    }
                } 
            }
            else
            {
                //printf("value: %c = 0x%02X = %d\n", c, c, c);
                int temp_i=0;
                char temp_c=0;
                for (temp_i=0;temp_i<read_char_len;temp_i++)
                {
                    temp_c=c[temp_i];
                    switch(temp_c)
                    {
                        case KEYCODE_BACKSPACE:
                        {
                            if(keyword_index>0)
                            {
                                if(keyword_position_index!=keyword_index)
                                {
                                    int keyword_var=0;
                                    for(keyword_var=0;keyword_var<keyword_index-keyword_position_index;keyword_var++)
                                    {
                                        printf("\033[C");
                                    }
                                    keyword_position_index=keyword_index;
                                    fflush(stdout);
                                }
                                printf(" \b\b \b");
                                fflush(stdout);
                                keyword_index--;
                                keyword_position_index=keyword_index;
                                keyword[keyword_index]=0x00;
                            }
                            break;
                        }
                        default:
                        {
                            if(temp_c=='\n')
                            {
                                if(keyword_index>0)
                                {
                                    //string tempStr=(string)(keyword);
                                    tcl_cmd_str=(string)(keyword);
                                    // printf("%d:%s:%s\n",keyword_index,keyword,tcl_cmd_str.c_str());
                                    auto history_it=std::find(history.begin(),history.end(),tcl_cmd_str);
                                    if(history_it!=history.end())
                                    {
                                        // exist;
                                    }
                                    else
                                    {
                                        history.push_back(tcl_cmd_str);
                                    }
                                }
                                else if(keyword_index==0)
                                {
                                    // tcl_cmd_str=history[history_index];
                                    tcl_cmd_str="";
                                }
                                else
                                {
                                    // nothing;
                                }
        
                                auto system_cmds_it=std::find(system_cmds.begin(),system_cmds.end(),tcl_cmd_str);
                                if(system_cmds_it!=system_cmds.end())
                                {
                                    //find command;
                                    printf("\nsystem command");
                                    string ls_cmd="ls";
                                    if(tcl_cmd_str==ls_cmd)
                                    {
                                        //printf("compare success\n");
                                        FILE *pipe=popen("ls","r");
                                        if(!pipe)
                                        {
                                            printf("exe fail\n");
                                        }
                                        char temp_buffer[128];
                                        std::string temp_result="";
                                        while(!feof(pipe))
                                        {
                                            if(fgets(temp_buffer,128,pipe)!=NULL)
                                            {
                                                temp_result+=temp_buffer;
                                            }
                                        }
                                        pclose(pipe);
                                        cout<<temp_result;
                                        printf("\nrun here");
                                    }
                                }
                                else
                                {
                                    const char *tcl_cmd_chs=tcl_cmd_str.c_str();
                                    Tcl_Eval(interp, tcl_cmd_chs);
                                    printf("\n");
                                    cout << Tcl_GetStringResult(interp);
                                    tcl_result=Tcl_GetStringResult(interp);
                                    if((string)tcl_result=="exit")
                                    {
                                        termios_raw.c_lflag |= (ICANON | ECHO);
                                        tcsetattr(kfd, TCSANOW, &termios_raw);
                                        printf("\n");
                                        Tcl_DeleteInterp(interp);
                                        fflush(stdout);
                                        exit(0);
                                    }
                                    /*
                                    if(tcl_cmd_str=="exit")
                                    {
                                        termios_raw.c_lflag |= (ICANON | ECHO);
                                        tcsetattr(kfd, TCSANOW, &termios_raw);
                                        printf("\n");
                                        fflush(stdout);
                                        Tcl_DeleteInterp(interp);
                                        exit(0);
                                    }
                                    else
                                    {
                                        
                                    }                                
                                    */
                                }
                                printf("\n%%->");
                                fflush(stdout);
                                keyword_index=0;
                                keyword_position_index=keyword_index;
                                memset(keyword,0,KEYWORD_MAX_LENGTH);
                            }
                            else
                            {
                                if((keyword_index==0)&&((temp_c>=0x21)&&(temp_c<=0x7e)))
                                {
                                    keyword[keyword_index]=temp_c;
                                    keyword_index++;
                                    keyword_position_index=keyword_index;
                                    printf("%c",temp_c);
                                    fflush(stdout);
                                } 
                                else if((keyword_index>0)&&((temp_c>=0x20)&&(temp_c<=0x7e)))
                                {
                                    if(keyword_position_index<keyword_index)
                                    {
                                        if(insert_mode)
                                        {
                                            // insert mode;
                                            memcpy(keyword_swap_buffer,keyword,keyword_index);
                                            memset(keyword,0,KEYWORD_MAX_LENGTH);
                                            memcpy(&keyword[0],&keyword_swap_buffer[0],keyword_position_index);
                                            keyword[keyword_position_index]=temp_c;
                                            memcpy(&keyword[keyword_position_index+1],&keyword_swap_buffer[keyword_position_index],(keyword_index-keyword_position_index));
                                            keyword_index++;
                                            printf("\r\033[K%%->%s",keyword);
                                            int keyword_var=0;
                                            for(keyword_var=0;keyword_var<(keyword_index-keyword_position_index);keyword_var++)
                                            {
                                                printf("\033[D");
                                            }
                                        }
                                        else
                                        {
                                            // replace mode;
                                            keyword[keyword_position_index]=temp_c;
                                            keyword_position_index++;
                                            printf("%c",temp_c);
                                        }                        
                                        fflush(stdout);
                                    }
                                    else
                                    {
                                        keyword[keyword_index]=temp_c;
                                        if(keyword_index<KEYWORD_MAX_LENGTH-1)
                                        {
                                            keyword_index++;
                                            keyword_position_index=keyword_index;
                                        }
                                        printf("%c",temp_c);
                                        fflush(stdout);
                                    }
                                }
                            }
                            
                            break;
                        }
                    }
                }
                
            }
        }
    }
    Tcl_DeleteInterp(interp);
    return 0;
}




