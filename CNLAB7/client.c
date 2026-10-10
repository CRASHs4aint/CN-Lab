// UDP client for Assignment 7. Usage: ./udp_client <server_ip> <roll>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "Usage: %s <server_ip> <roll>\n", argv[0]); return 1; }
    char *end; long roll = strtol(argv[2], &end, 10);
    if (*argv[2] == '\0' || *end || roll <= 0) { fprintf(stderr, "Roll number must be a positive integer.\n"); return 1; }
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    struct sockaddr_in server = {0}; server.sin_family = AF_INET;
    server.sin_port = htons((unsigned short)(30000 + roll % 1000));
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) != 1) { fprintf(stderr, "Invalid IPv4 address.\n"); close(sock); return 1; }
    char line[BUFFER_SIZE], packet[BUFFER_SIZE + 64], response[BUFFER_SIZE + 1];
    for (;;) {
        printf("> "); fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) strcpy(line, "EXIT");
        line[strcspn(line, "\r\n")] = '\0';
        int exiting = !strcmp(line, "EXIT");
        int n = snprintf(packet, sizeof packet, "%ld#%s", roll, exiting ? "EXIT" : line);
        if (n < 0 || (size_t)n >= sizeof packet) { fprintf(stderr, "Request is too long.\n"); continue; }
        if (sendto(sock, packet, (size_t)n, 0, (struct sockaddr *)&server, sizeof server) < 0) { perror("sendto"); close(sock); return 1; }
        ssize_t got = recvfrom(sock, response, sizeof response - 1, 0, NULL, NULL);
        if (got < 0) { perror("recvfrom"); close(sock); return 1; }
        response[got] = '\0'; printf("%s\n", response);
        if (exiting) break;
    }
    close(sock); return 0;
}
