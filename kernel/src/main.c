#include <stdlib.h>
#include <stdio.h>
#include <utils.h>
#include </home/utnso/tp-2024-1c-PentaCoders/kernel/include/kernel.h>

int main(int argc, char* argv[]) {
    //PROBAMOS AL KERNEL COMO SERVIDOR Y AL DISP I/O COMO CLIENTE
    logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

// Va a ser cliente y servidor ya que envia procesos a la cpu y pide cosas a memoria y recibe las interrupciones de los disp de I/O, respectivamente.
