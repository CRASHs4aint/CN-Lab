#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void handle_client(int client_sock)
{
    char buffer[BUFFER_SIZE];
    int n;

    memset(buffer, 0, sizeof(buffer));

    n = read(client_sock, buffer, sizeof(buffer) - 1);

    if (n <= 0)
    {
        close(client_sock);
        exit(0);
    }

    buffer[n] = '\0';

    /*
       Format received:
       Student_ID|file_data
    */

    char *separator = strchr(buffer, '|');

    if (separator == NULL)
    {
        char *msg = "Invalid request";
        write(client_sock, msg, strlen(msg));
        close(client_sock);
        exit(0);
    }

    *separator = '\0';

    char *student_id = buffer;
    char *file_data = separator + 1;

    char response[BUFFER_SIZE];
    char line[256];

    int found = 0;

    char *data_copy = strdup(file_data);
    char *saveptr;

    char *record = strtok_r(data_copy, "\n", &saveptr);

    while (record != NULL)
    {
        char id[20];

        if (sscanf(record, "%19s", id) == 1)
        {
            if (strcmp(id, student_id) == 0)
            {
                snprintf(response,
                         sizeof(response),
                         "Record Found : %s",
                         record);

                found = 1;
                break;
            }
        }

        record = strtok_r(NULL, "\n", &saveptr);
    }

    free(data_copy);

    if (!found)
    {
        strcpy(response, "Record Not Found");
    }

    write(client_sock, response, strlen(response));

    close(client_sock);

    exit(0);
}

int main()
{
    int server_sock;
    int client_sock;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock < 0)
    {
        perror("socket");
        exit(1);
    }

    int opt = 1;

    setsockopt(server_sock,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, 10) < 0)
    {
        perror("listen");
        close(server_sock);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        addr_size = sizeof(client_addr);

        client_sock = accept(server_sock,
                             (struct sockaddr *)&client_addr,
                             &addr_size);

        if (client_sock < 0)
        {
            perror("accept");
            continue;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            close(client_sock);
            continue;
        }

        if (pid == 0)
        {
            /*
               Child process
            */

            close(server_sock);

            handle_client(client_sock);
        }
        else
        {
            /*
               Parent process
            */

            close(client_sock);

            /*
               Prevent zombie processes
            */
            waitpid(-1, NULL, WNOHANG);
        }
    }

    close(server_sock);

    return 0;
}