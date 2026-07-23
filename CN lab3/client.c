# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<arpa/inet.h>
# define PORT 8081
# define BUFFER_PORT 1024



int main () {
int sock =0;
struct sockaddr_in serv_addr ;
char *message ="hello from clinet";
char buffer [BUFFER_PORT];


// Create socket
sock = socket ( AF_INET , SOCK_STREAM , 0) ;


// Configure server address
serv_addr . sin_family = AF_INET ;
serv_addr . sin_port = htons ( PORT ) ;


// Convert IP address (127.0.0.1)
inet_pton(AF_INET,"127.0.0.1", &serv_addr.sin_addr);


// Connect to server
while(1){
connect(sock,(struct sockaddr * )&serv_addr,sizeof(serv_addr));

// Communication loop ( to be implemented )

 send(sock,message,strlen(message),0);
//recive
read(sock,buffer,1024);
printf("server: %s\n",buffer);
}

close ( sock ) ;
return 0;
}
