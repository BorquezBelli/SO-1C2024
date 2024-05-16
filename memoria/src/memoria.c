#include "../include/memoria.h"
// CRAER HILOS COMO VARIABLES GLOBALES
pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;
struct config_memoria *valores_config;
int server_fd;
int main(int argc, char *argv[])
{

    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = config_memoria();

    // LEVANTAMOS EL SERVIDOR DE MEMORIA
    // levantarServidorMemoria(logger, valores_config->puerto_memoria, valores_config->ip_memoria);
    // CREAMOS ARGUMENTOS DE HILOS
    args_CPU = crearArgumento(valores_config->puerto_memoria, valores_config->ip_memoria);
    args_KERNEL = crearArgumento(valores_config->puerto_memoria, valores_config->ip_memoria);
    args_IO = crearArgumento(valores_config->puerto_memoria, valores_config->ip_memoria);

    crearHilos(args_CPU, args_IO, args_KERNEL);
    // pthread_join(hiloCpu, NULL);
    pthread_join(hiloKernel, NULL);
    // pthread_join(hiloIO, NULL);

    destruirConfig(valores_config->config);
    destruirLog(logger);
}
void crearHilos(t_args *args_CPU, t_args *args_IO, t_args *args_KERNEL)
{
    server_fd = iniciar_servidor(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);
    log_info(logger, "Servidor Memoria listo para recibir KERNEL, CPU Y IO");

    pthread_create(&hiloCpu, NULL, recibirCPU, (void *)args_CPU);
    pthread_create(&hiloKernel, NULL, recibirKernel, (void *)args_KERNEL);
    pthread_create(&hiloIO, NULL, recibirIO, (void *)args_IO);
}
void *recibirIO(void *ptr)
{
    return EXIT_SUCCESS;
}

void *recibirKernel(void *ptr)
{
    tipo_buffer *buffer = malloc(sizeof(tipo_buffer));
    t_args *argumento = malloc(sizeof(t_args));
    argumento = (t_args *)ptr;

    int cliente_fd = esperar_cliente(logger, "Memoria", "Kernel", server_fd);
    while (1)
    {
        // puede ser que haya un porblema de sincro, talvez agregar un sem?
        op_code cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_fd, buffer);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            buffer = recibir_buffer(cliente_fd);
            uint32_t pid_a_eliminar = leer_buffer_enteroUint32(buffer);
            eliminar_proceso(pid_a_eliminar);
            break;
        case ERROR_CLIENTE_DESCONECTADO:
            log_error(logger, "El KERNEL se desconecto. Terminando servidor");
            return EXIT_FAILURE;
            break;

        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            return EXIT_FAILURE;
            break;
        }
    }
}

t_cde *armarCde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));

    cde->pid = leer_buffer_enteroUint32(buffer);

    char *string = leer_buffer_string(buffer);

    // cde->path = malloc(strlen(string) + 2);
    // strcpy(cde->path, string );
    cde->path = string;

    return cde;
}

