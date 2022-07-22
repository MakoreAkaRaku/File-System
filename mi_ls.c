// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"

int main(int n, char *args[])
{
    char *buffer;
    if (n != 3 && n != 4)
    {
        printf("Error de sintaxis; ./mi_ls <disco> </ruta_directorio> |parametro|");
        return -1;
    }
    int num_entradas= 0;
    int es_simple = 1;

    if (n == 4 && strcmp(args[3], "-l") == 0)
    {
        es_simple = 0;
    }else if (n == 4){
        printf("Error de sintaxis; el parámetro no es -l");
        return 1;
    }
    if (bmount(args[1]) == -1)
    {
        printf("Error; no existe el disco\n");
        return 1;
    }
    buffer = malloc(TAMBUFFER); // Situa el buffer en la memoria
    num_entradas = mi_dir(args[2],buffer,es_simple);
    bumount();
    //En caso de error, salimos.
    if (num_entradas == -1){
        return 1;
    }
    // En caso de ser un directorio, mostramos el total de entradas.
    if (args[2][strlen(args[2]) - 1] == '/')
    {
        fprintf(stderr, "Total :%d\n", num_entradas);
    }
    if (num_entradas != 0)
    {
        if (!es_simple)
        {
            fprintf(stderr, "Tipo\tPermisos\tmTime\t\tTamaño\tNombre\n");
        }else{
            fprintf(stderr,"Nombres entradas\n");
        }
        fprintf(stderr, "-----------------------------------------------------------------------------\n");
        fprintf(stderr, "%s\n", buffer);
    }
}