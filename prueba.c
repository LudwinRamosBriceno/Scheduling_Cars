#include <stdlib.h>
#include <stdio.h>


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



