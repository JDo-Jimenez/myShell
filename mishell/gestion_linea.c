/***************************************************/
/*						   */
/* Alumno login y dni:				   */
/* juan domingo jimenez jerez    juajimje    74519973	   */
/*						   */
/***************************************************/



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "linea.h"



char internal_buffer[MAX_LINEA_ENTRADA];
char *buffer = internal_buffer;


void init(char *buffer, t_linea* estructura) {
    buffer[0] = '\0';
    estructura->n_ordenes = 0;
    estructura->plano2 = 0;
    int i = 0;
    for (i = 0; i < MAX_ORDENES_LINEA && estructura->ordenes[i] != NULL; i++) {
        t_orden *orden = estructura->ordenes[i];
        free((void*)(orden->argumentos_desde_0));
        free((void*)orden);
        estructura->ordenes[i] = NULL;
    }
}

void create_estructura(t_linea* estructura) {
    int i;
    //t_orden **p= (t_orden**)malloc(MAX_ORDENES_LINEA * sizeof (t_orden*));
    t_orden **p= (t_orden**)calloc(MAX_ORDENES_LINEA, sizeof (t_orden*));
    estructura->ordenes = p;
    for (i = 0; i < MAX_ORDENES_LINEA; i++) {
        estructura->ordenes[i] = NULL;
    }
}

extern int errno;

int leer_linea_ordenes(int desc_IN, char *buffer) {
  int n,i;
  // char *p = buffer;
  for (i= 0; i < MAX_LINEA_ENTRADA; i++) {
    n = read(desc_IN, buffer + i, 1);
    if (n < 0){
      if(errno==EINTR){
        buffer[i]=0;
        return i+1;
      }
      perror("en leer_linea_ordenes:");
      return n;
    }else if (n == 0) {
      buffer[i] = 0;
      return i;
    }else if( buffer[i] == FIN_LINEA){
      buffer[i] = 0;
      return i+1;
    }
  }
  fprintf(stderr, "ERROR l�nea demasiado larga\n");
  fflush(stderr);
  return -1;
}

char *siguiente_pal(char *pos) {
    while ((*pos) && (*pos == SEPARADOR)) {
        pos++;
    }
    return pos;
}

void eliminar_ESCAPE(char* buff) {
    char *p;
    do{
      p=buff;
      buff++;
      *p=*buff;
    }while(*buff);
}

char *salta_palabra(char *pos) {
    int escape_encontrado=0;
    while (*pos) {
        switch (*pos) {
            case ESCAPE: //este caracter habría que elminarlo con los siguentes
		if(escape_encontrado==1){			
			escape_encontrado=0;
			pos++;
		}else{
			eliminar_ESCAPE(pos);
			escape_encontrado=1;
		}		
                break;
            case SEPARADOR:
            case TUBO:
            case PLANO2:
            case REDIR_ENT:
            case REDIR_SAL:
		return pos;
                break;
            default:
                pos++;
        }
    }
    return pos;
}

int esControl(char p) {
    if (p == FIN_CADENA || p == TUBO || p == PLANO2 || p == REDIR_ENT || p == REDIR_SAL)
        return 1;
    return 0;
}

