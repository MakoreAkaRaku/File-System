// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "directorios.h"
static struct UltimaEntrada entradas_en_cache_wr[TAMCACHE];
static struct UltimaEntrada entradas_en_cache_rd[TAMCACHE];

/**
 * @brief Separa el contenido en dos; Guarda en inicial la porcion de camino comprendida entre los dos primeros '/'.
 * Cuando no hay segundo '/', copia camino en inicial sin el primer '/'.
 * @param camino
 * @param inicial
 * @param tipo
 * @return int
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    char *tmp2;
    char *tmp = strchr(camino, '/');
    if (*(camino) != '/') // Si no hay el guion inicial, error.
    {
        return -1;
    }
    if ((tmp2 = strchr(tmp + 1, '/'))) // si es distinto de null
    {
        *tipo = 'd';
        int diff = tmp2 - (tmp + 1);
        strcpy(final, tmp2);             // Copiamos el resto del camino.
        strncpy(inicial, tmp + 1, diff); // Copiamos solo el primer directorio
    }
    else // Si no hay otro /, es un file.
    {
        *tipo = 'f';
        strcpy(inicial, tmp + 1); // Copiamos solo el nombre del archivo.
        strcpy(final, "");        // No queda nada deel final, por ende queda vacío el final.
    }
    return 0;
}

/**
 * @brief Según el valor de error, mostrará el tipo de error por pantalla que le corresponde.
 *
 * @param error
 */
void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    case -8:
        fprintf(stderr, "ERROR: Fallo en buscar_entrada().\n");
        break;
    }
}

/**
 * @brief Busca una determinada entrada entre las entradas del inodo correspondiente a su directorio padre.
 *
 * @param camino_parcial
 * @param p_inodo_dir
 * @param p_inodo
 * @param p_entrada
 * @param reservar
 * @param permisos
 * @return int
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[TAMNOMBRE];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    if (strcmp(camino_parcial, "/") == 0) // camino_parcial es “/”
    {
        *p_inodo = SB.posInodoRaiz; // nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;
        return 0;
    }
    memset(inicial, 0x00, TAMNOMBRE);
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUGNIVEL7
    fprintf(stderr, "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif
    // buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4) // Si el inodo no tiene permisos de lectura.
        return ERROR_PERMISO_LECTURA;
    struct entrada buffer_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(&entrada, 0x00, sizeof(entrada));
    memset(buffer_entradas, 0x00, BLOCKSIZE);
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); // cantidad de entradas que contiene el inodo
    int num_bloque_entrada = 0;
    num_entrada_inodo = 0; // nº de entrada inicial
    if (cant_entradas_inodo > 0)
    {
        while (num_entrada_inodo < cant_entradas_inodo)
        {
            if (mi_read_f(*p_inodo_dir, buffer_entradas, num_bloque_entrada * sizeof(buffer_entradas), sizeof(buffer_entradas)) == -1)
            {
                return -1;
            }
            int indice_buffer = 0;
            while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, buffer_entradas[indice_buffer].nombre) && indice_buffer < (sizeof(buffer_entradas) / sizeof(struct entrada)))
            {
                indice_buffer++;
                num_entrada_inodo++;
            }
            // Si hemos encontrado la entrada, la copiamos al struct entrada y salimos. REVISAR
            if (indice_buffer < (sizeof(buffer_entradas) / sizeof(struct entrada)) && !strcmp(inicial, buffer_entradas[indice_buffer].nombre))
            {
                memcpy(&entrada, &buffer_entradas[indice_buffer], sizeof(struct entrada));
                break;
            }
            // Si no la hemos encontrado, seguimos procesando por bloques
            num_bloque_entrada++;
            memset(buffer_entradas, 0x00, sizeof(buffer_entradas));
        }
    }
    if (strcmp(inicial, entrada.nombre)) // Si la entrada no existe
    {
        switch (reservar)
        {
        case 0: // modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: // modo escritura
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // Si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            // Si es directorio comprobar que tiene permisos de escritura.
            if ((inodo_dir.permisos & 2) != 2)
                return ERROR_PERMISO_ESCRITURA;
            else
            {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd') // es un directorio
                {
                    if (!strcmp(final, "/")) // Si son iguales
                    {
                        entrada.ninodo = reservar_inodo('d', permisos);
                        if (entrada.ninodo == -1)
                        {
                            return ERROR;
                        }
#if DEBUGNIVEL7
                        fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n", entrada.ninodo, permisos, inicial);
                        fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", inicial, entrada.ninodo);
#endif
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else // Es un fichero
                {
                    entrada.ninodo = reservar_inodo('f', permisos);
                    if (entrada.ninodo == -1)
                    {
                        return ERROR;
                    }
#if DEBUGNIVEL7
                    fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n", entrada.ninodo, permisos, inicial);
                    fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", inicial, entrada.ninodo);
#endif
                }
                // Escribir la entrada en el directorio padre.
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1)
                {
                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
                    }
                    return -1;
                }
            }
        }
    }
    // Si hemos llegado al final del camino
    if (!strcmp(final, "/") || !strcmp(final, ""))
    {
        if (num_entrada_inodo < cant_entradas_inodo && reservar == 1)
        {
            // Modo escritura y la entrada ya existente
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos recursividad
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return 0;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return 0;
}
/**
 * @brief Crea un fichero/directorio y su entrada de directorio.
 * Devuelve 0 si ha ido bien. En caso contrario devuelve -1 y muestra el error.
 * @param camino
 * @param permisos
 * @return int
 */
