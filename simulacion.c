// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "simulacion.h"

static int acabados = 0;
int main(int n, char *args[]){
    
    if(n != 2)
    {
        fprintf(stderr, "Error de sintaxis; ./simulacion <disco>");
        return -1;
    }
    if (bmount(args[1]) == -1)
    {
        printf("Error; no existe el disco\n");
        return 1;
    }
    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);
    //Inicio de construcción del nuevo directorio en el inodo raíz.
    struct tm *ts;
    time_t t = time(NULL);
    ts = localtime(&t);
    char nombre_sim[TAMNOMBRE*TAMNOMBRE];
    memset(nombre_sim,0x00,TAMNOMBRE*TAMNOMBRE);
    strcat(nombre_sim,"/simul_");
    char tiempo_impreso[TAMNOMBRE];
    strftime(tiempo_impreso,TAMNOMBRE,"%Y%m%d%H%M%S",ts);
    strcat(nombre_sim,tiempo_impreso);
    strcat(nombre_sim,"/\0");
    //Fin de la construcción del nuevo directorio en el inodo raíz.
    //printf("%s",nombre_sim);
    //Creamos el directorio con su nombre en el inodo raíz.
    if(mi_creat(nombre_sim,6)){
        fprintf(stderr,"Error al crear el directorio %s",nombre_sim);
        return 1;
    }
    pid_t pid_act;
    signal(SIGCHLD, reaper);
    for(int proceso = 0; proceso < NUMPROCESOS; proceso++)
    {
        pid_act = fork();
        //Si es el proceso hijo, hacer lo que toque
        if (pid_act == 0)
        {
            //Añadimos el nombre del directorio al camino para crearlo.
            char num_pid_proc[TAMNOMBRE];
            strcat(nombre_sim,"proceso_");
            pid_t p_id_hijo = getpid();
            sprintf(num_pid_proc,"%d",p_id_hijo);
            strcat(nombre_sim,num_pid_proc);
            strcat(nombre_sim,"/\0");
            //Montamos el disco de nuevo para el proceso hijo evitando asi problemas de concurrencia
            if (bmount(args[1]) == -1)
            {
                printf("Error; no existe el disco\n");
                return 1;
            }
            //Creación directorio
            mi_creat(nombre_sim,6);
            strcat(nombre_sim,"pruebas.dat\0");
            //Creación fichero
            mi_creat(nombre_sim,6);
            srand(time(NULL) + p_id_hijo);
            for(int escrituras = 1; escrituras <= NUMESCRITURAS; escrituras++)
            {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = p_id_hijo;
                registro.nEscritura = escrituras;
                registro.nRegistro = rand() % REGMAX;
            #if DEBUGSIM
                printf("[simulación.c → Escritura %d en %s]\n",escrituras+1, nombre_sim);
            #endif
                if (mi_write(nombre_sim,&registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) <= 0){
                    return EXIT_FAILURE; 
                }
                my_sleep(50);
            }
            bumount();
            printf("[Proceso %d: Completadas %d escrituras en %s]\n", proceso+1, NUMESCRITURAS, nombre_sim);
            exit(0);
        }
        my_sleep(150);
    }
    while(acabados < NUMPROCESOS)
    {
        pause();
    }
    bumount();
}

void my_sleep(unsigned msec) { //recibe tiempo en milisegundos
   struct timespec req, rem;
   int err;
   req.tv_sec = msec / 1000; //conversión a segundos
   req.tv_nsec = (msec % 1000) * 1000000; //conversión a nanosegundos
   while ((req.tv_sec != 0) || (req.tv_nsec != 0)) {
       if (nanosleep(&req, &rem) == 0) 
// rem almacena el tiempo restante si una llamada al sistema
// ha sido interrumpida por una señal
           break;
       err = errno;
       // Interrupted; continue
       if (err == EINTR) {
           req.tv_sec = rem.tv_sec;
           req.tv_nsec = rem.tv_nsec;
       }
   }
}

void reaper(){
  pid_t ended;
  signal(SIGCHLD, reaper);
  while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
    acabados++;
  }
}