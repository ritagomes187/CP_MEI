// 1 Passo : incializar os pontos e centroids dos clusters
// 2 Passo: associar pontos aos clusters pro centroid mais perto,  calculando distancia euclidiana entre o ponto e o centroid do cluster
// 3 Passo: recalcular centroide fazendo média dos pontos associados
// 4 Passo: 2 e 3 até que não haja mais trocas de clusters

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <cuda.h>
#include "/usr/include/linux/cuda.h"

#include "../include/k_means.h"

// #define N 0

// #define K 0


// Inicialização dos clusters e pontos
void inicializa(Points *p, Points *clust, int N, int K)
{
    float x,y;
    srand(10);
    // inicializa os pontos
    for(int i = 0; i < N;) {
        x = (float) rand() / RAND_MAX;
        y = (float) rand() / RAND_MAX;
        p[i].x = x;
        p[i].y = y; 
        
        x = (float) rand() / RAND_MAX;
        y = (float) rand() / RAND_MAX;
        p[i+1].x = x;
        p[i+1].y = y;
        i+=2;

    }
    
    //incializa clusters
    for(int j = 0; j < K;) {
        clust[j] = p[j];
        clust[j+1] = p[j+1];
        j+=2;
        
    }
}
// Calcula distância euclidiana entre dois pontos
//  float calculaDist(Points p1, Points p2)
// {
//     float y = p1.x - p2.x;
//     float x = p1.y - p2.y;

//     float dist = sqrt(pow(y, 2) + pow(x, 2));

//     return dist;
// }

// Inicializar a associação dos pontos aos clusters mais próximos
__global__ void associaPontosInit(Points *cluster, Points *pontos, int N, int K)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if (i < N)
    {
        int clusterMin, y;
        float distMin;
        float *distancias = (float*) malloc(K * sizeof(float)); // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids
        Points ponto;
        distMin = 10000;
        clusterMin = 0;
        ponto = pontos[i];
        // local de paralelismo
        for (y = 0; y < K; y++)
        { 
            float z = ponto.x - cluster[y].x;
            float x = ponto.y - cluster[y].y;

            float dist = sqrt(pow(z, 2) + pow(x, 2));
            distancias[y] = dist;
            
        }
        for (y = 0; y < K; y++)
        {
            if (distancias[y] < distMin)
            {
                distMin = distancias[y];
                clusterMin = y;
            }
        }
        pontos[i].idC_size = clusterMin;
        atomicAdd(&cluster[clusterMin].idC_size, 1);
        
    }
    
}
// Associar os pontos aos clusters mais próximos
__global__ void associaPontos(Points *cluster, Points *pontos, int N, int K)
{
    // int flag = 0;
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if (i < N)
    {
        int clusterAntigo, clusterIdMin, j;
        float distMin;
        Points ponto, centroidMin;
        
        float *distancias = (float*) malloc(K * sizeof(float)); // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids 
        ponto = pontos[i];
        float z,x,dist;
        for (j = 0; j < K; j++)
        {
             z = ponto.x - cluster[j].x;
             x = ponto.y - cluster[j].y;

            dist = sqrt(pow(z, 2) + pow(x, 2));
            distancias[j] = dist;
        }
        clusterIdMin = ponto.idC_size;
        clusterAntigo = clusterIdMin;
        centroidMin = cluster[clusterAntigo];
        
        z = ponto.x - centroidMin.x;
        x = ponto.y - centroidMin.y;
        dist = sqrt(pow(z, 2) + pow(x, 2));
        distMin = dist;
        for (j = 0; j < K; j++)
        {
            if (distancias[j] < distMin)
            {
                distMin = distancias[j];
                clusterIdMin = j;
            }
        }
        // realizar troca de cluster
        if (clusterIdMin != clusterAntigo)
        {
            // flag=1;
            pontos[i].idC_size = clusterIdMin;

            atomicAdd(&cluster[clusterAntigo].idC_size, -1);
            atomicAdd(&cluster[clusterIdMin].idC_size, 1);
        }
        free(distancias);
    }

    // return flag;
}

void inicializaVectors(float acumulaX[],float acumulaY[],int K){
     // inicializa vetores que irão ter a soma dos pontos
    for (int z = 0; z<K;z++){
        acumulaX[z] = 0.0;
        acumulaY[z] = 0.0;
    }
}

    

// Recalcula centroid de cada cluster
__global__ void associaCentroid(Points *cluster ,int K,float acumulaX[], float acumulaY[]){
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    // Faz a media dos pontos e associa a um novo centroid a cada cluster
    if (i < K)
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

}


