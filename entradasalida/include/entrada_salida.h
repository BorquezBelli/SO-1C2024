#ifndef ENTRADA_SALIDA_H_
#define ENTRADA_SALIDA_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/serializacion.h"

// VARIABLES
extern t_log *logger;
int conexion_kernel, conexion_memoria, estoy_libre;
// ESTRUCTURAS
typedef struct
{
	t_config *config;
	int puerto_kernel;
	int puerto_memoria;
	char *tipo_interfaz;
	char *ip_kernel;
	char *ip_memoria;
	char *path_base_dialfs;
	int tiempo_unidad_trabajo;
	int block_size;
	int block_count;
	int retraso_compactacion;
} config_io;

void iniciar_modulo_io();
void liberar_modulo_io();
void levantar_interfaz(char *nombre, char *PATH);
t_interfaz *crear_interfaz(config_io *config, char *nombre);
enum_interfaz asignar_interfaz(char *nombre_Interfaz);
config_io *inicializar_config_IO(char *PATH);
void arrancar_interfaz(t_interfaz *interfaz);
void conectarse_kernel(t_interfaz *interfaz);
void conectarse_memoria(t_interfaz *interfaz);
void realizar_operacion(t_interfaz *interfaz);
void realizar_operacion_gen(t_interfaz *interfaz);
void realizar_operacion_stdin();
void realizar_operacion_stdout();
void realizar_operacion_dialfs();
t_list *convertir_a_numeros(char *texto);
void truncar_valor(char **valor_nuevo, char *valor_viejo, int limitante);
void int_a_char_y_concatenar_a_string(int valor, char *cadena);

#endif
