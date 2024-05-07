#include "../include/memoria.h"
// CRAER HILOS COMO VARIABLES GLOBALES
pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;

int main(int argc, char *argv[])
{

    logger = iniciar_logger("memoria.log", "MEMORIA");
    struct config_memoria *valores_config = config_memoria();

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
    // pthread_create(&hiloCpu, NULL, recibirCPU, (void *)args_CPU);
    pthread_create(&hiloKernel, NULL, recibirKernel, (void *)args_KERNEL);
    // pthread_create(&hiloIO, NULL, recibirIO, (void *)args_IO);
}
void *recibirIO(void *ptr)
{
    return EXIT_SUCCESS;
}

void *recibirKernel(void *ptr)
{
    t_args *argumento = malloc(sizeof(t_args));
    argumento = (t_args *)ptr;

    int server_fd = iniciar_servidor(logger, "Memoria", argumento->ip, argumento->puerto);
    log_info(logger, "Servidor Memoria listo para recibir KERNEL");
    int cliente_fd = esperar_cliente(logger, "Memoria", server_fd);
    t_list *lista = list_create();
    while (1)
    {
        // recibimos paquete proveniente del kernel
        // lista = recibir_paquete_cde(cliente_fd);
        // obtenemos el codigo de operacion
        op_code cod_op = recibir_operacion(cliente_fd);
        log_info(logger, "%d", cod_op);
        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            log_info(logger, "Me llego la Solicitud de Iniciar Proceso");
            
            iniciar_proceso();
            break;

        case SOLICITUD_FINALIZAR_PROCESO:
            log_info(logger, "Se aprueba finalizar el proceso");
            finalizar_proceso();
        case -1:
            log_error(logger, "El cliente se desconecto. Terminando servidor");
            // return (void *)EXIT_FAILURE;
            return EXIT_FAILURE; // version de la catedra, pero da un warning si no anda comentar el de arriba
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
    }
    return EXIT_SUCCESS;
}
/*
t_cde * leer_paquete_cte(){

}*/
void iniciar_proceso()
{
    // log_info(argumento->logger, "Me llego la Solicitud de Iniciar Proceso");
    // lista = recibir_paquete(cliente_fd);

    // t_cde *cde = leer_paquete_cte(lista);
    //  tipo_buffer *buffer = recibir_buffer_propio(sizeof(uint32_t), cliente_fd);

    // t_cde *contexto_ejecucion = leer_buffer_cde(buffer); DESEREALIZAR FALTA IMPLEMENTAR

    // t_list *listInstrucciones = leerArchivoConInstrucciones(char *pathArch);

    // agregar_Instrucciones_Al_CDE(listInstrucciones, buffer);
    //  agregamos insutrcciones al cde, identificamos al cde usando su id
    // log_info(argumento->logger, "La Solicitud de Iniciar Proceso se Realizo con Exito");
}

void finalizar_proceso()
{
}
void *recibirCPU(void *ptr)
{
    t_args *argumento; // CASTEAR
    argumento = (t_args *)ptr;
    int server_fd = iniciar_servidor(argumento->logger, "Memoria", argumento->ip, argumento->puerto);
    log_info(argumento->logger, "Servidor Memoria listo para recibir CPU");
    int cliente_fd = esperar_cliente(argumento->logger, "Memoria", server_fd);
    t_list *lista;
    while (1)
    {
        int cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case MENSAJE:
            recibir_mensaje(cliente_fd);
            break;
        case PAQUETE:
            lista = recibir_paquete(cliente_fd);
            log_info(argumento->logger, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void *)iterator);
            break;
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
    FILE *arch = fopen(pathArch, "r");

    if (arch == NULL)
    {
        perror("Error en abrir el archivo"); // me fijo si se pudo abrir el archivo
    }
    char *linea = malloc(sizeof(char *)); // este seria el buffer para ir leyendo el archivo
    while (fgets(linea, sizeof(linea), arch))
    { // voy leyendo el archivo
        strtok(linea, "\n");
        t_instruccion *unaInstruccion = crearInstruccion(linea);
        list_add(listInstrucciones, unaInstruccion); // agrego unaInstruccion a la lista
    }
    fclose(arch);
    return listInstrucciones;
}

/*
t_list* parsearArchivo(char* pathArchivo, t_log* logger){
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