int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)))
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}

/**
 * @brief extrae el estado del directorio/fichero y devuelve su respectivo numero de inodo.
 * En caso de error, devuelve -1.
 *
 * @return int
 */
int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada = 0;
    int error;
    // Si hubo un error, no podemos saber el estado del
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)))
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (mi_stat_f(p_inodo, p_stat))
        return -1;
    return p_inodo;
}

/**
 * @brief Pone el contenido del directorio en un buffer de memoria y devuelve el número de entradas
 *
 * @param camino
 * @param buffer
 * @return int
 */
int mi_dir(const char *camino, char *buffer, unsigned int es_simple)
{
    // Variables necesarias
    struct STAT stat;
    struct entrada entrada;
    struct tm *tm;
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int num_entrada_inodo = 0, error, cant_entradas_inodo;
    char Tipo[2], tmp[50], tambl[50];
    // Realizamos buscar_entrada() y comprobamos que no haya salido algún error
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    // Lectura del directorio padre o del fichero.
    if (mi_stat_f(p_inodo, &stat) == -1)
    {
        fprintf(stderr, "ERROR mi_dir: fallo en el primer mi_stat_f().");
        return EXIT_FAILURE;
    }
    tm = localtime(&stat.mtime);
    if ((stat.permisos & 4) != 4)
    {
        fprintf(stderr, "ERROR mi_dir: No tiene permisos de lectura.");
        return EXIT_FAILURE;
    }

    if (stat.tipo != 'd')
    {
        // Montaje del buffer para mostrar un fichero
        strcat(buffer, CYN);
        if (!es_simple)
        {
            sprintf(Tipo, "%c", stat.tipo);
            strcat(buffer, Tipo);
            strcat(buffer, "\t");
            if (stat.permisos & 4)
                strcat(buffer, "r");
            else
                strcat(buffer, "-");
            if (stat.permisos & 2)
                strcat(buffer, "w");
            else
                strcat(buffer, "-");
            if (stat.permisos & 1)
                strcat(buffer, "x");
            else
                strcat(buffer, "-");
            strcat(buffer, "\t");
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1,
                    tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");
            sprintf(tambl, "%d", stat.tamEnBytesLog);
            strcat(buffer, tambl);
            strcat(buffer, "\t");
        }
        strcat(buffer, entrada.nombre);
        strcat(buffer, RST);
        strcat(buffer, "\n");
        num_entrada_inodo++;
    }
    else
    {
        // Bucle para la lectura de la entrada y montaje del buffer para mostrar un directorio
        cant_entradas_inodo = stat.tamEnBytesLog / sizeof(struct entrada);
        while (num_entrada_inodo < cant_entradas_inodo)
        {
            if (mi_read_f(p_inodo, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1)
            {
                fprintf(stderr, "ERROR mi_dir: mi_read_f()");
                return EXIT_FAILURE;
            }
            if (mi_stat_f(entrada.ninodo, &stat) == -1)
            {
                fprintf(stderr, "ERROR mi_dir: fallo en el segundo mi_stat_f().");
                return EXIT_FAILURE;
            }
            if (stat.tipo == 'f')
            {
                strcat(buffer, CYN);
            }
            else
            {
                strcat(buffer, RED);
            }
            if (!es_simple)
            {
                sprintf(Tipo, "%c", stat.tipo);
                strcat(buffer, Tipo);
                strcat(buffer, "\t");
                if (stat.permisos & 4)
                    strcat(buffer, "r");
                else
                    strcat(buffer, "-");
                if (stat.permisos & 2)
                    strcat(buffer, "w");
                else
                    strcat(buffer, "-");
                if (stat.permisos & 1)
                    strcat(buffer, "x");
                else
                    strcat(buffer, "-");
                strcat(buffer, "\t");
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1,
                        tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);
                strcat(buffer, "\t");
                sprintf(tambl, "%d", stat.tamEnBytesLog);
                strcat(buffer, tambl);
                strcat(buffer, "\t");
            }
            strcat(buffer, entrada.nombre);
            strcat(buffer, RST);
            strcat(buffer, "\n");
            num_entrada_inodo++;
        }
    }
    return num_entrada_inodo;
}

