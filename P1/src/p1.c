#include "p1.h"
#include "circularBuffer.c"

// usage: gcc p1.c p1.h cicularBuffer.c circularBuffer.h -o p1
// time -h ./p1 ... --> mirar el temps
// ./p1 binary|text pathToFile sizeOfTheBuffer

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
    // no cal close, ho fa el main
    return sum;
}

//main
int main(int argc, char* argv[]){
    // Error check
    if(argc != 4) return 1;

    //1.obrir file --> CLI
    const char *mode = argv[1];
    const char *path = argv[2];
    SIZE = atoi(argv[4]);

    // Open file
    int fd = open(path, O_RDONLY);
    if(fd < 0) return 1;

    long long result = 0;
    //2. mirar si es txt o bin
    // A) Text Case:
    if (strcmp(mode, "text") == 0){
        result = istxt(fd);
    } else if (strcmp(mode, "text") == 0){
    // B) Binary case:
        result = isbin(fd);
    }else{
        //mode incorrecte
        close(fd);
        return 1;
    }
    close(fd);
    //3. Escriure resultat
    char out[64];
    sprintf(out, "%lld\n", result); // crea text
    write(STDOUT_FILENO, out, strlen(out)); // esciru
    
    return 0;
}

