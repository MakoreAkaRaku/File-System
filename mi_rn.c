// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, char *args[])
{
    if(n != 4)
    {
        printf("Error de sintaxis, uso correcto → ./mi_rn <disco> </ruta/antiguo> <nuevo>");
        return 1;
    }
    bmount(args[1]);
    mi_rn(args[2],args[3]);
    bumount();
}