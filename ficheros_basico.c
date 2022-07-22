// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros_basico.h"

/**
 * @brief Calcula el tamaño de bloques necesarios para el mapa de bits.
 *
 * @param nbloques
 * @return int
 */
int tamMB(unsigned int nbloques)
{
    int bitSize = 8 * BLOCKSIZE;
    int residuo = nbloques % bitSize;
    int nBloquesMB;
    if (residuo != 0)
    {
        nBloquesMB = (nbloques / bitSize) + 1;
    }
    else
    {
        nBloquesMB = nbloques / bitSize;
    }
    return nBloquesMB;
}

/**
 * @brief Calcula el tamaño en bloques del array de inodos.
 *
 * @param ninodos
 * @return int
 */
int tamAI(unsigned int ninodos)
{
    int tamAI;
    int bytes_totales = ninodos * INODOSIZE;
    int residuo = bytes_totales % BLOCKSIZE;
    tamAI = bytes_totales / BLOCKSIZE;
    if (residuo != 0)
    {
        tamAI++;
    }
    return tamAI;
}
/**
 * @brief Inicializa los datos del superbloque.
 *
 * @param nbloques
 * @param ninodos
 * @return int
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    // Comienzo de la inicialización
    struct superbloque *SB = malloc(sizeof(struct superbloque));
    if (SB == NULL)
    {
        perror("Error ");
        return -1;
    }

    SB->posPrimerBloqueMB = posSB + tamSB;
    SB->posUltimoBloqueMB = SB->posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB->posPrimerBloqueAI = SB->posUltimoBloqueMB + 1;
    SB->posUltimoBloqueAI = SB->posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB->posPrimerBloqueDatos = SB->posUltimoBloqueAI + 1;
    SB->posUltimoBloqueDatos = nbloques - 1;
    SB->posInodoRaiz = 0;
    SB->posPrimerInodoLibre = 0;
    SB->cantBloquesLibres = nbloques;
    SB->cantInodosLibres = ninodos;
    SB->totBloques = nbloques;
    SB->totInodos = ninodos;
    // fin de inicialización.
    if (bwrite(posSB, SB) == -1)
    {
        perror("Error en initSB: ");
        return -1;
    }
    free(SB);
    return EXIT_SUCCESS;
}
/**
 * @brief Inicializa el mapa de bits. Los bits a 1 marcan los bloques en uso.
 *
 * @return int
 */
int initMB()
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) // Leemos el superbloque.
    {
        perror("Error en initMB: ");
        return -1;
    }
    int bloques_ocupados = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);
    int bytes_ocupados = bloques_ocupados / 8;    // Cada bloque ocupado se representa en bits, por lo que lo pasamos a bytes.
    int bits_ocupados_res = bloques_ocupados % 8; // En caso de que haya bits restantes de la conversión, tenerlos en cuenta.
    int punteroMB = SB.posPrimerBloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0xFF, BLOCKSIZE);
    while (bytes_ocupados >= BLOCKSIZE) // Rellena todos los bloques de 1024 bytes con un 1 en caso
    {                                   // de que los bytes a marcar sea mayor que el tamaño de bloque.
        if (bwrite(punteroMB, bufferMB) == -1)
        {
            perror("Fallo en initMB: ");
            return -1;
        }
        punteroMB++;
        bytes_ocupados -= BLOCKSIZE;
    }
    memset(bufferMB, 0, BLOCKSIZE);         // Una vez rellenado los bloques completos, solo falta marcar los
    memset(bufferMB, 0xFF, bytes_ocupados); // bytes del último bloque (en este caso,solo una parte).
    char ultimo_byte = 0;
    unsigned char bit_significativo = 0x80; // Bit mas significativo a uno. Usado para poner a uno los bits del ultimo byte.
    while (bits_ocupados_res != 0)          // En caso de que haya bits restantes, marcarlos en el caracter adicional.
    {
        ultimo_byte |= bit_significativo; // Ponemos a 1 el bit correspondiente.
        bit_significativo = bit_significativo >> 1;
        bits_ocupados_res--;
    }
    bufferMB[bytes_ocupados] = ultimo_byte; // Es la penúltima posición + 1.
    SB.cantBloquesLibres = SB.cantBloquesLibres - bloques_ocupados;
    if (bwrite(SB.posPrimerBloqueMB, bufferMB) == -1 || bwrite(posSB, &SB) == -1) // Actualizamos el SB y el MB.
    {
        perror("Error escribiendo el MB o el SP en initMB(): ");
        return -1;
    }
    return EXIT_SUCCESS;
}
/**
 * @brief Se encarga de inicializar la lista de inodos libres.
 *
 * @return int
 */
