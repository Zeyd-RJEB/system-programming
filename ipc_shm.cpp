#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>

int main()
{
    const char *SHM_NAME = "/my_shm";
    const size_t SIZE = 4096;

    // create shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    // set size of shared memory
    if (ftruncate(shm_fd, SIZE) == -1)
    {
        perror("ftruncate");
        return 1;
    }

    // map shared memory in this process
    void *ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        // child process: read from shared memory after a short delay
        sleep(1); // wait to ensure parent wrote the message

        std::cout << "child reads: " << static_cast<char *>(ptr) << std::endl;

        // clean up: unmap and close shm fd
        munmap(ptr, SIZE);
        close(shm_fd);

        return 0;
    }
    else
    {
        // parent process: write message to shared memory
        const char *message = "Hello from parent via shared memory!";
        memcpy(ptr, message, strlen(message) + 1);

        // wait for child to finish
        waitpid(pid, nullptr, 0);

        // clean up shared memory (unmap, close, and unlink)
        munmap(ptr, SIZE);
        close(shm_fd);
        shm_unlink(SHM_NAME);

        return 0;
    }
}
