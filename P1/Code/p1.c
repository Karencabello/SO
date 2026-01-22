#include "p1.h"
#include "circularBuffer.c"

// usage: gcc p1.c p1.h cicularBuffer.c circularBuffer.h -o p1
// time -h ./p1 ... --> mirar el temps

char istxt(int fd){
    // a tenir en compte: separats per coma
    // creiem q loop
    // mirar si el buffer està ple per buidar-lo abans de seguir llegint
    // Cridem a cicularbuffer
    //read

}

int isbin(int fd){
    // 1. Mirem mides
    //Mida del element
    const int size_bin = sizeof(int);
    //buffer multiple del enter
    int buf = SIZE - (SIZE % size_bin);
    if (buf == 0) buf = size_bin; //si < sizeof(int)

    int *buffer = (int*)malloc((size_t)buf);
    if (!buffer) return 0; // Error

    long long sum = 0;

    // creiem q loop
    while(1){

        // read
        ssize_t n = read(fd, buffer, buf);
        if (n <= 0){
            break;
        } // Error

        // ajustem la mida si cal
        ssize_t n_ok = n - (n % size_bin);

        // traduir a text
        int count = (int)(n_ok / size_bin); // mirar quants numeros enters tenim

        // sumem
        for (int i = 0; i < count; i++){
            sum += buffer[i];
        }
    }
    free(buffer);
    return sum;
}

//main
int main(int argc, char* argv[]){

}
//1.obrir file --> CLI
//2. mirar si es txt o bin

// fer suma
