#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

void handler(int signum) {
    printf("Signal received: %d\n", signum);
}

int main() {
    struct sigaction sa;
    struct itimerval timer;

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    timer.it_interval = timer.it_value;

    setitimer(ITIMER_REAL, &timer, NULL);

    while (1) pause();

    return 0;
}
