

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
// #include <conio.h> //windows os;


#define KEYCODE_R 0x43
#define KEYCODE_L 0x44
#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_Q 0x71
#define KEYCODE_DELETE 0x7e
#define KEYCODE_BACKSPACE 0x7f

using namespace std;


char* myCppcmd() {
        return "This is return string.";
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


int kfd = 0;


int main(int argc, char **argv){
    //string input;
    std::string tcl_cmd_str="";
    struct termios cooked, raw;
    char c;
    // #define HISTORY_NUMBER 4
    std::vector<std::string> system_cmds={"ls","pwd"};
    std::vector<std::string> history;//={"ls","mv a.txt b.txt","mkdir bill","test"};
    int history_index=0;

    // get the console in raw mode
    tcgetattr(kfd, &cooked); // �õ� termios �ṹ�屣�棬Ȼ�����������ն�
    // exit(0);
    memcpy(&raw, &cooked, sizeof(struct termios));
    //raw.c_lflag &=~ (ICANON);
    raw.c_lflag &=~ (ICANON | ECHO);
    // Setting a new line, then end of file
    //raw.c_cc[VEOL] = 1;
    //raw.c_cc[VEOF] = 2;
    raw.c_cc[VMIN]=1;
    raw.c_cc[VTIME]=0;
    tcsetattr(kfd, TCSANOW, &raw);
    /*
    raw.c_lflag |= (ICANON | ECHO);
    tcsetattr(kfd, TCSANOW, &raw);
    exit(0);
    */


    
  
    // puts("Reading from keyboard");
    // puts("---------------------------");
    // puts("Use arrow keys to move the robot.");
    // puts("otherwise the key values will be printed");


    Tcl_Interp* interp = Tcl_CreateInterp();
    Tcl_CreateCommand(interp, "mycmd", MyTclCmd, NULL, NULL);
    #define KEYWORD_MAX_LENGTH 256
    int keyword_index=0;
    char keyword[KEYWORD_MAX_LENGTH];
    memset(keyword,0,KEYWORD_MAX_LENGTH);
    // set_raw_mode(STDIN_FILENO);
    printf("%%->");
    fflush(stdout);
    // while(true) 
    for(;;)
    {
        //cout << " % ";
        //getline(cin, input);
        //if(input=="exit") break;

        // get the next event from the keyboard
        if(read(kfd, &c, 1) < 0)
        {
            perror("read():");
            exit(-1);
        }
        // printf("value: %c = 0x%02X = %d\n", c, c, c);
        switch(c)
        {
            case KEYCODE_L:
            {
                //printf("\b");
                //printf("\x1b[D");
                printf("\033[D");
                fflush(stdout);
                keyword_index=0;
                memset(keyword,0,KEYWORD_MAX_LENGTH);
                break;
            }
            case KEYCODE_R:
            {
                printf("\033[C");
                fflush(stdout);
                keyword_index=0;
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
                keyword_index=0;
                memset(keyword,0,KEYWORD_MAX_LENGTH);
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
                keyword_index=0;
                memset(keyword,0,KEYWORD_MAX_LENGTH);
                break;
            }
            case KEYCODE_DELETE:
            {
                //printf("\033[3~");
                fflush(stdout);
                break;
            }
            case KEYCODE_BACKSPACE:
            {
                if(keyword_index>1)
                {
                    printf(" \b\b \b");
                    fflush(stdout);
                    keyword_index--;
                    keyword[keyword_index]=0x00;
                }
                break;
            }
            default:
            {
                //printf("value: %c = 0x%02X = %d\n", c, c, c);
                if(c=='\n')
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
                        tcl_cmd_str=history[history_index];
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
                        if(tcl_cmd_str=="exit")
                        {
                            raw.c_lflag |= (ICANON | ECHO);
                            tcsetattr(kfd, TCSANOW, &raw);
                            printf("\n");
                            fflush(stdout);
                            exit(0);
                        }
                        else
                        {
                            const char *tcl_cmd_chs=tcl_cmd_str.c_str();
                            Tcl_Eval(interp, tcl_cmd_chs);
                            printf("\n");
                            cout << Tcl_GetStringResult(interp);
                        }

                    }


                    printf("\n%%->");
                    fflush(stdout);
                    keyword_index=0;
                    memset(keyword,0,KEYWORD_MAX_LENGTH);
                }
                else
                {
                    if((keyword_index==0)&&((c>=0x21)&&(c<=0x7e)))
                    {
                        keyword[keyword_index]=c;
                        keyword_index++;
                    } 
                    else if((keyword_index>0)&&((c>=0x20)&&(c<=0x7e)))
                    {
                        keyword[keyword_index]=c;
                        if(keyword_index<KEYWORD_MAX_LENGTH-1)
                        {
                            keyword_index++;
                        }
                    }
                }
                if(c!='\n')
                {
                    //printf("%d:%c ",keyword_index,c);
                    printf("%c",c);
                    fflush(stdout);
                }
                break;
            }
        }
        
    }

    return 0;
}
