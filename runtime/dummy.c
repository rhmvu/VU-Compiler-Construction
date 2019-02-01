#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>


void __coco_dummy_print_allocation(int elems) {
    printf("Allocating %d elements on stack\n", elems);
}


void __coco_check_bounds(long offset, long arraysize){
    if (offset < 0 || offset >= arraysize) {
       perror("OUT OF BOUNDS EXCEPTION");
       exit(1); 
    }
}