#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>

using namespace std;

int f(int x)
{
    int m = x % 5;

    if (m == 1 || m == 4) {
        while (1);
    }

    return x % 2 == 0 ? 0 : 1;
}

int g(int x)
{
    int m = x % 5;

    if (m == 0) {
        return 1;
    }

    if (m == 1) {
        return 0;
    }

    if (m == 2 || m == 4) {
        while (1);
    }


    return x % 3 == 0 ? 0 : 1;
}



void printResult(int& a, int& b) {
    if (a == 0 && b == 0) {
        printf("f || g = %d", 0);
    }

    if (a == 1 || b == 1) {
        printf("f || g = %d", 1);
    }

}

int main(int argc, char** argv) {
    int child1_to_parent[2];
    int child2_to_parent[2];
    int parent_to_child1[2];
    int parent_to_child2[2];
    int res1 = -1;
    int res2 = -1;
    int val;
    int status1 = 0;
    int status2 = 0;

    pipe(child1_to_parent);
    pipe(child2_to_parent);
    pipe(parent_to_child1);
    pipe(parent_to_child2);

    pid_t firstChild, secondChild;

    int x;
    printf("Enter x: ");
    scanf("%d", &x);


    if ((firstChild = fork()) < 0) {
        perror("Can't fork process");
        exit(EXIT_FAILURE);
    }

    if (firstChild == 0) {
        read(parent_to_child1[0], &val, sizeof(val));
        res1 = f(val);
        write(child1_to_parent[1], &res1, sizeof(res1));
        exit(EXIT_SUCCESS);
    }

    if ((secondChild = fork()) < 0) {
        perror("Can't fork process");
        exit(EXIT_FAILURE);
    }

    if (secondChild == 0) {
        read(parent_to_child2[0], &val, sizeof(val));
        res2 = g(val);
        write(child2_to_parent[1], &res2, sizeof(res2));
        exit(EXIT_SUCCESS);
    }

    else {
        write(parent_to_child1[1], &x, sizeof(x));
        write(parent_to_child2[1], &x, sizeof(x));

        int time = 1;
        pid_t firstComplete = 0;
        pid_t secondComplete = 0;
        bool keepAsking = 1;
        bool firstKilled = 0;
        bool secondKilled = 0;

        while (1) {
            firstComplete = waitpid(firstChild, &status1, WNOHANG);
            secondComplete = waitpid(secondChild, &status2, WNOHANG);

            if (firstComplete && !firstKilled) {
                read(child1_to_parent[0], &res1, sizeof(res1));
                kill(firstChild, SIGKILL);
                firstKilled = 1;
            }
            if (secondComplete && !secondKilled) {
                read(child2_to_parent[0], &res2, sizeof(res2));
                kill(secondChild, SIGKILL);
                secondKilled = 1;
            }

            if (res1 == 1 || res2 == 1 || firstComplete && secondComplete) {
                firstKilled = 1;
                secondKilled = 1;
                break;
            }

            if (keepAsking && time % 5 == 0) {
                char response;

                printf("Do you want to continue calculations (y = yes, n = no, c = continue and don't ask again)? ");
                cin >> response;

                if (response == 'n') {
                    if (firstComplete && !firstKilled) {
                        read(child1_to_parent[0], &res1, sizeof(res1));
                    }
                    if (secondComplete && !secondKilled) {
                        read(child2_to_parent[0], &res2, sizeof(res2));
                    }

                    printResult(res1, res2);
                    kill(firstChild, SIGKILL);
                    kill(secondChild, SIGKILL);
                    return 0;
                }

                if (response == 'c') {
                    keepAsking = 0;
                }
            }
            sleep(1);
            time++;
        }
        printResult(res1, res2);
    }
}