/**
 * @brief Busca la entrada camino y, en caso de que exista, llama a la función mi_chmod_f() de ficheros.c
 *
 * @param camino
 * @param permisos
 * @return int
 */
int mi_chmod(const char *camino, unsigned char permisos)
{
    // Variables necesarias
    unsigned int p_inodo_dir = 0, p_entrada, p_inodo;
    int error;
    // Realizamos buscar_entrada() y comprobamos que no haya salido algún error
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    // realiza la función de ficheros.c
    if (mi_chmod_f(p_inodo, permisos) == -1)
    {
        fprintf(stderr, "ERROR directorios.c: mi_chmod_f()");
        return -1;
    }
    return 0;
}
/**
 * @brief Vuelca en el buff los nbytes leídos del fichero especificado en el camino.
 * Devuelve la cantidad de bytes leídos.
 *
 * @param camino
 * @param buff
 * @param offset
 * @param nbytes
 * @return int
 */
int mi_read(const char *camino, void *buff, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;
    int posEntradaCache;
    if ((posEntradaCache = check_entradas_cache(camino, 1)) == -1)
    {
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)))
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }

#if NIVEL9
        printf("[mi_read() → Actualizamos la caché de lectura]\n");
#endif
        // Buscamos la entrada con menor prioridad y la sustituimos.
        int indiceEntrada = busca_entrada_cache_mp(1);
#if NIVEL9
        printf("Índice de cache de lectura que sobrescribimos: %d\n", indiceEntrada);
        printf("Contenido antiguo: camino:\t%s\tinodo:%d\tprioridad:%d\n",entradas_en_cache_rd[indiceEntrada].camino,entradas_en_cache_rd[indiceEntrada].p_inodo, entradas_en_cache_rd[indiceEntrada].prioridad);
#endif
        strcpy(entradas_en_cache_rd[indiceEntrada].camino, camino);
        entradas_en_cache_rd[indiceEntrada].p_inodo = p_inodo;
        entradas_en_cache_rd[indiceEntrada].prioridad = 0;
#if NIVEL9
        printf("Contenido nuevo: camino:\t%s\tinodo:%d\tprioridad:%d\n",entradas_en_cache_rd[indiceEntrada].camino,entradas_en_cache_rd[indiceEntrada].p_inodo, entradas_en_cache_rd[indiceEntrada].prioridad);
