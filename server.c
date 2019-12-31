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
	dict=iniparser_load("server.ini");

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
	// printf("Key :%s\n",key );
	username = strtok(NULL,delimiter);
	// printf("Username :%s\n",username );
	password = strtok(NULL,delimiter);
	// printf("Password :%s\n",password );

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
		// printf("%s\n",userSearchKey );
		char *userSearchKeyPass = iniparser_getstring(dict,userSearchKey,"NULL");
		if ( strcmp(userSearchKeyPass,password) == 0)
		{ 
			me = true ;
			strcpy(userId,username);
			printf("user %s  connected successfully\n",username);
		}
		else
		{
			printf("%s\n%s\n",userSearchKeyPass,password );
			printf("UserId or Password Incorrect\n");
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
	dict=iniparser_load("server.ini");
    // to authenticate the user
	bool user_authenticated = false ;
	do
	{
		rcnt = send(fd, "Please write user followed by your userId and password\n ",strlen("Please write user followed by your userId and password\n"),0);
		rcnt = send(fd, "in the following format <user> <yourId> <yourpassword>\n ",strlen("in the following format <user> <yourId> <yourpassword>\n"),0);
		rcnt = recv(fd, recvbuf, recvbuflen, 0);
		user_authenticated = userAuth(recvbuf,rcnt,userId);

	}while(!user_authenticated);

	const char * welcomeMsg = iniparser_getstring(dict,"server:ServerMsg","NULL");
	rcnt = send(fd , welcomeMsg,strlen(welcomeMsg),0);
    //this is option Menu
	int userChoice = 0;
	char choise[1000];
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
		const char * serverRoot = iniparser_getstring(dict, "server:ServerRoot", NULL);
		char dirId[1000];
		char file_to_open[1000];
		char sRoot_title[100];
		char msg_title[100];
		char msg_content[10000];
        //case 2 var
		int numOfmsgs = 0 ,tu=1;
		char messgesArray[100][recvbuflen];
		char msgOfUser[100];
		char msgChoice[10];
		char msg_to_read_or_delete[100];
		int msg_id;
		char msgcopy[1000];


		switch(userChoice)
		{
			case 0:
			rcnt = send(fd,"wrong choice please try again........\n",strlen("wrong choice please try again........\n"),0);
			break;
			case 1:

			rcnt = send(fd, "\nPlease Enter userId of  whom you want to send a message : \n", strlen("\nPlease Enter userId of  whom you want to send a message :\n"), 0);
			rcnt = recv(fd, recvbuf, recvbuflen, 0);
			pDir = opendir("");
			if (pDir == NULL) 
			{
				dirExist = mkdir(serverRoot, 0775);
				if (!dirExist)
					printf("Directory created\n");
				else 
                  printf("Unable to create directory,already exist\n");                // strcpy(dirId,serverRoot);
              strcpy(dirId,serverRoot);
              strcat(dirId,"/");
              strncat(dirId, recvbuf, 4);
              dirExist = mkdir(dirId, 0775);
              if (!dirExist)
              	printf("Directory created\n");
              else 
              	printf("Unable to create directory,already exist\n");
          }
          strcpy(file_to_open, dirId);
          strcat(file_to_open, "/");
          strcpy(msg_title, toChar((unsigned long)time(NULL)));
          strcat(msg_title, "_");
          strncat(msg_title, recvbuf, 4);
          strcat(msg_title, "_");
          rcnt = send(fd, "Please type your message \n", strlen("Please type your message \n"), 0);
          rcnt = recv(fd, recvbuf, recvbuflen, 0);
          strncpy(msg_content, recvbuf,(rcnt-1)  );
            //to clear anything that was already in the buffer
          msg_content[rcnt-1] = '\0';
          printf("\nMessage is %s\n", msg_content);
          strcat(msg_title, toChar(rcnt-1));
          strcat(msg_title, ".msg");
          strcat(file_to_open,msg_title);
            //to save to userId folder
          fptr = fopen(file_to_open, "w+");
          fprintf(fptr, msg_content);
          fclose(fptr);
            //to save to root folder
          strcpy(sRoot_title,serverRoot);
          strcat(sRoot_title,"/");
          strcat(sRoot_title,msg_title);
          fptr = fopen(sRoot_title, "w+");
            // printf("root place :%s\n",sRoot_title);
          fprintf(fptr, msg_content);
          fclose(fptr);
          rcnt = send(fd,"message sended successfully",strlen("message sended successfully"),0);
          break;

          case 2:
          strcpy(dirId,serverRoot);
          strcat(dirId,"/");
          strncat(dirId, userId, 4);
          printf("%s\n",dirId );
          pDir=opendir(dirId);

          if ( pDir == NULL ) 
          {
          	printf("+Error , No message found in your inbox\n");
          	rcnt = send (fd,"+Error , No message found in your inbox\n",strlen("+Error , No message found in your inbox\n"),0);
          }
          else
          {

          	rcnt = send(fd,"\n-----this is your messages-------\n",strlen("\n-----this is your messages-------\n"),0);
          	pDirEnt = readdir( pDir );
                // numOfmsgs++;
          	while(pDirEnt != NULL)
          	{
                    //this is to avoid displaying hidden files
          		if(pDirEnt->d_name[0] != '.')
          		{
          			msgOfUser[0]='\0';
          			numOfmsgs++;
          			strcpy(messgesArray[numOfmsgs] , pDirEnt->d_name);
          			printf( "%d.%s\n",numOfmsgs,  pDirEnt->d_name);
          			strcat(msgOfUser,toChar(numOfmsgs));
          			strcat(msgOfUser,".");
          			strcat(msgOfUser,pDirEnt->d_name);
          			strcat(msgOfUser,"\n");
          			rcnt = send(fd,msgOfUser,strlen(msgOfUser),0);
          		}
          		pDirEnt = readdir( pDir );
          	}
          	closedir(pDir);
          	rcnt = send(fd,"use RET<msgId> to read the msg or del<msgId> to delete the msg\n",strlen("use RET<msgId> to read the msg or del<msgId> to delete the msg\n"),0);
          	rcnt = recv(fd,recvbuf,recvbuflen,0);
          	strncpy(msgChoice, recvbuf, 4);

          	if (strncasecmp(msgChoice,"ret",3 ) == 0) 
          	{
          		int c = msgChoice[3] - '0';
          		strcpy(msg_to_read_or_delete,dirId);
          		strcat(msg_to_read_or_delete,"/");
          		strcat(msg_to_read_or_delete,messgesArray[c]);
                    fptr = fopen(msg_to_read_or_delete, "r"); // read mode
                    printf("%s\n",msg_to_read_or_delete );

                    if (fptr == NULL)
                    {
                    	printf("Error while opening the message.\n");
                    	return;
                    }
                    else
                    {
                    	printf("This is the content of your message :\n");
                    	char ch;
                    	while((ch = fgetc(fptr)) != EOF)
                    	{
                    		strncat(msgcopy, &ch,1);
                    	}
                    	printf("%s\n",msgcopy );
                    	rcnt = send(fd, "This is the content of your message:\n",strlen("This is the content of your message:\n"),0);
                    	rcnt = send(fd,msgcopy,strlen(msgcopy),0);
                    	fclose(fptr);
                    }
                }
                if(strncasecmp(msgChoice,"del",3 ) == 0)
                {
                    //for char to int cast
                	int c = msgChoice[3] - '0';
                	strcpy(msg_to_read_or_delete,dirId);
                	strcat(msg_to_read_or_delete,"/");
                	strcat(msg_to_read_or_delete,messgesArray[c]);
                    // remove();
                    // printf("%d\n",c );
                	printf("%s\n",msg_to_read_or_delete );
                	if (remove(msg_to_read_or_delete) == 0) 
                	{ 
                		printf("Message deleted successfully");
                		rcnt = send(fd,"Message deleted successfully",strlen("Message deleted successfully"),0); 
                	}
                	else
                		printf("Unable to delete the message"); 

                }

            }
            break;
            case 3 :
            rcnt = send(fd, "\nGooooood Bye ! \n", strlen("\nGooooood Bye ! \n"), 0);
            return;
            break;
        }
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
	dictionary * dict;
	dict = iniparser_load("server.ini");   
	int PORT = iniparser_getint(dict, "server:ListenPort", -1);


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
