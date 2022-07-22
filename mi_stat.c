// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"
int main(int n, const char *args[])
{
    if (n != 3)
    {
        printf("Error de sintaxis, uso correcto -> ./mi_stat <disco> </ruta>");
        return 1;
    }
    if (bmount(args[1]) == -1)
        return 1;
    struct STAT stat;
    int num_inodo;
    if ((num_inodo = mi_stat(args[2], &stat))==-1)
        return 1;
    struct tm *ts;
    char atime[80];
    char ctime[80];
    char mtime[80];
    printf("Nº de Inodo: %d\n", num_inodo);
    printf("tipo : %c\n", stat.tipo);
    printf("permisos : %d\n", stat.permisos);
    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime : %s\n", atime);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ctime : %s\n", ctime);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("mtime : %s\n", mtime);
    printf("nlinks : %u\n", stat.nlinks);
    printf("tamEnBytesLog : %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados : %d\n", stat.numBloquesOcupados);
    bumount();
}