#endif
    }
    else
    {
#if NIVEL9
        printf("[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n");
#endif
        p_inodo = entradas_en_cache_rd[posEntradaCache].p_inodo;
        entradas_en_cache_rd[posEntradaCache].prioridad++;
    }
    int bytes_leidos = mi_read_f(p_inodo, buff, offset, nbytes);
    return bytes_leidos;
}
/**
 * @brief Escribe nbytes del buffer a partir del offset en el archivo especificado en el camino.
 *
 * @param camino
 * @param buf
 * @param offset
 * @param nbytes
 * @return int
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;
    int posEntradaCache;
    if ((posEntradaCache = check_entradas_cache(camino, 0)) == -1) // Si el inodo no está en las entradas en caché, actua normal.
    {
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)))
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }

#if NIVEL9
        printf("[mi_write() → Actualizamos la caché de escritura]\n");
#endif
        // Buscamos la entrada con menor prioridad y la sustituimos.
        int indiceEntrada = busca_entrada_cache_mp(0);
#if NIVEL9        
        printf("Índice de cache de escritura que sobrescribimos: %d\n", indiceEntrada);
        printf("Contenido antiguo: camino:\t%s\tinodo:%d\tprioridad:%d\n",entradas_en_cache_wr[indiceEntrada].camino,entradas_en_cache_wr[indiceEntrada].p_inodo, entradas_en_cache_wr[indiceEntrada].prioridad);
#endif
        strcpy(entradas_en_cache_wr[indiceEntrada].camino, camino);
        entradas_en_cache_wr[indiceEntrada].p_inodo = p_inodo;
        entradas_en_cache_wr[indiceEntrada].prioridad = 0;
#if NIVEL9
        printf("Contenido nuevo: camino:\t%s\tinodo:%d\tprioridad:%d\n",entradas_en_cache_wr[indiceEntrada].camino,entradas_en_cache_wr[indiceEntrada].p_inodo, entradas_en_cache_wr[indiceEntrada].prioridad);
#endif
    }
    else // Sinó simplemente lo saca de la caché.
    {

#if NIVEL9
        printf("[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n");
#endif
        p_inodo = entradas_en_cache_wr[posEntradaCache].p_inodo;
        entradas_en_cache_wr[posEntradaCache].prioridad++;
    }
    int bytes_escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    return bytes_escritos;
}

/**
 * @brief Chequea si el camino introducido es una entrada ya consultada, sea de escritura (is_rd = 0) o lectura.
 * Si lo es, devuelve el indice de la entrada. En caso contrario, devuelve -1.
 * @param camino
 * @return int
 */
int check_entradas_cache(const char *camino, int is_rd)
{

    int tamCache = (sizeof(entradas_en_cache_wr) / sizeof(struct UltimaEntrada));

    // si is_rd = 0, entonces buscamos por entradas_cache de escritura. En caso contrario a 0, buscamos por el de lectura.
    switch (is_rd)
    {
    case 0:
        for (int i = 0; i < tamCache; i++)
        {
            if (!strcmp(camino, entradas_en_cache_wr[i].camino))
            {
                return i;
            }
        }
        break;
    default:
        for (int i = 0; i < tamCache; i++)
        {
            if (!strcmp(camino, entradas_en_cache_rd[i].camino))
            {
                return i;
            }
        }
    }
    // En caso de no encontrarlo, devuelve -1;
    return -1;
}

/**
 * @brief Devuelve el indice de la entrada en caché con menor prioridad, sea de escritura (is_rd = 0) o lectura.
 *
 * @return int
 */
int busca_entrada_cache_mp(int is_rd)
{
    int tamEntradasCache = sizeof(entradas_en_cache_wr) / sizeof(struct UltimaEntrada);
    int indice = 0;
    int menor_prioridad = UINT_MAX;
    // Si is_rd = 0, entonces buscamos por las entradas_cache de escritura. En caso contrario, las de lectura.
    switch (is_rd)
    {
    case 0:
        for (int i = 0; i < tamEntradasCache; i++)
        {
            if (menor_prioridad > entradas_en_cache_wr[i].prioridad)
            {
                indice = i;
                menor_prioridad = entradas_en_cache_wr[i].prioridad;
            }
        }
        break;
    default:
        for (int i = 0; i < tamEntradasCache; i++)
        {
            if (menor_prioridad > entradas_en_cache_rd[i].prioridad)
            {
                indice = i;
                menor_prioridad = entradas_en_cache_rd[i].prioridad;
            }
        }
    }
    return indice;
}

/**
 * @brief Crea un enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1.
 * Devuelve 1 en caso de error, 0 si lo logra.
 * @param camino1
 * @param camino2
 * @return int
 */
