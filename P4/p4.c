/*
// synchronization  tools  to temporary stop producers when the buffer is full, 
// and to stop consumers from extracting from the buffer if it is empty.

* Producers will read blocks of 16,384 bytes (1024 × 16) from a file and place them into a shared finite length buffer.
    * producers --> block or pointer in the buffer
    *  wait when the buffer is full
*  Consumers will read blocks of 16,384 bytes from the buffer and update a global histogram of pixel values (0-255).
    * consumers --> extract elements from the buffer --> acumulate in the histogram
    * Check buffer i flag
    * wait when the buffer is empty
    * if the buffer is empty and producers_finished is set, they can safely exit.
    *  wake  up  all  sleeping  consumers
    *  recheck the termination condition and exit gracefully instead of remaining indefinitely blocked.
    * O semaphores o barriers
* Conditions endin threads
    1. producers  end  when  they  reach  the  EOF, 
    2) consumers  end  when  all producers have finished, and the buffer is empty.
* End
    * consumers terminate correctly once all producers have finished and no more data will be added to the buffer.
    * TIP --> shared flag producers_finished) that is set to 1 when the last producer ends.
** The buffer size, as well as the number of producers and consumers, will be provided as command-line arguments.
** USAGE: computeHistogram Data/heart.pgm Data/histogram.txt N_producers N_consumers sizeBuffer

*/

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parsePGM.h"
#include "circularBuffer.h"


#define BLOCK_SIZE (1024 * 16)

// Varibles globals 
static CircularBuffer cb;

// locks
static pthread_mutex_t lock_buf;
static pthread_cond_t not_full;
static pthread_cond_t not_empty;
static pthread_mutex_t lock_read;
static pthread_mutex_t lock_hist;


static int readpos = 0; // Posició global de lectura del fitxer
static int active_producers = 0; // Nombre de producers encara actius
static int producers_finished = 0; // Flag que indica que tots els producers han acabat
static int global_hist[256]; // Histograma global
static int header_size_global = 0; 
static int maxVal_global = 255;

typedef struct { 
    char* path; 
    int id;
} ProducerArgs;

typedef struct { 
    int id;
} ConsumerArgs;

// Producer --> Llegeix blocs del fitxer i els escriu al buffer
static void *Producer(void *arg){
    //Convertim l'argument genèric del thread al nostre tipus
    ProducerArgs *pa = (ProducerArgs *)arg;

    //obrim fitxer
    int fd = open(pa->path, O_RDONLY);

    

    // Buffer local on el producer llegeix blocs del fitxer
    unsigned char local_buffer[BLOCK_SIZE];

    while(1){
        // 1. Assignar posició de lectura 
        /* Tots els producers comparteixen la variable global
           readpos, que indica el següent offset del fitxer
           que encara no s'ha llegit.
           Fem servir lock_read per evitar que dos producers
           agafin la mateixa posició.
        */
        int pos;

        pthread_mutex_lock(&lock_read);
        pos = readpos; //guardem l'offset actual
        readpos += BLOCK_SIZE; //reservem el següent bloc
        pthread_mutex_unlock(&lock_read);

        // 2. Llegir bloc del fitxer
        /* pread permet llegir des d'una posició concreta del
           fitxer sense modificar el file pointer global
        */
        int n = pread(fd, local_buffer, BLOCK_SIZE, header_size_global + pos);
        
        if(n <= 0){
            break;
        }

        // 3. Posar dades al buffer circular
        /* El buffer pot estar ple, així que potser hem d'esperar*/
        int written = 0;

        while(written<n){
            //protegim acces al buffer circular
            pthread_mutex_lock(&lock_buf);

            //si buffer ple --> producer dorm fins que algun consumer trgui dades
            while(buffer_free_bytes(&cb) == 0){
                pthread_cond_wait(&not_full, &lock_buf);
            }

            //escribim dades al buffer circular
            buffer_push(&cb, local_buffer[written]);
            written++;

            //avisem a consumers que hi ha dades noves
            pthread_cond_signal(&not_empty);
            pthread_mutex_unlock(&lock_buf);
        }
    }

    close(fd);

    // 4. Aquest producer ha acabat 
    pthread_mutex_lock(&lock_buf);
    active_producers--;

    // Si era l'últim producer
    if(active_producers == 0){
        producers_finished = 1;
        pthread_cond_broadcast(&not_empty); // Despertar tots els consumers
    }

    pthread_mutex_unlock(&lock_buf);

    return NULL;
}    



