#ifndef LARGOPLAZO_H_
#define LARGOPLAZO_H_
#include <unistd.h>
#include "../include/kernel.h"

void* largo_plazo();
t_pcb* transicion_new_a_ready();
void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado);
void *transicion_exit_largo_plazo();

#endif