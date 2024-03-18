#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


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
char *where(char *argc,int len){

    char buffer[256]={0};
    int link[2];

    argc[find_nextline(argc,len)]=0;

    if(pipe(link)== -1){
        errorprint("There was a problem with pipe");
    }
    pid_t pid = fork();


    if(pid == -1){
        errorprint("There was a problem with fork");    
    }

    if (pid == 0) {
        dup2 (link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execlp("/usr/bin/which", "which", argc, NULL);
        exit(0);

    } else {
        close(link[1]);
        read(link[0], buffer, sizeof(buffer));

        int nextln=find_nextline(buffer,sizeof(buffer));
        char *path=malloc(nextln + 1);
        strcpy(path,buffer);
        path[nextln]='\0';

        wait(NULL);
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
execute(){

}

/* COMMENT */
int main(int argv, char *argc[]){
    
    char *newargc=malloc(sizeof(char)*256);

    write(1,"$",1);
    read(0,newargc,256);
    char **inp=split(newargc,tokens);
  // Print the tokens after split func
  printf("Tokens after split:\n");
  for (int i = 0; inp[i] != NULL; i++) {
    printf("-%s\n", inp[i]);
  }

    char *path=where(*inp,256);
    printf("\npath=%s\n",path);
    execute(path,inp);

    free(path);

}
