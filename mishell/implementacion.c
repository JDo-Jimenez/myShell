/***************************************************/
/*						   */
/* Alumno login y dni:				   */
/* Juan Domingo Jimenez Jerez    juajimje    74519973	   */
/*						   */
/***************************************************/


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#include "linea.h"




/**************************
 *
 *Bloque para ejecución de órdenes internas
 *
 *************************/
//variables y constantes para simplificar implementación de órdenes internas
const char* nombres_internas[] = {
    "cd",
    "pwd",
    "showpath",
    "clearpath",
    "delpath",
    "addpath",
    "showvar",
    "readvar",
    "quit",
    NULL
};

#define orden_CD        0
#define orden_PWD       1
#define orden_SHOWPATH  2
#define orden_CLEARPATH 3
#define orden_DELPATH   4
#define orden_ADDPATH   5
#define orden_SHOWVAR	6
#define orden_READVAR   7
#define orden_QUIT      8

int que_interna(char* q) {
    int i;
    if (q == NULL) { // la cadena de entrada debe ser válida
        return -1;
    }
    for (i = 0; nombres_internas[i] != NULL; i++) {
        if (strcmp(nombres_internas[i], q) == 0) {
            return i;
        }
    }
    return -1; // no es una orden interna
}

int ejecuta_interna(int idOrden, t_linea *lin, int ii) {
    int  desc_0, desc_1, desc_2, result, redir=0, n;
    char var1[50]; var1[0]=0;
    t_orden *orden = lin->ordenes[ii];
    static char *q;   //puntero auxiliar que utilizaremos para ir actualizando y leyendo PWD
    
    //extern char **environ;
    int setenv(const char *name, const char *value, int overwrite);
    char *getenv(const char *name);
    char *get_current_dir_name(void);
 
    
    if (orden->fichero_entrada!=NULL||orden->fichero_salida!=NULL||orden->fichero_err!=NULL){  //Si se ha hecho redireccion...
	if(lin->plano2==0){                                                                         //No segundo plano
    		result = hacer_redir_IN_OUT_ERR(orden, &desc_0, &desc_1, &desc_2);                       //...Recuperamos
		redir=1;
		if (result < 0) {
			return result;
    		}
	}else{                                                                                      //No segundo plano
		result = hacer_redir_IN_OUT_ERR(orden,NULL,NULL,NULL);                                   //..Recuperamos
		if (result<0){
			return result;
		}
	}
    }

    switch (idOrden) {
        case orden_CD:
		if (orden->argumentos_desde_0[1]==NULL){                    //Si NO hay parametro
		      	q=getenv("HOME");                                          //cambio direcc de trabajo por valor de la variable "HOME"
			result=chdir(q);                        
			if(result!=0){                                              //Error?
				fprintf(stderr,"Error en orden CD\n");
				result=-1;	
			}else{                                                      //No error?
				result=setenv("PWD",getenv("HOME"),1);                   //Actualizamos PWD    
			}
		}else{                                                      //Si hay parametro
			result=chdir(orden->argumentos_desde_0[1]);               //Cambio direcc de trabajo por valor de la direcc del parametro
			if(result!=0){
				fprintf(stderr,"Error en orden CD\n");
				result=-1;	
			}else{	
			       	q=get_current_dir_name();                                //Actualizamos PWD con el valor del direcc actual
	                        result=setenv("PWD",q,1);
				                 
			}
		}
		break;
        case orden_PWD:
		 q=get_current_dir_name();
	        if(q<0){
			perror("Error en orden PWD");
			result=-1;
		}else{   
			printf("%s\n",q);
			result=0;
		}
		break;
	case orden_SHOWPATH:
		{
			q=getenv("PATH");
			if (q<0){
				perror("Error en la adquisición de la variable PATH");
				result=-1;
			}else{
				printf("%s\n",q);
				result=0;
			}
		}
               	break;
        case orden_CLEARPATH:
		result=setenv("PATH","",1);		
		if(result!=0){
			perror("Error vaciando el PATH");
			result=-1;
		}else{
			result=0;
		}		
		break;
        case orden_DELPATH:
		{
		int buscapath(char *path,char *param);
		if (orden->argumentos_desde_0[1]==NULL){
			fprintf(stderr,"Error, no se ha indicado el directorio que quiere borrar del PATH\n");
		}
		else if(orden->argumentos_desde_0[1]!=NULL){
		q=getenv("PATH");
		  if(!buscapath(q,orden->argumentos_desde_0[1]))	fprintf(stderr,"Error:El directorio no existe\n");
		  else{
			int lon=strlen(q);
			char aux[lon+1],aux2[lon];
			aux[0]='\0'; aux2[0]='\0';
			char *car;
			strcat(aux,":");
			strcat(aux,q);
			car=strrchr(aux,':');
			while(car!=NULL){
				if((strcmp(orden->argumentos_desde_0[1],car+1))!=0){
					strcat(aux2,car+1);
					strcat(aux2,":");
					*car='\0';
				}
				else{
					*car='\0';
				}
				car=strrchr(aux,':');
			}
			lon=strlen(aux2);
			aux2[lon-1]='\0';
			strcpy(q,aux2);
		  }
		}
		}		
		break;
	case orden_ADDPATH:
		{
		q=getenv("PATH");
		int buscapath(char *path,char *param);
		if (orden->argumentos_desde_0[1]==NULL){
			fprintf(stderr,"Error, debe indicarse el directorio que se pretende añadir a la variable PATH\n");
			result=-1;
		}
		else if(orden->argumentos_desde_0[1]!=NULL){
		      if (buscapath(q,orden->argumentos_desde_0[1]))  fprintf(stderr,"Error: El directorio ya estaba\n");
		      else{
	                   char *s2="";  
			       if(strcmp( q, s2) == 0)  {                            //Si va a ser la primera direccion no hay que poner ":"
			                  strcpy(q,orden->argumentos_desde_0[1]); 
                                       }
			       else{                                                  //ya hay mas de una direccion--> ponemos ":"
				strcat(q,":");
				strcat(q,orden->argumentos_desde_0[1]);
			     }
		       }
	         } 
		}
		break;
	case orden_SHOWVAR:{
		char *var;
		if (orden->argumentos_desde_0[1]!=NULL){
			int i=1;			
			while ( orden->argumentos_desde_0[i]!=NULL ){
				var=getenv(orden->argumentos_desde_0[i]);
				if(var==NULL){                               //si la variable no existe...
				     printf("variable no encontrada \n");
				     break;
				 }
				else{
				     printf("%s = %s\n",orden->argumentos_desde_0[i],var);
				     i++;
				 }
			}
		}else{    //leer de la entrada estandar
		        printf("***Introduzca el nombre de la variable que desea visualizar***\n***Para salir pulse cualquier telca que no sea una variable***\n");
		        int flag=1;
			while(flag==1){
			      
				n=leer_linea_ordenes(0, var1);
				
				if (n == 0) {		//se ha llegado al final del fichero de entrada por defecto salimos
            				break;
        		       }else if (n < 0) {
            				result=n;
			       }else if (var1 > 0){
					var=getenv(var1);
				        if (var==NULL) {
					     printf("variable no encontrada\n");
					     flag=0;
					}else {
					     printf("=%s\n",var);
					}
				}	
				
			}	
		}
		}
		break;

	case orden_READVAR:
	        if(orden->argumentos_desde_0[2]!=NULL){                 //si hay mas de un parametro
		  printf("Error:Introduzca un unico parametro\n");
		  break;
		}else if(orden->argumentos_desde_0[1]==NULL){           //si no hay ningun parametro
		   printf("Error:Tiene que Introducir un parametro\n");
		}else {                                                  //si hay un único parametro
		    n=leer_linea_ordenes(0, var1);
		    if (n == 0) {
            	        break ;                  //se ha llegado al final del fichero de entrada
            	    } else if (n < 0) {
            		result=n;
		    }else if (var1 > 0){
			result=setenv(orden->argumentos_desde_0[1],var1,1);
		    }
		}
		break;
        case orden_QUIT:
            exit(0);
        default:
            fprintf(stderr, "Orden interna no valida\n");
            fflush(stderr);
            result = -1;
            break;
    }
    if (redir==1){
    	result=quitar_redir_IN_OUT_ERR(desc_0, desc_1, desc_2);
	redir=0;
    }
    
   
    
    return result;
}

 int buscapath(char *path,char *param){      //Funcion auxiliar que busca si el paramentro esta en el PATH
	int lon=strlen(path);                //Esta funcion la utilizaran orden_DELPATH y orden_ADDPATH
	char aux2[lon];
	char *car;
	char aux[60];
	strcpy(aux2,path);
	car=strrchr(aux2,':');
	while(car!=NULL){
		strcpy(aux,car+1);
		if (strcmp(param,aux)==0)	return 1;
		*car='\0';
		car=strrchr(aux2,':');
	}
	if(strcmp(param,aux2)==0)	return 1;
	else 				return 0;
}

