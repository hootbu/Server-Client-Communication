#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>

#include <pthread.h>

int A = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

void* increase(void* arg)
{
    int thread_id = *((int*)arg);

    while (1) {
        pthread_mutex_lock(&mutex);

        A++;
        printf("Thread %d incremented A to %d\n", thread_id, A);

        // Signal the other thread to proceed
        pthread_cond_signal(&cond);

        // Wait until signaled to continue
        pthread_cond_wait(&cond, &mutex);

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}

int main()
{   
    pid_t pid;
    pid = fork();

    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;

    if (pid < 0){
    // Fork failed
    perror("Fork failed");
    exit(1);
    }

    else if (pid == 0) {
    // Child process
    printf("This is the child process with PID: %d\n", getpid());
    // Continue execution in the child process
    // Add your code here

    // Initialize mutex and condition variable
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Create two threads
    pthread_create(&thread1, NULL, increase, &id1);
    pthread_create(&thread2, NULL, increase, &id2);

    char *ip = "127.0.0.1";
    int port = 8080;

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("[-]Socket creation failed");
        exit(1);
    }
    printf("[+]Server socket created\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (n < 0) {
        perror("[-]Binding failed");
        exit(1);
    }

    printf("[+]Bind to the port number: %d\n", port);

    listen(server_sock, 1);
    printf("Listening...\n");

    while(1)
    {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        printf("[+] Client connected.\n");

        bzero(buffer, 1024);
        recv(client_sock, buffer, sizeof(buffer), 0);
        printf("Client: %s\n", buffer);

        bzero(buffer, 1024);
        sprintf(buffer, "Sure, A's current value is: %d", A);
        printf("Server: %s\n", buffer);
        send(client_sock, buffer, strlen(buffer), 0);

        close(client_sock);
        printf("[+]Client disconnected\n\n");
    }



    // Signal one thread to start
    pthread_cond_signal(&cond);

    // Join the threads
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Destroy mutex and condition variable
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    }

    else {
    // Parent process
    printf("This is the parent process with PID: %d\n", getpid());
    // Terminate the parent process
    printf("Parent process is terminating.\n");
    exit(0);
    }
    // This code will only be executed by the child process
    printf("Child process continues execution.\n");


    return 0;
}