void* Consumer(void* arg) {
    
    // 1. Convertir arg a ConsumerArgs*
    ConsumerArgs* info = (ConsumerArgs*)arg;
    
    //Buffer local on el consumer guarda dades extretes del buffer circular
    unsigned char buffer[BLOCK_SIZE];

    while(1){

        pthread_mutex_lock(&lock_buf);

        //si buffer buit i encara producers treballant --> consumer dorm
        while(buffer_used_bytes(&cb) == 0 && !producers_finished){
            pthread_cond_wait(&not_empty, &lock_buf);
        }

        //si buffer buit i producers han acabat --> no hi haurà més dades
        if(buffer_used_bytes(&cb) == 0 && producers_finished){
            pthread_mutex_unlock(&lock_buf);
            break;
        }

        //2. treiem dades buffer
        int n = 0;

        while(n < BLOCK_SIZE && buffer_used_bytes(&cb) > 0){
            buffer[n++] = buffer_pop(&cb);
        }

        //avisem que ara hi ha espai lliure al buffer
        pthread_cond_signal(&not_full);
        
        pthread_mutex_unlock(&lock_buf);

        //3. Actualitzar histograma

        //protegim histograma global
        pthread_mutex_lock(&lock_hist);

        for(int i = 0; i<n; i++){
            //cada byte és un valor de pixel (0-255)
            global_hist[buffer[i]]++;
        }

        pthread_mutex_unlock(&lock_hist);
    }
    return NULL;    
}


int main(int argc, char* argv[]){
    
    // 1. Comprovem arguments
    if(argc != 6){
        fprintf(stderr, "Usage: %s input.pgm output.txt producers consumers buffer_size\n", argv[0]);
        return 1;
    }

    // Arguments de command line
    char* inputPath = argv[1];
    char* outputPath = argv[2];

    int num_producers = atoi(argv[3]);
    int num_consumers = atoi(argv[4]);
    int buffer_size = atoi(argv[5]);

    // llegim header
    int w, h, maxv;
    int header_size = parse_pgm_header(inputPath, &w, &h, &maxv);

    if(header_size < 0){
        printf("PGM header error\n");
        return NULL;
    }

    header_size_global = header_size;
    maxVal_global = maxv;

    //inicialitzem tots els producers estan actius
    active_producers = num_producers;

    // 2.Inicialitzar buffer

    //crear buffer circular amb la mida indicada
    buffer_init(&cb, buffer_size * BLOCK_SIZE);
    
    // 3. Inicialitzar mutex i cond
    pthread_mutex_init(&lock_buf, NULL);
    pthread_mutex_init(&lock_read, NULL);
    pthread_mutex_init(&lock_hist, NULL);

    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    for(int i=0;i<256;i++){
        global_hist[i] = 0;
    }

    // 4. Crear threads

    // Arrays per guardar identificadors de threads
    pthread_t prod[num_producers];
    pthread_t cons[num_consumers];

    // Arguments de cada thread
    ProducerArgs pa[num_producers];
    ConsumerArgs ca[num_consumers];

    // Crear producers
    for(int i=0;i<num_producers;i++){

        pa[i].path = inputPath;
        pa[i].id = i;

        pthread_create(&prod[i], NULL, Producer, &pa[i]);
    }

    // Crear consumers
    for(int i=0;i<num_consumers;i++){

        ca[i].id = i;

        pthread_create(&cons[i], NULL, Consumer, &ca[i]);
    }

    // 5. Esperar threads

    // Esperem que acabin tots els producers
    for(int i=0;i<num_producers;i++){
        pthread_join(prod[i], NULL);
    }

    // Esperem que acabin tots els consumers
    for(int i=0;i<num_consumers;i++){
        pthread_join(cons[i], NULL);
    }

    // 6. Escriure histograma final
    int fd = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // Escriure cada valor de l'histograma
    for(int i = 0;i < 255; i++){
        dprintf(fd,"%d,%d\n",i,global_hist[i]);
    }

    close(fd);

    return 0;
}