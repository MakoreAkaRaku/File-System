// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, char *args[])
{
    if (n != 4)
    {
        fprintf(stderr, "Error de Sintaxis: ./mi_chmod.c <disco> <permisos> </ruta>\n");
        return -1;
    }
    //Comprobamos si el valor de permisos es un valor entre 0 o 7
    if ((atoi(args[2]) < 0) || (atoi(args[2]) > 7))
    {
        fprintf(stderr, "Error mi_chmod: Valor de permisos incorrecto\n");
        return -1;
    }
    if (bmount(args[1]) == -1)
    {
        printf("Error; no existe el disco\n");
        return 1;
    }
    mi_chmod(args[3], atoi(args[2]));
    bumount();
}