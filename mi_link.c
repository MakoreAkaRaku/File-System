// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, char *args[])
{
    if (n != 4)
    {
        printf("Error de sintaxis, uso correcto → ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
        return 1;
    }
    // Si alguna de las rutas es un directorio, error.
    if (args[3][strlen(args[3]) - 1] == '/' || args[2][strlen(args[2]) - 1] == '/')
    {
        printf("Error, no se puede hacer un enlace de un directorio\n");
        return 1;
    }
    int error = 0;
    bmount(args[1]);
    error = mi_link(args[2], args[3]);
    bumount();
    return error;
}