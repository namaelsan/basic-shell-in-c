#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#define true 1
#define false 0

/* 
Alıntı Yapılan Yerler:
https://stackoverflow.com/questions/8257714/how-can-i-convert-an-int-to-a-string-in-c
https://stackoverflow.com/questions/7292642/grabbing-output-from-exec


 */





/* hata mesajını printler ve proccessi kapatır */
int errorprint(char *str){
    write(2,str,strlen(str));
    exit(1);
}

/* COMMENT */
int find_nextline(char *str, int len){
    for (int i=0; i<len; i++){
        if(str[i]== '\n'){
            return i;
        }
    }
    return -1;
}

/* COMMENT */
/* bu fonksiyonda stackoverflowdan alıntı yaptım. sayfanın linki dosyanın tepesindeki yorum */
char *where(char *argc,int len){


    char buffer[256]={0};
    int link[2];
    argc[find_nextline(argc,len)]=0;

    if(pipe(link)== -1)
        errorprint("There was a problem with pipe");
    
    pid_t pid = fork();
    if(pid == -1)
        errorprint("There was a problem with fork");    

    if (pid == 0) {
        dup2 (link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execlp("/usr/bin/which", "which", argc, NULL);
        exit(0);

    } else {
        close(link[1]);
        read(link[0], buffer, sizeof(buffer));

        int nextln=find_nextline(buffer,256);
        char *path=malloc(nextln + 1);
        strcpy(path,buffer);
        path[nextln]=0;

        wait(NULL);

        if(path[0]==0){
            return NULL;
        }

        return path;
    }
}

/* kullanıcı inputunu çift tırnakları da göze alarak parçalara ayırır ve array olarak döndürür */
char **split(char *str, char *tokens[]) {
    int i = 0;
    int quotes = 0; // çift tırnak olması durumunda kullanacağımız flag
    char *start = str;

    while (*str) {
        if (*str == '"') {
            quotes = !quotes;
            if (!quotes) {
                *str = '\0';
                // çift tırnak var ise çift tırnak içindeki ifadeyi token olarak al
                start++;
                if (start != str) {
                    tokens[i] = start;
                    i++;
                }
                start = str + 1; // çift tırnaklı ifadelerden sonra yeni tokene geçer
            }
        } else if (!quotes && (*str == ' ' || *str == '\t' || *str == '\n')) {
        
            *str = '\0';
            if (start != str) {
                tokens[i] = start;
                i++;
            }
            start = str + 1; // yeni tokene geçer
        }
        str++;
    }

    // dizide space'den sonra kalan bir şey olduysa token olarak alır
    if (start != str) {
        tokens[i] = start;
        i++;
    }

    tokens[i] = NULL;

    return tokens;
}

/* COMMENT */
int execute(char *path,char **inp){
    int status;

    pid_t pid = fork();
    if(pid == -1)
        errorprint("There was a problem with fork");    

    if (pid == 0) {
        execvp(path, inp);
        exit(1);

    } else {
        wait(&status);
        return status;
    }
}


/* COMMENT */
int main(int argv, char *argc[]){
    
    char *newargc=malloc(sizeof(char)*256);
    int status;
    char **inp;
    char **tokens=malloc(sizeof(char)*256*256);
    char *path;
    char *returnd=NULL;
    char temp[256]="./";
    temp[2]=0;

    while(true){

        for(int i=0;i<256;i++){
            newargc[i]=0;
        }

        write(1,"$",1);
        read(0,newargc,256);
        inp=split(newargc,(char **)tokens);
        path=where(inp[0],256);

        /* input exit için kontrol edilir */
        if(!strcmp(inp[0],"exit")){
            if (inp[1]==NULL){
                if(returnd){
                    free(returnd);
                }
                if(path){
                    free(path);
                }
                free(tokens);
                free(newargc);
                exit(0);
            }
            continue;
        }

        if(!path){
            if(access(tokens[0], F_OK) == 0){
                strcat(temp,tokens[0]);
                path=temp;
            }else{
                write(STDOUT_FILENO,newargc,strlen(newargc));
                write(STDOUT_FILENO,": command not found\n",20);
                continue;
            }
        }

        status=execute(path,inp);
        returnd=malloc(snprintf(NULL, 0,"%d",status)); /* stack overflowdan alıntı */
        sprintf(returnd, "%d", status); /* stack overflowdan alıntı */

        if(!(status==0)){
            write(STDOUT_FILENO,"There was an error while executing your command. \nreturn value:",63);
            write(STDOUT_FILENO, returnd,strlen(returnd));
            write(STDOUT_FILENO,"\n",1);
        }
    }
}
