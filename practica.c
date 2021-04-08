#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>




int main(int argc, char** argv) {																		
	// DECLARACIONES DE VARIABLES
	int id, i ;			
	int numeroProcesos;// Nº de procesos
	int etiquetaInferior = 1, etiquetaSuperior = 2, etiquetaNumeroP = 3, etiquetaDivisor = 4, etiquetaSumaAcumulada=5, etiquetaSumaAnterior=6, etiquetaTiempoUtilizado =7, etiquetaAuxiliar=8, etiquetaFinal=9;		// Etiquetas para el paso de mensajes
	unsigned long long int  limiteI = 0 , limiteS= 0;// Limite a calcular
	unsigned long long int  numeroP = atoll(argv[1]); //numero a calcular
	int recepcion; //si ha llegado un mensaje 												
	MPI_Status status;
	MPI_Request peti;																				
	double  tiempoInicial,tiempoTotal;//variables para realizar el calculo del tiempo 
	unsigned long long int  numeroCalcular= 0; //variable utilizada en el algoritmo del calculo de número perfecto
	double tiempoEmpleado;// tiempo final empleado en el proceso
	unsigned long long int  sumaAnterior= 0;//variable usada en el calculo de la suma acumulada
	unsigned long long int  temp= 0;
	unsigned long long int  acum = 0;//variable usasa en el calculo de la suma acumulada
	unsigned long long int  mitad = numeroP/2;//mitad del numero a calcular para poder realizar el calculo del algoritmo
	unsigned long long int  resta= 0;//variable empleada en el calculo de la resta
	double totalTiempo = 0;

	

	/******************************************************************************/
	/*-----------------------EMPIEZA EL PROGRAMA----------------------------------*/
	/******************************************************************************/

	

	
									
	MPI_Init(&argc, &argv); 							// Inicializamos el entorno de ejecución MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &id); 				// Almacenamos el identificador del proceso
	MPI_Comm_size(MPI_COMM_WORLD, &numeroProcesos);     // Almacenamos el nº de procesos
	tiempoInicial = MPI_Wtime();						// Tiempo inicial del programa
																											

	double tiempoCalculoProcesos[numeroProcesos];//vector que contiene los tiempos empleados por cada proceso		

	numeroProcesos = numeroProcesos-1;  //ya que el proceso 0 no realiza calculos							
	

	if(argc<2 || numeroP<=0){ //comprobamos que los argumentos que se pasan son correctos 

		if(id == 0){

			printf("ERROR, tiene que haber 2 argumentos y el numero de procesos tiene que ser mayor que 0");

		}

	}else if(numeroProcesos>1){//Si el numero de procesos es mayor que 2 se reparte el numero a calcular entre todos los demas procesos


		if(id == 0){

			/******************************************************************************/
			/*-----------------------ID -> 0----------------------------------------------*/
			/******************************************************************************/

			unsigned long long int limite = numeroP/2;//limite a calcular
			int resto = numeroP%2;//resto
			unsigned long long int rangos = limite / numeroProcesos;//rango de numeros a calcular por cada proceso
			unsigned long long int limiteInferior=1;//limite inferior desde el cual se parte a calcular
			unsigned long long int sumaProcesoCero = 0;
			unsigned long long int limiteSuperior = 0;//limite supeiror hasta donde se debe calcular
			unsigned long long int divisor= 0;// variable para almacebar los divisores
			unsigned long long int sumaAcumulada= 0;//variable para guardar la suma acumulada
			unsigned long long int sumaTotal= 0;//suma acumulada final
			unsigned long long int auxiliar= 0;
			unsigned long long int totalDivisores = 0;//numero total de divisores del numero
			bool nosSalimos = false;//flag para salir del bucle

			unsigned long long int vectorSumaAcumulada[numeroProcesos];//vector que guarda la sumua acumulada de cada proceso
			int numeroDisisores[numeroProcesos];//aquí guardamos el numero de divisores de cada proceso

			for(int u=0; u<=numeroProcesos;u++){ //iniciamos los vectores a 0 para evitar errores
				numeroDisisores[u]=0;
				vectorSumaAcumulada[u]=0;
				tiempoCalculoProcesos[u]=0;
			}

			for (int i = 1;i<=numeroProcesos;i++){ //calculamos los limites para cada proceso
				
				if(i==numeroProcesos ){
					
					limiteSuperior = limite;

				}else {

					limiteSuperior = limiteInferior+rangos;

				}

				
				MPI_Send(&numeroP, 1, MPI_UNSIGNED_LONG, i, etiquetaNumeroP, MPI_COMM_WORLD); 		//el proceso 0 envia el numero para calcular si es perfecto o no 
				MPI_Send(&limiteInferior, 1, MPI_UNSIGNED_LONG, i, etiquetaInferior, MPI_COMM_WORLD); //enviamos el limite inferior
				MPI_Send(&limiteSuperior, 1, MPI_UNSIGNED_LONG, i, etiquetaSuperior, MPI_COMM_WORLD); //enviamos el limite superior

				limiteInferior = limiteSuperior+1;	
			}

			printf("-----------------------------------------------------------------------------------------------------------------------------------\n");


			while(nosSalimos==false){ //bucle infinito para recibir los divisores, el tiempo y la suma acumulada. Acaba cuando el ultimo proceso le envia la suma acumulada

				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recepcion, &status);
				if(recepcion == 1){//indica si llega un mensaje

					if(status.MPI_TAG == etiquetaDivisor) //le llegan divisores
					{
						MPI_Irecv(&divisor, 1, MPI_UNSIGNED_LONG, MPI_ANY_SOURCE, etiquetaDivisor, MPI_COMM_WORLD,&peti);
						MPI_Wait(&peti,&status);

						numeroDisisores[status.MPI_SOURCE]=numeroDisisores[status.MPI_SOURCE]+1;
						vectorSumaAcumulada[status.MPI_SOURCE] = vectorSumaAcumulada[status.MPI_SOURCE] + divisor;
						resta = numeroP-divisor;

						printf("DIV: %15d, DIV RECV: %15lld, DIV ACU: %15d, SUMA ACU: %15lld (%lld)\n",status.MPI_SOURCE,divisor,numeroDisisores[status.MPI_SOURCE],vectorSumaAcumulada[status.MPI_SOURCE],resta);

					}else if(status.MPI_TAG == etiquetaSumaAcumulada){ //llega la suma acumulada de cada proceso

						MPI_Irecv(&sumaAcumulada, 1, MPI_UNSIGNED_LONG, MPI_ANY_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD, &peti);
						MPI_Wait(&peti,&status);

						if(sumaAcumulada == vectorSumaAcumulada[status.MPI_SOURCE]){
							int correcto = 1;
							MPI_Isend(&correcto, 1, MPI_INT, status.MPI_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD,&peti);
							printf("FIN:\t %15d, SUMA RECV:\t\t %lld, SUMA ACU:\t\t %lld, SUMA ACU OK\n",status.MPI_SOURCE,sumaAcumulada,vectorSumaAcumulada[status.MPI_SOURCE]);

						}else{
							int correcto = 0;
							MPI_Isend(&correcto, 1, MPI_INT, status.MPI_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD,&peti);

						}


					}else if(status.MPI_TAG == etiquetaTiempoUtilizado) //llega el tiempo utilizado de cada proceso
					{
						
						MPI_Irecv(&tiempoEmpleado, 1, MPI_DOUBLE, MPI_ANY_SOURCE, etiquetaTiempoUtilizado, MPI_COMM_WORLD, &peti);
						MPI_Wait(&peti,&status);
						tiempoCalculoProcesos[status.MPI_SOURCE]=tiempoEmpleado;

					}else if(status.MPI_TAG == etiquetaSumaAnterior) //llega la suma total calculada por el ultimo proceso, fin del bucle
					{
						MPI_Recv(&sumaTotal, 1, MPI_UNSIGNED_LONG, numeroProcesos, etiquetaSumaAnterior, MPI_COMM_WORLD, &status);
						printf("\n");
						tiempoTotal= MPI_Wtime();

						tiempoEmpleado= tiempoTotal - tiempoInicial;
						nosSalimos=true;
					}	
				}
			}

			//CALCULAMOS LOS RESULTADOS

			for(int m = 0; m<=status.MPI_SOURCE; m ++)
			{
				sumaProcesoCero = vectorSumaAcumulada[m] + sumaProcesoCero;
			} 

			if(sumaProcesoCero == sumaTotal){//lo usamos para comprobar si la suma recibida total es correcta

				printf("SUMA TOTAL OK: calculada %lld recibida %lld \n \n",sumaProcesoCero,sumaTotal);

			}else{
				printf("ERROR INESPERADO OK: calculada %lld recibida %lld \n",sumaProcesoCero,sumaTotal);
			}

			printf("Proceso  | Nº Divisores    |    Suma     |    Tiempo Calculo\n");
			printf("-----------------------------------------------------------------------\n");


			for(int y = 1; y<=numeroProcesos; y++){//calculamos el total de divisores

				totalDivisores = totalDivisores + numeroDisisores[y];
				

				printf(" %7d | %15d | %11lld | %10f\n",y,numeroDisisores[y],vectorSumaAcumulada[y],tiempoCalculoProcesos[y]);

			}

			printf("----------------------------------------------------------------------------------------------------\n");

			printf("TOTAL    |  %14lld |  %11lld \n\n",totalDivisores,sumaTotal);	

			//comprobaciones finales para determinar si el número es perfecto o no
			if(sumaTotal == numeroP){
				printf("El numero %lld es PERFECTO :)\n",numeroP);
			}else if(sumaTotal < numeroP){

				resta= numeroP-sumaTotal;
				printf("El numero %lld NO es PERFECTO, es DEFECTIVO (%lld) \n",numeroP,resta);

			}else if(sumaTotal > numeroP){

				resta= sumaTotal-numeroP;
				printf("El numero %lld NO es PERFECTO, es EXCESIVO (%lld) \n",numeroP,resta);

			}
			printf("Numero de procesos: %d \n",(numeroProcesos+1));
			printf("Tiempo procesamiento: %f \n",tiempoEmpleado);
		

		}else{

			/******************************************************************************/
			/*-----------------------ID DISTINTO DE 0-------------------------------------*/
			/******************************************************************************/

			unsigned long long int acum = 0;
			int correcto = 0;
			double tiempoFinal = 0;

			MPI_Recv(&numeroCalcular, 1, MPI_UNSIGNED_LONG, 0, etiquetaNumeroP, MPI_COMM_WORLD, &status); // recibimos el numero para calcular si es perfecto

			MPI_Recv(&limiteI, 1, MPI_UNSIGNED_LONG, 0, etiquetaInferior, MPI_COMM_WORLD, &status); // recibimos los limites

			MPI_Recv(&limiteS, 1, MPI_UNSIGNED_LONG, 0, etiquetaSuperior, MPI_COMM_WORLD, &status);

			for(unsigned long long int j=limiteI;j<=limiteS;j++){ //bucle para calcular los divisores

				if( numeroCalcular%j == 0 )
				{
					acum += j;
					MPI_Isend(&j, 1, MPI_UNSIGNED_LONG, 0, etiquetaDivisor, MPI_COMM_WORLD,&peti); 					
				}	
			}

			

			do{ //este bucle sirve para evitar que llegue la suma acumulada antes que los divisores

				MPI_Send(&acum, 1, MPI_UNSIGNED_LONG, 0, etiquetaSumaAcumulada, MPI_COMM_WORLD);
				MPI_Recv(&correcto, 1, MPI_INT, MPI_ANY_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD, &status);

			}while(correcto == 0);

			tiempoFinal= MPI_Wtime();

			tiempoEmpleado= tiempoFinal - tiempoInicial;
			MPI_Isend(&tiempoEmpleado, 1, MPI_DOUBLE, 0, etiquetaTiempoUtilizado, MPI_COMM_WORLD, &peti); //pasamos el tiempo utilizado para calcular 
			
			

			if(id ==1){ // si es el primer proceso no tiene que recibir ninguna suma 

				temp = id+1;
				MPI_Send(&acum, 1, MPI_UNSIGNED_LONG, temp, etiquetaSumaAnterior, MPI_COMM_WORLD);	

			} else if(id >1 && id < numeroProcesos){ // recibe la suma del proceso anterior, se la suma a la suya y se la envia al siguiente
				
				temp = id-1;
				MPI_Recv(&sumaAnterior, 1, MPI_UNSIGNED_LONG, temp, etiquetaSumaAnterior, MPI_COMM_WORLD, &status);
				temp = id +1;
				acum = sumaAnterior+acum;
				MPI_Send(&acum, 1, MPI_UNSIGNED_LONG, temp, etiquetaSumaAnterior, MPI_COMM_WORLD);

			}else if(id == numeroProcesos){ // el ultimo proceso le envia la suma calculada al proceso 0, acabando asi el programa

				temp = id-1;
				MPI_Recv(&sumaAnterior, 1, MPI_UNSIGNED_LONG, temp, etiquetaSumaAnterior, MPI_COMM_WORLD, &status);
				acum = sumaAnterior+acum;
				MPI_Send(&acum, 1, MPI_UNSIGNED_LONG, 0, etiquetaSumaAnterior, MPI_COMM_WORLD);

			}
		}
	}else if(numeroProcesos==0){ //si el numero de procesos es igual a 1 el proceso 0 es el que calcula todos los divisores.
		
		if(id==0){		

			/******************************************************************************/
			/*-----------------------ID -> 0----------------------------------------------*/
			/******************************************************************************/
			
			double tiempoFinal=0;

			for(unsigned long long int j=1;j<=mitad;j++){

				if( numeroP%j == 0 )
				{
					acum = j+acum;
					resta = numeroP-acum;
					printf("DIV:\t %lld SUMA ACU:\t %lld (%lld)\n",j,acum,resta);

					
				}	
			}

			tiempoFinal= MPI_Wtime();

			tiempoEmpleado= tiempoFinal - tiempoInicial;
			printf("\n");
			printf("El tiempo empleado es: %f \n ",tiempoEmpleado);
			if(acum == numeroP){
				printf("El numero %lld es PERFECTO :)\n",numeroP);
			}else if(acum < numeroP){

				resta= numeroP-acum;
				printf("El numero %lld NO es PERFECTO, es DEFECTIVO (%lld) \n",numeroP,resta);

			}else if(acum > numeroP){

				resta= acum-numeroP;
				printf("El numero %lld NO es PERFECTO, es EXESIVO (%lld) \n",numeroP,resta);

			}
			


		}
	}else if(numeroProcesos==1){ // si el numero de procesos es 2 el proceso 0 le manda el numero a calcular al proceso 1 y recibe su resultado.
		
		if(id==0){

			unsigned long long int sumaDeDivisores=0;//suma de los divisores del numero
			unsigned long long int sumaAcu = 0;//varuable para donde se almacena la suma acumulada
			unsigned long long int limite = numeroP/2;//limite a calcular del numero
			int resto = numeroP%2;//resto
			unsigned long long int rangos = limite / numeroProcesos;//rango de numeros donde trabaja cada proceso a la hora de hacer los calculos
			unsigned long long int limiteInferior=1;//limite inferior inicial calculado
			unsigned long long int sumaProcesoCero = 0;//
			unsigned long long int limiteSuperior = 0;//limite superior calculado
			unsigned long long int divisor= 0;//variable para almacenar divisores
			unsigned long long int sumaAcumulada= 0;//variable para la suma acumulada
			unsigned long long int sumaTotal= 0;//suma acumulada final
			unsigned long long int auxiliar= 0;
			unsigned long long int totalDivisores = 0;//numero total de divisores del numero
			bool nosSalimos = false;//flag para salir del bucle
			
			
			MPI_Send(&numeroP, 1, MPI_UNSIGNED_LONG, 1, etiquetaNumeroP, MPI_COMM_WORLD); 		//el proceso 0 envia el numero para calcular si es perfecto o no 

			while(nosSalimos==false){

				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recepcion, &status);
				if(recepcion == 1){

					if(status.MPI_TAG == etiquetaDivisor)
					{
						MPI_Irecv(&divisor, 1, MPI_UNSIGNED_LONG, MPI_ANY_SOURCE, etiquetaDivisor, MPI_COMM_WORLD,&peti);
						MPI_Wait(&peti,&status);

						sumaDeDivisores = sumaDeDivisores+1;
						sumaAcu=sumaAcu+divisor;
						resta = numeroP-divisor;

						printf("DIV: %5d, DIV RECV: %15lld, DIV ACU: %15lld, SUMA ACU: %15lld (%lld)\n",status.MPI_SOURCE,divisor,sumaDeDivisores,sumaAcu,resta);

					}else if(status.MPI_TAG == etiquetaSumaAcumulada){

						MPI_Recv(&sumaAcumulada, 1, MPI_UNSIGNED_LONG, MPI_ANY_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD, &status);

						if(sumaAcumulada == sumaAcu){
							int correcto = 1;
							MPI_Isend(&correcto, 1, MPI_INT, status.MPI_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD,&peti);
							printf("FIN:\t %d, SUMA RECV:\t\t %lld, SUMA ACU:\t\t %lld, SUMA ACU OK\n",status.MPI_SOURCE,sumaAcumulada,sumaAcu);

						}else{
							int correcto = 0;
							MPI_Isend(&correcto, 1, MPI_INT, status.MPI_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD,&peti);

						}


					}else if(status.MPI_TAG == etiquetaTiempoUtilizado)
					{
						
						MPI_Irecv(&tiempoEmpleado, 1, MPI_DOUBLE, MPI_ANY_SOURCE, etiquetaTiempoUtilizado, MPI_COMM_WORLD, &peti);
						MPI_Wait(&peti,&status);
						tiempoCalculoProcesos[status.MPI_SOURCE]=tiempoEmpleado;

						tiempoTotal= MPI_Wtime();

						tiempoEmpleado= tiempoTotal - tiempoInicial;
						nosSalimos=true;


					}	
				}
			}
			
			if(sumaAcu == sumaAcumulada){

				printf("\n SUMA TOTAL OK: calculada %lld recibida %lld \n \n",sumaAcu,sumaAcumulada);

			}else{
				printf("ERROR INESPERADO OK: calculada %lld recibida %lld \n",sumaAcu,sumaAcumulada);
			}

			printf("Proceso  | Nº Divisores    |    Suma     |    Tiempo Calculo\n");
			printf("--------------------------------------------------------------------\n");

			printf(" 1       | %15lld | %11lld | %10f\n \n",sumaDeDivisores,sumaAcu,tiempoCalculoProcesos[1]);

			if(sumaAcu == numeroP){
				printf("El numero %lld es PERFECTO :)\n",numeroP);
			}else if(sumaAcu < numeroP){

				resta= numeroP-sumaAcu;
				printf("El numero %lld NO es PERFECTO, es DEFECTIVO (%lld) \n",numeroP,resta);

			}else if(sumaAcu > numeroP){

				resta= sumaAcu-numeroP;
				printf("El numero %lld NO es PERFECTO, es EXESIVO (%lld) \n",numeroP,resta);

			}
			printf("Numero de procesos: %d \n",(numeroProcesos+1));
			printf("Tiempo procesamiento: %f \n",tiempoEmpleado);



		}else{

			/******************************************************************************/
			/*-----------------------ID DISTINTO DE 0-------------------------------------*/
			/******************************************************************************/

			double tiempoFinal=0;
			int correcto = 0;
			unsigned long long int laMitad = 0;

			MPI_Recv(&numeroCalcular, 1, MPI_UNSIGNED_LONG, 0, etiquetaNumeroP, MPI_COMM_WORLD, &status);

			laMitad = numeroCalcular/2;


			for(unsigned long long int j=1;j<=laMitad;j++){

				if( numeroCalcular%j == 0 )
				{
					MPI_Isend(&j, 1, MPI_UNSIGNED_LONG, 0, etiquetaDivisor, MPI_COMM_WORLD,&peti);
					acum += j;
					
					
				}	
			}

			do{

				MPI_Isend(&acum, 1, MPI_UNSIGNED_LONG, 0, etiquetaSumaAcumulada, MPI_COMM_WORLD,&peti);
				MPI_Recv(&correcto, 1, MPI_INT, MPI_ANY_SOURCE, etiquetaSumaAcumulada, MPI_COMM_WORLD, &status);

			}while(correcto == 0);

			tiempoFinal= MPI_Wtime();

			tiempoEmpleado= tiempoFinal - tiempoInicial;
			MPI_Isend(&tiempoEmpleado, 1, MPI_DOUBLE, 0, etiquetaTiempoUtilizado, MPI_COMM_WORLD, &peti); //pasamos el tiempo utilizado para calcular 



		}
	}

	MPI_Finalize(); 
	return 0;
}