// 1 Passo : incializar os pontos e centroids dos clusters
// 2 Passo: associar pontos aos clusters pro centroid mais perto,  calculando distancia euclidiana entre o ponto e o centroid do cluster
// 3 Passo: recalcular centroide fazendo média dos pontos associados
// 4 Passo: 2 e 3 até que não haja mais trocas de clusters 


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "../include/k_means.h"

//#define N 0 


//#define K 0 



// Inicialização dos clusters e pontos
void inicializa(Points *p , Points *clust,int N, int K){
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
float calculaDist(Points p1 , Points p2){
    float y = p1.x - p2.x;
    float x = p1.y - p2.y;

    float dist = sqrt(pow(y,2)+pow(x,2));
    
    return dist;
}


//Inicializar a associação dos pontos aos clusters mais próximos 
void associaPontosInit(Points *cluster, Points *pontos, int N, int K){;
    #pragma omp parallel for 
    for (int i = 0; i < N ; i++){
        int clusterMin,y;
        float distMin;
        float distancias[K]; // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids
        Points ponto;
        clusterMin = 0;
        ponto = pontos[i];
        // local de paralelismo
        for ( y = 0; y < K; y++){
            distancias[y] = calculaDist(ponto, cluster[y]);
            distancias[y+1] = calculaDist(ponto, cluster[y+1]);
            y++;
        }
        distMin = distancias[0];
        for (y = 1; y < K; y++){
            if(distancias[y] < distMin){
                distMin = distancias[y];
                clusterMin = y;
            }
        }
        pontos[i].idC_size = clusterMin;
        #pragma omp atomic
        cluster[clusterMin].idC_size++;
    }

}
// Associar os pontos aos clusters mais próximos 
 void associaPontos(Points *cluster, Points *pontos, int N, int K){
    //int flag = 0;
    int i; 
    #pragma omp parallel for 
    for(i = 0; i < N; i++){
            int clusterAntigo,clusterIdMin,j;
            float distMin; 
            Points ponto,centroidMin;
            float distancias[K]; // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids
            ponto = pontos[i];  
            for(j = 0; j < K; j++){
                distancias[j] = calculaDist(ponto,cluster[j]);
                distancias[j+1] = calculaDist(ponto,cluster[j+1]);
                j++;
             }
             clusterIdMin = ponto.idC_size;
             clusterAntigo = clusterIdMin;
             centroidMin = cluster[clusterAntigo];
             distMin = calculaDist(ponto,centroidMin);
             for (j = 0; j < K; j++)
             {
                if (distancias[j]<distMin){
                    distMin = distancias[j];
                    clusterIdMin = j;  
                }
             }   
            //realizar troca de cluster
             if(clusterIdMin!=clusterAntigo){
                 //flag=1;
                 pontos[i].idC_size = clusterIdMin;
                 #pragma omp atomic
                 cluster[clusterAntigo].idC_size--;
                 #pragma omp atomic
                 cluster[clusterIdMin].idC_size++;
             }
         }
        
       //return flag;  
 }



void inicializaVectors(float acumulaX[],float acumulaY[],int K){
     // inicializa vetores que irão ter a soma dos pontos
    #pragma omp for
    for (int z = 0; z<K;z++){
        acumulaX[z] = 0.0;
        acumulaY[z] = 0.0;
    }
} 
// Recalcula centroid de cada cluster
 void recalculaCentroid(Points *cluster, Points *pontos, int N, int K, float acumulaX[], float acumulaY[]){
      
     /* realiza soma dos pontos */
     // local de paralelismo com reduce com cuidado
     #pragma omp for reduction(+:acumulaX[:K]) reduction(+:acumulaY[:K])
     for(int y = 0; y < N ;y++){    
         int clustId;
         Points ponto;
         ponto = pontos[y];
         clustId = ponto.idC_size;  
         acumulaX[clustId] += ponto.x;   
         acumulaY[clustId] += ponto.y;          
     }
     // Faz a media dos pontos e associa a um novo centroid a cada cluster 
     
     #pragma omp for
     for (int i = 0; i < K; i++)
     {
         int size;
         float mediaX , mediaY; 
         size = cluster[i].idC_size;
         mediaX = acumulaX[i] / (float)size;
         mediaY = acumulaY[i] / (float)size;
         // ponto de paralelismo
         cluster[i].x = mediaX;
         cluster[i].y = mediaY;
     }
    

 }

int main(int argc, char *argv[]){
    int N,K, threads;
    N = atoi(argv[1]);
    K = atoi(argv[2]);
    threads = (argc > 3)? atoi(argv[3]): 1;
    omp_set_num_threads(threads);
    Points *pontos = NULL;
    pontos = initPoints(N);
    Points *clust = NULL;
    clust = initPoints(K);
    //int flag = 1;
    int it = 0;
    float acumulaX[K];
    float acumulaY[K]; 
    inicializa(pontos,clust,N,K);
    associaPontosInit(clust,pontos,N,K);
    Points ponto;
    for(int j = 0; j < K; j++){
         ponto = clust[j];
         printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y,ponto.idC_size); 
    }
     while (it < 2)
     {
        inicializaVectors(acumulaX,acumulaY,K);
        recalculaCentroid(clust,pontos,N,K,acumulaX,acumulaY);
        associaPontos(clust,pontos,N,K);
        //ponto = clust[0];
        //printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y,ponto.idC_size); 
        it++;

     }
    printf("N = %d, K = %d\n", N,K);
    for(int j = 0; j < K; j++){
         ponto = clust[j];
         printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y,ponto.idC_size); 
     }
    printf("Iterations: %d\n", it);
    freePoints(pontos);
    freePoints(clust);

    return 0;
    
}