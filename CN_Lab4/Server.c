#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024

void find_student(char *reg_no, char *response)
{
    FILE *fp = fopen("students.txt", "r");

    if (fp == NULL)
    {
        strcpy(response, "Error␣opening␣file");
        return;
    }
    char file_reg[50], name[50], branch[50], college[100];
    int found = 0;
    while (fscanf(fp, "%s␣%s␣%s␣%s", file_reg, name, branch, college) != EOF)
    {
        if (strcmp(file_reg, reg_no) == 0)
        {
            sprintf(response, "Name:␣%s,␣Branch:␣%s,␣ College:␣%s", name, branch, college);
            found = 1;
            break;
        }
    }
}

if (!found)
{
    strcpy(response, "Data␣not␣found");
}
fclose(fp);
}


int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

     // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Define address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

     // 3. Bind socket
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  
// 4. Listen for connections
    listen(server_fd, 3);

    printf("Server waiting on port %d...\n", PORT);


   
}
while (1)

 {
        memset(buffer, 0, BUFFER_SIZE);

        // Receive registration number
        recv(new_socket, buffer, BUFFER_SIZE - 1, 0);

        buffer[strcspn(buffer, "\r\n")] = '\0';

        printf("Received: %s\n", buffer);

        // Check for bye
        if (strcmp(buffer, "bye") == 0)
        {
            strcpy(response, "Connection closed");

            send(new_socket,
                 response,
                 strlen(response),
                 0);

            break;
        }

        // Search student
        find_student(buffer, response);

        // Send result to client
        send(new_socket,
             response,
             strlen(response),
             0);
    }

    // 7. Close connection
    close(new_socket);
    close(server_fd);

    printf("Server closed.\n");
return 0;