int mi_link(const char *camino1, const char *camino2)
{
    struct inodo inodo;
    struct entrada entrada;
    int error;
    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0;
    unsigned int p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;
    mi_waitSem();
    // Verificamos que el fichero existe y sacamos su inodo.
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0)) != 0)
    {
        fprintf(stderr, "Error en buscar_entrada() con camino1;\n");
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return 1;
    }
    mi_signalSem();
    mi_waitSem();
    // Creamos el fichero nuevo en el camino2.
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) != 0)
    {
        fprintf(stderr, "Error en buscar_entrada() con camino2;\n");
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return 1;
    }
    mi_signalSem();
    // Leemos la entrada del fichero creado por el camino2.
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) <= 0)
    {
        fprintf(stderr, "mi_link() → Error; no se ha podido leer la entrada del camino2\n");
        return 1;
    }
    mi_waitSem();
    // Liberamos el inodo del nuevo fichero y asignamos el inodo del fichero que queremos enlazar.
    liberar_inodo(entrada.ninodo);
    mi_signalSem();
    entrada.ninodo = p_inodo1;
    // Escribimos la entrada en donde estaba con el inodo modificado.
    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) <= 0)
    {
        fprintf(stderr, "mi_link() → Error; no se ha podido escribir la entrada del camino2\n");
        return 1;
    }
    mi_waitSem();
    // Leemos el inodo del fichero de camino1 para luego modificar su ctime e incrementar nlinks en uno.
    if (leer_inodo(p_inodo1, &inodo) == -1)
    {
        fprintf(stderr, "mi_link() → Error; fallo al leer el inodo del camino1.");
        mi_signalSem();
        return 1;
    }
    // Sumamos uno a la cantidad de enlaces del inodo y actualizamos el tiempo de modificación.
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    // Escribimos los cambios.
    if (escribir_inodo(p_inodo1, inodo))
    {
        fprintf(stderr, "mi_link() → Error escribiendo el inodo");
        mi_signalSem();
        return 1;
    }
    mi_signalSem();
    return 0;
}
/**
 * @brief Desvincula el inodo del fichero/directorio y borra éste fichero, decrementando su numero de enlaces.
 * Si es el último enlace, borra de la memoria el fichero completamente y libera su inodo.
 *
 * @param camino
 * @return int
 */
