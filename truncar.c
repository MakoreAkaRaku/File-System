// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros.h"
void formateaTiempo(char* newTime,time_t t);

int main(int n, char *args[])
{   
    struct STAT st;

    if (n != 4)
    {
        fprintf(stderr,"Error de sintaxis; truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return 1;
    }
    bmount(args[1]);

    if(atoi(args[3]) == 0)
    {
        liberar_inodo(atoi(args[2]));
    }else
    {
        mi_truncar_f(atoi(args[2]),atoi(args[3]));
    }
    mi_stat_f(atoi(args[2]), &st);
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&st.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&st.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&st.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    fprintf(stderr, "DATOS INODO %d:\n",atoi(args[2]));
    fprintf(stderr, "tipo: %c\n", st.tipo);
    fprintf(stderr, "permisos: %d\n", st.permisos);
    fprintf(stderr, "atime: %s\n", atime);
    fprintf(stderr, "ctime: %s\n", ctime);
    fprintf(stderr, "mtime: %s\n", mtime);
    fprintf(stderr, "nlinks: %d\n", st.nlinks);
    fprintf(stderr, "tamEnBytesLog: %d\n", st.tamEnBytesLog);
    fprintf(stderr, "numBloquesOcupados: %d\n",st.numBloquesOcupados);
    bumount();
    return 0;
}