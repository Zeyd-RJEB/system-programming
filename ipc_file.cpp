#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    const char *FILE_PATH = "/tmp/ipc_file.txt";

    pid_t pid = fork();

    if (pid == -1)
    {
        std::cerr << "fork failed\n";
        return 1;
    }

    if (pid == 0)
    {
        // child process - reader
        sleep(1); // wait for parent to write

        std::ifstream file(FILE_PATH);
        if (!file)
        {
            std::cerr << "child: failed to open file\n";
            return 1;
        }

        std::string message;
        std::getline(file, message);
        std::cout << "child read: " << message << '\n';
    }
    else
    {
        // parent process - writer
        std::ofstream file(FILE_PATH);
        if (!file)
        {
            std::cerr << "parent: failed to open file\n";
            return 1;
        }

        std::string message = "hello from parent!";
        file << message << '\n';
        std::cout << "parent wrote: " << message << '\n';

        file.close();
        wait(nullptr); // wait for child to finish
    }
}