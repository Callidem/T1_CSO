#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>

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

void timeConv(double seconds, char result[100]) {
    int days = seconds / (24 * 3600);
    seconds = (int)seconds % (24 * 3600);
    int hours = seconds / 3600;
    seconds = (int)seconds % 3600;
    int minutes = seconds / 60;
    seconds = (int)seconds % 60;

    snprintf(result, 100, "%d dias, %d horas, %d minutos, %.0f segundos",
             days, hours, minutes, seconds);
}

int onlyDigits(const char *str) {
    int i = 0;

    if (str[0] == '\0')
        return 0;

    while (str[i] != '\0') {
        if (!isdigit((unsigned char)str[i]))
            return 0;
        i++;
    }

    return 1;
}

int main() {
    FILE *fptr;

    // 1. Versão do sistema e kernel
    char kernel[100] = "";
    char release[100] = "";

    fptr = fopen("/proc/sys/kernel/ostype", "r");
    if (fptr != NULL) {
        fgets(kernel, sizeof(kernel), fptr);
        nLineTrim(kernel);
        fclose(fptr);
    }

    fptr = fopen("/proc/sys/kernel/osrelease", "r");
    if (fptr != NULL) {
        fgets(release, sizeof(release), fptr);
        nLineTrim(release);
        fclose(fptr);
    }

    // 2. Uptime
    // 3. Tempo ocioso
    double aux1 = 0.0, aux2 = 0.0;
    char uptime[100] = "";
    char idleSys[100] = "";

    fptr = fopen("/proc/uptime", "r");
    if (fptr != NULL) {
        fscanf(fptr, "%lf %lf", &aux1, &aux2);
        fclose(fptr);

        timeConv(aux1, uptime);
        timeConv(aux2, idleSys);
    }

    // 4. Data e hora do sistema (RTC)
    int h = 0, min = 0, s = 0, d = 0, m = 0, y = 0;
    char dateTime[100] = "";
    char rtcLine[256];

    fptr = fopen("/proc/driver/rtc", "r");
    if (fptr != NULL) {
        while (fgets(rtcLine, sizeof(rtcLine), fptr) != NULL) {
            if (strncmp(rtcLine, "rtc_time", 8) == 0) {
                sscanf(rtcLine, "rtc_time : %d:%d:%d", &h, &min, &s);
            }
            else if (strncmp(rtcLine, "rtc_date", 8) == 0) {
                sscanf(rtcLine, "rtc_date : %d-%d-%d", &y, &m, &d);
            }
        }
        fclose(fptr);

        snprintf(dateTime, sizeof(dateTime), "%02d/%02d/%04d %02d:%02d:%02d",
                 d, m, y, h, min, s);
    }

    // 5. Modelo do processador, velocidade e número de núcleos
    char linha[256];
    char modelo[256] = "";
    float velocidade = 0.0;
    int nucleos = 0;

    fptr = fopen("/proc/cpuinfo", "r");
    if (fptr != NULL) {
        while (fgets(linha, sizeof(linha), fptr) != NULL) {
            if (strncmp(linha, "model name", 10) == 0 && modelo[0] == '\0') {
                char *p = strchr(linha, ':');
                if (p != NULL) {
                    p += 2;
                    strncpy(modelo, p, sizeof(modelo) - 1);
                    modelo[sizeof(modelo) - 1] = '\0';
                    nLineTrim(modelo);
                }
            }
            else if (strncmp(linha, "cpu MHz", 7) == 0 && velocidade == 0.0) {
                sscanf(linha, "cpu MHz : %f", &velocidade);
            }
            else if (strncmp(linha, "processor", 9) == 0) {
                nucleos++;
            }
        }
        fclose(fptr);
    }

    // 6. Carga do sistema
    float loadAvg[3] = {0.0, 0.0, 0.0};

    fptr = fopen("/proc/loadavg", "r");
    if (fptr != NULL) {
        fscanf(fptr, "%f %f %f", &loadAvg[0], &loadAvg[1], &loadAvg[2]);
        fclose(fptr);
    }

    // 7. Capacidade ocupada do processador (percentual)
    long long int user1 = 0, nice1 = 0, system1 = 0, idle1 = 0, iowait1 = 0, irq1 = 0, softirq1 = 0, steal1 = 0;
    long long int user2 = 0, nice2 = 0, system2 = 0, idle2 = 0, iowait2 = 0, irq2 = 0, softirq2 = 0, steal2 = 0;
    double cpuUsage = 0.0;

    fptr = fopen("/proc/stat", "r");
    if (fptr != NULL) {
        fscanf(fptr, "cpu  %lld %lld %lld %lld %lld %lld %lld %lld",
               &user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1, &steal1);
        fclose(fptr);

        usleep(200000);

        fptr = fopen("/proc/stat", "r");
        if (fptr != NULL) {
            fscanf(fptr, "cpu  %lld %lld %lld %lld %lld %lld %lld %lld",
                   &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2, &steal2);
            fclose(fptr);

            long long int idleDelta = (idle2 + iowait2) - (idle1 + iowait1);
            long long int total1 = user1 + nice1 + system1 + idle1 + iowait1 + irq1 + softirq1 + steal1;
            long long int total2 = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2 + steal2;
            long long int totalDelta = total2 - total1;

            if (totalDelta > 0) {
                cpuUsage = ((double)(totalDelta - idleDelta) / (double)totalDelta) * 100.0;
            }
        }
    }

    // 8. Quantidade de memória RAM total e usada (em megabytes)
    long long int memTotal = 0, memFree = 0, memAvailable = 0;

    fptr = fopen("/proc/meminfo", "r");
    if (fptr != NULL) {
        while (fgets(linha, sizeof(linha), fptr) != NULL) {
            if (strncmp(linha, "MemTotal:", 9) == 0) {
                sscanf(linha, "MemTotal: %lld kB", &memTotal);
            }
            else if (strncmp(linha, "MemFree:", 8) == 0) {
                sscanf(linha, "MemFree: %lld kB", &memFree);
            }
            else if (strncmp(linha, "MemAvailable:", 13) == 0) {
                sscanf(linha, "MemAvailable: %lld kB", &memAvailable);
            }
        }
        fclose(fptr);
    }

    // 9. Operações sobre o sistema de entrada e saída (leituras e escritas)
    long long int totalReadSectors = 0;
    long long int totalWriteSectors = 0;
    char diskName[64];
    long long int readsCompleted, readsMerged, sectorsRead, msReading;
    long long int writesCompleted, writesMerged, sectorsWritten, msWriting;

    fptr = fopen("/proc/diskstats", "r");
    if (fptr != NULL) {
        while (fgets(linha, sizeof(linha), fptr) != NULL) {
            if (sscanf(linha,
                       "%*d %*d %63s %lld %lld %lld %lld %lld %lld %lld %lld",
                       diskName,
                       &readsCompleted, &readsMerged, &sectorsRead, &msReading,
                       &writesCompleted, &writesMerged, &sectorsWritten, &msWriting) == 9) {

                if (strncmp(diskName, "ram", 3) != 0 && strncmp(diskName, "loop", 4) != 0) {
                    totalReadSectors += sectorsRead;
                    totalWriteSectors += sectorsWritten;
                }
            }
        }
        fclose(fptr);
    }

    // 10. Sistemas de arquivos suportados pelo kernel
    char filesystem[100][100];
    int fsCount = 0;

    fptr = fopen("/proc/filesystems", "r");
    if (fptr != NULL) {
        while (fsCount < 100 && fgets(filesystem[fsCount], sizeof(filesystem[fsCount]), fptr) != NULL) {
            nLineTrim(filesystem[fsCount]);
            fsCount++;
        }
        fclose(fptr);
    }

    // 11. Dispositivos (caracter e bloco) e grupos
    char devices[100][100];
    int devCount = 0;

    fptr = fopen("/proc/devices", "r");
    if (fptr != NULL) {
        while (devCount < 100 && fgets(devices[devCount], sizeof(devices[devCount]), fptr) != NULL) {
            nLineTrim(devices[devCount]);
            devCount++;
        }
        fclose(fptr);
    }

    // 12. Dispositivos de rede
    char netDevices[100][120];
    int netCount = 0;

    fptr = fopen("/proc/net/dev", "r");
    if (fptr != NULL) {
        while (netCount < 100 && fgets(netDevices[netCount], sizeof(netDevices[netCount]), fptr) != NULL) {
            nLineTrim(netDevices[netCount]);
            netCount++;
        }
        fclose(fptr);
    }

    // Cabeçalho CGI - precisa vir antes de qualquer outra saída
    printf("Content-Type: text/html\r\n\r\n");

    printf("<!DOCTYPE html>");
    printf("<html lang='pt-BR'>");
    printf("<head>");
    printf("<meta charset='UTF-8'>");
    printf("<meta http-equiv='refresh' content='5'>");
    printf("<title>Monitor do Sistema</title>");
    printf("</head>");
    printf("<body>");

    printf("<h1>Informações do Sistema</h1>");
    // printf("System Information:\\n");

    printf("<h2>1. Versão do sistema e kernel</h2>");
    printf("<p>%s %s</p>", kernel, release);
    // printf("Versão do sistema e Kernel: %s %s\\n", kernel, release);

    printf("<h2>2. Uptime</h2>");
    printf("<p>%s</p>", uptime);

    printf("<h2>3. Tempo ocioso</h2>");
    printf("<p>%s</p>", idleSys);
    // printf("Uptime: %s \\nIdle time: %s\\n", uptime, idleSys);

    printf("<h2>4. Data e hora do sistema</h2>");
    printf("<p>%s</p>", dateTime);
    // printf("Data e hora do sistema: %s\\n", dateTime);

    printf("<h2>5. Processador</h2>");
    printf("<p><b>Modelo:</b> %s</p>", modelo);
    printf("<p><b>Velocidade:</b> %.2f MHz</p>", velocidade);
    printf("<p><b>Núcleos lógicos:</b> %d</p>", nucleos);
    // printf("Modelo: %s\\n", modelo);
    // printf("Velocidade: %.2f MHz\\n", velocidade);
    // printf("Número de núcleos lógicos: %d\\n", nucleos);

    printf("<h2>6. Carga do sistema</h2>");
    printf("<p>%.2f (1 min), %.2f (5 min), %.2f (15 min)</p>",
           loadAvg[0], loadAvg[1], loadAvg[2]);
    // printf("Carga média do sistema: %.2f(1min), %.2f(5min), %.2f(15min)\\n", loadAvg[0], loadAvg[1], loadAvg[2]);

    printf("<h2>7. Capacidade ocupada do processador</h2>");
    printf("<p>%.2f%%</p>", cpuUsage);
    // printf("Uso de CPU: %.2f%%\\n", cpuUsage);

    printf("<h2>8. Memória RAM</h2>");
    printf("<p><b>Total:</b> %lld MB</p>", memTotal / 1024);
    printf("<p><b>Usada:</b> %lld MB</p>", (memTotal - memAvailable) / 1024);
    printf("<p><b>Livre:</b> %lld MB</p>", memFree / 1024);
    printf("<p><b>Disponível:</b> %lld MB</p>", memAvailable / 1024);
    // printf("Memory: Total: %lld MB, Usada: %lld MB\\n", memTotal / 1024, (memTotal - memAvailable) / 1024);

    printf("<h2>9. Operações de entrada e saída</h2>");
    printf("<p><b>Setores lidos:</b> %lld</p>", totalReadSectors);
    printf("<p><b>Setores escritos:</b> %lld</p>", totalWriteSectors);
    // printf("Read Sectors: %lld, Write Sectors: %lld\\n", totalReadSectors, totalWriteSectors);

    printf("<h2>10. Sistemas de arquivos suportados pelo kernel</h2>");
    printf("<ul>");
    for (int i = 0; i < fsCount; i++) {
        printf("<li>%s</li>", filesystem[i]);
        // printf("%s\\n", filesystem[i]);
    }
    printf("</ul>");

    printf("<h2>11. Dispositivos (caractere e bloco) e grupos</h2>");
    printf("<pre>");
    for (int i = 0; i < devCount; i++) {
        printf("%s\n", devices[i]);
    }
    printf("</pre>");
    // printf("Devices ...\\n");

    printf("<h2>12. Dispositivos de rede</h2>");
    printf("<pre>");
    for (int i = 0; i < netCount; i++) {
        printf("%s\n", netDevices[i]);
    }
    printf("</pre>");
    // printf("Network Devices ...\\n");

    printf("<h2>13. Lista de processos em execução</h2>");
    printf("<table border='1' cellspacing='0' cellpadding='4'>");
    printf("<tr><th>PID</th><th>Nome</th></tr>");

    DIR *procDir;
    struct dirent *entry;

    procDir = opendir("/proc");
    if (procDir != NULL) {
        while ((entry = readdir(procDir)) != NULL) {
            if (onlyDigits(entry->d_name)) {
                char path[256];
                char processName[256];
                FILE *procFile;

                snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

                procFile = fopen(path, "r");
                if (procFile != NULL) {
                    if (fgets(processName, sizeof(processName), procFile) != NULL) {
                        nLineTrim(processName);
                        printf("<tr><td>%s</td><td>%s</td></tr>", entry->d_name, processName);
                        // printf("PID: %s | Nome: %s\\n", entry->d_name, processName);
                    }
                    fclose(procFile);
                }
            }
        }
        closedir(procDir);
    }

    printf("</table>");
    printf("</body>");
    printf("</html>");

    return 0;
}
