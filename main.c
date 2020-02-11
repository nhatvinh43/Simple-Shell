#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAXLINE 80

const char prompt[] = "osh>";

typedef struct redirectDataset
{
    char command[MAXLINE];
    int fileDescriptor;
}redirectDataset;

int tokenizeCommand(char* command, char** args)
{
    char* commands = strdup(command);
            
    char* token = strtok(commands," ");
    args[0]=token;
    int i=1;
    
    while(token!=NULL)
    {
        token=strtok(NULL, " ");
        args[i]=token;
        i++;
    }
    args[i]=NULL;
    
    return i-1;
}

void saveHistory(char* cmd)
{
    FILE* fp = fopen("history.txt","w");
    fprintf(fp,"%s",cmd);
    fclose(fp);
}

void standardize(char* src)
{
    while (src[strlen(src)-1] == ' ')
        src[strlen(src)-1]='\0';
}

redirectDataset redirectCommand(char operator, char* command)
{
    redirectDataset result;
    int fileDescriptor;
    char separator;
    int redirectMode;
    if (strstr(command, ">") != NULL) {
        separator = '>';
        redirectMode = 1;
    } else {
        separator = '<';
        redirectMode = 2;
    }

    int length = strlen(command);
    int index = 0;
    char* tempCommand1 = (char*) malloc(MAXLINE); //chua phan command
    char* tempCommand2 = (char*) malloc(MAXLINE); //chua phan ten file redirect

    for (int i = 0; i < length; i++) //Vong lap tach chuoi + bo dau khoang trang cuoi chuoi moi
    {
        if (command[i] == separator) {
            if (tempCommand1[i - 1] == ' ') //Neu truoc dau > la khoang trang
            {
                tempCommand1[i - 1] = '\0';
            } else tempCommand1[i] = '\0';
            index = i;
            break;
        }
        tempCommand1[i] = command[i];
    }
    index++;
    
    while(command[index]==' ') //Bo qua cac khoang trang
    {
        index++;
    }
    
    //Bat dau copy phan redirect
    int k = 0;
    for (int i = index; i < length; i++) {
        tempCommand2[k] = command[i];
        k++;
    }
    tempCommand2[k] = '\0';


    char* filename = strdup(tempCommand2);
    command = strdup(tempCommand1);

    //Mo file va lay file description
    if (separator == '>') //Set quyen chi ghi
    {
        remove(filename);
        fileDescriptor = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
    } else //Set quyen chi doc
    {
        fileDescriptor = open(filename, O_RDONLY | O_CREAT, S_IRWXU);
    }
    
    strcpy(result.command,command);
    result.fileDescriptor=fileDescriptor;
    return result;
}

