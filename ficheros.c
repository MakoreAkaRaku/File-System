// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros.h"
/**
 * @brief Escribe un fichero/directorio de tamaño nbytes en el ninodo indicando
 * la posición de escritura inicial offset (en bytes).
 *
 * @param ninodo
 * @param buf_original
 * @param offset
 * @param nbytes
 * @return int
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo i;
    if (leer_inodo(ninodo, &i) == -1)
        return -1;
    if ((i.permisos & 2) != 2)
    {
        perror("No tiene permisos de escritura este directorio archivo");
        return 0;
    }
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned char buffer[BLOCKSIZE];
    int numBF;
    int bytes_leidos = 0;
    if (primerBL == ultimoBL)
    {
        mi_waitSem(); // Bloqueamos acceso por modificación del ctime y numBloquesOcupados.
        numBF = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem(); // Liberamos el acceso.
        if (bread(numBF, &buffer) == -1)
            return -1;
        memcpy((buffer + desp1), buf_original, nbytes);
        if (bwrite(numBF, buffer) == -1)
            return -1;
        bytes_leidos = nbytes;
    }
    else
    {
        mi_waitSem(); // Bloqueamos acceso por modificación del ctime y numBloquesOcupados.
        numBF = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem(); // Liberamos el acceso.
        if (bread(numBF, &buffer) == -1)
            return -1;
        // Escritura en el primer bloque
        memcpy((buffer + desp1), buf_original, (BLOCKSIZE - desp1));
        if (bwrite(numBF, buffer) == -1)
            return -1;
        bytes_leidos = (BLOCKSIZE - desp1);

        // Escritura entre el primer bloque y el último bloque
        for (int x = primerBL + 1; x < ultimoBL; x++)
        {
            mi_waitSem(); // Bloqueamos acceso por modificación del ctime y numBloquesOcupados.
            numBF = traducir_bloque_inodo(ninodo, x, 1);
            mi_signalSem(); // Liberamos el acceso.
            if (bwrite(numBF, (buf_original + (BLOCKSIZE - desp1) + (x - (primerBL + 1)) * BLOCKSIZE)) == -1)
                return -1;
            bytes_leidos += BLOCKSIZE;
        }

        mi_waitSem(); // Bloqueamos acceso por modificación del ctime y numBloquesOcupados.
        // Escritura en el último bloque
        numBF = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        mi_signalSem(); // Liberamos el acceso.
        if (bread(numBF, &buffer) == -1)
            return -1;
        memcpy(buffer, buf_original + (nbytes - (desp2 + 1)), (desp2 + 1));
        if (bwrite(numBF, buffer) == -1)
            return -1;
        bytes_leidos += desp2 + 1;
    }
    // Bloqueamos al actualizar la metainformación del inodo.
    mi_waitSem();
    // Actualizar metainformación del inodo
    if (leer_inodo(ninodo, &i) == -1)
    {
        mi_signalSem(); // Liberamos en caso de error para evitar deadlock.
        return -1;
    }
    time_t t = time(NULL);
    if ((offset + bytes_leidos) > i.tamEnBytesLog)
    {
        i.tamEnBytesLog = offset + bytes_leidos;
        i.ctime = t;
    }
    i.mtime = t;
    if (escribir_inodo(ninodo, i) == -1)
    {
        mi_signalSem(); // Liberamos en caso de error para evitar deadlock.
        return -1;
    }
    mi_signalSem();
    return bytes_leidos;
}
/**
 * @brief Lee nbytes de un fichero/directorio ninodo y la almacena en buf_original, indicando la posición
 * de lectura inicial offset con respecto al inodo (en bytes).
 *
 * @param ninodo
 * @param buf_original
 * @param offset
 * @param nbytes
 * @return int
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo i;
    if (leer_inodo(ninodo, &i) == -1)
        return -1;
    if ((i.permisos & 4) != 4)
    {
        // Tratar como error?
        perror("No tiene permisos de escritura este directorio archivo");
        return 0;
    }
    // La funcion no puede leer mas alla del EOF, por eso, controlamos si supera el EOF.
    int bytes_leidos = 0;
    if (offset >= i.tamEnBytesLog)
        return bytes_leidos;
    if ((offset + nbytes) >= i.tamEnBytesLog)
        nbytes = i.tamEnBytesLog - offset;

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned char buffer[BLOCKSIZE];
    int numBF;

    if (primerBL == ultimoBL)
    {
        numBF = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (numBF != -1)
        {
            if (bread(numBF, &buffer) == -1)
                return -1;
            memcpy(buf_original, (buffer + desp1), nbytes);
        }
        bytes_leidos = nbytes;
    }
    else
    {
        numBF = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (numBF != -1)
        {
            if (bread(numBF, &buffer) == -1)
                return -1;
            // Lectura del primer bloque
            // memcpy((buf_original  + desp1), buffer , (BLOCKSIZE - desp1));
            memcpy(buf_original, buffer + desp1, (BLOCKSIZE - desp1));
        }
        bytes_leidos = (BLOCKSIZE - desp1);
        // Lectura entre el primer bloque y el último bloque
        for (int x = primerBL + 1; x < ultimoBL; x++)
        {
            numBF = traducir_bloque_inodo(ninodo, x, 0);
            if (numBF != -1)
            {
                if (bread(numBF, &buffer) == -1)
                    return -1;
                memcpy((buf_original + (BLOCKSIZE - desp1) + (x - (primerBL + 1)) * BLOCKSIZE), buffer, BLOCKSIZE);
            }
            bytes_leidos += BLOCKSIZE;
        }

        // Lectura en el último bloque
        numBF = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (numBF != -1)
        {
            if (bread(numBF, &buffer) == -1)
                return -1;
            memcpy(buf_original + (nbytes - (desp2 + 1)), buffer, (desp2 + 1));
        }
        bytes_leidos += desp2 + 1;
    }
    mi_waitSem(); // Bloqueamos acceso por modificación del inodo.
    // Actualizar metainformación del inodo
    if (leer_inodo(ninodo, &i) == -1)
    {
        mi_signalSem(); // Liberamos en caso de error para evitar deadlock.
        return -1;
    }
    i.atime = time(NULL);
    if (escribir_inodo(ninodo, i) == -1)
    {
        mi_signalSem(); // Liberamos en caso de error para evitar deadlock.
        return -1;
    }
    mi_signalSem(); // Liberamos acceso.
    return bytes_leidos;
}
/**
 * @brief Vuelca el estado del ninodo al p_stat.
 * En caso de error, devuelve -1.
 *
 * @param ninodo
 * @param p_stat
 * @return int
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo i;
    if (leer_inodo(ninodo, &i) == -1)
        return -1;
    p_stat->tipo = i.tipo;
    p_stat->permisos = i.permisos;
    p_stat->atime = i.atime;
    p_stat->ctime = i.ctime;
    p_stat->mtime = i.mtime;
    p_stat->nlinks = i.nlinks;
    p_stat->tamEnBytesLog = i.tamEnBytesLog;
    p_stat->numBloquesOcupados = i.numBloquesOcupados;
    return EXIT_SUCCESS;
}

/**
 * @brief Cambia los permisos del inodo del fichero/directorio.
 * En caso de error, devuelve -1.
 *
 * @param ninodo
 * @param permisos
 * @return int
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    mi_waitSem(); // bloqueamos el acceso por modificar el access time y permisos.
    struct inodo i;
    if (leer_inodo(ninodo, &i) == -1)
    {
        mi_signalSem(); // Liberamos en caso de error para no crear un deadlock.
        return -1;
    }
    i.permisos = permisos;
    i.ctime = time(NULL);
    if (escribir_inodo(ninodo, i) == -1)
    {
        mi_signalSem(); // Liberamos en caso de error para no crear un deadlock.
        return -1;
    }
    mi_signalSem(); // Una vez leído, modificado y escrito el inodo, liberamos el acceso.
    return EXIT_SUCCESS;
}

/**
 * @brief Libera y borra los datos del ninodo a partir de nbytes.
 *
 * @param ninodo
 * @param nbytes
 * @return int
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    int nBLib, primerBL;
    struct inodo inodo;

    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        perror("Error en mi_truncar_f()");
        return -1;
    }
    if ((inodo.permisos & 2) != 2) // Comprueba si tiene permisos de escritura
    {
        perror("Error: No tiene permisos de escritura.");
        return -1;
    }

    // Calculamos el primer bloque lógico
    if (nbytes % BLOCKSIZE == 0)
    {
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    // Calculamos el número de bloques inodo liberados
    nBLib = liberar_bloques_inodo(primerBL, &inodo);
    if (nBLib == -1)
    {
        perror("Error en mi_truncar_f()");
        return -1;
    }
    // Actualizamos y cambiamos variables del inodo
    time_t t = time(NULL);
    inodo.ctime = t;
    inodo.mtime = t;
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - nBLib;
    /// NOTA: CREO QUE ESTO SOBRA
    /*if (!inodo.numBloquesOcupados)  //Si el inodo queda vacío, lo tachamos como libre.
    {
        inodo.tipo = 'l';
    }*/

    // Escribimos el inodo
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        fprintf(stderr, "Error en mi_truncar_f()");
        return -1;
    }
    // Devolver número de bloques inodo liberados
    return nBLib;
}