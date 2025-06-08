#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>

const char *SOCKET_PATH = "/tmp/mysocket";

int main()
{
    unlink(SOCKET_PATH); // ensure it's not left from before

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (auto ret = bind(server_fd, (sockaddr *)&addr, sizeof(addr)); ret == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (auto ret = listen(server_fd, 1); ret == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // child process acting as a client.
        sleep(1); // give the server tie to set up.
        auto sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == -1)
        {
            perror("socket (client)");
            exit(EXIT_FAILURE);
        }

        sockaddr_un client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sun_family = AF_UNIX;
        strcpy(client_addr.sun_path, SOCKET_PATH);

        if (connect(sock, (sockaddr *)&client_addr, sizeof(client_addr)) == -1)
        {
            perror("connect");
            exit(EXIT_FAILURE);
        }

        char buffer[1024];
        strcpy(buffer, "client: hola");
        if (write(sock, buffer, sizeof(buffer)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        if (read(sock, buffer, sizeof(buffer)) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        std::cout << "client received: " << buffer << '\n';
        close(sock);
    }
    else
    {
        // parent process acting as a server
        std::cout << "server waiting...\n";
        int client_fd = accept(server_fd, nullptr, nullptr);
        char buffer[1024];
        read(client_fd, buffer, sizeof(buffer));
        std::cout << "server received: " << buffer << '\n';
        strcpy(buffer, "server: hola");
        write(client_fd, buffer, sizeof(buffer));
        close(client_fd);
        close(server_fd);
        unlink(SOCKET_PATH);
        wait(nullptr);
    }
}