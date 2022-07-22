// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "bloques.h"

static int descriptor = 0; // variable global que tiene el id del file descriptor
static sem_t *mutex;
static unsigned int inside_sc = 0; // Para codigo reentrante
// Monta un dispositivo virtual y lo abre. Devuelve el id del descriptor y en caso de error, EXIT_FAILURE o -1.
int bmount(const char *camino)
{
    if(descriptor > 0)
    {
        close(descriptor);
    }

    if (!mutex)
    {
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            perror("Error: mutex error");
            return -1;
        }
    }

    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, PERMISOSRDWR);
    // Si no se ha podido abrir, sera igual a -1.
    if (descriptor == -1)
    {
        perror("Error: ");
        return -1;
    }
    return descriptor;
}

/**
 * @brief Desmonta el dispositivo virtual. Devuelve 0 (EXIT_SUCCESS) si ha sido un éxito y -1 (EXIT_FAILURE) en caso contrario.
 *        Eliminamos el semáforo.
 *
 * @return int
 */
int bumount()
{
    descriptor = close(descriptor);
    // Si no ha podido cerrar el fichero, entonces devuelve EXIT_FAILURE.
    if (close(descriptor) == -1)
    {
        perror("Error: ");
        return -1;
    }
    deleteSem();
    return EXIT_SUCCESS;
}

int bwrite(unsigned int nbloque, const void *buf)
{
    size_t pos = nbloque * BLOCKSIZE;
    if (lseek(descriptor, pos, SEEK_SET) == -1) // Posicionamos el puntero al offset calculado.
    {                                           // Si da error, devolvemos -1 y notificamos.
        perror("Error: ");
        return -1;
    }
    ssize_t bytes_escritos = write(descriptor, buf, BLOCKSIZE); // Escribimos los datos en el bloque de tamaño BLOCKSIZE.
    if (bytes_escritos == -1)                                   // Si no se ha escrito nada en el bloque, notificamos y devolvemos -1.
    {
        perror("Error: ");
        return -1;
    }
    return bytes_escritos; // Devolvemos la cantidad de bytes escritos.
}

int bread(unsigned int nbloque, void *buf)
{
    size_t pos = nbloque * BLOCKSIZE;
    if (lseek(descriptor, pos, SEEK_SET) == -1) // Posicionamos el puntero al offset calculado.
    {
        perror("Error: ");
        return -1;
    }
    ssize_t bytes_leidos = read(descriptor, buf, BLOCKSIZE); // Leemos el bloque de tamaño BLOCKSIZE y lo volcamos en el buffer.
    if (bytes_leidos == -1)                                  // Si no se ha podido leer, notificamos y devolvemos -1.
    {
        perror("Error: ");
        return -1;
    }
    return bytes_leidos; // En caso de leerse, devolvemos el numero de bytes leídos.
}

void mi_waitSem()
{
    if (!inside_sc)
        waitSem(mutex);
    inside_sc++;
}

void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
        signalSem(mutex);
}
