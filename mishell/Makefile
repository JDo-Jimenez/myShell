CC:=gcc
CFLAGS:= -g -Wall
MODULOS=mishell.o implementacion.o gestion_linea.o 

.PHONY:clean

all : mishell 

%.o:%.c
	$(CC) $(CFLAGS) -c $< 

mishell:${MODULOS}
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f mishell $(MODULOS)
