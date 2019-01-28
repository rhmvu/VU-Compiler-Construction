#include <stdio.h>
#include <stdlib.h>

void __cococheckbounds(long offset, long arraysize){
    if (offset < 0 || offset >= arraysize) {
       perror("OUT OF BOUNDS EXCEPTION");
       exit(1); 
    }
}