// Autores: Marc Roman Colom y Rafael Ramírez Berrocal
#include "ficheros.h"

int main(int n, char *args[]){
    if (n < 2)
    {
        printf("Error de sintaxis; ./leer <nombre_dispositivo>  <ninodo>");
        return 1;
    }
    bmount(args[1]);
    unsigned char buffer[TAMBUFFER];
    int bytes_leidos = 0;
    memset(buffer,0x00,TAMBUFFER);
    int leido = mi_read_f(atoi(args[2]), buffer, bytes_leidos, TAMBUFFER);
    while (leido > 0)
    {
        write(1,buffer,leido);
        memset(buffer,0x00,leido);
        bytes_leidos += leido;
        leido = mi_read_f(atoi(args[2]), buffer, bytes_leidos, TAMBUFFER);
    }
    fprintf(stderr,"\nNº bytes leídos: %d\n",bytes_leidos);
    struct inodo i;
    leer_inodo(atoi(args[2]),&i);
    fprintf(stderr,"\ntamEnBytesLog: %d\n",i.tamEnBytesLog);
    bumount();
}