int lee_orden(char *buffer, t_linea *linea_de_ordenes, int idx, int *nbytes) {
    char *origen = buffer,*t1;
    int redir_hecha = 0,j,len;
    buffer = siguiente_pal(buffer);
    if (!(*buffer)) return 0; // no había ninguna orden
    if (esControl(*buffer) || strncmp(buffer,"2>",2)==0) {
        fprintf(stderr, "Error en formato de orden\n");
        return -1;
    }
    //al menos una palabra valida
    int n_elem = 0;

    //inicialización de la orden
    linea_de_ordenes->ordenes[idx] = (t_orden*)malloc(sizeof (t_orden));
    t_orden *la_orden = linea_de_ordenes->ordenes[idx];
    if (la_orden == NULL) {
        fprintf(stderr, "Error reservando memoria\n");
        return -1;
    }
    la_orden->fichero_entrada = NULL;
    la_orden->fichero_err = NULL;
    la_orden->fichero_salida = NULL;
    la_orden->salida_append=0;
    la_orden->error_append=0;
    la_orden->fdin = -1;
    la_orden->fdout = -2;
    len=strlen(buffer)/2; if(len<3)len=3;
    la_orden->argumentos_desde_0 = malloc(sizeof(char*) * len);
    la_orden->argumentos_desde_0[0] = buffer;
    for(j=1;j<len;j++){
	 la_orden->argumentos_desde_0[j] = NULL;
    }
    n_elem++;
    buffer = salta_palabra(buffer);
    while (*buffer) {
        switch (*buffer) {
            case ESCAPE: //este caracter habría que elminarlo con los siguentes
                eliminar_ESCAPE(buffer);
                buffer++; // el siguiente caracter no es de control
                break;
            case SEPARADOR:
                t1 = buffer;
                buffer = siguiente_pal(buffer);
                *t1 = 0;
                if (strncmp(buffer, "2>", 2) == 0) {
                    // hay redirección de salida de error
                    if (la_orden->fichero_err != NULL) {
                        fprintf(stderr, "Formato de orden no valido\n");
                        fflush(stderr);
                        return -1;
                    }
                    redir_hecha=1;
                    if (buffer[2] == REDIR_SAL) {
                        // estamos en un append
                        buffer++;
                        la_orden->error_append = 1;
                    }
                    buffer = siguiente_pal(buffer + 2); //saltamos la cadena 2>

                    if (esControl(*buffer)) {
                        fprintf(stderr, "Formato de orden no valido\n");
                        fflush(stderr);
                        return -1;
                    } else {
                        if (*buffer == ESCAPE) {
                            buffer++;
                        }
                        la_orden->fichero_err = buffer;
                        buffer = salta_palabra(buffer+1);
                    }
                } else if (!esControl(*buffer)) {
                    //otra parámetro de la orden si no
                    if (redir_hecha) {
                        // en caso de haber hecho redireccion no podemos
                        // encontrar otro parámetro
                        fprintf(stderr, "Formato de orden no valido\n");
                        fflush(stderr);
                        return -1;
                    }
                    la_orden->argumentos_desde_0[n_elem] = buffer;
                    n_elem++;
                    buffer = salta_palabra(buffer);
                    //único lugar para comprobar redir Error
                }
                break;
            case TUBO:
                // hemos acabado de leer la orden -> ponemos un NULL por si
                // acaso y salimos de la función
                *buffer = 0;
                buffer++;
                *nbytes = buffer - origen;
                return n_elem;
                break;
            case PLANO2:
                linea_de_ordenes->plano2 = 1;
                buffer++;
                buffer = siguiente_pal(buffer);
                if (*buffer) {
                    //este caracter debería ser el último de la línea-> error
                    fprintf(stderr, "Caracter & no es el último de la línea\n");
                    fflush(stderr);
                    return -1;
                }
                break;
            case REDIR_ENT:
                if (la_orden->fichero_entrada != NULL) {
                    fprintf(stderr, "Error formato de orden: 2 red. entrada\n");
                    return -1;
                }
                *buffer = 0;
                // comprobamos si el siguiente es de control
                buffer++;
                buffer = siguiente_pal(buffer);
                if (esControl(*buffer)) {
                    fprintf(stderr,
                            "Error formato de orden: caracter no esperado\n");
                    return -1;
                }
                la_orden->fichero_entrada = buffer;
                buffer = salta_palabra(buffer);
                redir_hecha = 1;
                break;
            case REDIR_SAL:
                if (la_orden->fichero_salida != NULL) {
                    fprintf(stderr, "Error formato de orden: 2 red. entrada\n");
                    return -1;
                }
                *buffer = 0;
                buffer++;
                // comprobamos si hay append
                if(*buffer==REDIR_SAL){
                    //es un append
                    la_orden->salida_append=1;
                    buffer++;
                };
                // comprobamos si el siguiente es de control
                buffer = siguiente_pal(buffer);
                if (esControl(*buffer)) {
                    fprintf(stderr,
                            "Error formato de orden: caracter no esperado\n");
                    return -1;
                }
                la_orden->fichero_salida = buffer;
                buffer = salta_palabra(buffer);
                redir_hecha = 1;
 
                redir_hecha = 1;
                break;
        }
    }
    //hemos acabado con la orden y con la línea de órdenes
    *nbytes = buffer - origen;
    return n_elem;
}

int analizar_orden(char *buffer, t_linea *linea_de_ordenes) {
    buffer = siguiente_pal(buffer);
    if (!*buffer) {
        return 0;
    }
    // al menos hay una palabra -> estamos en primera orden
    int i = 0;
    do {
        int n_chars; //número de char consumidos del buffer
        int res = lee_orden(buffer, linea_de_ordenes, i, &n_chars);
        if (res < 0) return res;
        i++;
        buffer += n_chars;
    } while (*buffer);
    linea_de_ordenes->n_ordenes=i;
    return i;
}
