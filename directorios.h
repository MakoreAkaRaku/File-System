// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros.h"
#include "string.h"

#define ERROR_CAMINO_INCORRECTO -1
#define ERROR_PERMISO_LECTURA -2
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -3
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -4
#define ERROR_PERMISO_ESCRITURA -5
#define ERROR_ENTRADA_YA_EXISTENTE -6
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -7
#define ERROR -8
#define DEBUGNIVEL7 0
#define TAMNOMBRE 60   // tamaño del nombre de directorio o fichero, en Ext2 = 256
#define PROFUNDIDAD 32 // tamaño de la profundidad del camino.
#define BLK "\e[0;30m" // Colores para mi_ls
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m" // Fin colores para mi_ls
#define RST "\e[0m"    // Resetea el color
#define NIVEL9 1       // Constante para el preprocesado i debugging del nivel9
#define TAMCACHE 4     // Constante para determinar el tamaño de los buffers de caché de entradas. Nivel9
#define NIVEL10 0      // Constante para tapar los prints de debugging
#define NIVEL10R NIVEL10
struct entrada{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

// Nivel 9
struct UltimaEntrada{
    char camino[TAMNOMBRE * PROFUNDIDAD];
    int p_inodo;
    unsigned int prioridad;
};
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error);

// Nivel 8

int mi_creat(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
int mi_dir(const char *camino, char *buffer, unsigned int es_simple);
int mi_chmod(const char *camino, unsigned char permisos);

// Nivel 9

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
// función opcional para busqueda de entrada en la memoria caché.

int check_entradas_cache(const char *camino, int is_rd);
int busca_entrada_cache_mp(int is_rd);

// Nivel 10

int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino, int borrado_recursivo);
int elimina_entrada_dir(int p_inodo_dir, int p_entrada, int p_inodo);
int mi_rn(const char *camino, const char *nombre);
int mi_mv(const char *camino_archivo, const char *camino_destino);