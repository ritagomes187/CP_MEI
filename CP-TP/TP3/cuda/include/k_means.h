#ifndef KMEANS
#define KMEANS


typedef struct points 
{
    int idC_size; // esta variável pode ser ou o id do cluster associado ou o tamanho do clustes 
    float x;
    float y;
}Points; 



void freePoints(Points *p){
    free(p);
}


void inicializa(Points *p , Points *clust, int N, int K);

//float calculaDist(Points p1 , Points p2);

 
__global__ void associaPontosInit(Points *cluster, Points *pontos, int N, int K);

__global__ void associaPontos(Points *cluster, Points *pontos, int N, int K);
 void inicializaVectors(float acumulaX[],float acumulaY[],int K);
__global__ void associaCentroid(Points *cluster ,int K,float acumulaX[], float acumulaY[]);
__global__ void recalculaCentroid(Points *cluster, Points *pontos, int N, float acumulaX[], float acumulaY[]);


#endif