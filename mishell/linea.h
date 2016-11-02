/* 
 * File:   linea.h
 * Author: mimateo
 *
 * Created on 22 de febrero de 2010, 12:30
 *
 * Definiciones de estructuras de datos necesarias para ejecución.
 *
 */

#ifndef _LINEA_H
#define	_LINEA_H

#ifdef	__cplusplus
extern "C" {
#endif

    //Lo usaremos en más de un sitio
    extern char **environ;

    // tamanys maximos
    #define MAX_LINEA_ENTRADA 1024
    #define MAX_ORDENES_LINEA 100
 

    //caracteres especiales
    #define     FIN_LINEA   '\n'
    #define     ESCAPE      '\\'
    #define     TUBO        '|'
    #define     SEPARADOR   ' '
    #define     PLANO2      '&'
    #define     REDIR_SAL   '>'
    #define     REDIR_ENT   '<'
    #define     FIN_CADENA  '\0'
    #define     PATH_SEPA   '/'

    // ESTRUCTURAS DE DATOS

    typedef struct orden_struct{
        // que orden y que parametros
        char    *   aEjecutar;
        char    **   argumentos_desde_0;
        // redireccion de tuberias
        int     fdin;  //descriptor para redireccionar entrada (-1: no redir)
        int     fdout; //descriptor para redireccionar salida (-1: no redir)
        //redireccion de ficheros... con y sin append
        char  * fichero_entrada;
        char  * fichero_salida;
        char  * fichero_err;
        int     salida_append;
        int     error_append;
    } t_orden;

    typedef struct linea_struct{
        int     plano2;
        int     n_ordenes;
        t_orden **ordenes;
    } t_linea;

    
    // Base de ejecución
    int ejecuta_tubos(t_linea *linea);
    int ejecuta_orden_simple(t_linea *linea); // no hay tubos

    //decide qué ejecutar y hace redirecciones
    int ejecuta_orden(t_linea *lin,int i);
    
    //ejecución real de las órdenes
    int ejecuta_interna(int idInterna,t_linea *lin,int i);
    int ejecuta_externa(t_linea *lin,int i);

    // devuelve un entero que índica que orden interna se ha de ejecutar
    int que_interna(char*);

    // hace las redirecciones pertinentes de la orden (ficheros) y devuelve en
    // los parámetros los antiguos descriptores si los punteros pasados no
    // fueran NULL
    int hacer_redir_IN_OUT_ERR(t_orden *laorden,int *d_0, int *d_1, int *d_2);

    // quita las redirecciones realizadas anteriormente.
    // Sólo utilizará los parámetros que no sean -1
    // Cuando salga sólo se estarán usando los descriptores mínimos posibles.
    int quitar_redir_IN_OUT_ERR(int d_0, int d_1, int d_2);

    int ejecuta_tubos_profes(t_linea *linea);
    int ejecuta_orden_profes(t_linea *lin,int i);
    int ejecuta_orden_simple_profes(t_linea *linea);
    int ejecuta_externa_profes(t_linea *lin,int i);

    int hacer_redirProfes(t_orden*);
    int quitar_redirProfes(int d_0,int d_1, int d_2);


    // funcion para inicializar las estructuras de datos
    void init(char *buffer,t_linea* estructura);
    void create_estructura(t_linea* estructura);

    //lee una línea de órdenes del descriptor indicado
    int leer_linea_ordenes(int desc_IN,char *buffer);
    int analizar_orden(char *buffer,t_linea *linea_de_ordenes);

#ifdef	__cplusplus
}
#endif

#endif	/* _LINEA_H */

