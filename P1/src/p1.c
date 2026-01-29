#include "p1.h"
#include "circularBuffer.h"

// usage: gcc p1.c p1.h cicularBuffer.c circularBuffer.h -o p1
// time -h ./p1 ... --> mirar el temps
// ./p1 binary|text pathToFile sizeOfTheBuffer

int SIZE = 1024;

long long istxt(int fd){
    //creem el buffer circular
    CircularBuffer cb;
    buffer_init(&cb, SIZE);

    //creem buffer lineal
    char buffer[SIZE];

    long long sum = 0; //long long per evitar overflow
    int reachedEOF = 0; //si 0 = no EOF, si 1 = EOF
    int elem_size;

    //creem loop (while true)
    while(1){
        //read
        ssize_t n = read(fd, buffer, SIZE);

        if(n == 0) reachedEOF = 1; //read() retorna n=0 quan EOF
        if(n<0){
            //error al llegir
            //read() retorna n=-1 quan error
            perror("read");
            break;
        }

        // ietrem sobre els bits llegits
        int i = 0;
        while (i < n) {
            // 1. Intentem push al buffer circular
            if(buffer_free_bytes(&cb) > 0){
                buffer_push(&cb, buffer[i]);
                i++; // si hi ha lloc avancem
            } 
            else {
                // 2. Buffer ple --> processem un numero per a fer espai
                elem_size = buffer_size_next_element(&cb, ',', 0);
                
                if(elem_size > 0){
                    char num[elem_size + 1];
                    for(int k = 0; k < elem_size; k++){
                        num[k] = buffer_pop(&cb);
                    }
                    // Al no ser EOF, siempreacabaran en coma
                    num[elem_size - 1] = '\0';
                    sum += atoll(num);
                } else {
                    //si buffer ple i sense comes tenim numero més gran que la mida del buffer
                    perror("buffer ple i no delimitador");
                    break;
                }
            }
        }
        //3. Processem elements restants del buffer(buffer no ple però comes o EOF)
        while((elem_size = buffer_size_next_element(&cb, ',', reachedEOF)) > 0){
            
            char num[elem_size + 1];

            for(int k = 0; k < elem_size; k++){
                num[k] = buffer_pop(&cb);
            }

            if(reachedEOF){
                //pot no haver coma al final del arxiu
                num[elem_size] = '\0';
            } else{
                //si no EOF, acabarà en coma
                num[elem_size - 1] = '\0';
            }

            sum += atoll(num); //atoll() per evitar overflow
        }

        if(reachedEOF) break; 
    }

    buffer_deallocate(&cb);
    return sum;
}


long long isbin(int fd){
    // 1. Mirem mides
    //Mida del element
    //const int size_bin = sizeof(int);
    const int size_bin = sizeof(unsigned int);
    //buffer multiple del enter
    int buf = SIZE - (SIZE % size_bin);
    if (buf == 0) buf = size_bin; //si < sizeof(int)

    //int *buffer = (int*)malloc((size_t)buf);
    unsigned int *buffer = malloc(buf);
    if (!buffer) return 0; // Error

    int sum = 0;

    // creiem q loop
    while(1){

        // read
        ssize_t n = read(fd, buffer, buf);
        if (n <= 0){
            perror("read error"); //mostra error
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
    SIZE = atoi(argv[3]);

    // Open file
    int fd = open(path, O_RDONLY);
    if(fd < 0) return 1;

    long long result = 0;
    //2. mirar si es txt o bin
    // A) Text Case:
    if (strcmp(mode, "text") == 0){   //strcmp(str1,str2) funció de <string.h> que compara dos cadenes de caracters
        result = istxt(fd);
    } else if (strcmp(mode, "binary") == 0){    
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