__global__ void recalculaCentroid(Points *cluster, Points *pontos, int N, float acumulaX[], float acumulaY[])
{
    int y = threadIdx.x + blockIdx.x * blockDim.x;
/* realiza soma dos pontos */
    if (y < N)
    {
        int clustId;
        Points ponto;
        ponto = pontos[y];
        clustId = ponto.idC_size;
        atomicAdd(&acumulaX[clustId], ponto.x);
        atomicAdd(&acumulaY[clustId], ponto.y);
    }
    
    
}

int main(int argc, char *argv[])
{
    int N, K;
    N = atoi(argv[1]);
    K = atoi(argv[2]);
    int threadsPerBlock = 256;
    int blocksPerGrid =  (N + threadsPerBlock - 1) / threadsPerBlock;
    
    float acumulaX[K];
    float acumulaY[K];
    Points *d_pontos;
    Points *d_cluster;
    // O cudaMalloc pode ser usado para alocar memória na GPU para os dados que serão processados pelo seu kerne
    //  Aloca memória para o array pontos
    cudaMalloc(&d_pontos, N * sizeof(Points));
    
    cudaMalloc(&d_cluster, K * sizeof(Points));
    
    // Aloca memória para o array cluster
    

    float *d_acumulaX, *d_acumulaY;
    cudaMalloc(&d_acumulaX, K * sizeof(float));
    cudaMalloc(&d_acumulaY, K * sizeof(float));


    Points *pontos = NULL;
    Points *cluster = NULL;
    pontos = (Points*) malloc(N * sizeof(Points));
    cluster = (Points*) malloc(K * sizeof(Points));
    inicializa(pontos, cluster, N, K);
    

    // Copia os dados dos arrays na CPU para os arrays na GPU


    // Executa o kernel
    // o segundo argumento <<<1,1>>> especifica o número de threads em cada dimensão do grid
    // e o número de threads em cada dimensão do bloco

    // Copia os dados dos arrays na GPU de volta para os arrays na CPU
    // int flag = 1;
    int it = 0;
    //inicializa(p<<<blocksPerGrid,threadsPerBlock>>>ontos, clust, N, K);
    
    
    cudaMemcpy(d_pontos, pontos, N * sizeof(Points), cudaMemcpyHostToDevice);
    cudaMemcpy(d_cluster, cluster, K * sizeof(Points), cudaMemcpyHostToDevice);
    
    cudaError_t err;
    associaPontosInit<<<blocksPerGrid,threadsPerBlock>>>(d_cluster, d_pontos, N, K);
    err = cudaMemcpy(cluster, d_cluster, K * sizeof(Points), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        printf("Error3: %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    Points ponto;
    for (int j = 0; j < K; j++)
    {
        ponto = cluster[j];
        printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y, ponto.idC_size);
    }

    //cudaMemcpy(cluster, d_cluster, K * sizeof(Points), cudaMemcpyDeviceToHost);
    while (it < 20)
    {
        
        inicializaVectors(acumulaX, acumulaY, K);
        cudaMemcpy(d_acumulaX, acumulaX, K * sizeof(Points), cudaMemcpyHostToDevice);
        cudaMemcpy(d_acumulaY, acumulaY, K * sizeof(Points), cudaMemcpyHostToDevice);
    
        recalculaCentroid<<<blocksPerGrid,threadsPerBlock>>>(d_cluster, d_pontos, N,d_acumulaX, d_acumulaY);
        associaCentroid<<<blocksPerGrid,threadsPerBlock>>>(d_cluster ,K,d_acumulaX, d_acumulaY);
        associaPontos<<<blocksPerGrid,threadsPerBlock>>>(d_cluster, d_pontos, N, K);
        
        //printf("oi\n");
        //ponto = cluster[0];
        //printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y, ponto.idC_size);
        it++;
    }
    printf("N = %d, K = %d\n", N, K);
    err = cudaMemcpy(cluster, d_cluster, K * sizeof(Points), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        printf("Error: %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j < K; j++)
    {
        ponto = cluster[j];
        printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y, ponto.idC_size);
    }
    printf("Iterations: %d\n", it);
    freePoints(pontos);
    freePoints(cluster);

    // Liberta a memória da GPU quando não for mais necessária
    cudaFree(d_pontos);
    cudaFree(d_cluster);
    cudaFree(d_acumulaX);
    cudaFree(d_acumulaY);
    return 0;
}