// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros_basico.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Error: debe tener 3 argumentos.\nEjemplo: ./mi_mkfs <nombre_dispositivo> <tamaño_disco>");
        return EXIT_FAILURE;
    }

    int nbloques = atoi(argv[2]);
    if (bmount(argv[1]) == -1)
    {
        perror("Error: ");
        return EXIT_FAILURE;
    }
    int ninodos = nbloques / 4;
    unsigned char *datos = malloc(sizeof(char) * BLOCKSIZE);
    memset(datos, 0, BLOCKSIZE);
    for (size_t i = 0; i < nbloques; i++)
    {
        if (bwrite(i, datos) != BLOCKSIZE)
        {
            perror("Error: ");
            break;
        }
    }
    if (initSB(nbloques,ninodos)==-1)
    {
        perror("Error inicializando el SB: ");
        return EXIT_FAILURE;
    }
    if (initMB()==-1)
    {
        perror("Error inicializando el MB: ");
        return EXIT_FAILURE;
    }
    if (initAI()==-1)
    {
        perror("Error inicializando el AI: ");
        return EXIT_FAILURE;
    }
    reservar_inodo('d',7);
    bumount();
    free(datos);
    return EXIT_SUCCESS;
}