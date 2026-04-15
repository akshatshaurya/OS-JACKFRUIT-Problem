#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>

#define SOCKET_PATH "/tmp/mini_runtime.sock"

/* ================= SUPERVISOR ================= */

static int run_supervisor(const char *rootfs)
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[1024];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Supervisor running...\n");

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        int n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Received command: %s\n", buffer);

            pid_t pid = fork();

            if (pid == 0) {
                /* Step 3: namespace isolation */
                if (unshare(CLONE_NEWPID | CLONE_NEWNS) < 0) {
                    perror("unshare");
                    exit(1);
                }

                pid_t pid2 = fork();

                if (pid2 == 0) {
                    printf("Inside container\n");

                    /* Step 5: attach to cgroup BEFORE chroot */
                    FILE *f = fopen("/sys/fs/cgroup/mycontainer/cgroup.procs", "w");
                    if (f) {
                        fprintf(f, "%d\n", getpid());
			fflush(f);
			fclose(f);
                    } else {
                        perror("cgroup attach failed");
                    }

                    /* Step 4: filesystem isolation */
                    if (chroot(rootfs) < 0) {
                        perror("chroot failed");
                        exit(1);
                    }

                    if (chdir("/") < 0) {
                        perror("chdir failed");
                        exit(1);
                    }

                    execl("/bin/sh", "/bin/sh", NULL);

                    perror("exec failed");
                    exit(1);
                }
                else if (pid2 > 0) {
                    wait(NULL);
                }
                else {
                    perror("fork failed");
                }

                exit(0);
            }
            else if (pid > 0) {
                printf("Container started with PID: %d\n", pid);
            }
            else {
                perror("fork failed");
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}

/* ================= CLIENT ================= */

static int send_control_request()
{
    int sockfd;
    struct sockaddr_un addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    char msg[] = "start request";
    write(sockfd, msg, strlen(msg));

    close(sockfd);
    return 0;
}

/* ================= MAIN ================= */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s supervisor <rootfs> | start\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "supervisor") == 0) {
        if (argc < 3) {
            printf("Usage: supervisor <rootfs>\n");
            return 1;
        }
        return run_supervisor(argv[2]);
    }

    if (strcmp(argv[1], "start") == 0) {
        return send_control_request();
    }

    printf("Invalid command\n");
    return 1;
}