/****
 *bloque de ejecución de líneas
 *
 ****/
int ejecuta_orden_simple(t_linea *linea) {

	t_orden *laorden = linea->ordenes[0];
	int result,interna,muerto;
	laorden->fdin = -1;        //No va a haber tubería
	laorden->fdout = -1;
	interna = que_interna(laorden->argumentos_desde_0[0]);   //devuelve orden simple
	if (interna>=0 && linea->plano2==0) {                    //interna, NO segundo plano
		result=ejecuta_interna(interna, linea, 0);
	} else {                                                 //Segundo plano
		int hijo = fork();
		if (hijo==0){              //Ejecuta el hijo
			if (interna<0) {                                  //si NO es interna..
				result=ejecuta_externa(linea, 0);
			} else {                                          //si es interna..
				result=ejecuta_interna(interna, linea, 0);
			}
			exit(result);
		} else if ( linea->plano2==0){  //padre           //NO segundo plano
			int motivo;
			do{
				muerto = wait(&motivo);   
			}while (muerto!=hijo);
			result=motivo;
		}else{
		   result=-1;
		} 
	}
	return result;
	//return ejecuta_orden_simple_profes(linea);
}


int ejecuta_tubos(t_linea *linea) {

	int descr[2];	//Iniciamos los descriptores de las tuberias
	int anterior_out, cont=linea->n_ordenes, i, muerto;
	t_orden *miorden;
	pid_t pid[MAX_ORDENES_LINEA];
	miorden=linea->ordenes[0];

        pipe(descr);        
        pid[0]=fork();     //creo el hijo que será el primer proceso
	if(pid[0]==0){       		                          //Si es el primer proceso
        	miorden->fdin=-1;        	                        //Por ser primera tuberia: no redir de entrada
                miorden->fdout=descr[1];	                        //redir de salida se asocia con la salida de la tuberia
		close(descr[0]);					//Cierro entrada de la tuberia dejando un hueco en la tabla de descrptores
                ejecuta_orden(linea,0);                                 //ejecucion de la orden
	}else{ 							  //Padre
                for(i=1; i<(linea->n_ordenes-1); i++){	               // Todas las ordenes intermedias
                	miorden=linea->ordenes[i];
                	anterior_out=descr[0];                              //salida anterior se asocia con la entrada de la tuberia
                	close(descr[1]);		                    //Cierro la salida de la tuberia dejando un hueco en la tabla de descptores
                	pipe(descr);			                    //Creo la siguiente tubería. descr[0] ocupara el hueco q habia dejado descr[1] anterior
			pid[i]=fork();		                            //Creo el proceso que relacionado con la nueva tubería
			if(pid[i]==0){		                            //Si es un hijo
				miorden->fdin=anterior_out;                        //redir entrada asociada con la entrada de la primera tuberia
				miorden->fdout=descr[1];                           //redir salida  asociada con la salida de la nueva tubería
				close(descr[0]);	                           //Cierro entrada de la nueva tubería
				ejecuta_orden(linea,i);                            //ejecucion de la orden intermedia
			                                       
			}else{                                               //Padre
				close(anterior_out);                               //cierra la entrada de la primera tubería
			}
		}
	}
	pid[linea->n_ordenes-1]=fork();	                             //Ultima orden
	if(pid[linea->n_ordenes-1]==0){ 
		miorden=linea->ordenes[linea->n_ordenes-1];
		miorden->fdin=descr[0];
		miorden->fdout=-1;                                   //Por ser ultima tuberia: no redir de salida
		close(descr[1]);		//Cierro salida de la tuberia
		ejecuta_orden(linea,linea->n_ordenes-1);
	}else{
		close(descr[0]);              //padre cierra la tuberia
		close(descr[1]);
		
	}

	if (linea->plano2!=1 && pid[0]!=0){		//Si no estamos en segundo plano
   		while(cont > 0){                        //EL padre espera la muerte de todos sus hijos
                        int motivo;
			muerto=wait(&motivo);
			for(i=0; i<linea->n_ordenes;i++){
				if (muerto==pid[i]) cont--;
			}
		}
	}
	
	return 0;

    //return ejecuta_tubos_profes(linea);
}


