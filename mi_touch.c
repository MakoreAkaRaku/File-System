// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"
int main(int n, char *args[]){
    if (n != 4 )
    {
        printf("Error de sintaxis, uso correcto -> ./mi_touch <disco> <permiso> </ruta>");
    }
    int permisos = atoi(args[2]); // Pasamos de caracter a valor los permisos
    // Si los permisos no son entre 0-7, mostrar error.
    if (permisos > 7 || permisos < 0)
    {
        printf("Error, los permisos no estan en el rango 0-7\n");
        return 1;
    }
    if (bmount(args[1]) == -1)
    {
        printf("Error, no existe el disco %s\n", args[1]);
        return 1;
    }
    int length = strlen(args[3]);
    if (args[3][length-1] == '/')
    {
        printf("Error, no se puede crear un directorio con mi_touch\n");
        return 1;
    }
    mi_creat(args[3], permisos);
    bumount();
}