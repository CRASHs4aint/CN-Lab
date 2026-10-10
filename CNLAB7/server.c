// Roll: 24527 Service Type: C d = 2
// Port: 30000 + 527 = 30527

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef ROLL_NUMBER
#define ROLL_NUMBER 24527
#endif
#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)

#define BUFFER_SIZE 4096

static int service_d(void) {
    int n = ROLL_NUMBER % 1000, sum = 0;
    while (n > 0) { sum += n % 10; n /= 10; }
    return sum % 3;
}

static void reply_error(char *out, size_t cap, const char *message) {
    snprintf(out, cap, "ERROR: %s", message);
}

static void process_a(const char *request, char *out, size_t cap) {
    const char *bar = strchr(request, '|');
    if (!bar) { reply_error(out, cap, "Expected COMMAND|TEXT"); return; }
    size_t cmdlen = (size_t)(bar - request);
    const char *text = bar + 1;
    if (cmdlen == 2 && strncmp(request, "UP", 2) == 0) {
        size_t i; for (i = 0; text[i] && i + 1 < cap; ++i) out[i] = (char)toupper((unsigned char)text[i]); out[i] = '\0';
    } else if (cmdlen == 3 && strncmp(request, "LOW", 3) == 0) {
        size_t i; for (i = 0; text[i] && i + 1 < cap; ++i) out[i] = (char)tolower((unsigned char)text[i]); out[i] = '\0';
    } else if (cmdlen == 3 && strncmp(request, "REV", 3) == 0) {
        size_t len = strlen(text), i; if (len >= cap) len = cap - 1;
        for (i = 0; i < len; ++i) out[i] = text[len - 1 - i]; out[len] = '\0';
    } else reply_error(out, cap, "Unknown text operation");
}

static void process_b(const char *request, char *out, size_t cap) {
    char copy[BUFFER_SIZE]; snprintf(copy, sizeof copy, "%s", request);
    char *op = strtok(copy, "|"), *count_s = strtok(NULL, "|"), *values = strtok(NULL, "");
    if (!op || !count_s || !values || strtok(NULL, "|")) { reply_error(out, cap, "Expected OP|n|integers"); return; }
    char *end; errno = 0; long n = strtol(count_s, &end, 10);
    if (errno || *end || n < 1 || n > 1000) { reply_error(out, cap, "Invalid integer count"); return; }
    long long sum = 0, min = 0, max = 0;
    char *p = values;
    for (long i = 0; i < n; ++i) {
        while (isspace((unsigned char)*p)) ++p;
        if (!*p) { reply_error(out, cap, "Not enough integers"); return; }
        errno = 0; long long v = strtoll(p, &end, 10);
        if (errno || end == p || (*end && !isspace((unsigned char)*end))) { reply_error(out, cap, "Invalid integer list"); return; }
        p = end; sum += v; if (i == 0 || v < min) min = v; if (i == 0 || v > max) max = v;
    }
    while (isspace((unsigned char)*p)) ++p;
    if (*p) { reply_error(out, cap, "Too many integers"); return; }
    if (!strcmp(op, "SUM")) snprintf(out, cap, "%lld", sum);
    else if (!strcmp(op, "MAX")) snprintf(out, cap, "%lld", max);
    else if (!strcmp(op, "MIN")) snprintf(out, cap, "%lld", min);
    else if (!strcmp(op, "AVG")) snprintf(out, cap, "%.2f", (double)sum / (double)n);
    else reply_error(out, cap, "Unknown integer operation");
}

static void process_c(const char *request, char *out, size_t cap) {
    size_t len = strlen(request); unsigned long vowels = 0, consonants = 0, digits = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)request[i];
        if (isalpha(c)) {
            int lower = tolower(c);
            if (lower=='a'||lower=='e'||lower=='i'||lower=='o'||lower=='u') ++vowels;
            else ++consonants;
        } else if (isdigit(c)) ++digits;
    }
    snprintf(out, cap, "LEN=%zu;VOWELS=%lu;CONSONANTS=%lu;DIGITS=%lu", len, vowels, consonants, digits);
}

int main(void) {
    if (ROLL_NUMBER <= 0) { fprintf(stderr, "Set ROLL_NUMBER to your full roll number before compiling.\n"); return 1; }
    int port = 30000 + (ROLL_NUMBER % 1000), d = service_d();
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    struct sockaddr_in server = {0}; server.sin_family = AF_INET; server.sin_addr.s_addr = htonl(INADDR_ANY); server.sin_port = htons((unsigned short)port);
    if (bind(sock, (struct sockaddr *)&server, sizeof server) < 0) { perror("bind"); close(sock); return 1; }
    printf("Server running on port %d (Service Type %c)\n", port, 'A' + d); fflush(stdout);
    for (;;) {
        char in[BUFFER_SIZE], out[BUFFER_SIZE]; struct sockaddr_in client; socklen_t client_len = sizeof client;
        ssize_t received = recvfrom(sock, in, sizeof in - 1, 0, (struct sockaddr *)&client, &client_len);
        if (received < 0) { if (errno == EINTR) continue; perror("recvfrom"); break; }
        in[received] = '\0';
        char ip[INET_ADDRSTRLEN]; inet_ntop(AF_INET, &client.sin_addr, ip, sizeof ip);
        char *hash = strchr(in, '#');
        if (!hash) reply_error(out, sizeof out, "Unauthorized client");
        else {
            *hash = '\0'; const char *body = hash + 1;
            if (strcmp(in, STRINGIFY(ROLL_NUMBER)) != 0) reply_error(out, sizeof out, "Unauthorized client");
            else if (!strcmp(body, "EXIT")) { snprintf(out, sizeof out, "Server shutting down"); }
            else if (d == 0) process_a(body, out, sizeof out);
            else if (d == 1) process_b(body, out, sizeof out);
            else process_c(body, out, sizeof out);
        }
        printf("Client %s:%u\nReceived: %s\n", ip, ntohs(client.sin_port), hash ? hash + 1 : in);
        sendto(sock, out, strlen(out), 0, (struct sockaddr *)&client, client_len);
        printf("Sent: %s\n", out); fflush(stdout);
        if (hash && !strcmp(in, STRINGIFY(ROLL_NUMBER)) && !strcmp(hash + 1, "EXIT")) break;
    }
    close(sock); return 0;
}
