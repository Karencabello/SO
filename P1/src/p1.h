
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>


extern int SIZE;

char istxt(int fd);
int isbin(int fd);
int main(int argc, char* argv[]);