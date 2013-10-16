#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "string.h"

int my_pow(int base, int exp) {
    int result = 1, i=0;

    if (exp==0) {
        return result;
    }
    
    for(i=0 ; i<exp ; i++) {
        result = result * base;
    }
    return result;
}

void swap(char * str) {
    int i=0, len=0;
    char c;

    len=strlen(str);
    for(i=0;i<len/2;i++){
        c = str[i];
        str[i]=str[len-i-1];
        str[len-i-1]=c;
    }

    return;
}

int itoa(int num, char * buf, int buflen) {
    uint8_t neg = 0;
    int idx=0;

    if(num > my_pow(10,buflen)-1) {
        //won't fit in buffer
        return -1;
    }

    if(num<0) {
        neg = 1;
        num = -num;
    }

    idx=0;
    while(num!=0) {
        buf[idx++]='0'+num%10;
        num=num/10;
    }

    if(neg) {
        buf[idx++] = '-';
    }
    buf[idx]='\0';

    swap(buf);

    return 0;
}

int atoi(const char * strn) {
    int res=0, i=0, len=0;
    uint8_t neg = 0;

    //validate input

    if(strn[0] == '-') {
        neg = 1;
        strn++;
    }

    len=strlen(strn);
    for(i=0;i<len;i++){
        res += (strn[len-1-i]-'0')*my_pow(10,i);
        if(res<0) {
            //OVERFLOW
            return res;
        }
    }
    if(neg) {
        res = -res;
    }
    return res;
}

int main(int argc, char **argv) {
    
    int result = 0;
    char mybuf[255];

    if(argc != 2) {
        exit(1);
    }

    result = atoi(argv[1]);
    fprintf(stdout,"converted value is: %d\n",result);
    itoa(result,mybuf, 6);
    fprintf(stdout,"converted back to string: %s\n",mybuf);

    exit(0);

}