int initAI()
{
    struct superbloque SB;
    if (bread(posSB, &SB) != -1) // Leemos el superbloque para revisar los datos.
    {
        int inodos_en_bloque = BLOCKSIZE / INODOSIZE;
        struct inodo inodos[BLOCKSIZE / INODOSIZE];
        int contInodos = SB.posPrimerInodoLibre + 1;
        for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) // Para cada bloque del AI
        {
            // Leer bloque.
            if (bread(i, inodos) == -1)
            {
                perror("Error en initAI: ");
                return -1;
            }

            // leer bloque de inodos y del dispositivo virtual.
            for (size_t j = 0; j < inodos_en_bloque; j++)
            {
                inodos[j].tipo = 'l';
                if (contInodos < SB.totInodos)
                {
                    inodos[j].punterosDirectos[0] = contInodos;
                    contInodos++;
                }
                else
                {
                    inodos[j].punterosDirectos[0] = UINT_MAX;
                    break;
                }
            }
            // Escribir el bloque de inodos i en el dispositivo virtual.
            if (bwrite(i, inodos) == -1)
            {
                perror("Error en initAI: ");
                return -1;
            }
        }
    }
    return EXIT_SUCCESS;
}

// Fin nivel 2 y comienzo del nivel 3

/**
 * @brief Modifica la posición del nbloque en el MB según el valor de bit. También modifica el SB.
 * Devuelve -1 en caso de fallar.
 * @param nbloque
 * @param bit
 * @return int
 */
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en escribir bit(): ");
        return -1;
    }
    // Buscamos el bloque del MB.
    char bloque[BLOCKSIZE];
    int posbyte = nbloque / 8;             // Pasamos el numero de bit a numero de byte.
    int bloqueRelMB = posbyte / BLOCKSIZE; // Sacamos la pos relativa del bloque en el MB.
    posbyte = posbyte % BLOCKSIZE;         // Sacamos el modulo para buscar el byte dentro del bloque.
    int posBit = nbloque % 8;              // Sacamos la posicion del bit.
    int bloqueMB = bloqueRelMB + SB.posPrimerBloqueMB;
    if (bread(bloqueMB, bloque) == -1) // Leemos el bloque del MB.
    {
        perror("Error en escribir_bit():");
        return -1;
    }
    unsigned char mascara = 0x80 >> posBit; // Creamos la mascara, desplazando el bit hacia la derecha.
    if (bit)
    {
        bloque[posbyte] |= mascara; // Caso de poner el bit a uno.
        SB.cantBloquesLibres--;
    }
    else
    {
        bloque[posbyte] &= ~mascara; // Caso de poner el bit a cero.
        SB.cantBloquesLibres++;
    }

    if (bwrite(bloqueMB, bloque) == -1 || bwrite(posSB, &SB) == -1) // Reescribimos el bloque de MB y de SB.
    {
        perror("Error en escribir_bit():");
        return -1;
    }
    return EXIT_SUCCESS;
}
/**
 * @brief Lee el bit del MB respectivo al nbloque.
 *
 * @param nbloque
 * @return char
 */
char leer_bit(unsigned int nbloque)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en leer_bit()");
        return -1;
    }

    // Buscamos el bloque del MB.
    char bloque[BLOCKSIZE];
    int posbyte = nbloque / 8;             // Pasamos el numero de bit a numero de byte.
    int bloqueRelMB = posbyte / BLOCKSIZE; // Sacamos la pos relativa del bloque en el MB.
    posbyte = posbyte % BLOCKSIZE;         // Sacamos el modulo para buscar el byte dentro del bloque.
    int posBit = nbloque % 8;              // Sacamos la posicion del bit.
    int bloqueMB = bloqueRelMB + SB.posPrimerBloqueMB;
    if (bread(bloqueMB, bloque) == -1)
    {
        perror("Error en leer_bit()");
        return -1;
    }
    unsigned char mascara = 0x80 >> posBit; // Creamos la mascara y la desplazamos a la posición del bit a leer.
    mascara &= bloque[posbyte];             // Hacemos un AND para ver que valor tiene (si 0 o 1).
    mascara >>= (7 - posBit);               // Desplazamos el resultado al bit menos significativo.
    return mascara;
}
/**
 * @brief Encuentra el primer bloque libre y lo reserva, devolviendo así su posición.
 * En caso de error devuelve -1;
 *
 * @return int
 */
