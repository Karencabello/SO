/*Program flow: 
The general execution flow of the shell follows a simple pattern, in an infinite loop: 
    ● It will read a line, and determine the execution mode. If it is EXIT it will end the process. 
    ● Then it will read the command and arguments in the next line. It will create a new 
    process using fork(). The child process then replaces its program image by invoking 
    execvp() with the parsed command and its arguments. 
        ○ If it is a PIPED execution, it will need to read a second line and create a second 
        process,  as  well  as  creating  the  pipe  and  use  dup2()to  connect  both 
        processes before the execvp. 
    ● If it is not a CONCURRENT execution, use  waitpid() to wait for the previous process 
    (in the case of the piped, you will need to wait for both of them). Search in the linux 
    documentation how to use waitpid, and how it is slightly different from the wait 
    command seen in class. 
Hints: 
1)  You need to use strcmp for comparing strings. Remember that read will also return the 
‘\n’. 
2)  The command line must be parsed into a NULL-terminated array of strings suitable 
for use with execvp. You can do it yourself, or use the given function split_command 
3)  When reading from the keyboard with a large buffer, each read will return information 
in a line. However, when the standard input is redirected to a file you will need to use 
either a circular buffer, as in last practice to correctly split in lines. */


/*(SINGLE | PIPED | CONCURRENT)*/

/*that reads instructions from standard 
input and executes them according to a specified execution mode operates in a 
loop, repeatedly reading instructions from the standard input, interpreting them, and launching 
the corresponding processes. Execution continues until an explicit termination instruction is 
encountered. */

#include "splitCommand.h" 
#include "circularBuffer.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <stdbool.h>

// Lllegeix linia de stdin 
// retorna --> 1 si linea, 0 si EOF i res pendent i -1 si error 
static int read_line(CircularBuffer *cb, char *line, int maxlen, int *reachedEOF){
    char chunk[512];

    while(1){
        // 1) Mira si linia completa cb
        int n = buffer_size_next_element(cb, '\n', *reachedEOF);

        if(n != -1){
            // Ja tenim linia completa
            int i = 0;

            // copiem fins maxlen-1 (per deixar espai per '\0)
            while(i < n && i < maxlen -1){
                line[i++] = (char)buffer_pop(cb);
            }
            line[i] = '\0';

            // si la linia era més llarga que maxlem descartem la resta
            while (i < n){
                (void)buffer_pop(cb);
                i++;
            }
            return 1;
        }

        // 2) No hi ha \n (linia completa) --> llegim més bytes
        int r = read(0, chunk, sizeof(chunk));

        if (r < 0) return -1;

        // 3) EOF
        if (r == 0){
            *reachedEOF = 1;
            if (buffer_used_bytes(cb) == 0) return 0;
            // si queda alguna cosa al buffer,
            //al seguent loop n != -1 i la retornarà
            continue;
        }

        // 4) Possem bytes llegits dins el cb
        for(int k = 0; k < r; k++){
            if(buffer_free_bytes(cb) == 0) return -1;
            buffer_push(cb, (unsigned char)chunk[k]);
        }
    }
}

int main(int argc, char* argv[]){

    CircularBuffer cb;
    buffer_init(&cb,1024); 

    int reachedEOF = 0;

    bool ispipe = false;
    bool issingle = false;
    bool isconcurrent = false;
    bool isexit = false;

    char mode[128];
    char command1[512];
    char command2[512];

    while(!isexit){

        // 1. Llegim el mode
        int ok = read_line(&cb, mode, sizeof(mode), &reachedEOF);
        if(ok <= 0) break; // EOF o error

        // Per comparar linies treiem '\n'
        size_t len = strlen(mode);
        if (len > 0 && mode[len - 1] == '\n') {
            mode[len - 1] = '\0';
        }

        // 2. Reset de les flags (evitar errors)
        ispipe = false;
        issingle = false;
        isconcurrent = false;
        isexit = false;

        // 3. Determinem el mode
        if (strcmp(mode, "SINGLE") == 0){
            issingle = true;
        }else if(strcmp(mode, "CONCURRENT") == 0){
            isconcurrent = true;
        }else if(strcmp(mode, "PIPE") == 0){
            ispipe = true;
        }else if(strcmp(mode, "EXIT") == 0){
            isexit = true;
            break;
        }

        // 4. Llegim la comanda i determinem execution mode
        ok = read_line(&cb, command1, sizeof(command1), &reachedEOF);
        if(ok <= 0) break; // EOF o error

        // split_command --> ja ens retorna arv NULL-terminated i elimina \n per fer strcmp
        char **argv1 = split_command(command1);
        if( !argv1 || !argv1[0]){
            free(argv1);
            continue;
        } // error

        //6. mode == single:
        if (issingle){
            //  - fer fork()
            //  - fill: execvp(argv1[0], argv1)
            //  - pare: waitpid

        }
        
        // 7. mode == concurrent:
        else if(isconcurrent){
            //  - fer fork()
            //  - fill: execvp(argv1[0], argv1)
            //  - pare: torna al bucle i llegeix seguent mode
        }

        // 8. Si es PIPE:
        else if(ispipe){
            //  - llegir segona linea (command2) amb readline (com a dalt)
            //  - fer char **argv2 = split_command(command2)
            //  - comprovar error!
            //  - crear pipe
            //  - fork1 --> connectar stdout al pipe write amb dup2(fd[1], 1), execvp(argv1...)
            //  - fork2 --> connectar stdin al pipe read amb dup2(fd[0], 0), execvp(argv2...)
            //  - pare: tancar fd[0], fd[1] i waitpid(p1 i p2)
            // IMPORTANT: en PIPE també has de fer free(argv2) al final.

        }
        free(argv1);


    }
    buffer_deallocate(&cb);
    return 0;

}