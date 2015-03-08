#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <string.h>

void hexdump(void *ptr, int buflen) {
    unsigned char *buf = (unsigned char *) ptr;
    int i, j;
    for (i = 0; i < buflen; i += 16) {
        printf("%06x: ", i);
        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");
        printf(" ");
        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
        printf("\n");
    }
}

void hexdumpraw(void *ptr, char *result, int buflen) {
    unsigned char *buf = (unsigned char *) ptr;
    int a = 0;
    for (a = 0; a < buflen; a++) {
        sprintf(result + (a * 2), "%02X", buf[a]);
    }
}

/* Convert IP string to 32 bit unsigned int */
uint32_t ip2int(char *ip) {
    struct in_addr a;
    if (!inet_aton(ip, &a)) {
        // IP was invalid - return 0
        return ((uint32_t) 0);
    }
    return a.s_addr;
}

/* Convert 32bit unsigned int to a string representation */
void int2ip(uint32_t ip, char *result) {
    struct in_addr a;
    a.s_addr = ip;
    strcpy(result, inet_ntoa(a));
}
