#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void handle_signal(int signum)
{
    std::cout << "child received signal: " << signum << '\n';
}

int main()
{
    if (pid_t pid = fork(); pid == -1)
    {
        std::cout << "error creating a new process\n";
    }
    else if (pid == 0)
    {
        // child process
        signal(SIGUSR1, handle_signal);
        std::cout << "child wating for signal...\n";
        pause();
    }
    else
    {
        // parent process
        sleep(1); // give the child time to set up
        std::cout << "parent sending SIGUSR1 to child\n";
        kill(pid, SIGUSR1);
        wait(NULL); // wait for child to finish
    }
}
