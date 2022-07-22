// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros.h"

int main(int n, char *args[])
{
    if (n != 4)
    {
        printf("\nError de Sintaxis: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>");
        printf("\nOffsets: 9000, 209000, 30725000, 409605000, 480000000");
        printf("\nSi diferentes_inodos=0, se reserva un solo inodo para todos los offsets\n");
        return -1;
    }

    int ninodo;
    int length;
    int nbytes;
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};
    struct STAT st;

    if (bmount(args[1]) == -1)
    {
        perror("Error en el bmount() de escribir.c");
        return -1;
    }

    length = strlen(args[2]);
    char buffer[length];
    strcpy(buffer, args[2]);
    printf("\nLongitud de texto: %d\n", length);
    if (atoi(args[3]) == 0)
    {
        ninodo = reservar_inodo('f', 6);
        if (ninodo == -1)
            return -1;
        for (int x = 0; x < 5; x++)
        {
            printf("\nNº inodo reservado: %d", ninodo);
            printf("\nOffset: %d", offsets[x]);
            nbytes = mi_write_f(ninodo, buffer, offsets[x], length);
            if (nbytes == -1)
                return -1;
            printf("\nBytes escritos: %d", nbytes);
            if (mi_stat_f(ninodo, &st) == -1)
                return -1;
            printf("\nstat.tamEnBytesLog = %d", st.tamEnBytesLog);
            printf("\nstat.numBloquesOcupados = %d\n", st.numBloquesOcupados);
        }
    }
    else
    {
        for (size_t i = 0; i < 5; i++)
        {
            ninodo = reservar_inodo('f', 6);
            if (ninodo == -1)
                return -1;
            printf("\nNº inodo reservado: %d", ninodo);
            printf("\nOffset: %d", offsets[i]);
            nbytes = mi_write_f(ninodo, buffer, offsets[i], length);
            if (nbytes == -1)
                return -1;
            printf("\nBytes escritos: %d", nbytes);
            if (mi_stat_f(ninodo, &st) == -1)
                return -1;
            printf("\nstat.tamEnBytesLog = %d", st.tamEnBytesLog);
            printf("\nstat.numBloquesOcupados = %d\n", st.numBloquesOcupados);
        }
    }
    bumount();
}