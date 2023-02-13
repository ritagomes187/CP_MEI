// 1 Passo : incializar os pontos e centroids dos clusters
// 2 Passo: associar pontos aos clusters pro centroid mais perto,  calculando distancia euclidiana entre o ponto e o centroid do cluster
// 3 Passo: recalcular centroide fazendo média dos pontos associados
// 4 Passo: 2 e 3 até que não haja mais trocas de clusters 


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../include/k_means.h"

#define N 10000000


#define K 4 



// Inicialização dos clusters e pontos
void inicializa(Points *p , Points *clust){
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
void associaPontosInit(Points *cluster, Points *pontos){;
    int clusterMin,y,i;
    float distMin;
    float distancias[K]; // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids
    Points ponto;
    for ( i = 0; i < N ; i++){
        distMin = 10000;
        clusterMin = 0;
        ponto = pontos[i];
        for ( y = 0; y < K; y++){
            distancias[y] = calculaDist(ponto, cluster[y]);
            distancias[y+1] = calculaDist(ponto, cluster[y+1]);
            y++;
        }
        for (y = 0; y < K; y++){
            if(distancias[y] < distMin){
                distMin = distancias[y];
                clusterMin = y;
            }
        }
        pontos[i].idC_size = clusterMin;
        cluster[clusterMin].idC_size++;
    }

}
// Associar os pontos aos clusters mais próximos 
 int associaPontos(Points *cluster, Points *pontos){
    int flag = 0;
    int clusterAntigo,clusterIdMin,i,j;
    float distMin; 
    float distancias[K]; // vetor que irá conter as distancias euclidianas entre cada ponto e os centroids
    Points ponto,centroidMin;
    for(i = 0; i < N; i++){
             ponto = pontos[i]; 
             clusterIdMin = ponto.idC_size;
             clusterAntigo = clusterIdMin;
             centroidMin = cluster[clusterAntigo];
             distMin = calculaDist(ponto,centroidMin);
             for(j = 0; j < K; j++){
                distancias[j] = calculaDist(ponto,cluster[j]);
                distancias[j+1] = calculaDist(ponto,cluster[j+1]);
                j++;
             }
             for (j = 0; j < K; j++)
             {
                if (distancias[j]<distMin){
                    distMin = distancias[j];
                    clusterIdMin = j;  
                }
             }
             
            //realizar troca de cluster
             if(clusterIdMin!=clusterAntigo){
                 flag=1;
                 pontos[i].idC_size = clusterIdMin;
                 cluster[clusterAntigo].idC_size--;
                 cluster[clusterIdMin].idC_size++;
             }
         }
        
       return flag;  
 }




// Recalcula centroid de cada cluster
 void recalculaCentroid(Points *cluster, Points *pontos){
     float acumulaX[K];
     float acumulaY[K];
     Points ponto;
     // inicializa vetores que irão ter a soma dos pontos
     for (int z = 0; z<K;){
        acumulaX[z] = acumulaX[z+1] = 0.0;
        acumulaY[z] = acumulaY[z+1] = 0.0;
        z+=2;
     }
     int clustId;
     // realiza soma dos pontos
     for(int y = 0; y < N ;){    
         ponto = pontos[y];
         clustId = ponto.idC_size;  
         acumulaX[clustId] += ponto.x;   
         acumulaY[clustId] += ponto.y;   
         
         ponto = pontos[y+1];
         clustId = ponto.idC_size;  
         acumulaX[clustId] += ponto.x;   
         acumulaY[clustId] += ponto.y;  
         y+=2;       
     }
     int size;
     float mediaX , mediaY;
     // Faz a media dos pontos e associa a um novo centroid a cada cluster 
     for (int i = 0; i < K; i++)
     {
         size = cluster[i].idC_size;
         mediaX = acumulaX[i] / (float)size;
         mediaY = acumulaY[i] / (float)size;
         cluster[i].x = mediaX;
         cluster[i].y = mediaY;
     }
    

 }

int main(){
    Points *pontos = NULL;
    pontos = initPoints(N);
    Points *clust = NULL;
    clust = initPoints(K);
    int flag = 1;
    int it = 0;
    
    inicializa(pontos,clust);
    associaPontosInit(clust,pontos);
     while (flag == 1)
     {
        recalculaCentroid(clust,pontos);
        flag = associaPontos(clust,pontos);
        it++; 
     }
    printf("N = %d, K = %d\n", N,K);
    Points ponto;
    for(int j = 0; j < K; j++){
         ponto = clust[j];
         printf("Center: (%f,%f) : Size: %d\n", ponto.x, ponto.y,ponto.idC_size); 
     }
    printf("Iterations: %d\n", it);
    freePoints(pontos);
    freePoints(clust);

    return 0;
    
}