int main(void)
{
    char *args[MAXLINE/2 +1];
    int shouldrun = 1;
    char* command = (char*)malloc(MAXLINE*sizeof(char));
    char* HistoryCommand = (char*)malloc(MAXLINE*sizeof(char));  
    
    char* command1= NULL; //Tao 2 chuoi de luu cac lenh pipe
    char* command2= NULL;
    
    
    // read history file
    
    FILE* fp = fopen("history.txt","r");
    fscanf(fp,"%[^\n]%*c", HistoryCommand);
    fclose(fp);
    
    while (shouldrun)
    {
        int pipeFlag=0; //Check co yeu cau flag hay khong
        int redirectMode=0; //Bien check xem co yeu cau redirect khong, gia tri 0 tuc la ko co, 1 la redirect >, 2 la redirect <
    	int fileDescriptor=1; //Bien de luu tru gia tri trong truong hop redirect;

        printf("%s", prompt);
        
        fflush(stdin);
        
        gets(command);
        
        standardize(command);
        
        int flag = 0;
        
        if(strcmp(command,"!!")!=0)  //Khong co yeu cau !!, luu cau lenh hien tai
        {
             HistoryCommand = strdup(command);
             saveHistory(HistoryCommand);
        }
        else //Co yeu cau !!
        {
            if(strlen(HistoryCommand)==0) //Khong tim thay cau lenh nao gan day
            {
                printf("No commands in history!\n");
        {
            
        }  continue;
            }
            else //Da tim thay cau lenh, gan cau lenh hien tai bang cau lenh cu
            {
                command=strdup(HistoryCommand);
                printf("%s\n",command);
               
            }  
        }
        
        if (strstr(command,"&")!=NULL)
            flag = 1;
        
        char* tempCmd ;
        
        if (flag == 1)
        {
            tempCmd = (char*)malloc(MAXLINE*sizeof(char));
            for (int i=0;i<=strlen(command);i++)
            {
                tempCmd[i]=command[i];
            }
            tempCmd[strlen(command)-1] = '\0';
        }
        else
        {
            if (tempCmd!=NULL)
            {
                memset(tempCmd,'\0',strlen(tempCmd));
            }
        }
        

        if(strstr(command,">")!= NULL || strstr(command,"<")!= NULL ) //Co yeu cau redirect
        {
            int length=strlen(command);
            char tempString[MAXLINE];
            strcpy(tempString, command);
            char separator;
            
            if(strstr(command,">")!= NULL)
            {
                redirectMode=1;
                separator='>';
            }
            else 
            {
                redirectMode=2;
                separator='<';
            }
            
            redirectDataset temp;
            temp=redirectCommand(separator, tempString);
            command=strdup(temp.command);
            fileDescriptor=temp.fileDescriptor;
        }

        
        //Check yeu cau su dung pipe:
        if(strstr(command,"|")!= NULL) //Co ky tu |
        {
            pipeFlag=1;
            command1= (char*) malloc(MAXLINE); //Tao 2 chuoi de luu cac lenh
            command2= (char*) malloc(MAXLINE);
            int length = strlen(command);
            int index = 0;
            
            for (int i = 0; i < length; i++) //Vong lap tach chuoi
            {
                if (command[i] == '|') 
                {
                    command1[i]='\0';
                    index = i+1;
                    break;
                }
                command1[i] = command[i];
            }
            
            while(command[index]==' ')
            {
                index++;
            }
            
            int k=0;
            for (int i = index; i < length; i++) //Vong lap tach chuoi + bo dau khoang trang cuoi chuoi moi
            {
                command2[k] = command[i];
                k++;
            }
            command2[index]="\0";
            
        }
        
        if (strcmp(command, "\0")==0)
        {
            fflush(stdin);
        }
        else if (strcmp(command,"exit")==0)
        {
            shouldrun = 0;
        }
        else 
        {
            
            int count;
            if (flag == 1)
            {
                count = tokenizeCommand(tempCmd, args);
            }
            else count = tokenizeCommand(command,args);
            
            pid_t childPid = fork();
            
            if(childPid==0) //Process con
            {
                if(redirectMode==1) //Redirect voi operator >
                {
                    dup2(fileDescriptor, 1);
                }
                else if(redirectMode==2) //Redirect voi operator <
                {
                    dup2(fileDescriptor,0);
                }
                
                if(pipeFlag==1) //Co yeu cau pipe
                {
                    //Tao pipe cho process con 1  va 2
                    // fd[0] la READ_END; fd[1] la WRITE_END
                    int fd[2];
                    pipe(fd);
                    
                    //Fork ra process con thu 2
                    pid_t nestedChildrenPid = fork();
                    if(nestedChildrenPid==0) //Process con thu 1
                    {
                        //Truyen output sang WRITE_END
                        dup2(fd[1],1);
                        close(fd[0]);
                        close(fd[1]);
                        tokenizeCommand(command1,args);
                        if(execvp(args[0],args)==-1)
                        {
                            printf("Error executing command!\n");
                            exit(0);
                        }
                        else exit(0);
                    }
                    else if(nestedChildrenPid>0) //Process con thu 2
                    {
                        //Doc output tu READ_END
                        dup2(fd[0],0);
                        close(fd[0]);
                        close(fd[1]);
                        tokenizeCommand(command2,args);
                        if(execvp(args[0],args)==-1)
                        {
                            printf("Error executing command!\n");
                            exit(0);
                        }
                        else exit(0);    
                    }
                    else 
                    {
                        printf("Error executing pipe command!\n");
                    }
                    exit(0);
                }
                
                if (execvp(args[0],args)==-1)
                {
                    printf("Error executing command!\n");
                    close(fileDescriptor);
                    exit(0);
                }
                else
                {
                    close(fileDescriptor);
                    exit(0);
                }
            }
            else if (childPid>0)
            {
                wait(NULL);
                if (flag==1)
                {  
                    
                    continue;
                }
                else wait(&childPid);
            }
            else      
            {
                printf("Error creating process!");
            }
        }     
    }
    //free pointer
    free(command);
    free(command1);
    free(command2);
    free(HistoryCommand);
    return EXIT_SUCCESS;
    
}