void iniciar_proceso(int cliente_fd, tipo_buffer *buffer)
{
    log_info(logger, "Me llego la Solicitud de Iniciar Proceso");
    buffer = recibir_buffer(cliente_fd);
    t_cde *cde = armarCde(buffer);
    destruir_buffer(buffer);
    // IMPLEMENTAR leerArchivoConINstrucciones
    cde->lista_instrucciones = leerArchivoConInstrucciones(cde->path);
    if (cde->path != NULL)
    {
        enviar_cod_enum(cliente_fd, INICIAR_PROCESO_CORRECTO);
        log_info(logger, "Se inicio el proceso de PID: %d y PATH: %s", cde->pid, cde->path);
    }
    else
    {
        enviar_cod_enum(cliente_fd, ERROR_INICIAR_PROCESO);
    }
}
void *recibirCPU(void *ptr)
{
    t_args *argumento; // CASTEAR
    argumento = (t_args *)ptr;
    int cliente_fd = esperar_cliente(logger, "Memoria", "CPU", server_fd);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case PEDIDO_INSTRUCCION:
            /*             tipo_buffer *buffer = recibir_buffer(socket_cpu);

                        int pid = leer_buffer(buffer);
                        int pc = leer_buffer(buffer);
                        destruir_buffer(buffer);
                        t_pcb*proceso= buscarPCBEnColaPorPid(pid);

                        t_list *listaInstrucciones = list_get(proceso->cde->instrucciones);
                        buffer = crear_buffer();
                        escribir_buffer(buffer, instruccion);
                        enviar_buffer(buffer, socket_cpu);
                        destruir_buffer(); */
            printf("Se aprueba pedido instruccion");
        // case ACCESO_ESPACIO_USUARIO:
        case -1:
            log_error(argumento->logger, "El cliente se desconecto. Terminando servidor");
            // return (void *)EXIT_FAILURE;
            return EXIT_FAILURE; // version de la catedra, pero da un warning si no anda comentar el de arriba
        default:
            log_warning(argumento->logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
        return EXIT_SUCCESS;
    }
}

void eliminar_proceso(uint32_t pid_a_eliminar)
{
    // lo elimina
}
t_instruccion *crearInstruccion(char *linea)
{
    t_instruccion *instruccion = malloc(sizeof(instruccion));
    char *token = strtok(linea, " "); // El primer token es el código de la instrucción.
    instruccion->codigo = atoi(strdup(token));

    return instruccion;
}

t_list *leerArchivoConInstrucciones(char *pathArch)
{
    t_list *listInstrucciones = list_create(); // creo el puntero a la lista
    char path_1[] = "/home/utnso/tp-2024-1c-PentaCoders/memoria/pruebas/";
    strcat(path_1, pathArch);
    log_info(logger, "El path del archivo es : %s", path_1);
    FILE *arch = fopen(path_1, "r");

    if (arch == NULL)
    {
        perror("Error en abrir el archivo"); // me fijo si se pudo abrir el archivo
    }
    int largo=50;
    char linea[50]; // este seria el buffer para ir leyendo el archivo
    while (fgets(linea, largo, arch))
    { // voy leyendo el archivo
        strtok(linea, "\n");
        t_instruccion *unaInstruccion = crearInstruccion(linea);
        log_info(logger, "LInea %s", linea);
        list_add(listInstrucciones, unaInstruccion); // agrego unaInstruccion a la lista
    }
    fclose(arch);
    return listInstrucciones;
}

/* t_list* parsearArchivo(char* pathArchivo, t_log* logger){
    t_list* listaInstrucciones = list_create();

    FILE* archivoInstrucciones = fopen(pathArchivo, "r");
    int length = 50;

    // Aca guarda un renglon del .txt
    char instruccion[50];

    // Aca guarda los parametros recibidos
    char* parametro;

    t_instruccion* instr;

    while(fgets(instruccion, length, archivoInstrucciones)){
        strtok(instruccion, "\n");

        instr = malloc(sizeof(t_instruccion));
        instr->par1 = NULL;
        instr->par2 = NULL;
        instr->par3 = NULL;

        parametro = strtok(instruccion, " ");

        instr->codigo = obtenerCodigoInstruccion(parametro);

        int indice = 1;

        parametro = strtok(NULL, " ");
        while(parametro != NULL){
            escribirCharParametroInstruccion(indice, instr, parametro);
            indice++;
            parametro = strtok(NULL, " ");
        }

        list_add(listaInstrucciones, instr);
    }
    fclose(archivoInstrucciones);
    return listaInstrucciones;
}*/
struct config_memoria *config_memoria()
{
    struct config_memoria *valores_config = malloc(sizeof(struct config_memoria));

    // creo el config
    valores_config->config = iniciar_config("memoria.config");

    valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP");
    valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA");

    return valores_config;
}