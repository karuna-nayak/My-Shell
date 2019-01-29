/****************************************************************
 * Name        : Karuna Nayak                                   *
 * Class       : CSC 415                                        *
 * Date        : 10/06/2018                                     *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>


#define BUFFERSIZE 256
#define PROMPT "myShell "
#define PROMPTSIZE sizeof(PROMPT)



//check for pipes
int checkPipes(char* in_string, char **token1, char **token2){
  int pipe_flag = 0;
  char *pipe_check = NULL;
  char *input = in_string;
  int counter = 0;
  
  pipe_check = strrchr(in_string, '|');
  if(pipe_check != NULL){
    pipe_flag = 1;       
    *token2 = pipe_check + 2;
    strtok(in_string,"|");
    *token1 = in_string;
  }
  return pipe_flag;

}

//check for redirection
int isRedirect(char **user_input, char **file ){
  char* redirect_append = NULL;
  //char* file = NULL;
  int redirect_flag = 0;
  redirect_append= strstr(*user_input, ">>");
  if (redirect_append != NULL) {
    *file = redirect_append + 3;
    redirect_flag = 3;
    strtok(*user_input, ">>");
  }
  
  else {
  // Check for > character in input 
    redirect_append = strrchr(*user_input, '>');
    //if redirecting output exists
    if (redirect_append != NULL){
      *file = redirect_append + 2;
      redirect_flag = 2;
      strtok(*user_input, ">") ;
    } 

    else {
    // Check for < character in input
      redirect_append = strchr(*user_input, '<');
     if(redirect_append != NULL){
        *file = redirect_append + 2;
        strtok(*user_input, "<");
        redirect_flag = 1;
      } 
    }
  }  
  return  redirect_flag;
}



int main(int* argc, char** argv)
{

char *buffer = NULL;
char *prompt = (char *) malloc (BUFFERSIZE) ;
buffer = (char *) malloc (BUFFERSIZE);
int execv_num;;
int myargc = 0;
//char *buf_input = NULL;
char buf_input[1024];
char *user_input = NULL;
char* token;
char **myargv = NULL;
char *pipe_strings1 = NULL;
char *pipe_strings2 = NULL ;
char *file = NULL;
char* redirect = NULL;
char* redirect_input = NULL;
char* infile = NULL;
char* pipe_check = NULL;
int check_redirect = 0;
int file_desc;
int fd = 0;
int i = 0;
int fspipe[2];
char *first_token = NULL;
int pipe_exist = 0;
int backgroundflag = 0;
char exit_code[] = "exit";
char *homedir = NULL;
size_t len = 0;
char *check_home = NULL;
char *new_prompt = NULL;
char ch;
int command_num =0;
static struct termios oldt, newt;
char **command_list = NULL;
int counter =0;

if((getcwd(buf_input, sizeof(buf_input))) == NULL){
      perror("pwd error");      
    } 
homedir = getenv("HOME");
check_home = strstr(buf_input, homedir);

if ( check_home != NULL){
  new_prompt = check_home + strlen(homedir);
  if (strcmp(new_prompt,"\0") != 0){
    new_prompt = new_prompt + 1;
  }
  strcpy(prompt,PROMPT);
  strcat(prompt, "~/");
  strcat(prompt,new_prompt) ;
  strcat(prompt, " >> ");
}
else{
  strcpy(prompt,PROMPT);  
  strcat(prompt,buf_input) ;
  strcat(prompt, " >> ");
}



printf("%s",prompt);
tcgetattr(STDIN_FILENO, &oldt);	
newt = oldt;	
newt.c_lflag &= ~(ICANON);	
tcsetattr(STDIN_FILENO, TCSANOW, &newt);

sleep(0.1);
fflush(stdin);  
  do{        
      ch = getchar();
      if((int)ch == 27){
        getchar();
        getchar();
        printf("\33[2K\r");
        printf("%s",prompt);
      }
      else{  
        if((int) ch == 127){
          
          int buf_len = counter;          
          if(buf_len != 0){
            buffer[buf_len-1] = 0;
            counter = counter -1;          
          }
          printf("\33[2K\r");
          printf("%s",prompt);
          printf("%s",buffer);
        } 
        else {
          if((int)ch != 10){
            buffer = realloc(buffer, sizeof(char *) * (++counter));
            buffer[counter -1] = ch;
          }  
          else{
              buffer = realloc(buffer, sizeof(char *) * (++counter));
              buffer[counter -1] = 0;
            }
        }
            

      }
        
  }while((int)ch != 10);
  
  //store the command in the array
  command_list = realloc(command_list, sizeof(char *) * (++command_num));
  command_list[command_num -1] =(char *) malloc(BUFFERSIZE);
  strcpy(command_list[command_num - 1],buffer);  


while( (strcmp(buffer, exit_code)) != 0){
  
   
  strtok(buffer, "\n");  
  user_input = (char *) malloc (BUFFERSIZE);

  
  if (strcmp(&buffer[strlen(buffer) -1], "&") == 0){
    backgroundflag = 1;
    strtok(buffer, "&");
  }
  //user_input = buffer;
  strcpy(user_input, buffer);
  
 // check if command is cd
  first_token = strtok(buffer, " ");
  
  if (strcmp(first_token, "cd") == 0){    
    if(chdir(strtok(NULL," ")) < 0){
      perror("chdir");      
    }
  }
  //check for pwd command
  else if(strcmp(first_token, "pwd") == 0){
      printf("%s",buf_input);
      printf("\n");  
    }

  else{
    //check for pipes
    pipe_exist = checkPipes(user_input, &pipe_strings1, &pipe_strings2);
  
    if(pipe_exist == 0){
      //if pipe does not exist, process
      free(myargv); 
      pid_t pid;
     //sleep(0.1); 
      fflush(stdout);
      pid = fork();
      if (pid == 0){
        //check for redirection
        check_redirect = isRedirect(&user_input, &file); 

        switch(check_redirect){
          case 0:
            break;
          case 1:
            file_desc = open(file, O_RDONLY);
            fd = dup2(file_desc,0);
            if (fd < 0){
              perror("dup2 ");
              exit(-1);
            }  
            close(file_desc);
            break;

          case 2: 
          //Write output to the file
            file_desc = open(file, O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC, S_IRWXG |S_IRWXU|S_IRWXO);
            fd = dup2(file_desc, 1);
            if (fd < 0){
              perror("dup2 ");
              exit(-1);
            }  
            close(file_desc);
            break;
          case 3:
          //append output to a file
            file_desc = open(file, O_WRONLY|O_APPEND|O_CREAT,S_IRWXG |S_IRWXU|S_IRWXO);
            fd = dup2(file_desc, 1);
            if (fd < 0){
              perror("dup2 ");
              exit(-1);
            }  
            close(file_desc);
            break; 

        }

        token = strtok(user_input, " ");
    
        while(token != NULL){
          myargv = realloc(myargv, sizeof(char *) * ++myargc);
          if(myargv == NULL){
            exit(-1);//Memory allocation failed
            printf("Memory allocation failed");
          }
            
          myargv[myargc - 1] = token;
          token = strtok(NULL, " ");
        }
          
        myargv = realloc(myargv, sizeof(char *) * (myargc+1));
        myargv[myargc] = 0;       
        execv_num = execvp(&*myargv[0], myargv);   
        
        if (execv_num < 0){
          perror("Eroor in child process");
          exit(-1);
        }
        exit(0);  

      }
      else if(pid >  0){
        pid_t cpid = pid;
        int status;
        //parent process
        if(backgroundflag == 0){
          waitpid(cpid, &status, 0);
          
        }
        
      }
      else{
        printf("process fail");
        perror("pid");
      } 

    }  

    else if(pipe_exist == 1) { 
      
      sleep(0.1);
      fflush(stdout);
      fflush(stdin);
      pipe(fspipe);
      pid_t pid1 = fork();
      if (pid1 == (pid_t) 0){
        //tokens for LHS of |
        token = strtok(pipe_strings1, " ");
      
        while(token != NULL){
          myargv = realloc(myargv, sizeof(char *) * ++myargc);
          if(myargv == NULL){
            exit(-1);//Memory allocation failed
            printf("Memory allocation failed");
          }
            
          myargv[myargc - 1] = token;
          token = strtok(NULL, " ");
        }
            
        myargv = realloc(myargv, sizeof(char *) * (myargc+1));
        myargv[myargc] = 0;

        close(fspipe[0]);
        dup2(fspipe[1], 1);      
        close(fspipe[1]);
        
        execvp(&*myargv[0], myargv);
        exit(0);
        
      }
      else if (pid1 > (pid_t) 0){
        free(myargv);
        fflush(stdout);
        fflush(stdin);
        pid_t pid2 = fork();
        if (pid2 == (pid_t) 0){
          //tokens for RHS of |
          token = strtok(pipe_strings2, " ");
        
          while(token != NULL){
            myargv = realloc(myargv, sizeof(char *) * ++myargc);
            if(myargv == NULL){
              exit(-1);//Memory allocation failed
              printf("Memory allocation failed");
            }
              
            myargv[myargc - 1] = token;
            token = strtok(NULL, " ");
          }
              
          myargv = realloc(myargv, sizeof(char *) * (myargc+1));
          myargv[myargc] = 0;

          close(fspipe[1]);
          dup2(fspipe[0], 0);          
          close(fspipe[0]);
          
          if(execvp(&*myargv[0], myargv) < 0){
            perror("execvp ");
            exit(-5);
          }
          exit(0);
        }
        else if (pid2 > (pid_t) 0){
          close(fspipe[0]);
          close(fspipe[1]);
          wait(NULL);
          
          wait(NULL);
        }
        else{
          perror("pid2");
        }
      }
      else{
        perror("pid1 ");
      }

      

    }

  }  
  //code for extra credit2 :displaying current working dir
  if((getcwd(buf_input, sizeof(buf_input))) == NULL){
      perror("pwd error");      
    } 
  check_home = strstr(buf_input, homedir);
  if ( check_home != NULL){
    new_prompt = check_home + strlen(homedir);
   
    if (strcmp(new_prompt,"\0") != 0){
      new_prompt = new_prompt + 1;
    }
    strcpy(prompt,PROMPT);
    strcat(prompt, "~/");
    strcat(prompt,new_prompt) ;
    strcat(prompt, " >> ");
  }
  else{
    strcpy(prompt,PROMPT);  
    strcat(prompt,buf_input) ;
    strcat(prompt, " >> ");
  }  

  check_home = NULL; 
  new_prompt = NULL;
  check_redirect = 0;
  pipe_exist = 0;
  backgroundflag = 0; 
  free(buffer);  
  buffer = NULL;  
  pipe_strings1 = NULL;
  pipe_strings2 = NULL;
  free(myargv);
  free(user_input);
  user_input = NULL;
  myargc = 0;
  printf("%s",prompt);

  fflush(stdin);
  counter = 0;
  int temp_num = 0; 
  if(command_num != 0){
    temp_num = command_num;
  }
  else{
    temp_num = 0;
  }   
//code for extracredit3:command history
  do{          
      ch =getchar();
      if((int)ch == 27){
          getchar();
          int arrow_key = getchar();
          //check for up arrow
          if(arrow_key == 65){
              if(temp_num != 0){
                free(buffer);
                temp_num = temp_num -1;
                buffer = (char *)malloc(BUFFERSIZE);
                strcpy(buffer,command_list[temp_num]);
                // temp_num = temp_num -1;
                counter = strlen(buffer);
              }
              printf("\33[2K\r");
              printf("%s",prompt);
              printf("%s",command_list[temp_num]);          

          }
          //check for down key
          else if(arrow_key == 66){
            if(temp_num < command_num -1){
                free(buffer);
                temp_num = temp_num +1; 
                buffer = (char *)malloc(BUFFERSIZE);
                strcpy(buffer,command_list[temp_num]);
                counter = strlen(buffer);
              }
              printf("\33[2K\r");
              printf("%s",prompt);
              if(temp_num != command_num){
                printf("%s",command_list[temp_num]);
              }
              
              
          }
          
      }
      //check for backspace
      else if((int) ch == 127){
        int buf_len = counter;
        
        if(buf_len != 0){
          buffer[buf_len-1] = 0;
          counter = counter -1;          
        }
        printf("\33[2K\r");
        printf("%s",prompt);
        printf("%s",buffer);
      }
      else{
          if((int)ch != 10){
            
            buffer = realloc(buffer, sizeof(char *) * (++counter));
            buffer[counter -1] = ch;
          }
          else{
            buffer = realloc(buffer, sizeof(char *) * (++counter));
            buffer[counter -1] = 0;            
          }
          
      }    
  }while((int)ch != 10);
  
  //store the command in the array
  command_list = realloc(command_list, sizeof(char *) * (++command_num));
  command_list[command_num -1] =(char *) malloc(BUFFERSIZE);
  strcpy(command_list[command_num - 1],buffer);
  
  
  

}
    
return 0;
}
