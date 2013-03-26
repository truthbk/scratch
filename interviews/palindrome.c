#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>

int is_palindrome(char * s, size_t sz) {
    for(int i=0 ; i<sz/2 ; i++) {
        if(s[i] != s[sz-i-1]) {
            return 0;
        }
    }
    return 1;
}

int min_palindrome(char * s, size_t sz) {
    
    int aux_1, aux_2;
    int n_palindromes = sz;


    if(sz <= 1) {
        return sz;
    }

    if(is_palindrome(s, sz)) {
        return 1;
    }

    for(int i=1 ; i<sz ; i++){
        aux_1 = min_palindrome(s, i);
        aux_2 = min_palindrome(s+i, sz-i);
        if(aux_1 + aux_2 < n_palindromes) {
            n_palindromes = aux_1+aux_2;
        }

    }

    return n_palindromes;

}

int main(int argc, char ** argv) {
    char * str="abcdeeffgghhggffeedcba";
    char * str2="abacbbcXX";
    //char * str2="abcddcbaeeffgghhggffeexxzzxx";

    fprintf(stdout,"Minimum number of palindromes for: %s is %d\n", 
            str, min_palindrome(str, strlen(str)));
    fprintf(stdout,"Minimum number of palindromes for: %s is %d\n", 
            str2, min_palindrome(str2, strlen(str2)));

}
