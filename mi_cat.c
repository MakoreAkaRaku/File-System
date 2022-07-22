// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, char *args[])
{
    if (n != 3)
    {
        printf("Error de sintaxis, uso correcto ➡ ./mi_cat <disco> </ruta_fichero>\n");
        return 1;
    }
    int length = strlen(args[2]);
    // Miramos si es un fichero o un directorio.
    if (args[2][length - 1] == '/')
    {
        printf("Error; no se puede leer un directorio\n");
        return 1;
    }
    if (bmount(args[1]) == -1)
    {
        printf("Error; no existe el disco\n");
        return 1;
    }
    unsigned char buffer[TAMBUFFER];
    int index = 0;
    int bytes_totales = 0;
    int bytes_leidos = 0;
    memset(buffer, 0x00, TAMBUFFER);
    while ((bytes_leidos = mi_read(args[2], buffer, index * TAMBUFFER, TAMBUFFER)) > 0)
    {
        write(1,buffer,bytes_leidos);    // o printf(buffer);
        memset(buffer, 0x00, TAMBUFFER);
        index++;
        bytes_totales += bytes_leidos;
    }
    bumount();
    if(bytes_totales != 0)
        fprintf(stderr,"\nBytes totales leídos: %d\n",bytes_totales);
    return 0;
}