int reservar_bloque()
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en reservar_bloque()");
        return -1;
    }
    char bloque[BLOCKSIZE];
    char bloque_lleno[BLOCKSIZE];
    memset(bloque_lleno, 0xFF, BLOCKSIZE);
    int posBloqueMB;
    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++)
    {
        if (bread(i, bloque) == -1)
        {
            perror("Error en reservar_bloque()");
            return -1;
        }
        if (memcmp(bloque_lleno, bloque, BLOCKSIZE)) // Si el bloque no esta lleno, hay un bloque libre.
        {
            posBloqueMB = i;
            break;
        }
    }
    int posByte;
    // Localizamos el byte que tiene el primer bloque libre.
    for (size_t i = 0; i < BLOCKSIZE; i++)
    {
        if (bloque[i] != (char)0xFF)
        {
            posByte = i;
            break;
        }
    }
    int posBit = 0;
    unsigned char mascara = 0x80;
    while (bloque[posByte] & mascara) // Localizamos el bit del primer bloque libre.
    {
        mascara >>= 1;
        posBit++;
    }
    int nbloque = posBit + ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posByte) * 8;
    if (escribir_bit(nbloque, 1))
    {
        perror("Error en reservar_bloque()");
        return -1;
    }

    return nbloque;
}
/**
 * @brief Libera el nbloque.
 *
 * @param nbloque
 * @return int
 */