/***
 *funciones de ejecución sencillas
 *
 */

//Ejecuta una orden en una línea con tuberías
//la orden puede ser interna o externa
int ejecuta_orden(t_linea *linea, int i) {
    

	t_orden *laorden = linea->ordenes[i];
	int result,orden;	
		
	if (laorden->fdin >= 0){                 //Fdin fue rellenado en ejecuta_tubos()
		result=dup2(laorden->fdin,0);         //entrada estandar es referida al descrip para redir entrada
		if (result < 0) fprintf(stderr,"Error al redirecionar tubería de la orden %s (fdin)\n",laorden->argumentos_desde_0[i]);
	}

	if (laorden->fdout >= 0){               //Fdout fue rellenado en ejecuta_tubos()
		result=dup2(laorden->fdout,1);       //salida estandar es referida al descrip para redir salida
		if (result < 0) fprintf(stderr,"Error al redirecionar tubería de la orden %s (fdout)\n",laorden->argumentos_desde_0[i]);
	}

	orden = que_interna(laorden->argumentos_desde_0[i]); //buscamos que orden hay que ejecutar
	if (orden>=0) {                                            //INTERNA?
		result=ejecuta_interna(orden, linea,i);                 //ejecutamos
	}else{                                                       //EXTERNA?
		result=ejecuta_externa(linea, i);                         //ejecutamos
	}

	return result;
	

	//return ejecuta_orden_profes(linea,0);
    }

