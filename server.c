/* C header files */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/* Socket API headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "iniparser.h"

/* Definations */
#define DEFAULT_BUFLEN 512
//#define PORT 4239
DIR * pDir;
struct dirent * pDirEnt;
int dirExist;
int parse_ini_file(char * ini_name);
FILE * fptr;
char * toChar(int n) 
{
    char * convText, text[100];
    sprintf(text, "%d", n);
    convText = text;
    return convText;
}

bool userAuth(char *a,int size,char userId[])
{
    //to remove the next line at the end
    a[size-1]=0;
    dictionary *dict;
    dict=iniparser_load("a.ini");

    //there is always on extra char in socket buffer
    int numOfChars = size - 1,i=0;
    bool me =false;
    char *key,*username,*password,*token;
    /*this is the delimiter when retreving different value
        from the string */
    const char *delimiter=" ";

    /*first string is the keyword 
      Second userId and finally password*/
    key=strtok(a,delimiter);
    printf("Key :%s\n",key );
    username = strtok(NULL,delimiter);
    printf("Username :%s\n",username );
    password = strtok(NULL,delimiter);
    printf("Password :%s\n",password );

    if (strcasecmp(key,"user") != 0)
    {
        printf("Error wrong key used should be user or USER\n");
    }
    else
    {
        printf("checking username and password\n");
        char userSearchKey[100];
        strcpy(userSearchKey,"users:");
        strcat(userSearchKey,username);
        printf("%s\n",userSearchKey );
        char *userSearchKeyPass = iniparser_getstring(dict,userSearchKey,"NULL");
        if ( strcmp(userSearchKeyPass,password) == 0)
        { 
            me = true ;
            strcpy(userId,username);
            printf("Hello %s  your password is %s\n",username,userSearchKeyPass );
        }
        else
        {
            printf("%s\n%s\n",userSearchKeyPass,password );
            printf("Password Incorrect\n");
        }
    }
    return me;
}


void do_job(int fd) 
{
    int length,rcnt;
    char recvbuf[DEFAULT_BUFLEN];
    int  recvbuflen = DEFAULT_BUFLEN;
    char userId[4];
    dictionary *dict;
    dict=iniparser_load("a.ini");
    // to authenticate the user
    bool user_authenticated = false ;
    do
    {
        rcnt = send(fd, "Please write user followed by your userId and password\n ",strlen("Please write user followed by your userId and password\n"),0);
        rcnt = send(fd, "in the following format <user> <yourId> <yourpassword>\n ",strlen("in the following format <user> <yourId> <yourpassword>\n"),0);
        rcnt = recv(fd, recvbuf, recvbuflen, 0);
        user_authenticated = userAuth(recvbuf,rcnt,userId);
       
    }while(!user_authenticated);
    while (1) 
    {

        rcnt = send(fd, "\n-----------------------------------", strlen("\n-----------------------------------"), 0);
        rcnt = send(fd, "\nPlease choose what you want to do\n", strlen("\nPlease choose what you want to do\n"), 0);
        rcnt = send(fd, "'SEND'.To send  messages to another users\n", strlen("'SEND'.To send  messages to another users\n"), 0);
        rcnt = send(fd, "'LIST'.To list/delete  messages\n", strlen("'LIST'.To list/delete  messages\n"), 0);
        rcnt = send(fd, "'EXIT'.To exit\n", strlen("'EXIT'.To exit\n"), 0);
        rcnt = send(fd, "-----------------------------------", strlen("-----------------------------------"), 0);
        rcnt = send(fd, "\nYour choise :", strlen("\nYour choise :"), 0);
        rcnt = recv(fd, recvbuf, recvbuflen, 0);
        strncpy(choise, recvbuf, 4);

        if (strncasecmp(choise,"send",4 ) == 0) 
        {
            userChoice = 1;
        }
        else if (strncasecmp(choise,"list",4 ) == 0) 
        {
          userChoice = 2;
        }
        else if(strncasecmp(choise,"exit",4 ) == 0)
        {
            userChoice = 3;
        }
        else
        {
            userChoice = 0;
        }
        switch(userChoice)
        {
            case 0:
            rcnt = send(fd,"wrong choice please try again........\n",strlen("wrong choice please try again........\n"),0);
            break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
    }
}



int main()
{
int server, client;
struct sockaddr_in local_addr;
struct sockaddr_in remote_addr;
int length,fd,rcnt,optval;
pid_t pid;

/* Open socket descriptor */
if ((server = socket( AF_INET, SOCK_STREAM, 0)) < 0 ) { 
    perror("Can't create socket!");
    return(1);
}


/* Fill local and remote address structure with zero */
memset( &local_addr, 0, sizeof(local_addr) );
memset( &remote_addr, 0, sizeof(remote_addr) );

/* Set values to local_addr structure */
local_addr.sin_family = AF_INET;
local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
local_addr.sin_port = htons(PORT);

// set SO_REUSEADDR on a socket to true (1):
optval = 1;
setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

if ( bind( server, (struct sockaddr *)&local_addr, sizeof(local_addr) ) < 0 )
{
    /* could not start server */
    perror("Bind error");
    return(1);
}

if ( listen( server, SOMAXCONN ) < 0 ) {
        perror("listen");
        exit(1);
}

printf("Concurrent  socket server now starting on port %d\n",PORT);
printf("Wait for connection\n");

while(1) {  // main accept() loop
    length = sizeof remote_addr;
    if ((fd = accept(server, (struct sockaddr *)&remote_addr, \
          &length)) == -1) {
          perror("Accept Problem!");
          continue;
    }

    printf("Server: got connection from %s\n", \
            inet_ntoa(remote_addr.sin_addr));

    /* If fork create Child, take control over child and close on server side */
    if ((pid=fork()) == 0) {
        close(server);
        do_job(fd);
        printf("Child finished their job!\n");
        close(fd);
        exit(0);
    }

}

// Final Cleanup
close(server);

}
