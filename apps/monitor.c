// T1 Construção de Sistemas Operacionais - Monitor do Sistema
// Alunos: Bruno Elkfury Monticelli, Gian Lucca Casapiccola Lucena, Pedro Henrique Nunes Oliveira, Rodrigo Susin de Cenço
// 15/04/2026
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

    double ramPercent = 0.0;
    double ramUsedMB = (double)(memTotal - memAvailable) / 1024.0;
    double ramTotalMB = (double)memTotal / 1024.0;

    if (memTotal > 0) {
        ramPercent = ((double)(memTotal - memAvailable) / (double)memTotal) * 100.0;
    }

    if (cpuUsage < 0.0) cpuUsage = 0.0;
    if (cpuUsage > 100.0) cpuUsage = 100.0;
    if (ramPercent < 0.0) ramPercent = 0.0;
    if (ramPercent > 100.0) ramPercent = 100.0;

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

    // Cabeçalho CGI
    printf("Content-Type: text/html\r\n\r\n");

    printf("<!DOCTYPE html>");
    printf("<html lang='pt-BR'>");
    printf("<head>");
    printf("<meta charset='UTF-8'>");
    printf("<meta http-equiv='refresh' content='5'>");
    printf("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    printf("<title>Monitor do Sistema</title>");

    printf("<style>");
    printf("body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f4f6f8; color: #222; box-sizing: border-box; }");
    printf("* { box-sizing: border-box; }");
    printf("h1 { margin-top: 0; margin-bottom: 24px; font-size: 32px; }");
    printf("h2 { margin: 0 0 12px 0; font-size: 18px; }");
    printf(".section-title { margin: 28px 0 14px 0; font-size: 24px; }");
    printf(".cards-container { display: flex; flex-wrap: wrap; gap: 16px; align-items: stretch; }");
    printf(".card { background: #ffffff; border-radius: 14px; padding: 18px; box-shadow: 0 2px 10px rgba(0,0,0,0.08); flex: 1 1 320px; min-width: 280px; }");
    printf(".card.full-width { flex: 1 1 100%%; min-width: 100%%; }");
    printf(".metric-value { font-size: 28px; font-weight: bold; margin: 6px 0 8px 0; }");
    printf(".metric-subtext { font-size: 14px; color: #666; margin-bottom: 12px; }");
    printf(".bar-container { width: 100%%; height: 22px; background: #e4e8ee; border-radius: 999px; overflow: hidden; }");
    printf(".bar-fill { height: 100%%; background: #4a90e2; border-radius: 999px; }");
    printf(".info-line { margin: 8px 0; line-height: 1.4; }");
    printf(".muted { color: #666; }");
    printf("table { width: 100%%; border-collapse: collapse; background: #fff; }");
    printf("th, td { border: 1px solid #d7dde5; padding: 8px 10px; text-align: left; }");
    printf("th { background: #eef3f8; }");
    printf("pre { margin: 0; white-space: pre-wrap; word-wrap: break-word; font-family: monospace; }");
    printf("ul { margin: 0; padding-left: 20px; }");
    printf("li { margin: 4px 0; }");
    printf("@media (max-width: 720px) {");
    printf("  body { padding: 14px; }");
    printf("  .card { flex: 1 1 100%%; min-width: 100%%; }");
    printf("  h1 { font-size: 26px; }");
    printf("}");
    printf("</style>");

    printf("</head>");
    printf("<body>");

    printf("<h1>Monitor do Sistema</h1>");

    printf("<h2 class='section-title'>Métricas</h2>");
    printf("<div class='cards-container'>");

    printf("<div class='card'>");
    printf("<h2>CPU</h2>");
    printf("<div class='metric-value'>%.2f%%</div>", cpuUsage);
    printf("<div class='metric-subtext'>Uso atual do processador</div>");
    printf("<div class='bar-container'>");
    printf("<div class='bar-fill' style='width: %.2f%%;'></div>", cpuUsage);
    printf("</div>");
    printf("</div>");

    printf("<div class='card'>");
    printf("<h2>RAM</h2>");
    printf("<div class='metric-value'>%.2f%%</div>", ramPercent);
    printf("<div class='metric-subtext'>%.0f MB usados de %.0f MB</div>", ramUsedMB, ramTotalMB);
    printf("<div class='bar-container'>");
    printf("<div class='bar-fill' style='width: %.2f%%;'></div>", ramPercent);
    printf("</div>");
    printf("</div>");

    printf("</div>");

    printf("<h2 class='section-title'>Informações gerais</h2>");
    printf("<div class='cards-container'>");

    printf("<div class='card'>");
    printf("<h2>Sistema e Kernel</h2>");
    printf("<div class='info-line'><b>Sistema:</b> %s</div>", kernel);
    printf("<div class='info-line'><b>Release:</b> %s</div>", release);
    printf("</div>");

    printf("<div class='card'>");
    printf("<h2>Tempo</h2>");
    printf("<div class='info-line'><b>Uptime:</b> %s</div>", uptime);
    printf("<div class='info-line'><b>Tempo ocioso:</b> %s</div>", idleSys);
    printf("<div class='info-line'><b>Data/Hora:</b> %s</div>", dateTime);
    printf("</div>");

    printf("<div class='card'>");
    printf("<h2>Processador</h2>");
    printf("<div class='info-line'><b>Modelo:</b> %s</div>", modelo);
    printf("<div class='info-line'><b>Velocidade:</b> %.2f MHz</div>", velocidade);
    printf("<div class='info-line'><b>Núcleos lógicos:</b> %d</div>", nucleos);
    printf("</div>");

    printf("<div class='card'>");
    printf("<h2>Carga do sistema</h2>");
    printf("<div class='info-line'><b>1 min:</b> %.2f</div>", loadAvg[0]);
    printf("<div class='info-line'><b>5 min:</b> %.2f</div>", loadAvg[1]);
    printf("<div class='info-line'><b>15 min:</b> %.2f</div>", loadAvg[2]);
    printf("</div>");

    printf("<div class='card'>");
    printf("<h2>Memória detalhada</h2>");
    printf("<div class='info-line'><b>Total:</b> %lld MB</div>", memTotal / 1024);
    printf("<div class='info-line'><b>Usada:</b> %lld MB</div>", (memTotal - memAvailable) / 1024);
    printf("<div class='info-line'><b>Livre:</b> %lld MB</div>", memFree / 1024);
    printf("<div class='info-line'><b>Disponível:</b> %lld MB</div>", memAvailable / 1024);
    printf("</div>");

    printf("<div class='card'>");
    printf("<h2>Entrada e Saída</h2>");
    printf("<div class='info-line'><b>Setores lidos:</b> %lld</div>", totalReadSectors);
    printf("<div class='info-line'><b>Setores escritos:</b> %lld</div>", totalWriteSectors);
    printf("</div>");

    printf("</div>");

    printf("<h2 class='section-title'>Sistemas de arquivos suportados</h2>");
    printf("<div class='cards-container'>");
    printf("<div class='card full-width'>");
    printf("<ul>");
    for (int i = 0; i < fsCount; i++) {
        printf("<li>%s</li>", filesystem[i]);
    }
    printf("</ul>");
    printf("</div>");
    printf("</div>");

    printf("<h2 class='section-title'>Dispositivos (caractere e bloco) e grupos</h2>");
    printf("<div class='cards-container'>");
    printf("<div class='card full-width'>");
    printf("<pre>");
    for (int i = 0; i < devCount; i++) {
        printf("%s\n", devices[i]);
    }
    printf("</pre>");
    printf("</div>");
    printf("</div>");

    printf("<h2 class='section-title'>Dispositivos de rede</h2>");
    printf("<div class='cards-container'>");
    printf("<div class='card full-width'>");
    printf("<pre>");
    for (int i = 0; i < netCount; i++) {
        printf("%s\n", netDevices[i]);
    }
    printf("</pre>");
    printf("</div>");
    printf("</div>");

    printf("<h2 class='section-title'>Lista de processos em execução</h2>");
    printf("<div class='cards-container'>");
    printf("<div class='card full-width'>");
    printf("<table>");
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
                    }
                    fclose(procFile);
                }
            }
        }
        closedir(procDir);
    }

    printf("</table>");
    printf("</div>");
    printf("</div>");

    printf("</body>");
    printf("</html>");

    return 0;
}
