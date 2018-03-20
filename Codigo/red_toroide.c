//
//  red_toroide.c
//  
//
//  Created by Benjamin Cadiz de Gracia on 27/2/18.
//

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define DATOS "Datos.dat"
#define L 3

void obtain_neighbors(int* north, int* south, int* east, int* west, int rank,
                      int numtasks){
    
    *south = (rank + L) % numtasks;
    
    *north = (rank - L) % numtasks;
    if(*north < 0){
        *north += numtasks;
    }
    
    *east = rank + 1;
    if(*east % L == 0){
        *east -= L;
    }
    
    *west = (rank - 1) % numtasks;
    if(*west % L == L-1){
        *west += L;
    }else if(*west == -1){
        *west = L-1;
    }
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
    FILE* file;
    
    int rank,size,numtask;
    int  north,south,east,west;
    
    // Inicializacion de MPI.
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtask);
    
    //Obtenemos vecinos.
    obtain_neighbors(&north,&south,&east,&west,rank,numtask);
    
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
    for( int i =0;i<L-1;i++){
        rc = MPI_Send(&token,1, MPI_FLOAT, north,i,MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS){
            printf("Receive error in task %d\n", rank);
            MPI_Finalize();
            exit(1);
        }
        rc = MPI_Recv(&new_token,1,MPI_FLOAT,south,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        if (rc != MPI_SUCCESS){
            printf("Receive error in task %d\n", rank);
            MPI_Finalize();
            exit(1);
        }
        if(new_token < token){
            token = new_token;
        }
    }
    
    for( int i =0;i<L-1;i++){
        rc = MPI_Send(&token,1, MPI_FLOAT, west,i,MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS){
            printf("Receive error in task %d\n", rank);
           	MPI_Finalize();
            exit(1);
        }
        rc = MPI_Recv(&new_token,1,MPI_FLOAT,east,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        if (rc != MPI_SUCCESS){
            printf("Receive error in task %d\n", rank);
            MPI_Finalize();
            exit(1);
        }
        if(new_token < token)
            token = new_token;
    }
    
    if(rank ==0)
        printf("The minimun value is:  %g \n",token);
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}
