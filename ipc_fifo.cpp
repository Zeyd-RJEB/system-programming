#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <sys/wait.h>

const char *FIFO_PATH = "/tmp/my_fifo";

int main()
{
    // create the FIFO if it doesn't exist
    if (access(FIFO_PATH, F_OK) == -1)
    {
        if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST)
        {
            perror("mkfifo");
            return EXIT_FAILURE;
        }
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        // child process: writer
        int fd = open(FIFO_PATH, O_WRONLY);
        if (fd == -1)
        {
            perror("writer open");
            return EXIT_FAILURE;
        }

        const char *message = "hello dad";
        if (write(fd, message, strlen(message)) == -1)
        {
            perror("writer write");
            close(fd);
            return EXIT_FAILURE;
        }

        std::cout << "writer: message sent.\n";
        close(fd);
    }
    else
    {
        // parent process: reader
        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd == -1)
        {
            perror("reader open");
            return EXIT_FAILURE;
        }

        char buffer[100];
        auto bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read == -1)
        {
            perror("reader read");
            close(fd);
            return EXIT_FAILURE;
        }

        buffer[bytes_read] = '\0';
        std::cout << "reader got: " << buffer << '\n';
        close(fd);
        wait(nullptr); // wait for writer to exit
    }

    return EXIT_SUCCESS;
}
