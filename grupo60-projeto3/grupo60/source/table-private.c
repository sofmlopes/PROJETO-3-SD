/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include "table-private.h"
#include "stdio.h"
#include "sdmessage.pb-c.h"

int hash_code(char *key, int n) {

    if (key == NULL) 
        return -1; 
    

    unsigned long soma = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        soma += key[i]; 
    }

    return (int)(soma % n);
}