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
#include "producer.c"


typedef struct { 
    char* path; 
    int offset;       // Offset from the beginning of the file (including header) 
    int bytesToRead; 
    int local_histogram[256]; // Histograma local per a cada thread
} ThreadInfo;

void* consumer(void* arg) {
    
    // 1. Convertir arg a ThreadInfo*
    ThreadInfo* info = (ThreadInfo*)arg;
    // -> Ja tenim path, offset, ...

    // 2. Mirar buffer
    // locks
    // mirar
    // unlock



   
        // 3. Actualitzar histograma local
        for(int i = 0; i < n; i++){
            unsigned char pixelValue = buffer[i];
            info->local_histogram[pixelValue]++;    
        }
        

    /// --- MIRAR BE
   // 4. Tancar coses
    close(fd);
    // 5. exitt
    pthread_exit(NULL);
}


int main(int argc, char* argv[]){
    
    // 1. Llegir arguments
    if(argc != 4){
        fprintf(stderr, "Usage: %s <input_image.pgm> <output_histogram.txt> <num_threads>\n", argv[0]);
        return 1;
    }

    // Variables --CAnviar
    char* inputPath = argv[1];
    char* outputPath = argv[2];
    int numproducers = atoi(argv[3]);
    int numcomsumers = atoi(argv[4]);
    int buffer_size = atoi(argv[5]);

      

    int width, height, maxVal;

    // 2. Llegir header amb parse_pgm_header()
    int headerSize = parse_pgm_header(inputPath, &width, &height, &maxVal);
    
    if(headerSize < 0){
        fprintf(stderr, "Error parsing PGM header.\n");
        return 1;
    }

    // 3. Calcular datasize = width * height
    int dataSize = width * height;

    // 4. Dividir treball entre els threads producers --> quants blocs llegeixen, cada bloc 16,384 bytes (1024 × 16)
    int block_producer;        
    
    // 5. Crear threads i passar-los la informació necessària (ThreadInfo)
    
    // Reservem memoria per identificadors producer
    pthread_t* threads_prod = malloc(numproducers * sizeof(pthread_t));

    // Reservem memoria per guardar info de cada producer
    ThreadInfo* threadInfos_prod = malloc(numproducers * sizeof(ThreadInfo));

    // Reservem memoria per identificadors consumer
    pthread_t* threads_cons = malloc(numcomsumers * sizeof(pthread_t));

    // Reservem memoria per guardar info de cada consumer
    ThreadInfo* threadInfos_cons = malloc(numcomsumers * sizeof(ThreadInfo));

    if(!threads_prod || !threadInfos_prod || !threads_cons || !threadInfos_cons){
        fprintf(stderr, "Memory allocation failed.\n");
        free(threads_prod);
        free(threadInfos_prod);
        free(threads_cons);
        free(threadInfos_cons);
        return 1;
    }

    // Histograma global --> resultat
    int global_histogram[256] = {0}; // Inicialitzat a 0

    // ------------------------------------------------------------------ REVISAR ---------------------------------
    // Crear threads producer
    for(int i = 0; i < numproducers; i++){

        // indica on comença a llegir cada producer i quants bytes ha de llegir (headersize pq volem com si fossin pixels)
        int offset = headerSize + i * bytesPerThread;
        
        // Calculem quants bytes queden a partir d'aquest ultim thread
        int remainingBytes = dataSize - i * bytesPerThread;
        
        // Si és ñ'ultim thread i queden menys bytes, li donem més als que queden --> evita llegir més enllà del final de les dades
        int bytesToRead;
        if (remainingBytes < bytesPerThread) {
            bytesToRead = remainingBytes;
        } else {
            bytesToRead = bytesPerThread;
        }

        // Omplim la info del thread
        threadInfos_prod[i].path = inputPath;
        threadInfos_prod[i].offset = offset;
        threadInfos_prod[i].bytesToRead = bytesToRead;

        // Inicialitzem el histograma
        for (int j = 0; j < 256; j++){
            threadInfos_prod[i].local_histogram[j] = 0;
        }
        
        // Creem el thread producer
        if(pthread_create(&threads_prod[i], NULL, handler, &threadInfos_prod[i]) != 0){
            fprintf(stderr, "Error creating producer thread %d\n", i);
            free(threads_prod);
            free(threadInfos_prod);
            free(threads_cons);
            free(threadInfos_cons);
            return 1;
        }
    }

    // Crear threads consumer
    for(int i = 0; i < numcomsumers; i++){

        // indica on comença a llegir cada consumer i quants bytes ha de llegir (headersize pq volem com si fossin pixels)
        int offset = headerSize + i * bytesPerThread;
        
        // Calculem quants bytes queden a partir d'aquest ultim consumer
        int remainingBytes = dataSize - i * bytesPerThread;
        
        // Si és ñ'ultim consumer i queden menys bytes, li donem més als que queden --> evita llegir més enllà del final de les dades
        int bytesToRead;
        if (remainingBytes < bytesPerThread) {
            bytesToRead = remainingBytes;
        } else {
            bytesToRead = bytesPerThread;
        }

         // Omplim la info del consumer
         threadInfos_cons[i].path = inputPath;
         threadInfos_cons[i].offset = offset;
         threadInfos_cons[i].bytesToRead = bytesToRead;

         // Inicialitzem el histograma
         for (int j = 0; j < 256; j++){
             threadInfos_cons[i].local_histogram[j] = 0;
         }
         
         // Creem el thread consumer
         if(pthread_create(&threads_cons[i], NULL, consumer, &threadInfos_cons[i]) != 0){
             fprintf(stderr, "Error creating consumer thread %d\n", i);
             free(threads_prod);
             free(threadInfos_prod);
             free(threads_cons);
             free(threadInfos_cons);
             return 1;
         }
     }
    for(int i = 0; i < numThreads; i++){

        // indica on comença a llegir cada thread i quants bytes ha de llegir (headersize pq volem com si fossin pixels)
        int offset = headerSize + i * bytesPerThread;
        
        // Calculem quants bytes queden a partir d'aquest ultim thread
        int remainingBytes = dataSize - i * bytesPerThread;
        
        // Si és ñ'ultim thread i queden menys bytes, li donem més als que queden --> evita llegir més enllà del final de les dades
        int bytesToRead;
        if (remainingBytes < bytesPerThread) {
            bytesToRead = remainingBytes;
        } else {
            bytesToRead = bytesPerThread;
        }

        // Omplim la info del thread
        threadInfos[i].path = inputPath;
        threadInfos[i].offset = offset;
        threadInfos[i].bytesToRead = bytesToRead;

        // Inicialitzem el histograma
        for (int j = 0; j < 256; j++){
            threadInfos[i].local_histogram[j] = 0;
        }
        // Creem el thread
        if(pthread_create(&threads[i], NULL, handler, &threadInfos[i]) != 0){
            fprintf(stderr, "Error creating thread %d\n", i);
            free(threads);
            free(threadInfos);
            return 1;
        }
    }

    // 6. Esperar a que acabin i sumar els resultats parcials per obtenir l'histograma final
    for(int i = 0; i < numThreads; i++){
        pthread_join(threads[i], NULL);
        // Sumar histograma local al global
        for(int j = 0; j < 256; j++){
            global_histogram[j] += threadInfos[i].local_histogram[j];
        }
    }
    // 7. Escriure l'histograma a un fitxer de text
    int fd_out = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(fd_out < 0){
        fprintf(stderr, "Error opening output file: %s\n", outputPath);
        free(threads);
        free(threadInfos);
        return 1;
    }

    // Buffer per escriure cada linia
    char line[256];

    for(int i = 0; i < 256; i++){

        // Construim string separada per comes
        int len = snprintf(line, sizeof(line), "%d,%d\n", i, global_histogram[i]);
        
        // Escribim al fitxer
        write(fd_out, line, len);
    }

    close(fd_out);
    free(threads);
    free(threadInfos);
    return 0;
}