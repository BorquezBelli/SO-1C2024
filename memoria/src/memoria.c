#include "../include/memoria.h"
// CRAER HILOS COMO VARIABLES GLOBALES
pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;
config_memoria *valores_config;
int server_fd;
int PID_buscado;
// sem_t *sem_kernel;

t_list *lista_contextos;
t_list *lista_instrucciones;

int main(int argc, char *argv[])
{

    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = configuracion_memoria();

    lista_contextos = list_create();
    lista_instrucciones = list_create();

    crearHilos();

    pthread_join(hiloCpu, NULL);
    pthread_join(hiloKernel, NULL);
    pthread_join(hiloIO, NULL);
    destruirConfig(valores_config->config);
    destruirLog(logger);
}

void crearHilos()
{
    server_fd = iniciar_servidor(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);
    log_info(logger, "Memoria lista para recibir KERNEL, CPU Y dispositivos de ENTRADA/SALIDA");

    pthread_create(&hiloCpu, NULL, recibirCPU, NULL);
    pthread_create(&hiloKernel, NULL, recibirKernel, NULL);
    pthread_create(&hiloIO, NULL, recibirIO, NULL);
}

void *recibirIO()
{
    // va a recibir interfaces que le van a pedir acceso al espacio del usuario (stdin,stdout y dialfs) :D
    return EXIT_SUCCESS;
}

void *recibirKernel()
{
    tipo_buffer *buffer = crear_buffer();
    int cliente_fd = esperar_cliente(logger, "Memoria", "Kernel", server_fd);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_fd, buffer);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            finalizar_proceso(cliente_fd, buffer);
            break;
        case -1:
            log_error(logger, " El KERNEL se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
            break;
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            return (void *)EXIT_FAILURE;
            break;
        }
    }
}

void iniciar_proceso(int cliente_fd, tipo_buffer *buffer)
{
    log_info(logger, "SOLICITUD INICIAR PROCESO");
    buffer = recibir_buffer(cliente_fd);
    t_cde *cde = armarCde(buffer);
    log_info(logger, "La ruta del archivo recibida es:>%s<",cde->path);

    destruir_buffer(buffer);
    cde->lista_instrucciones = leerArchivoConInstrucciones(cde->path);
    if (cde->path == NULL || cde->lista_instrucciones == NULL)
    {
        enviar_cod_enum(cliente_fd, ERROR_INICIAR_PROCESO);
    }
    else
    {
        list_add(lista_contextos, cde); // agrego el cde del proceso creado
        list_add(lista_instrucciones, cde->lista_instrucciones);
        enviar_cod_enum(cliente_fd, INICIAR_PROCESO_CORRECTO);
        log_info(logger, "Se inicio el proceso de PID: %d y PATH: %s", cde->pid, cde->path);
    }
}

t_cde *armarCde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    cde->path = leer_buffer_string(buffer);
    cde->lista_instrucciones = list_create();
    return cde;
}

t_list *leerArchivoConInstrucciones(char *nombre_archivo)
{
    t_list *list_instrucciones = list_create(); // creo el puntero a la lista
    char *ruta_completa = string_new();
    char *ruta_acceso[1024];

    if (getcwd(ruta_acceso, sizeof(ruta_acceso)) == NULL)
    {
        log_info(logger, "No se pudo obtener la raiz");
        return NULL;
    }
    //printf("La ruta local es:%s \n",ruta_acceso);
    string_append(&ruta_completa, ruta_acceso);
    string_append(&ruta_completa, "/pruebas/");
    string_append(&ruta_completa, nombre_archivo);
    FILE *archivo = fopen(ruta_completa, "r");
    if (archivo == NULL)
    {
        log_warning(logger, "No se pudo abrir el archivo: >%s<", ruta_completa);
        return NULL;
    }
    char linea_instruccion[1024]; // este seria el buffer para ir leyendo el archivo
    while (fgets(linea_instruccion, sizeof(linea_instruccion), archivo) != NULL)
    { // voy leyendo el archivo
        strtok(linea_instruccion, "\n");
        list_add(list_instrucciones, linea_instruccion); // agrego una instruccion a la lista MOV AX BX
    }

    fclose(archivo);
    free(ruta_completa);
    return list_instrucciones;
}

void *recibirCPU()
{
    int cliente_cpu = esperar_cliente(logger, "Memoria", "CPU", server_fd);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_cpu);
        switch (cod_op)
        {
        case PEDIDO_INSTRUCCION:
            pedido_instruccion_cpu_dispatch(cliente_cpu, lista_contextos);
            break;
            // case ACCESO_ESPACIO_USUARIO:
        case -1:
            log_error(logger, "El cliente se desconecto. Terminando servidor");
            exit(EXIT_FAILURE);
            return (void *)EXIT_FAILURE;
            break;
            // return EXIT_FAILURE; // version de la catedra, pero da un warning si no anda comentar el de arriba
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
    }
}

