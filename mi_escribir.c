// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, const char *args[])
{
    if (n != 5)
    {
        printf("Error de sintaxis, uso correcto ➡ ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return 1;
    }
    int length = strlen(args[2]);
    if (args[2][length-1]== '/')
    {
        printf("Error; No se puede escribir en un directorio\n");
        return 1;
    }
    
    if (bmount(args[1]) == -1)
    {
        printf("Error; no existe el disco\n");
        return 1;
    }
    int bytes_escritos;
    length=strlen(args[3]);
    printf("Longitud texto: %d\n",length);
    if((bytes_escritos = mi_write(args[2], args[3], atoi(args[4]), length)) == -1){
        printf("Error; Fallo en la escritura\n");
        return -1;
    }
    printf("Bytes escritos: %d\n",bytes_escritos);
    bumount();
    return 0;
}