// Ejecución de una orden externa, tiene que llamar a redirecciones
// buscar en el path y hacer el exec... se supone que el fork ya
// se ha hecho
int ejecuta_externa(t_linea *lin, int i) {
	t_orden *porden= lin->ordenes[i];
	int result=0;
	if (porden->fichero_entrada!=NULL||porden->fichero_salida!=NULL||porden->fichero_err!=NULL){  //Si se ha redireccionado la orden
 		result = hacer_redir_IN_OUT_ERR(porden,NULL,NULL,NULL);                                      //recuperamos
		if (result!=0){
			perror("Error al hacer la redirección E/S");
			return -1;		
		}
	}
	
	char *orden=porden->argumentos_desde_0[0];
	char *car=strchr(orden,'/');
	if (car!=NULL){   // si se introduce una ruta...
		porden->aEjecutar=porden->argumentos_desde_0[i]; 	// ...se ejecuta el fichero indicado por el usuario
	} else {                                             // Buscar orden el el path
	 	char orden_probar[100];
		char* orden=porden->argumentos_desde_0[0];
        	char* path=getenv("PATH");
        	char* copia_path=malloc(strlen(path)+1);
        	strcpy(copia_path,path);            //copia de path
        	int encontrado=0;
        	char* dir=strtok(copia_path,":");   //descomponemos en tokens(direcciones)
        	while (dir!=NULL) {                 //comprobamos token (direccion)
           		strcpy(orden_probar,dir);                  //almacenamos token en el dirrectorio a probar
			strcat(orden_probar,"/");
           		strcat(orden_probar, orden);               //añadimos al directorio una barra "/" y la "orden"--> dir/orden
           		if( access(orden_probar, F_OK)==0){        //Existe ?
                		encontrado=1;
				porden->aEjecutar=orden_probar;
                		break;
           		}
           	dir=strtok(NULL,":");
        	}
        	if(encontrado!=1){
                	fprintf(stderr,"orden (%s) no encontrada en el path\n",orden);
                	return -1;
        	}
       	}
	result=execve(porden->aEjecutar, porden->argumentos_desde_0, environ);
	exit(1);
    //return ejecuta_externa_profes(lin, i);
}