int liberar_bloque(unsigned int nbloque)
{
    if (escribir_bit(nbloque, 0) == -1)
    {
        perror("Error en liberar_bloque()");
        return -1;
    }
    return nbloque;
}
/**
 * @brief Escribe el contenido de una variable de tipo inodo en un determinado inodo del array de inodos.
 * Si falla, devuelve -1. En caso contrario, devuelve 0
 * @param ninodo
 * @param inodo
 * @return int
 */
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en escribir_inodo()");
        return -1;
    }
    int numInodosBloque = BLOCKSIZE / INODOSIZE;
    struct inodo inodos[numInodosBloque];
    int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / numInodosBloque); // Encontramos el bloque de inodos a modificar.
    int posInodo = ninodo % numInodosBloque;                                // Encontramos que inodo del bloque modificar.
    if (bread(posBloqueInodo, inodos) == -1)                                // Leemos del array de inodos los inodos del bloque.
    {
        perror("Error en escribir_inodo()");
        return -1;
    }
    inodos[posInodo] = inodo;                 // Reescribimos el inodo a la posición correspondiente.
    if (bwrite(posBloqueInodo, inodos) == -1) // Reescribimos el bloque modificado.
    {
        perror("Error en escribir_inodo()");
        return -1;
    }
    return EXIT_SUCCESS;
}
/**
 * @brief Lee un determinado inodo del array de inodos y lo vuelca en el inodo pasado por referencia.
 * En caso de error devuelve -1.
 * @param ninodo
 * @param inodo
 * @return int
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en leer_inodo()");
        return -1;
    }
    int numInodosBloque = BLOCKSIZE / INODOSIZE;
    struct inodo inodos[numInodosBloque];
    int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / numInodosBloque); // Encontramos el bloque de inodos.
    int posInodo = ninodo % numInodosBloque;                                // Encontramos que inodo del bloque deseamos copiar.
    if (bread(posBloqueInodo, inodos) == -1)                                // Leemos del array de inodos los inodos del bloque.
    {
        perror("Error en leer_inodo()");
        return -1;
    }
    *inodo = inodos[posInodo]; // Volcamos los valores del inodo al puntero.
    return EXIT_SUCCESS;
}
/**
 * @brief Encuentra el primer inodo libre, lo reserva, devuelve su número y actualiza la lista de inodos libres.
 * En caso de error devuelve -1.
 *
 * @param tipo
 * @param permisos
 * @return int
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en reservar_inodo() ");
        return -1;
    }
    // Comprobar si hay inodos libres.
    if (SB.posPrimerInodoLibre == -1)
    {
        perror("Error en reservar_inodo() ");
        return -1;
    }
    int posInodoReservado = SB.posPrimerInodoLibre;
    int ninodosEnBloque = BLOCKSIZE / INODOSIZE;
    struct inodo inodos[ninodosEnBloque];
    int posAbsBloqueInodos = SB.posPrimerBloqueAI + (SB.posPrimerInodoLibre / ninodosEnBloque); // Localizamos el bloque del inodo.
    int posInodoLibre = SB.posPrimerInodoLibre % ninodosEnBloque;                               // Localizamos la posición del inodo libre en el bloque.
    if (bread(posAbsBloqueInodos, inodos) == -1)                                                // Leemos los inodos.
    {
        perror("Error en reservar_inodo() ");
        return -1;
    }

    SB.posPrimerInodoLibre = inodos[posInodoLibre].punterosDirectos[0]; // Actualizamos el nuevo primer inodo libre.
    time_t t = time(NULL);
    struct inodo inodo = {
        // Declaramos el nuevo inodo con su tipo y permiso.
        .tipo = tipo,
        .permisos = permisos,
        .nlinks = 1,
        .tamEnBytesLog = 0,
        .atime = t,
        .ctime = t,
        .mtime = t,
        .numBloquesOcupados = 0,
        .punterosDirectos = {0},
        .punterosIndirectos = {0},
    };

    if (escribir_inodo(posInodoReservado, inodo) == -1) // Escribimos el inodo en su posición.
    {
        perror("Error en reservar_inodo()");
        return -1;
    }
    SB.cantInodosLibres--;        // Decrementamos la cantidad de inodos libres en el superbloque.
    if (bwrite(posSB, &SB) == -1) // Reescribimos el SB.
    {
        perror("Error en reservar_inodo() ");
        return -1;
    }
    return posInodoReservado;
}
/**
 * @brief Devuelve el indice de los bloques de punteros.
 *
 * @param nblogico
 * @param nivel_punteros
 * @return int
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0)
    {
        return nblogico - DIRECTOS;
    }
    else if (nblogico < INDIRECTOS1)
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2)
    {

        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return -1; // Error en caso de llegar aquí.
}
/**
 * @brief Devuelve el nivel en el que se encuentra el bloque logico. En caso de error, devuelve -1.
 *
 * @param inodo
 * @param nblogico
 * @param ptr
 * @return int
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{

    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        perror("Bloque lógico fuera de rango");
        return -1;
    }
}
/**
 * @brief Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 * Enmascara la gestión de los diferentes rangos de punteros directos e indirectos del inodo, de manera que
 * funciones externas no tienne que preocuparse de cómo acceder a los bloques físicos apuntados desde el inodo.
 *
 * @param ninodo
 * @param nblogico
 * @param reservar
 * @return int
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar)
{
    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    if (leer_inodo(ninodo, &inodo) == -1) // Leemos el inodo correspondiente.
    {
        perror("Error en traducir_bloque_inodo ");
        return -1;
    }
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;
    while (nivel_punteros > 0) // Si encontramos que hay un nivel de punteros superior al 0, sacamos el bloque
    {
        if (ptr == 0) // Si no cuelgan bloques de punteros
        {
            if (reservar == 0)
                return -1; // bloque inexistente.
            salvar_inodo = 1;
            ptr = reservar_bloque(); // reservamos un bloque de punteros.
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);       // Actualizamos la fecha
            if (nivel_punteros == nRangoBL) // Si el bloque está en el inodo
            {
                #if DEBUGNIVEL4
                printf("\n[traducir_bloque_inodo()-> inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel %d)]", (nRangoBL - 1), ptr, ptr, nivel_punteros); // Imprimimos el nº del bloque.
                #endif
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
            }
            else
            {
                buffer[indice] = ptr;
                #if DEBUGNIVEL4
                printf("\n[traducir_bloque_inodo()-> punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel %d)]", (nivel_punteros + 1), indice, ptr, ptr, nivel_punteros); // Imprimimos el nº del bloque.
                #endif
                if (bwrite(ptr_ant, buffer) == -1)
                    return -1;
            }
            memset(buffer, 0, BLOCKSIZE); // Ponemos a 0 todos los punteros del buffer
        }
        else if (bread(ptr, buffer) == -1)
            return -1;                                     // en caso de que ya exista el bloque, lo leemos.
        indice = obtener_indice(nblogico, nivel_punteros); // Obtenemos el indice de donde se encuentra nuestro bloque
        ptr_ant = ptr;                                     // Guardamos el puntero actual.
        ptr = buffer[indice];                              // y lo desplazamos al siguiente nivel
        nivel_punteros--;                                  // Decrementamos un nivel al haber navegado en el nivel.
    }
    if (ptr == 0) // Si no existe el bloque de datos
    {
        if (reservar == 0)
            return -1;              // Y reservar vale 0, error de lectura.
        salvar_inodo = 1;           // Sinó, guardaremos el inodo.
        ptr = reservar_bloque();    // reservamos el bloque de datos.
        inodo.numBloquesOcupados++; // Aumenta el numero de bloques que ocupa.
        inodo.ctime = time(NULL);   // Ponemos la fecha de modificación de datos.
        if (nRangoBL == 0)
        {
            #if DEBUGNIVEL4
            printf("\n[traducir_bloque_inodo()-> inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]", nblogico, ptr, ptr, nblogico);
            #endif
            inodo.punterosDirectos[nblogico] = ptr;
        }
        else
        {
            #if DEBUGNIVEL4
            printf("\n[traducir_bloque_inodo()-> punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]", (nivel_punteros + 1), indice, ptr, ptr, nblogico); // Imprimimos el nº del bloque.
            #endif
            buffer[indice] = ptr;                                                                                                                              // Asignamos la dirección del bloque de datos.
            if (bwrite(ptr_ant, buffer) == -1)
                return -1; // Guardamos el buffer de punteros modificados
        }
    }
    if (salvar_inodo == 1) // Si hemos actualizado el inodo, lo guardamos.
    {
        escribir_inodo(ninodo, inodo);
    }
    return ptr;
}

// nivel 6
/**
 * @brief Libera un inodo y actualiza la cantidad de inodos libres y el primer inodo libre
 *
 * @param ninodo
 * @return int
 */
