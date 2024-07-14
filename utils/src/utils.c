#include "../include/utils.h"

t_log *iniciar_logger(char *path_log, char *nombre_log)
{
	t_log *nuevo_logger;
	nuevo_logger = log_create(path_log, nombre_log, 1, LOG_LEVEL_INFO);
	if (nuevo_logger == NULL)
	{
		return NULL;
	};
	return nuevo_logger;
}

t_config *iniciar_config(char *config_path)
{
	t_config *nuevo_config;
	nuevo_config = config_create(config_path);
	if (nuevo_config == NULL)
	{
		log_error(logger, "No se pudo leer la config");
		exit(2);
	}
	return nuevo_config;
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	destruirLog(logger);
	destruirConfig(config);
	liberar_conexion(conexion);
}

void destruirLog(t_log *logger)
{
	if (logger != NULL)
	{
		log_destroy(logger);
	}
}

void destruirConfig(t_config *config)
{
	if (config != NULL)
	{
		config_destroy(config);
	}
}

void liberarConexion(int conexion)
{
	if (conexion != 0)
	{
		liberar_conexion(conexion);
	}
}

void sleep_ms(int milliseconds)
{
	usleep(milliseconds * 1000);
}

void enviar_op_code(int socket_servidor, uint32_t cod)
{
	send(socket_servidor, &cod, sizeof(uint32_t), 0);
}

op_code recibir_op_code(int socket_cliente)
{
	op_code cod_op;
	while (1)
	{
		if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0)
			return cod_op;
		else
		{
			close(socket_cliente);
			return -1;
		}
	}
}