// redireccion de entrada y salida
// si algunos de los puntos d_X no es NULL se debe hacer en el una copia
// del descriptor para poder deshacer la redirección más adelante
int hacer_redir_IN_OUT_ERR(t_orden *laorden, int *d_0, int *d_1, int *d_2) {

    int result = 0,entrada,salida,error;
    fprintf(stderr,"%s\n",laorden->argumentos_desde_0[0]);
	
	if(laorden->fichero_entrada != NULL) {                                                    //entrada  redireccionada
		if(d_0!=NULL)*d_0 = dup(0);                                                           //si no es null hacemos copia para poder deshacer luego 
		entrada=open(laorden->fichero_entrada,O_RDONLY);                                      //habro en modo lectura el fichero apuntado por fichero_entrada
		if (entrada!=-1){                           					  //SI no hay error
			result=dup2(entrada,0);                                                       //descriptor del fichero asignado a entrada estandar
			if (result==-1){
				perror("Error al redireccionar la entrada estándar");
				return -1;
			}
			close(entrada);
		}else{
			fprintf(stderr,"Error al abrir el fichero %s\n",laorden->fichero_entrada);
			return -1;
		}
	}
	result=0;
	if(laorden->fichero_salida != NULL) {							  //salida  redireccionada
		if(d_1!=NULL) *d_1 = dup(1);							    //si no es null hacemos copia para poder deshacer luego 
		if(laorden->salida_append == 1) {		                                    //si no se quiere truncar fich destino
			salida=open(laorden->fichero_salida,O_WRONLY | O_APPEND | O_CREAT,0666);        //solo W, modo apendice, crea fichero (si existia escribe encima)
		} else {									    //si se quiere truncar fich destino
			salida=open(laorden->fichero_salida,O_WRONLY | O_TRUNC | O_CREAT,0666);          //truca
		}
		if (salida!=-1){                                                                   //si no hay error
			result=dup2(salida,1);                                                           //descriptor del fichero asignado a salida estandar 
			if (result==-1){								
				perror("Error al redireccionar la salida estándar");
				return -1;
			}
			close(salida);                                                                   //si todo va bien cerramos
		}else{
			fprintf(stderr,"Error al abrir el fichero %s\n",laorden->fichero_salida);
			return -1;
		}
	}
	result=0;

	if(laorden->fichero_err != NULL) {                                                       //salida de error  redireccionada
		if(d_2!=NULL)*d_2 = dup(2);                                                      //Hace lo mismo que salida 
		if(laorden->error_append == 1) { 
			error=open(laorden->fichero_err,O_WRONLY | O_APPEND | O_CREAT,0666);
		} else {
			error=open(laorden->fichero_err,O_WRONLY | O_TRUNC | O_CREAT,0666);
		}
		if (error!=-1){
			result=dup2(error,2);
			if (result==-1){
				perror("Error al redireccionar la salida de error estándar");
				return -1;
			}
			close(error);
		}else{
			fprintf(stderr,"Error al abrir el fichero %s\n",laorden->fichero_err);
			return -1;
		}
	}	
 
	return 0;

    //return hacer_redirProfes(laorden);
}


// sólo se quitan redirecciones si tenemos duplicados de descriptores válidos
int quitar_redir_IN_OUT_ERR(int d_0, int d_1, int d_2) {	
	int result;   
    	if (d_0 >= 0) {
		result=dup2(d_0,0);
		if (result<0){
			perror("Fallo al deshacer redirección de entrada");
			return -1;
		}
		close(d_0);
    	}
    	if (d_1 >= 0) {
		result=dup2(d_1,1);
		if (result<0){
			perror("Fallo al deshacer redirección de salida");
			return -1;
		}
		close(d_1);
    	}
    	if (d_2 >= 0) {
		result=dup2(d_2,2);
		if (result<0){
			perror("Fallo al deshacer redirección de salidad de error");
			return -1;
		}
		close(d_2);
    	}
		
	
	return result;
    	//return quitar_redirProfes(d_0, d_1, d_2);
}




