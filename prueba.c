#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

void display_message(int s) {
    write(STDOUT_FILENO, "copyit: Still working...\n", 24);
    signal(SIGALRM, display_message); // Reinstalar (algunos sistemas lo requieren)
}

int main(void) {
    signal(SIGALRM, display_message);
    
    // Configurar temporizador con mayor precisión (ej. 500 ms)
    struct itimerval timer = {
        .it_interval = { .tv_sec = 0, .tv_usec = 500000 }, // Intervalo
        .it_value = { .tv_sec = 0, .tv_usec = 500000 }    // Primera expiración
    };
    setitimer(ITIMER_REAL, &timer, NULL);
    int n = 0;
    while (1) {
        ++n;
    }
    return 0;
}

/*
void func1(int** ptr){
    //printf("Puntero 2 apunta a: %p \n", ptr);
    //printf("Puntero 2 dirección: %p \n", &ptr);
    printf("Puntero 2 dirección: %p \n", *ptr);

}


int main(){
    int var = 10;
    int *ptr_int = &var;
    func1(&ptr_int);

    //printf("Puntero 1 apunta a: %p \n", ptr_int);
    //printf("Puntero 1 dirección: %p \n", &ptr_int);
    printf("Puntero 1 dirección: %p \n", &var);

    return 0;
}
*/

/*
#include <stdio.h>
#include <signal.h>


void display_message(int s) {
     printf("copyit: Still working...\n" );
     alarm(1);    //for every second
     signal(SIGALRM, display_message);
}

int main(void) {
    signal(SIGALRM, display_message);
    alarm(1);
    int n = 0;
    while (1) {
        ++n;
    }
    return 0;
}
*/

