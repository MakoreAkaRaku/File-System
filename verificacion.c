// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "verificacion.h"

int main(int n, char *args[])
{
    if (n != 3)
    {
        fprintf(stderr, "Error de sintaxis; ./verificacion <nombre_dispositivo> <directorio_simulación>");
        return -1;
    }
    if (bmount(args[1]) == -1)
    {
        printf("Error; no existe el disco\n");
        return 1;
    }
    struct STAT stat;
    // Sacamos el estado del inodo directorio para saber cuantas entradas hay.
    if (mi_stat(args[2], &stat) == -1)
    {
        return EXIT_FAILURE;
    }
    int numEntradas = stat.tamEnBytesLog / sizeof(struct entrada);
    // Si hay diferentes entradas a la cantidad de numero de procesos, error (no se han escrito o se escribieron de mas)
    if (numEntradas != NUMPROCESOS)
    {
        fprintf(stderr, "Error verificacion.c; No se crearon los directorios de cada proceso\n");
        return -1;
    }
    char informe[TAMNOMBRE * TAMNOMBRE];
    char proceso[TAMNOMBRE * TAMNOMBRE];
    memset(informe, 0x00, sizeof(informe));
    strcat(informe, args[2]);
    strcat(informe, "informe.txt\0");
    // Creamos el buffer done volcar todas las entradas directorio.
    struct entrada entradas[numEntradas];
    // Creamos el fichero "informe.txt" dentro del directoio simul_XXXX.
    if (mi_creat(informe, 6) < 0)
    {
        return EXIT_FAILURE;
    }else{
        printf("\nCreado el informe en %s\n",informe);
    }
    // Leemos las entradas del directorio simul_XXXX.
    if (mi_read(args[2], entradas, 0, stat.tamEnBytesLog) <= 0)
    {
        fprintf(stderr, "Error leyendo las entradas del directorio simul\n");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "dir_sim: %s\nnumentradas: %d NUMPROCESOS: %d\n", args[2], numEntradas, NUMPROCESOS);
    struct INFORMACION info;
    int offsetAct = 0;
    struct REGISTRO *buff_registros;
    // Usado para escribir en el fichero "informe.txt"
    char buff_escritura[1000];
    // Por cada entrada dentro del directorio simul_XXX hacer
    for (int i = 0; i < numEntradas; i++)
    {
        memset(proceso, 0x00, sizeof(proceso));
        strcat(proceso, args[2]);
        // Almacenamos el pid del proceso en info.
        info.pid = atoi((strchr(entradas[i].nombre, '_') + 1));
        strcat(proceso, entradas[i].nombre);
        strcat(proceso, "/pruebas.dat\0");
        if (mi_stat(proceso, &stat) == -1)
        {
            return EXIT_FAILURE;
        }
        // Sacamos la cantidad de registros que hay en el pruebas.dat
        // del directorio proceso en el que estamos.
        int cant_reg_buff_escr = stat.tamEnBytesLog / sizeof(struct REGISTRO);
        buff_registros = malloc(sizeof(struct REGISTRO) * cant_reg_buff_escr);
        memset(buff_registros, 0x00, sizeof(struct REGISTRO));
        info.nEscrituras = 0;
        if (mi_read(proceso, buff_registros, 0, cant_reg_buff_escr * sizeof(struct REGISTRO)) > 0)
        {
            for (int j = 0; j < cant_reg_buff_escr; j++)
            {
                if (info.pid == buff_registros[j].pid)
                {
                    // Si es la primera escritura validada inicializamos todos los valores.
                    // Ya se sabe que es la de menor posición en este caso.
                    if (!info.nEscrituras)
                    {
                        info.PrimeraEscritura = buff_registros[j];
                        info.UltimaEscritura = buff_registros[j];
                        info.MenorPosicion = buff_registros[j];
                        info.MayorPosicion = buff_registros[j];
                        info.nEscrituras++;
                    }
                    else
                    {
                        // Comparamos el número de escritura.
                        // Si el supuesto registro de la primera escritura es mayor al registro
                        // a observar, pasa a ser la nueva primera escritura.
                        if (info.PrimeraEscritura.nEscritura > buff_registros[j].nEscritura)
                        {
                            info.PrimeraEscritura = buff_registros[j];
                        }
                        // Observamos el caso contrario para la última escritura.
                        if (info.UltimaEscritura.nEscritura < buff_registros[j].nEscritura)
                        {
                            info.UltimaEscritura = buff_registros[j];
                        }
                        // Caso de que la supuesta menor escritura sea mayor que el registro observado.
                        if (info.MenorPosicion.nRegistro > buff_registros[j].nRegistro)
                        {
                            info.MenorPosicion = buff_registros[j];
                        }
                        // Caso de que la supuesta mayor escritura sea menor que el registro observado.
                        if (info.MayorPosicion.nRegistro < buff_registros[j].nRegistro)
                        {
                            info.MayorPosicion = buff_registros[j];
                        }
                        info.nEscrituras++;
                    }
                }
            }
        }
        else
        {
            fprintf(stderr, "Error leyendo los registros de %s\n", proceso);
            return EXIT_FAILURE;
        }
        free(buff_registros);
        memset(buff_escritura, 0x00, sizeof(buff_escritura));
        // Para impresion de tiempos
        char *p_ultiempo, *p_pritiempo, *p_maytiempo, *p_mentiempo;
        p_ultiempo = asctime(localtime(&info.UltimaEscritura.fecha));
        p_pritiempo = asctime(localtime(&info.PrimeraEscritura.fecha));
        p_maytiempo = asctime(localtime(&info.MayorPosicion.fecha));
        p_mentiempo = asctime(localtime(&info.MenorPosicion.fecha));
        // Damos formato al buffer para escribir el informe del proceso.
        sprintf(buff_escritura, "PID: %d\nNumero de escrituras: %d\n"
                                "Primera escritura\t%d\t%d\t%s"
                                "Ultima Escritura\t%d\t%d\t%s"
                                "Menor Posición  \t%d\t%d\t%s"
                                "Mayor Posición  \t%d\t%d\t%s\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, p_pritiempo,
                info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, p_ultiempo,
                info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, p_mentiempo,
                info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, p_maytiempo);
        // Acto seguido escribimos el informe del proceso al fichero informe.txt.
        int bytes_escritos = 0;
        if ((bytes_escritos = mi_write(informe, buff_escritura, offsetAct, strlen(buff_escritura))) > 0)
        {
            offsetAct += bytes_escritos;
        }
        else
        {
            return EXIT_FAILURE;
        }
        fprintf(stderr, "[%d) %d Escrituras validadas en %s]\n", i+1, info.nEscrituras, proceso);
    }

    bumount();
}