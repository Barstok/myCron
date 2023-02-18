mycron: main.c mycron.c mycron.h
	gcc -o build/mycron main.c mycron.c -pthread -lrt -I.
