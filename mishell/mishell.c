/***************************************************/
/*  				                   */
/* Alumno login y dni:			           */
/* Juan Domingo Jimenez Jerez   juajimje   74519973	   */
/*			                           */
/***************************************************/
	
#include <malloc.h> 

#include <unistd.h>
#include <stdlib.h>

#include "linea.h"

/*
 * Programa principal del shell
 *
 */
int main(int argc, char** argv) {
    int desc_IN = 0, n;
    t_linea linea_de_ordenes;
    create_estructura(&linea_de_ordenes);
    char buffer[ MAX_LINEA_ENTRADA ];
    while (1) {
		//inicializacion de estructuras internas
        init(buffer, &linea_de_ordenes);	
        write(1,"mishell$ ",9);
	fsync(1);
        n = leer_linea_ordenes(desc_IN, buffer);
        if (n == 0) {
            //se ha llegado al final del fichero de entrada
            //por defecto salimos
            return (EXIT_SUCCESS);
        } else if (n < 0) {
            return (n);
        }else if (analizar_orden(buffer, &linea_de_ordenes) > 0) {
            if (linea_de_ordenes.n_ordenes == 1) {
                //No tenemos tuberías... situación más sencilla de ejecución
                ejecuta_orden_simple(&linea_de_ordenes);
            } else {
                //como hay tuberías tendremos un hijo por cada orden
                //sea interna o externa.
                ejecuta_tubos(&linea_de_ordenes);
            }
        }
    }
    return (EXIT_SUCCESS);
}

