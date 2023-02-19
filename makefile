mycron: main.c mycron.c mycron_service.c mycron_client.c mycron.h 
	gcc -o build/mycron main.c mycron_service.c mycron_client.c mycron.c ../LoggerLib/logger.c -pthread -lrt -I.