int mi_unlink(const char *camino, int borrado_recursivo)
{
    int nBLiberados = 0;
    struct inodo inodo;
    int error;
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int len = strlen(camino);
    // Si el camino es el directorio raíz, no se debe poder eliminar.
    if (len == 1 && camino[len - 1] == '/')
    {
        fprintf(stderr, "Error; no se puede borrar el directorio raíz.\n");
        return 1;
    }
    mi_waitSem();
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)))
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return 1;
    }
    mi_signalSem();
    // Leemos el inodo fichero/directorio
    leer_inodo(p_inodo, &inodo);
    // Si es un fichero o un directorio vacío, borramos la entrada de directorio y decrementamos sus nlinks.
    if (inodo.tipo == 'f' || (inodo.tipo == 'd' && inodo.tamEnBytesLog == 0))
    {
        // Eliminamos la entrada del directorio.
        if ((nBLiberados = elimina_entrada_dir(p_inodo_dir, p_entrada, p_inodo)) < 0)
        {
            printf("mi_unlink() → error eliminando la entrada del directorio\n");
            return 1;
        }
#if NIVEL10
        fprintf(stderr, "mi_unlink() → se ha borrado %s correctamente\n", camino);
#endif
    }
    else if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0)
    {
        /*borrar recursivamente*/
        if (borrado_recursivo)
        {
            // Cantidad de entradas que hay en un bloque.
            int totalEntradasBloque = BLOCKSIZE / sizeof(struct entrada);
            struct entrada buffer_entradas[totalEntradasBloque];
            struct STAT estado_entrada;
            char camino_entrada[len + TAMNOMBRE];
            memset(camino_entrada, 0x00, strlen(camino_entrada));
            strcpy(camino_entrada, camino);
            int nEntradasTotal = inodo.tamEnBytesLog / sizeof(struct entrada);
            for (int i = 0; i < inodo.numBloquesOcupados && 0 < nEntradasTotal; i++)
            {
                // Leemos un bloque de entradas; Siempre leemos el mismo bloque debido al borrado de los archivos
                // durante la recursividad.
                if (mi_read_f(p_inodo, buffer_entradas, 0, BLOCKSIZE) <= 0)
                {
                    fprintf(stderr, "mi_unlink() → Error leyendo bloque de entradas para %s\n", camino);
                    return 1;
                }
                // Por cada entrada la procedemos a borrar.
                for (int j = 0; j < totalEntradasBloque && j < nEntradasTotal; j++)
                {
                    // Leemos el estado para averiguar si es un fichero o directorio.
                    if (mi_stat_f(buffer_entradas[j].ninodo, &estado_entrada) == -1)
                    {
                        fprintf(stderr, "mi_unlink() → Error leyendo el estado de la entrada\n");
                        return 1;
                    }
                    // Concatenamos el camino entrada actual con el nombre
                    strcat(camino_entrada, buffer_entradas[j].nombre);
                    // Si la entrada es un directorio, le añadimos la barra.
                    if (estado_entrada.tipo == 'd')
                    {
                        strcat(camino_entrada, "/");
                    }
                    // Hacemos recursividad por la entrada llamando mi_unlink() y se borra el fichero/directorio.
                    if (mi_unlink(camino_entrada, borrado_recursivo) == 1)
                    {
                        fprintf(stderr, "mi_unlink(recursivo) → Error en la recursividad\n");
                        return 1;
                    }
#if NIVEL10R
                    fprintf(stderr, "mi_unlink(recursivo) → se ha borrado %s correctamente\n", buffer_entradas[j].nombre);
#endif
                    memset(camino_entrada, 0x00, strlen(camino_entrada));
                    strcpy(camino_entrada, camino);
                }
                // Reducimos el num de entradas totales debido a su borrado.
                nEntradasTotal = nEntradasTotal - totalEntradasBloque;
                memset(buffer_entradas, 0x00, BLOCKSIZE);
            }
            // Por ultimo, eliminamos el directorio actual una vez eliminados sus datos.
            if ((nBLiberados = elimina_entrada_dir(p_inodo_dir, p_entrada, p_inodo)) < 0)
            {
                printf("mi_unlink(recursivo) → error eliminando la entrada del directorio\n");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "mi_unlink() → Error, no se puede borrar un directorio con contenido dentro (usar -r)\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "mi_unlink() → Error con el inodo; no es ni fichero ni directorio\n");
        return 1;
    }
    printf("Liberados: %d\n", nBLiberados);
    return 0;
}
/**
 * @brief Elimina la entrada que apunta p_entrada del inodo directorio que apunta p_inodo_dir.
 * Devuelve el numero de bloques liberados, en caso contrario, devuelve -1.
 *
 * @param p_inodo_dir
 * @param p_entrada
 * @param p_inodo
 * @return int
 */
int elimina_entrada_dir(int p_inodo_dir, int p_entrada, int p_inodo)
{
    int nBLiberados = 0;
    struct inodo inodo_dir, inodo;
    if (leer_inodo(p_inodo_dir, &inodo_dir))
    {
        fprintf(stderr, "elimina_entrada_dir() → Error leyendo el inodo directorio\n");
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo))
    {
        fprintf(stderr, "elimina_entrada_dir() → Error leyendo el inodo\n");
        return -1;
    }
    // Calculamos cuantos inodos hay en el directorio.
    int nTotalEntradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    if (p_entrada != (nTotalEntradas - 1))
    {
        struct entrada ultEntrada;
        // Leemos la ultima entrada.
        if (mi_read_f(p_inodo_dir, &ultEntrada, (nTotalEntradas - 1) * sizeof(struct entrada), sizeof(struct entrada)) <= 0)
        {
            fprintf(stderr, "elimina_entrada_dir() → Error leyendo la ultima entrada de directorio\n");
            return -1;
        }
        // Borramos la entrada del directorio sobreescribiendo su posición de donde estaba con la ultima entrada y decrementando tamEnBytesLog.
        if (mi_write_f(p_inodo_dir, &ultEntrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) <= 0)
        {
            fprintf(stderr, "elimina_entrada_dir() → Error leyendo la ultima entrada de directorio\n");
            return -1;
        }
    }
    mi_waitSem();
    // Truncamos el inodo directorio para eliminar la última entrada y actualizar el tamEnBytesLog
    if ((nBLiberados = mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada))) < 0)
    {
        fprintf(stderr, "mi_unlink() → Error truncando el inodo directorio\n");
        mi_signalSem();
        return -1;
    }
    mi_signalSem();

    // Decrementamos la cantidad de enlaces que tiene el inodo de la entrada por haberlo eliminado.
    inodo.nlinks--;
    mi_waitSem();
    // Si era el ultimo enlace, liberamos el inodo.
    if (inodo.nlinks == 0)
    {
        if (liberar_inodo(p_inodo) != p_inodo)
        {
            fprintf(stderr, "elimina_entrada_dir()→ Error liberando inodo");
            mi_signalSem();
            return -1;
        }
    }
    else
    {
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, inodo))
        {
            fprintf(stderr, "elimina_entrada_dir() → Error escribiendo inodo");
            mi_signalSem();
            return -1;
        }
    }
    mi_signalSem();
    return nBLiberados;
}