void pedido_instruccion_cpu_dispatch(int cliente_fd, t_list *contextos)
{
    log_info(logger, "PEDIDO DE INSTRUCCION POR CPU DISPATCH");

    tipo_buffer *buffer = recibir_buffer(cliente_fd);
    uint32_t PID = leer_buffer_enteroUint32(buffer);
    uint32_t PC = leer_buffer_enteroUint32(buffer);
    t_cde *contexto = malloc(sizeof(t_cde));

    contexto = obtener_contexto_en_ejecucion(PID, contextos);

    tipo_buffer *buffer_instruccion = crear_buffer();
    char *instruccion = string_new();
    instruccion = list_get(contexto->lista_instrucciones, PC);
    enviar_cod_enum(cliente_fd, ENVIAR_INSTRUCCION_CORRECTO);
    agregar_buffer_para_string(buffer_instruccion, instruccion);

    enviar_buffer(buffer_instruccion, cliente_fd);
    destruir_buffer(buffer_instruccion);
    destruir_buffer(buffer);
    log_info(logger, "SE APRUEBA PEDIDO DE INSTRUCCION");
}

t_cde *obtener_contexto_en_ejecucion(int PID, t_list *contextos)
{
    PID_buscado = PID;
    t_cde *cde_proceso = malloc(sizeof(t_cde));

    cde_proceso = list_find(contextos, estaElContextoConCiertoPID); // problema al hacer list find, no guarda la lsita de ionstrucciones corracmentae

    cde_proceso->lista_instrucciones = list_get(lista_instrucciones, cde_proceso->pid);

    log_info(logger, "SE OBTUVO EL PROCESO PID: %d CON PATH: %s Y CON: %d INSTRUCCIONES", cde_proceso->pid, cde_proceso->path, list_size(cde_proceso->lista_instrucciones));
    log_info(logger, "INSTRUCCIONES: %s %d", cde_proceso->lista_instrucciones->head->data, cde_proceso->lista_instrucciones->head->data);
    return cde_proceso;
}
char *obtener_char_instruccion(t_tipoDeInstruccion instruccion_code)
{
    if (instruccion_code == SET)
    {
        return "SET";
    }
    if (instruccion_code == MOV_IN)
    {
        return "MOV_IN";
    }
    if (instruccion_code == MOV_OUT)
    {
        return "MOV_OUT";
    }
    if (instruccion_code == SUM)
    {
        return "SUM";
    }
    if (instruccion_code == SUB)
    {
        return "SUB";
    }
    if (instruccion_code == JNZ)
    {
        return "JNZ";
    }
    if (instruccion_code == RESIZE)
    {
        return "RESIZE";
    }
    if (instruccion_code == COPY_STRING)
    {
        return "COPY_STRING";
    }
    if (instruccion_code == WAIT)
    {
        return "WAIT";
    }
    if (instruccion_code == SIGNAL)
    {
        return "SIGNAL";
    }
    if (instruccion_code == IO_GEN_SLEEP)
    {
        return "IO_GEN_SLEEP";
    }
    if (instruccion_code == IO_STDIN_READ)
    {
        return "IO_STDIN_READ";
    }
    if (instruccion_code == IO_STDOUT_WRITE)
    {
        return "IO_STDOUT_WRITE";
    }
    if (instruccion_code == IO_FS_CREATE)
    {
        return "IO_FS_CREATE";
    }
    if (instruccion_code == IO_FS_DELETE)
    {
        return "IO_FS_DELETE";
    }
    if (instruccion_code == IO_FS_TRUNCATE)
    {
        return "IO_FS_TRUNCATE";
    }
    if (instruccion_code == IO_FS_WRITE)
    {
        return "IO_FS_WRITE";
    }
    if (instruccion_code == IO_FS_READ)
    {
        return "IO_FS_READ";
    }
    if (instruccion_code == EXIT)
    {
        return "EXIT";
    }
    return -1;
}

_Bool estaElContextoConCiertoPID(t_cde *contexto)
{
    return contexto->pid == PID_buscado;
}

void finalizar_proceso(int cliente_fd, tipo_buffer *buffer) // HACER
{
    buffer = recibir_buffer(cliente_fd);
    uint32_t pid_a_eliminar = leer_buffer_enteroUint32(buffer);
    eliminar_proceso(pid_a_eliminar);
}

void eliminar_proceso(int pid_a_eliminar) // HACER
{
}

config_memoria *configuracion_memoria()
{
    config_memoria *valores_config = malloc(sizeof(config_memoria));

    valores_config->config = iniciar_config("memoria.config");
    valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP");
    valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA");
    valores_config->path_instrucciones = config_get_string_value(valores_config->config, "PATH_INSTRUCCIONES");
    valores_config->tam_memoria = config_get_int_value(valores_config->config, "TAM_MEMORIA");
    valores_config->tam_pagina = config_get_int_value(valores_config->config, "TAM_PAGINA");
    valores_config->retardo_respuesta = config_get_int_value(valores_config->config, "RETARDO_RESPUESTA");
    return valores_config;
}
int cantidad_de_parametros_instruccion(t_instruccion *instruccion)
{
    if (instruccion->codigo == SET)
    {
        return 2;
    }
    else if (instruccion->codigo == SUM)
    {
        return 2;
    }
    else if (instruccion->codigo == SUB)
    {
        return 2;
    }
    else if (instruccion->codigo == JNZ)
    {
        return 2;
    }
    else if (instruccion->codigo == IO_GEN_SLEEP)
    {
        return 2;
    }
    else if (instruccion->codigo == EXIT)
    {
        return 0;
    }
}