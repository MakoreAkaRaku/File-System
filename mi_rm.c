// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"
int main(int n, char *args[])
{
    if (n != 3 && n != 4)
    {
        printf("Error de sintaxis, uso correcto → ./mi_rm disco </ruta> <parametro>\n");
        return 1;
    }
    int borrado_recursivo;
    // Si se pide borrado recursivo para un directorio
    if (n == 4 && !strcmp(args[3], "-r"))
    {
        borrado_recursivo = 1;
    }
    else if (n != 4)
    {
        borrado_recursivo = 0;
    }
    else
    {
        printf("Error; parámetro no válido, usa -r para borrado recursivo\n");
        return 1;
    }

    bmount(args[1]);
    mi_unlink(args[2], borrado_recursivo);
    bumount(args[1]);
    return 0;
}