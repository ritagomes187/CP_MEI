// 1 Passo : incializar os pontos e centroids dos clusters
// 2 Passo: associar pontos aos clusters pro centroid mais perto,  calculando distancia euclidiana entre o ponto e o centroid do cluster
// 3 Passo: recalcular centroide fazendo média dos pontos associados
// 4 Passo: 2 e 3 até que não haja mais trocas de clusters

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>

#include "../include/k_means.h"

// #define N 0

// #define K 0

// Inicialização dos clusters e pontos
void inicializa(Points *p, Points *clust, int N, int K)
{
    float x, y;
    srand(10);
    // inicializa os pontos
    for (int i = 0; i < N;)
    {
        x = (float)rand() / RAND_MAX;
        y = (float)rand() / RAND_MAX;
        p[i].x = x;
        p[i].y = y;

        x = (float)rand() / RAND_MAX;
        y = (float)rand() / RAND_MAX;
        p[i + 1].x = x;
        p[i + 1].y = y;
        i += 2;
    }

    // incializa clusters
    for (int j = 0; j < K;)
    {
        clust[j] = p[j];
        clust[j + 1] = p[j + 1];
        j += 2;
    }
}

// Calcula distância euclidiana entre dois pontos
float calculaDist(Points p1, Points p2)
{
    float y = p1.x - p2.x;
    float x = p1.y - p2.y;

    float dist = sqrt(pow(y, 2) + pow(x, 2));

    return dist;
}

// Inicializar a associação dos pontos aos clusters mais próximos
void associaPontosInit(Points *cluster, Points *pontos, int N, int K)
{

    /*int chunk_size = N / num_procs;
    int start = rank * chunk_size;
    int end = start + chunk_size;
    if (rank == num_procs - 1)
    {
        end = N; // último processo pega o que sobrou
    }*/

    for (int i = 0; i < N; i++)
    {
        int clusterMin, y;
        float distMin;
        float distancias[K]; // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids
        Points ponto;
        distMin = 10000;
        clusterMin = 0;
        ponto = pontos[i];
        // local de paralelismo
        for (y = 0; y < K; y++)
        {
            distancias[y] = calculaDist(ponto, cluster[y]);
        }
        for (y = 0; y < K; y++)
        {
            if (distancias[y] < distMin)
            {
                distMin = distancias[y];
                clusterMin = y;
            }
        }
        // para evitar concorrencia (garantir a exclusao mutua)
        /*MPI_Atomic_lock(&cluster[clusterMin].idC_size);
        cluster[clusterMin].idC_size++;
        MPI_Atomic_unlock(&cluster[clusterMin].idC_size);*/
        
        
        /*MPI_Lock(&cluster[clusterMin].idC_size);
        cluster[clusterMin].idC_size++;
        MPI_Unlock(&cluster[clusterMin].idC_size);*/
        pontos[i].idC_size = clusterMin;
        cluster[clusterMin].idC_size++;
    }
    
}

// Associar os pontos aos clusters mais próximos
// dividir o trabalho entre os processos MPI, de modo que cada processo faça a
// atribuição dos pontos aos clusters apenas para uma parte dos pontos
void associaPontos(Points *cluster, Points *pontos, int N, int K)
{
    int rank; int numProcs;
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    // Dividir os pontos entre os processos
    int localN = N / numProcs; // número de pontos para cada processo
    int resto = N % numProcs;  // resto da divisão
    if (rank < resto)
        localN++; // os primeiros "resto" processos recebem um ponto a mais

    Points *localPontos = malloc(localN * sizeof(Points)); // vetor de pontos para cada processo

    // MPI_Scatter para distribuir os pontos entre os processos
    MPI_Scatter(pontos, localN * sizeof(Points), MPI_BYTE, localPontos, localN * sizeof(Points), MPI_BYTE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < localN; i++)
    {
        int clusterAntigo, clusterIdMin;
        float distMin;
        Points ponto, centroidMin;
        ponto = localPontos[i];
        clusterIdMin = ponto.idC_size;
        clusterAntigo = clusterIdMin;
        centroidMin = cluster[clusterAntigo];
        distMin = calculaDist(ponto, centroidMin);
        for (int j = 0; j < K; j++)
        {
            float dist = calculaDist(ponto, cluster[j]);
            if (dist < distMin)
            {
                distMin = dist;
                clusterIdMin = j;
            }
        }
        // realizar troca de cluster
        if (clusterIdMin != clusterAntigo)
        {
            localPontos[i].idC_size = clusterIdMin;
            //cluster[clusterAntigo].idC_size--;
            //cluster[clusterIdMin].idC_size++;
        }
    }
    // MPI_Gather para reunir os resultados finais num único vetor de pontos
    /*O processo de rank 0 é o responsável por reunir os resultados e o vetor pontos será
    atualizado com os novos valores dos pontos após a atribuição aos clusters*/
    MPI_Gather(localPontos, localN * sizeof(Points), MPI_BYTE, pontos, localN * sizeof(Points), MPI_BYTE, 0, MPI_COMM_WORLD);
    free(localPontos);
}

