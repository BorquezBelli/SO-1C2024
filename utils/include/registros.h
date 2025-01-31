#ifndef REGISTROS_H_
#define REGISTROS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "instrucciones.h"
#include "utils.h"
typedef struct
{
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_registros;
typedef struct
{
    uint32_t pid;
    uint32_t PC;
    t_registros *registros;
    t_list *lista_instrucciones;
    char *path;
} t_cde;

#endif