// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, char *args[])
{
    if (n != 4)
    {
        printf("Error, sintaxis incorrecto, uso correcto → ./mi_mv <disco> </origen/nombre> </destino/>\n");
        return 1;
    }
    //Si las rutas no empiezan por el directorio raiz, error.
    if(args[3][strlen(args[3])-1] != '/'){
        printf("Error, el camino destino no es un directorio\n");
        return 1;
    }
    bmount(args[1]);
    int exit = mi_mv(args[2],args[3]);
    bumount();
    return exit;
}