#include <stdio.h>
#include <string.h>

void nLineTrim(char *str) {
    int i = 0;

    while (str[i] != '\0') {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
        i++;
    }
}

int main()
{
    FILE *fptr;

    fptr = fopen("/proc/sys/kernel/ostype", "r");
    char kernel[100];
    fgets(kernel, 100, fptr);
    nLineTrim(kernel);
    fclose(fptr);

    fptr = fopen("/proc/sys/kernel/osrelease", "r");
    char release[100];
    fgets(release, 100, fptr);
    nLineTrim(release);
    fclose(fptr);

    fptr = fopen("/proc/cpuinfo", "r");

    char linha[256];
    char modelo[256] = "";
    float velocidade = 0.0;
    int nucleos = 0;

    while (fgets(linha, sizeof(linha), fptr) != NULL) {
        if (strncmp(linha, "model name", 10) == 0 && modelo[0] == '\0') {
            sscanf(linha, "model name : %[^\n]", modelo);
        }
        else if (strncmp(linha, "cpu MHz", 7) == 0 && velocidade == 0.0) {
            sscanf(linha, "cpu MHz : %f", &velocidade);
        }
        else if (strncmp(linha, "processor", 9) == 0) {
            nucleos++;
        }
    }
    fclose(fptr);

    fptr = fopen("/proc/meminfo", "r");
    long long int memTotal, memFree, memAvailable;
    fscanf(fptr, "MemTotal: %lld kB\n", &memTotal);
    fscanf(fptr, "MemFree: %lld kB\n", &memFree);
    fscanf(fptr, "MemAvailable: %lld kB\n", &memAvailable);
    fclose(fptr);


printf("\n");
    printf("████████████████████████████████████████████████\n");
    //printf("█████████████████BEM-VINDO %s████████████████\n", user);
    printf("███████████████████BEM-VINDO████████████████████\n");
    printf("████████████████████████████████████████████████\n");
    printf("████T1 - Construção de Sistemas Operacionais████\n");
    printf("████████████████████████████████████████████████\n");
    printf("█████████████Prof. Angelo Dal Zotto█████████████\n");
    printf("████████████████████████████████████████████████\n");
    printf("██Bruno M., Gianlucca C., Pedro M. e Rodrigo S.█\n");
    printf("████████████████████████ ███████████████████████\n");
    printf("\n");
    printf("Kernel: %s %s\n", kernel, release);
    printf("CPU: %s %d núcleos @ %.2f MHz\n", modelo, nucleos, velocidade);
    printf("Memória: Total: %lld kB, Livre: %lld kB, Disponível: %lld kB\n", memTotal, memFree, memAvailable);

    return 0;
}
