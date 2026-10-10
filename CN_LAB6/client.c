#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main()
{
    int sockfd;

    struct sockaddr_in server_addr;

    char file_data[3500];
    char student_id[50];
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    /*
       Create socket
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    /*
       Server address
    */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1",
              &server_addr.sin_addr);

    /*
       Connect to server
    */
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    /*
       Open data.txt
    */
    FILE *fp = fopen("data.txt", "r");

    if (fp == NULL)
    {
        perror("data.txt");
        close(sockfd);
        exit(1);
    }

    /*
       Read file
    */
    size_t bytes = fread(file_data,
                         1,
                         sizeof(file_data) - 1,
                         fp);

    file_data[bytes] = '\0';

    fclose(fp);

    /*
       Ask Student ID
    */
    printf("Enter Student ID to search : ");
    scanf("%49s", student_id);

    /*
       Prepare request
    */
    snprintf(request,
             sizeof(request),
             "%s|%s",
             student_id,
             file_data);

    /*
       Send request
    */
    write(sockfd,
          request,
          strlen(request));

    /*
       Receive server response
    */
    memset(response, 0, sizeof(response));

    int n = read(sockfd,
                 response,
                 sizeof(response) - 1);

    if (n > 0)
    {
        response[n] = '\0';

        printf("Server Response : %s\n",
               response);
    }

    close(sockfd);

    return 0;
}