
#include <stdlib.h>
#include <stdio.h>


#define ARR_LEN 10

void play_array(int * arr) {
    int i;

    for (i=0 ; i<ARR_LEN ; i++) {
        arr[i] += ARR_LEN-i;
    }
    return;
}

int main(int argc, char **argv) {
    int idx;
    int arr[ARR_LEN];

    for(idx=0 ; idx<ARR_LEN ; idx++) {
        arr[idx] = idx;
    }
    for(idx=0 ; idx<ARR_LEN ; idx++) {
        fprintf(stdout, "%d\n", arr[idx]);
    }
    play_array(arr);
    for(idx=0 ; idx<ARR_LEN ; idx++) {
        fprintf(stdout, "%d\n", arr[idx]);
    }
}
