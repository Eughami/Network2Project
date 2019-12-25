#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PORT 1080
#define BUF_LEN  2000
#define STDIN 0
#define SADDR "127.0.0.1"
#define TV_SEC 10


void test_fds(fd_set testfds, int sock) {
int nread;
char buffer[BUF_LEN];

  if (FD_ISSET(STDIN, &testfds)) {
ioctl(STDIN,FIONREAD,&nread);
if (nread == 0) {
        printf("Keyboard done!\n");
  exit(0);
}
        nread = read(STDIN, buffer, nread);
buffer[nread] = '\0';
write(sock, buffer, nread);
      }

      if (FD_ISSET(sock, &testfds)) {
ioctl(sock,FIONREAD,&nread);
if (nread == 0) {
        printf("Socket done!\n");
  exit(0);
}
        nread = read(sock, buffer, nread);
buffer[nread] = '\0';
        printf("%s", buffer);
      }
}

int main(int argc, char **argv)
{
    int sock, bytes_recieved;
    char send_data[BUF_LEN],recv_data[BUF_LEN];
    struct sockaddr_in server_addr;
    fd_set readfds,testfds;
    struct timeval tv;
int maxfd,stdeof,c,ret;
char *saddr = NULL, *sport = NULL;


while ((c = getopt (argc, argv, "p:s:")) != -1)
    switch (c)
    {
        case 's':
            saddr = optarg;
            break;
        case 'p':
            sport = optarg;
            break;
        case '?':
            fprintf(stderr,"Unrecognized option: -%c\n", optopt);
            return 1;
        default:
            abort ();
    }

    /* Open socket descriptor */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    /* Fill structure with 0 */
    bzero(&(server_addr.sin_zero),sizeof server_addr);
    server_addr.sin_family = AF_INET;

    /* Port number defined? */
    if ( sport != NULL )
        server_addr.sin_port = htons(atoi(sport));
    else
        server_addr.sin_port = htons(PORT);

    /* Remote ip address defined? */
    if ( saddr != NULL )
        inet_pton(AF_INET, saddr, &(server_addr.sin_addr));
    else
        inet_pton(AF_INET, SADDR , &(server_addr.sin_addr));


    /* Connect to remote server */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Connect");
        exit(1);
    }

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(sock, &readfds);

while (1) {
    testfds=readfds;

    tv.tv_sec = TV_SEC;
    tv.tv_usec = 0;

// don't care about writefds and exceptfds:
    ret=select(FD_SETSIZE, &testfds, (fd_set *)NULL, (fd_set *)NULL, &tv);

    switch (ret) {
case 0:
          printf("Timed out.\n");
  break;
case -1:
          printf("Select problem.\n");
          exit(1);
    default:
      test_fds(testfds,sock);
  break;
    }
  }
}
