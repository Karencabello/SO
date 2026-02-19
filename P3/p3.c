

/* INFO:
    - To inspect the beginning of an image file, you may use: 
    head -c 500 Data/heart.pgm  

    - To compile:
    gcc P3_sequential.c parsePGM.c -o build/computeHistogramSequential 
    build/computeHistogramSequential Data/heart.pgm Data/histogram_heart.txt 
    python showHistogram.py Data/histogram_heart.txt

    - To run:
    ./program heart.pgm out.txt 1
    ./program heart.pgm out.txt 4
    ./program heart.pgm out.txt 8

*/

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parsePGM.h"

typedef struct { 
    char* path; 
    int offset;       // Offset from the beginning of the file (including header) 
    int bytesToRead; 
    int local_histogram[256]; // Histograma local per a cada thread
} ThreadInfo; 

void* handler(void* arg) {

    // 1. Convertir arg a ThreadInfo*
    ThreadInfo* info = (ThreadInfo*)arg;
    // -> Ja tenim path, offset, ...

    // 2. Obrir el fitxer d'imatge
    int fd = open(info->path, O_RDONLY);
    
    if (fd < 0) {   
        fprintf(stderr, "Error opening file: %s\n", info->path);
        pthread_exit(NULL);
    }

    // 3. posicionar-se a l'offset (fseek)
    if (lseek(fd, info->offset, SEEK_SET) < 0) {
        fprintf(stderr, "Error seeking in file: %s\n", info->path);
        close(fd);
        return NULL;
    }   
    // Preparar buffer (Max 1024 bytes)
    unsigned char buffer[1024];

    int bytesLeft = info->bytesToRead;

    // Llegim per blocs
    /*
    Mentre encara quedin bytes per llegir:
    Llegeix un bloc (≤ 1024 bytes)
    Recorre el buffer
    Actualitza histograma local
    Redueix bytes pendents
    */

    while(bytesLeft > 0){

        // Decidor quants bytes llegim
        int bytesToRead = bytesLeft < sizeof(buffer) ? bytesLeft : sizeof(buffer);

        int n = read(fd, buffer, bytesToRead);

        if (n < 0) {
            fprintf(stderr, "Error reading file: %s\n", info->path);
            close(fd);
            return NULL;
        }   
        if (n == 0) {
            // EOF inesperat
            break;
        }

        // Actualitzar histograma local
        for(int i = 0; i < n; i++){
            unsigned char pixelValue = buffer[i];
            info->local_histogram[pixelValue]++;    
        }
        bytesLeft -= n;
    }
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

    // Variables
    char* inputPath = argv[1];
    char* outputPath = argv[2];
    int numThreads = atoi(argv[3]);

    if(numThreads <= 0){
        fprintf(stderr, "Number of threads must be a positive integer.\n");
        return 1;
    }   

    int width, height, maxVal;

    // 2. Llegir header amb parse_pgm_header()
    int headerSize = parse_pgm_header(inputPath, &width, &height, &maxVal);
    
    if(headerSize < 0){
        fprintf(stderr, "Error parsing PGM header.\n");
        return 1;
    }

    // 3. Calcular datasize = width * height
    int dataSize = width * height;

    // 4. Dividir treball entre els threads --> threads de 4 bytes, l'ultim menys 
    int bytesPerThread = (dataSize + numThreads - 1) / numThreads; // ceil division
    
    // 5. Crear threads i passar-los la informació necessària (ThreadInfo)
    
    // Reservem memoria per identificadors threads
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));

    // Reservem memoria per guardar info de cada thread
    ThreadInfo* threadInfos = malloc(numThreads * sizeof(ThreadInfo));

    if(!threads || !threadInfos){
        fprintf(stderr, "Memory allocation failed.\n");
        free(threads);
        free(threadInfos);
        return 1;
    }

    // Histograma global --> resultat
    int global_histogram[256] = {0}; // Inicialitzat a 0

    // Crear threads
    for(int i = 0; i < numThreads; i++){

        // indica on comença a llegir cada thread i quants bytes ha de llegir (headersize pq volem com si fossin pixels)
        int offset = headerSize + i * bytesPerThread;
        
        // Calculem quants bytes queden a partir d'aquest ultim thread
        int remainingBytes = dataSize - i * bytesPerThread;
        
        // Si és ñ'ultim thread i queden menys bytes, li donem més als que queden --> evita llegir més enllà del final de les dades
        int bytesToRead = remainingBytes < bytesPerThread ? remainingBytes : bytesPerThread;

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

        // Construim string
        int len = snprintf(line, sizeof(line), "%d %d\n", i, global_histogram[i]);
        
        // Escribim al fitxer
        write(fd_out, line, len);
    }

    close(fd_out);
    free(threads);
    free(threadInfos);
    return 0;
}