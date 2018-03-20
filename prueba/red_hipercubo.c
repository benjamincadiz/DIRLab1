//
//  red_hipercubo.c
//  
//
//  Created by Benjamin Cadiz de Gracia on 16/3/18.
//

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DATOS "Datos.dat"
#define D 4

int obtain_neighbor(int rank, int dimension){
    int neighbor, node;
    for(node = 0; node < (int)pow(2,D); node++){
        if((rank ^ node) == (int)pow(2,dimension - 1)){
            neighbor = node;
            break;
        }
    }
    return neighbor;
}
void distribuir(int numtask, int rank, float* token){
    int i = 0;
    int rc;
    FILE* file;
    float num[16];
    MPI_Status status;
    MPI_Request request;
    float element=0;
    
    //Leemos el archivo.
    if ((file=fopen(DATOS,"r"))==NULL){
		fprintf(stderr, "Error opening the file\n");
        exit(EXIT_FAILURE);
    }else
        while(!feof (file) & (i < numtask)){
            fscanf (file,"%g,", &element);
            
            //Enviamos en el primer elemento.
            if(i == rank){
                *token = element;
            }else{
                rc = MPI_Isend(&element, 1, MPI_FLOAT, i, 1, MPI_COMM_WORLD, &request);
                if (rc != MPI_SUCCESS) {
                    printf("Send error in task %d\n", rank);
                    MPI_Finalize();
                    exit(1);
                }
                MPI_Wait(&request, &status);
            }
            i++;
        }
    fclose(file);
}
void obtain_token(float* token, int rank){
    int rc;
    
    MPI_Status status;
    rc = MPI_Recv(token, 1, MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (rc != MPI_SUCCESS) {
        printf("Receive error in task %d\n", rank);
        MPI_Finalize();
        exit(1);
    }
}

int main(int argc, char** argv) {
    float token;
    
    int rank,size,numtask;
    FILE* file;
    
    // Inicializacion de MPI.
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtask);
    
    
    //Distribuimos los elementos del fichero en nuestros nodos.
    if(rank == 0){
        distribuir(numtask,rank, &token);
    }else{
        obtain_token(&token, rank);
    }
    
    //Realizamos el algoritmo.
    int rc;
    float new_token;
    MPI_Status status;
    for( int i =1;i<=D;i++){
        rc = MPI_Send(&token,1, MPI_FLOAT,obtain_neighbor(rank, i),i,MPI_COMM_WORLD);
        
        if (rc != MPI_SUCCESS){
            printf("Receive error in task %d\n", rank);
            MPI_Finalize();
            exit(1);
        }
        rc = MPI_Recv(&new_token,1,MPI_FLOAT,obtain_neighbor(rank, i),MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        if (rc != MPI_SUCCESS){
            printf("Receive error in task %d\n", rank);
            MPI_Finalize();
            exit(1);
        }
        if(new_token > token){
            token = new_token;
        }
    }
    
    if(rank ==0)
        printf("The maximum value is:  %g \n",token);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

