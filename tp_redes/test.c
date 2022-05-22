#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int just_numbers(char *validation) {
    if (strlen(validation) != 2)
        return -1;

    if (validation[0] != '0')
        return -1;
    
    if (validation[1] < '1' || validation[1] > '4')
        return -1;

    return 0;
}

int main(int argc, char **argv){
    /*
            TESTANDO SPLITAR STRINGS
    */
    // char *pt = {"Bom!dia.simpatia Bom dia"};
    // char *aux = (char *)malloc(sizeof(char)*1024);
    // char *to_free = aux;
    // strcpy(aux, pt);
    // printf("%s\n", aux);

    // strtok(aux, "!. ");

    // //char *aux2 = aux;
    // while(aux){
    //     printf("token: %s\n", aux);
    //     aux = strtok(NULL, "!. ");
    // }

    // free(to_free);

    // printf("%s\n", pt);

    /*
        TESTANDO TRANSFORMAR STRING EM NUMERO
    */

    // char number[10] = {"2@log5"};
    // char **stop_string = NULL;
    // long l;
    // l = strtol(number, stop_string, 10);
    // printf("%ld\n", l);

    /*
        TESTANDO A FUNÇÃO JUST_NUMBERS
    */

    // printf("%d\n", just_numbers("00"));
    // printf("%d\n", just_numbers("01"));
    // printf("%d\n", just_numbers("02"));
    // printf("%d\n", just_numbers("03"));
    // printf("%d\n", just_numbers("04"));
    // printf("%d\n", just_numbers("54sdfsd"));
    // printf("%d\n", just_numbers("in"));
    // printf("%d\n", just_numbers("4i"));
    // printf("%d\n", just_numbers("0a"));
    // printf("%d\n", just_numbers("8b"));
    // printf("%d\n", just_numbers(""));
    // printf("%d\n", just_numbers("gsddasf"));
    // printf("%d\n", just_numbers("-3"));

    /*
        TESTANDO STRCAT
    */
    // #define LIM 40000
    // int j;
    // char p[LIM + 1];    /* +1 for terminating null byte */
    // time_t base;

    // base = time(NULL);
    // p[0] = '\0';

    // for (j = 0; j < LIM; j++) {
    //     if ((j % 10000) == 0)
    //         printf("%d %ld\n", j, (long) (time(NULL) - base));
    //     strcat(p, "a");
    // }

    // printf("%s\n",p);

    /*
        TESTANDO TIRAR ULTIMO CARACTERE
    */

    // char str[10] = {"String"};
    // printf("tamanho antes: %d\n", (int)strlen(str));
    // str[strlen(str)-1] = '\0';
    // printf("tamanho depois: %d\n", (int)strlen(str));

    /*
        TESTANDO GERAR NÚMEROS ALEATÓRIOS
    */
    srand((unsigned int)time(NULL));
    printf("%d\n", rand());
    printf("%.2f\n", ((float)rand()/(float)(RAND_MAX)) * 5.);

    return 0;
}