int liberar_inodo(unsigned int ninodo)
{
    // Variables
    struct inodo inodo;
    int numBLib;
    struct superbloque SB;

    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error en leer_inodo(), liberar_inodo()\n");
        return -1;
    }
    // Sacamos el número de bloques liberados
    numBLib = liberar_bloques_inodo(0, &inodo);
    // Actualización y cambiamos de variables del inodo
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - numBLib;
    inodo.tamEnBytesLog = 0;
    inodo.tipo = 'l';

    if (bread(posSB, &SB) == -1) // Leemos el superbloque
    {
        fprintf(stderr, "Error en bread(), liberar_inodo()\n");
        return -1;
    }
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;

    // Escribimos el inodo
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        fprintf(stderr, "Error en escribir_inodo(), liberar_inodo()\n");
        return -1;
    }
    // Escribir el superbloque
    if (bwrite(posSB, &SB) == -1)
    {
        fprintf(stderr, "Error en bwrite(), liberar_inodo()\n");
        return -1;
    }

    return ninodo;
}

/**
 * @brief Libera todos los bloques ocupados a partir del bloque lógico primerBL (inclusive).
 *
 * @param primerBL
 * @param inodo
 * @return int
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    // Variables temporales--
    int nReads = 0, nWrites = 0;
    //--
    unsigned int nivel_punteros, ptr, nBL, ultimoBL, indice;
    int indices[3];
    int ptr_nivel[3];
    int nBLiberados = 0;
    // Para comprobar si en  un bloque índice al eliminar el puntero concreto ya no le quedan mas punteros ocupados y así liberar este bloque indice.
    unsigned char bufAux_punteros[BLOCKSIZE];
    unsigned int bloque_punteros[3][NPUNTEROS];
    if (inodo->tamEnBytesLog == 0) // En caso de que el fichero esté vacío, devolvemos 0.
    {
        return nBLiberados;
    }
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) // Miramos si ocupan todos el tamaño de un bloque.
    {
        ultimoBL = inodo->tamEnBytesLog / (BLOCKSIZE - 1);
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
    memset(bufAux_punteros, 0, BLOCKSIZE); // Para comparar si el bloque esta vacío más adelante.
    ptr = 0;
    int nRangoBL;
    #if NIVEL6
    fprintf(stderr, "[liberar_bloques_inodo()-> primerBL: %d,ultimoBL: %d]\n", primerBL, ultimoBL);
    #endif
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0)
            return -1;
        nivel_punteros = nRangoBL;
        while (ptr > 0 && nivel_punteros > 0) // Mientras cuelguen bloques de punteros
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if (!indice || nBL == primerBL) // Si el indice = 0 o observamos el primer bloque, leemos de disco el bloque de punteros.
            {
                if (bread(ptr, bloque_punteros[nivel_punteros - 1]) == -1)
                    return -1;
                nReads++;
            }
            // Almacenamos y actualizamos la dirección fisica del puntero y del indice del bloque logico.
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloque_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0) // Si existe el bloque de datos
        {
            liberar_bloque(ptr);
            nBLiberados++;
            #if NIVEL6
            fprintf(stderr, "[liberar_bloques_inodo()-> liberado BF %d de datos para BL %d]\n", ptr, nBL);
            #endif
            if (!nRangoBL) // Si es  un puntero directo
            {
                inodo->punterosDirectos[nBL] = 0;
            }
            else // Si es puntero indirecto
            {
                nivel_punteros = 1;
                #if NIVEL6
                int tmpBL = nBL;                   // Nos sirve para saber a la hora de imprimir de que bloque logico se trata la liberación.
                #endif
                while (nivel_punteros <= nRangoBL) // Si el bloque pertenece a un puntero indirecto, iteramos el arbol de bloques.
                {
                    indice = indices[nivel_punteros - 1];
                    bloque_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloque_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) // Si el bloque esta vacío, liberamos.
                    {
                        liberar_bloque(ptr);
                        nBLiberados++;
                        // Inicio de la mejora para saltar los bloques y no sea necesario explorar.
                        switch (nivel_punteros - 1)
                        {
                        case 0: // Caso de que estemos en el nivel 1.
                            nBL += NPUNTEROS - indices[nivel_punteros - 1] - 1;
                            break;
                        case 1: // Caso de que estemos en el nivel 2.
                            nBL += (NPUNTEROS * (NPUNTEROS - indices[nivel_punteros - 1]) - 1);
                            break;
                        case 2: // Caso de que estemos en el nivel 3.
                            nBL += ((NPUNTEROS * NPUNTEROS) * (NPUNTEROS - indices[nivel_punteros - 1]) - 1);
                            break;
                        default:
                            return -1;
                        }
                        // Fin de mejora para saltar los bloques para que no sea necesario explorar.
                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosDirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                        #if NIVEL6
                        fprintf(stderr, "[liberar_bloques_inodo()-> liberado BF %d de punteros nivel%d correspondiente al BL %d]\n", ptr, (nivel_punteros - 1), tmpBL);
                        #endif
                    }
                    else // Sinó, escribimos el bloque de punteros modificado.
                    {
                        if (bwrite(ptr, bloque_punteros[nivel_punteros - 1]) == -1)
                            return -1;
                        nWrites++;
                        // Salimos del bucle
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
        else
        {
            // Inicio Mejora de no explorar si ptr = 0
            switch (nivel_punteros - 1)
            {
            case 1: // Nivel 1
                nBL += NPUNTEROS - 1;
                break;
            case 2: // Nivel 2
                nBL += NPUNTEROS * NPUNTEROS - 1;
                break;
            case 3: // Nivel 3
                nBL += NPUNTEROS * NPUNTEROS * NPUNTEROS - 1;
                break;
            default:
                break;
            }
            // Fin inicio mejor de no explorar si ptr = 0
        }
    }
    #if NIVEL6
    fprintf(stderr, "[liberar_bloques_inodo()-> total bloques liberados: %d total breads: %d, total_bwrites: %d]\n", nBLiberados, nReads, nWrites);
    #endif
    return nBLiberados;
}