void inicializaVectors(float acumulaX[], float acumulaY[], int K)
{
    // inicializa vetores que irão ter a soma dos pontos
    for (int z = 0; z < K; z++)
    {
        acumulaX[z] = 0.0;
        acumulaY[z] = 0.0;
    }
}
// Recalcula centroid de cada cluster
void recalculaCentroid(Points *cluster, Points *pontos, int N, int K)
{
     int rank, numProcs;
    // MPI_Init(NULL, NULL);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_CO MM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    int chunk_size = N / numProcs;
    int start = rank * chunk_size;
    int end = start + chunk_size;
    if (rank == numProcs - 1)
    {
        end = N; // último processo pega o que sobrou
    }
    float auxX[K];
    float auxY[K];
    inicializaVectors(auxX, auxY, K);
    //printf("Rank: %d , Num_process: %d , i = %d , %d\n", rank, numProcs,start,pontos[start].idC_size);
    /* realiza soma dos pontos */
    for (int y = start; y < end; y++)
    {
        int clustId;
        Points ponto;
        ponto = pontos[y];
        clustId = ponto.idC_size;
        auxX[clustId] += ponto.x;
        auxY[clustId] += ponto.y;
    }
    float acumulaX[K];
    float acumulaY[K];
    // sincronizar os resultados entre os processos
    MPI_Allreduce(auxX, acumulaX, K, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(auxY, acumulaY, K, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    //printf("soma do cluster 0: %f,%f\n", acumulaX[0], acumulaY[0]); 
    // Faz a media dos pontos e associa a um novo centroid a cada cluster
    chunk_size = K / numProcs;
    start = rank * chunk_size;
    end = start + chunk_size;
    if (rank == numProcs - 1)
    {
        end = K; // último processo pega o que sobrou
    }
    for (int i = start; i < end; i++)
    {
        int size;
        float mediaX, mediaY;
        size = cluster[i].idC_size;
        mediaX = acumulaX[i] / (float)size;
        mediaY = acumulaY[i] / (float)size;
        // ponto de paralelismo
        cluster[i].x = mediaX;
        cluster[i].y = mediaY;
    }
    //MPI_Finalize();
}

int main(int argc, char *argv[])
{
    //printf("OK");
    //printf("OK");
    int N, K, threads, rank, numProcs;
    //printf("OK");
    N = atoi(argv[1]);
    K = atoi(argv[2]);
    //threads = (argc > 3) ? atoi(argv[3]) : 1;
    //omp_set_num_threads(threads);
    Points *pontos = NULL;
    pontos = initPoints(N);
    Points *clust = NULL;
    clust = initPoints(K);
    // int flag = 1;
    int it = 0;
    float acumulaX[K];
    float acumulaY[K];
    inicializa(pontos, clust, N, K);
    associaPontosInit(clust, pontos, N, K);
    Points ponto;
    MPI_Init(&argc, &argv);
    while (it < 20)
    {
        //inicializaVectors(acumulaX, acumulaY, K);
        //printf("%d\n",it);
        recalculaCentroid(clust, pontos, N, K);
        //printf("%d\n",it);
        associaPontos(clust, pontos, N, K);
        ponto = clust[0];
        //printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y, ponto.idC_size);
        it++;
    }
    MPI_Finalize();
    printf("N = %d, K = %d\n", N, K);
    //Points ponto;
    for (int j = 0; j < K; j++)
    {
        ponto = clust[j];
        printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y, ponto.idC_size);
    }
    printf("Iterations: %d\n", it);
    freePoints(pontos);
    freePoints(clust);
    return 0;
}