#ifndef KMEANS
#define KMEANS


typedef struct points 
{
    int idC_size; // esta variável pode ser ou o id do cluster associado ou o tamanho do clustes 
    float x;
    float y;
}Points; 


Points* initPoints(int N){
    Points *pontos = malloc(sizeof (Points) * N);
    if (pontos == NULL) return NULL;
    return pontos;
}


void freePoints(Points *p){
    free(p);
}


void inicializa(Points *p , Points *clust);

float calculaDist(Points p1 , Points p2);

 
void associaPontosInit(Points *cluster, Points *pontos);

int associaPontos(Points *cluster, Points *pontos);

void recalculaCentroid(Points *cluster, Points *pontos);


#endif