/*
Enhar Apuhan 22120205012
Mervenur Saraç 22120205055
2024-03-30

Program system call'lari kullanarak kullanicidan alinan komutlar calistirilir.which ile komutun hangi dizide bulundugu belirlenir.fork ve execvp cagrilari ile 
yeni process olusturulur ve komutlar islenir.Girilen inputlar ve outputlar lox.txt isimli dosyaya yazdirilir.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#define true 1
#define false 0

/* 

Alıntı Yapılan Yerler:
https://stackoverflow.com/questions/8257714/how-can-i-convert-an-int-to-a-string-in-c

*/

/*
split fonksiyonu yazılırken chatGPT'den yardım alınmıştır.
*/





/* hata mesajını printler ve proccessi kapatır */
int errorprint(char *str){
    write(2,str,strlen(str));
    exit(1);
}

/* string içindeki ilk \n in konumunu bulur ve döndürür */
int find_nextline(char *str, int len){
    for (int i=0; i<len; i++){
        if(str[i]== '\n'){
            return i;
        }
    }
    return -1;
}

/* kullanıcıdan alınan komutun hangi dizinde olduğunu belirler ve döndürür */  
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
        exit(1);

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

    tokens[i++] = NULL;

    for(;i<256;i++){
        tokens[i]=NULL;
    }

    return tokens;
}

/* verilen değişkenler child process içinde çalıştırılır ve status döndürür */
int execute(char *path,char **inp){
    int status;

    pid_t pid = fork();
    if(pid == -1)
        errorprint("There was a problem with fork");    

    if (pid == 0) {
        execvp(path, inp);
        exit(1);

    } else {
        waitpid(pid, &status, 0); 
        return status;
    }
}
/*  */
char *trim(char *str){
    int len=strlen(str);

    for(int i=0;i<len;i++){
        if(str[i]==' '){
            if(str[i+1]=='\n'){
                str[i+1]=0;
                str[i]='\n';
                return &str[i];
            }
        }
        else{
            return &str[i];
        }
    }
}




/* ana fonksiyon */
int main(){
    
    char *argv=malloc(sizeof(char)*256);
    int status,statuinfo;
    char **inp;
    char **tokens=malloc(sizeof(char *)*256);
    char *path;
    char *returnd=NULL;
    char *header=malloc(sizeof(char)*256);
    char *newargv;

    
    // dosyayı okuma modu ile açar dosya yoksa oluşturur
    int fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(fd == -1)
       errorprint("there was a problem with opening file");
    
    

    while(true){

        /* which'le konumu bulunamayan dosyayı çalıştırırken kullanılan header sıfırlanır */
        header[0]='.';header[1]='/';
        memset(header+2,0,253);

        /* inputu alan değişken sıfırlanır */
        memset(argv,0,256);

        /* input alınır */
        write(1,"$",1);
        read(0,argv,256);
        newargv=trim(argv);

        /* boş string girildiyse işlem yapılmaz */
        if (newargv[0] =='\n'){
            continue;
        }

        /* gettimeofday() ile komutun alındığı zaman log.txt dosyasına yazdırılır */
        struct timeval tv; 
        gettimeofday(&tv,NULL); 
        char time[30];       
        strftime(time,sizeof(time),"%Y-%m-%d %H:%M:%S\t", localtime(&tv.tv_sec));
        write(fd,time,strlen(time));            
        write(fd, newargv, strlen(newargv));

        inp=split(newargv,(char **)tokens); /* input stringi tokenlere ayrılır */
        path=where(inp[0],256); /* programın konumu bulunur */

        /* input exit için kontrol edilir */
        if(!strcmp(inp[0],"exit")){
            if (inp[1]==NULL){
                if(returnd){
                    free(returnd);
                }
                if(path){
                    free(path);
                }
                free(header);
                free(tokens);
                free(argv);
                close(fd);
                exit(0);
            }
            continue;
        }

        /* dosyanın bulunmadığı durumda ya hata verilir yada klasördeyse ilk argümanın başına ./ koyulur */
        if(!path){
            if(access(tokens[0], F_OK) == 0){
                strcat(header,tokens[0]);
                path=header;
            }else{
                write(STDOUT_FILENO,newargv,strlen(newargv));
                write(STDOUT_FILENO,": command not found\n",20);
                continue;
            }
        }


        /* uygulama hata vererek kapandıysa hata kodunu printle */
        statuinfo=execute(path,inp);
        if ( WIFEXITED(statuinfo) ) {
            status = WEXITSTATUS(statuinfo);
            returnd=malloc(sizeof(char)*snprintf(NULL, 0,"%d",status)+1); /* stack overflowdan alıntı */
            sprintf(returnd, "%d", status); /* stack overflowdan alıntı */
            if(!(status==0)){
                write(STDOUT_FILENO,"There was an error while executing your command. \nreturn value:",63);
                write(STDOUT_FILENO, returnd,strlen(returnd));
                write(STDOUT_FILENO,"\n",1);
            }
        }
    }
}
