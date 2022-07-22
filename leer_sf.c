// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "leer_sf.h"
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error: mala sintaxis. Ejemplo: ./leer_sf <nombre_dispositivo>");
        return 1;
    }
    struct superbloque SB;
    bmount(argv[1]);
    if (bread(0, &SB) == -1)
    {
        perror("Error leyendo el SB: ");
        return 1;
    }
    printsuperbloque(SB);

    #if NIVEL2
    printf("\n\nsizeof struct superbloque: %ld", sizeof(struct superbloque));
    printf("\nsizeof struct inodo: %ld", sizeof(struct inodo));
    int posRelAI = SB.posPrimerInodoLibre;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    printf("\n\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        if (bread(i, inodos) == -1)
        {
            perror("Error: ");
            return 1;
        }
        for (size_t j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        {
            posRelAI = inodos[j].punterosDirectos[0];
            printf("%d ", posRelAI);
        }
    }
    #endif

    #if NIVEL3
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int nbloque = reservar_bloque();    //reserva un bloque
    bread(posSB,&SB);
    printf("Se ha reservado el bloque fisico nº%d que era el 1º libre indicado por el MB\n", nbloque);
    printf("SB.cantBloquesLibres = %d", SB.cantBloquesLibres);
    liberar_bloque(nbloque);    //Libera el bloque reservado
    bread(posSB, &SB);
    printf("\nLiberamos ese bloque y después SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);

    printf("\n\nMAPA DE BITS");
    //Mostrar en pantalla la lectura de bits
    printf("\nleer_bit(%d)=%d",posSB,leer_bit(posSB));
    printf("\nleer_bit(%d)=%d",SB.posPrimerBloqueMB,leer_bit(SB.posPrimerBloqueMB));
    printf("\nleer_bit(%d)=%d",SB.posUltimoBloqueMB,leer_bit(SB.posUltimoBloqueMB));
    printf("\nleer_bit(%d)=%d",SB.posPrimerBloqueAI,leer_bit(SB.posPrimerBloqueAI));
    printf("\nleer_bit(%d)=%d",SB.posUltimoBloqueAI,leer_bit(SB.posUltimoBloqueAI));
    printf("\nleer_bit(%d)=%d",SB.posPrimerBloqueDatos,leer_bit(SB.posPrimerBloqueDatos));
    printf("\nleer_bit(%d)=%d",SB.posUltimoBloqueDatos,leer_bit(SB.posUltimoBloqueDatos));
    
    printf("\n\nDATOS DEL DIRECTORIO RAIZ");

    //inicialización
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    
    struct inodo inodo;
    int ninodo = SB.posInodoRaiz;

    //lectura del inodo y mostrar en pantalla la informacion de este
    leer_inodo(ninodo,&inodo);
    printf("\ntipo: %c",inodo.tipo);
    printf("\npermisos: %d",inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("\nID: %d",ninodo);
    printf("\nATIME: %s",atime);
    printf("\nMTIME: %s",mtime);
    printf("\nCTIME: %s",ctime);
    printf("\nnlinks: %d",inodo.nlinks);
    printf("\ntamEnBytesLog: %d",inodo.tamEnBytesLog);
    printf("\nnumBloquesOcupados: %d",inodo.numBloquesOcupados);
    printf("\n");
    #endif

    #if NIVEL4
    int ninodo = reservar_inodo('f',6);
    printf("\nINODO %d. TRADUCCION DE LOS BLOQUES LOGICOS 8,204,3004,400004 Y 468750",ninodo);
    traducir_bloque_inodo(ninodo, 8, 1);
    traducir_bloque_inodo(ninodo, 204, 1);
    traducir_bloque_inodo(ninodo, 3004, 1);
    traducir_bloque_inodo(ninodo, 400004, 1);
    traducir_bloque_inodo(ninodo, 468750, 1);
    printf("\n");
    imprimir_inodo(ninodo);
    bread(posSB,&SB);
    printf("\n\nSB.posPrimerInodoLibre = %d",SB.posPrimerInodoLibre);
    //Oculto para ver mejor la muestra del Nivel 4

    printf("\n\nsizeof struct superbloque: %ld", sizeof(struct superbloque));
    printf("\nsizeof struct inodo: %ld", sizeof(struct inodo));
    int posRelAI = SB.posPrimerInodoLibre;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    printf("\n\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        if (bread(i, inodos) == -1)
        {
            perror("Error: ");
            return 1;
        }
        for (size_t j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        {
            posRelAI = inodos[j].punterosDirectos[0];
            printf("%d ", posRelAI);
        }
    }
    #endif

    #if NIVEL7
    mostrar_buscar_entrada("pruebas/",1);
    mostrar_buscar_entrada("/pruebas/",0);
    mostrar_buscar_entrada("/pruebas/docs/",1);
    mostrar_buscar_entrada("/pruebas/",1);
    mostrar_buscar_entrada("/pruebas/docs/",1);
    mostrar_buscar_entrada("/pruebas/docs/doc1",1);
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11",1);
    mostrar_buscar_entrada("/pruebas/",1);
    mostrar_buscar_entrada("/pruebas/docs/doc1",0); 
    mostrar_buscar_entrada("/pruebas/docs/doc1",1);
    mostrar_buscar_entrada("/pruebas/casos/",1);
    mostrar_buscar_entrada("/pruebas/docs/doc2",1); 
    #endif

    printf("\n");
    bumount();
    return EXIT_SUCCESS;
}

void printsuperbloque(struct superbloque SB)
{
    printf("-------------------------------SUPERBLOQUE----------------------------------");
    printf("\n\t\t\tPos Pri Block MB: %d", SB.posPrimerBloqueMB);
    printf("\n\t\t\tPos Ult Block MB: %d", SB.posUltimoBloqueMB);
    printf("\n\t\t\tPos Pri Block AI: %d", SB.posPrimerBloqueAI);
    printf("\n\t\t\tPos Ult Block AI: %d", SB.posUltimoBloqueAI);
    printf("\n\t\t\tPos Pri Data Block: %d", SB.posPrimerBloqueDatos);
    printf("\n\t\t\tPos Ult Data Block: %d", SB.posUltimoBloqueDatos);
    printf("\n\t\t\tPos inodo root: %d", SB.posInodoRaiz);
    printf("\n\t\t\tPos Pri inodo free: %d", SB.posPrimerInodoLibre);
    printf("\n\t\t\tQtt block free: %d", SB.cantBloquesLibres);
    printf("\n\t\t\tQtt inodo free: %d", SB.cantInodosLibres);
    printf("\n\t\t\tTotal blocks: %d", SB.totBloques);
    printf("\n\t\t\tTotal Inodos: %d", SB.totInodos);
}

void imprimir_inodo(int ninodo){
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    struct inodo i;
    leer_inodo(ninodo,&i);
    ts = localtime(&i.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&i.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&i.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("\nDATOS DEL INODO RESERVADO %d", ninodo);
    printf("\ntipo: %c", i.tipo);
    printf("\npermisos: %d", i.permisos);
    printf("\natime: %s",atime);
    printf("\nctime: %s",ctime);
    printf("\nmtime: %s",mtime);
    printf("\nnlinks: %d",i.nlinks);
    printf("\ntamEnBytesLog: %d",i.tamEnBytesLog);
    printf("\nnumBloquesOcupados: %d", i.numBloquesOcupados);
}

void mostrar_buscar_entrada(char *camino, char reservar)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) mostrar_error_buscar_entrada(error);
    printf("********************************************************************************\n");
}