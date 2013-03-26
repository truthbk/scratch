#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <limits.h>


void merge(int64_t * a, uint32_t p, uint32_t r ) {

    uint64_t * temp = NULL;
    
    int i = 0;
    int j = p;
    int k = (p+r+1) / 2;
    int d = k;

    uint64_t aux_1, aux_2;

    uint32_t tmp_sz = r - p + 1;
    
    if(!(temp = malloc(sizeof(int64_t) * tmp_sz))) {
        return;
    }

    for (i=0 ; i<tmp_sz ; i++) {

        if(j>=d) {
            aux_1 = INT_MAX; 
        } else {
            aux_1 = a[j];
        }
        if(k>r) {
            aux_2 = INT_MAX; 
        } else {
            aux_2 = a[k];
        }

        if(aux_1 < aux_2) {
            temp[i] = aux_1;
            j++;
        } else {
            temp[i] = aux_2;
            k++;
        }
    }

    memcpy(a+p, temp, tmp_sz*sizeof(int64_t));
    free(temp);
};


void merge_sort(int64_t * a, uint32_t sz) {
    int sub_sz = sz/2;
    if(sz <= 1)
        return;

    merge_sort(a, sub_sz);
    merge_sort(a+sub_sz, sz-sub_sz);
    merge(a, 0, sz-1);
};


int main(int argc, char ** argv) {
    int64_t a[12] = { 5, 7, 1, 9, 2, 6, 1, 4, 2, 0, 8, 3 };

    merge_sort(a, 12);
    for(int i=0 ; i<12 ; i++) {
        fprintf(stdout, "%lld ", a[i]);
    }
    fprintf(stdout, "\n");
}