int mi_rn(const char *camino, const char *nombre)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct entrada entrada;
    int error;
    // Buscamos la entrada a renombrar.
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return 1;
    }
    // Leemos la entrada que queremos renombrar.
    if ((mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada))) <= 0)
    {
        fprintf(stderr, "mi_rn() → Error leyendo la entrada del camino\n");
        return 1;
    }
    // Leemos el estado para saber si es un fichero o directorio.
    struct STAT estado_inodo;
    if (mi_stat_f(entrada.ninodo, &estado_inodo))
    {
        fprintf(stderr, "Error leyendo el estado del archivo de camino origen\n");
        return 1;
    }
    int tam_cam_tmp;
    if (estado_inodo.tipo == 'd')
    {
        tam_cam_tmp = strlen(camino) - (1 + strlen(entrada.nombre));
    }
    else
    {
        tam_cam_tmp = strlen(camino) - strlen(entrada.nombre);
    }

    char camino_temp[strlen(camino)];
    memset(camino_temp,0x00,strlen(camino));
    // Si es un directorio, le restas uno más para eliminar el '/', en caso contrario, se trata como fichero.
    strncpy(camino_temp, camino, tam_cam_tmp);
    strcat(camino_temp, nombre);
    if (estado_inodo.tipo == 'd')
    {
        strcat(camino_temp, "/");
    }

    // Buscamos si existe alguna entrada con ese nombre en el directorio correspondiente.
    unsigned int p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;
    // Comprobamos que el nuevo archivo no existe en el directorio.
    if ((error = buscar_entrada(camino_temp, &p_inodo_dir2, &p_inodo2, &p_entrada2, 0, 0)) != ERROR_NO_EXISTE_ENTRADA_CONSULTA)
    {
        if (error == 0)
        {
            fprintf(stderr, "Error, ya existe un archivo con ese nombre\n");
        }
        else
        {
            mostrar_error_buscar_entrada(error);
        }
        return 1;
    }

    // Copiamos el nuevo nombre a la entrada, sobreescribiendo el antiguo.
    strcpy(entrada.nombre, nombre);
    if ((mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada))) <= 0)
    {
        fprintf(stderr, "mi_rn() → Error escribiendo la nueva entrada al camino\n");
        return 1;
    }
    return 0;
}

/**
 * @brief Mueve el archivo al directorio destino.
 *
 * @param camino_origen
 * @param camino_destino
 * @return int
 */
int mi_mv(const char *camino_origen, const char *camino_destino)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct entrada entrada;
    int error;
    // Buscamos la entrada en el camino origen.
    if ((error = buscar_entrada(camino_origen, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return 1;
    }
    // Leemos la entrada
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
    {
        fprintf(stderr, "mi_mv()→ error leyendo entrada del camino_origen\n");
        return 1;
    }
    char camino_destino_final[strlen(camino_destino) + strlen(entrada.nombre)];
    memset(camino_destino_final, 0x00, sizeof(camino_destino_final));
    strcpy(camino_destino_final, camino_destino);
    strcat(camino_destino_final, entrada.nombre);
    // Si es un directorio, le concatenamos el final del path
    if (camino_origen[strlen(camino_origen) - 1] == '/')
        strcat(camino_destino_final, "/");
    // Creamos un enlace al camino destino con ese nombre
    if (mi_link(camino_origen, camino_destino_final))
    {
        return 1;
    }
    // Borramos la entrada del directorio enlace.
    elimina_entrada_dir(p_inodo_dir, p_entrada, p_inodo);
    return 0;
}