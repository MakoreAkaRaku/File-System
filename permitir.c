// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros.h"

int main(int n, char *args[])
{
    if (n != 4)
    {
        printf("Error de sintaxis; permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return 1;
    }
    bmount(args[1]);
    mi_chmod_f(atoi(args[2]), *args[3]);
    printf("\n# # # # Cambiamos los permisos del inodo %d a %d # # # # #\n", atoi(args[2]), atoi(args[3]));
    bumount();
    return 0;
}