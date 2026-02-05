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

#include "splitCommand.h" // mirar com ficar be
#include "circularBuffer.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <stdbool.h>

int getLine(CircularBuffer *cb, char *line, int size){

    int i = 0;

    // mentres hi hagi dades al cb --> treiem char per char fins
    while( buffer_free_bytes(&cb) != 0 && i < size -1){
        char ch = buffer_pop(&cb); // treiem un char de cb
        line[i] = ch;
        i++;
        // fins que arribem al final de la linea
        if(ch == '\n'){
            line[i] == '\0';
            return 1; // linea completa
        }
    }
    line[i] = '\0'; 
    return 0; // --> no hem trobat final
}

int main(int argc, char* argv[]){

    bool ispipe = false;
    bool issingle = false;
    bool isconcurrent = false;
    bool isexit = false;

    char type;
    char command;
    char argument;
    char tmp[256]; // per si queden coses a linia
    char line[512]; // una linea sencera

    int expecting_type = 1; // per saber quan hem de mirar type
        
    CircularBuffer cb;
    buffer_init(&cb,1024); // mirar si size bé

    while(!isexit){

        // read line

        // 1. mirem que la linia estigui sencera
        while(getLine(&cb, line, sizeof(line)) == 1){

            // determinem execution mode
            if(expecting_type){
                // treiem type de la linia
                if(line[0] == '\n' || line[0] == '\0') continue;
                type = line[0];
                expecting_type = 0; // ja el sabem
            }else{
                // mirem el command line i fer coses
            }

            expecting_type = 1;
        }


        
        



        // detrmine execution mode

        //if ispipe(){
        // fer 2n proces (fork)

        //dup2 per conectar-los abans execvp!!!!
        //}

        // llegir comando i arguments (següent linia)

        // fork 

        // al final
        // si NO es concurrent --> waitpid
        // fill --> replaces its program image by invoking  execvp() with the parsed command and its arguments.

    }

    //
}