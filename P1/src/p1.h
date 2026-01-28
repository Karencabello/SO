
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>


extern int SIZE;

long long istxt(int fd);
long long isbin(int fd);
int main(int argc, char* argv[]);