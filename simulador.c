#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int frame;
    bool valid;
    bool referenced;
} PageTableEntry;

typedef struct {
    int pid;
    int page;
    bool referenced;
} PhysicalFrame;

PhysicalFrame *frames;
PageTableEntry **pageTables;
//int *freeFrames;
int freeFrameCount;
int totalFrames;
int pageSize;
int processCount;
int clockPointer;
int totalAccesses;
int totalPageFaults;

void readConfig(const char *filename);
void initializeSimulation();
void processAccesses(const char *filename, const char *algorithm);
void handleAccess(int pid, int address, const char *algorithm);
int handlePageFault(int pid, int page, const char *algorithm, int *oldPid, int *oldPage);
//int findFreeFrame();
int fifoReplacement();
int clockReplacement();
void updatePageTable(int pid, int page, int frame, bool referenced);
void printHit(int pid, int address, int page, int offset, int frame);
void printPageFaultFree(int pid, int address, int page, int offset, int frame);
void printPageFaultReplace(int pid, int address, int page, int offset, 
                           int oldPid, int oldPage, int frame);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <fifo|clock> <arquivo_config> <arquivo_acessos>\n", argv[0]);
        return 1;
    }

    const char *algorithm = argv[1];
    const char *configFile = argv[2];
    const char *accessFile = argv[3];

    if (strcmp(algorithm, "fifo") != 0 && strcmp(algorithm, "clock") != 0) {
        printf("Erro: Algoritmo deve ser 'fifo' ou 'clock'\n");
        return 1;
    }

    readConfig(configFile);
    initializeSimulation();

    processAccesses(accessFile, algorithm);

    printf("--- Simulação Finalizada (Algoritmo: %s)\n", algorithm);
    printf("Total de Acessos: %d\n", totalAccesses);
    printf("Total de Page Faults: %d\n", totalPageFaults);

    for (int i = 0; i < processCount; i++) {
        if (pageTables[i] != NULL) {
             free(pageTables[i]);
        }
    }
    free(pageTables);
    free(frames);
    //free(freeFrames);

    return 0;
}

void readConfig(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro: Não foi possível abrir o arquivo de configuração %s\n", filename);
        exit(1);
    }

    if (fscanf(file, "%d", &totalFrames) != 1 ||
        fscanf(file, "%d", &pageSize) != 1 ||
        fscanf(file, "%d", &processCount) != 1) {
        printf("Erro ao ler parâmetros iniciais do arquivo de configuração.\n");
        fclose(file);
        exit(1);
    }

    pageTables = malloc(processCount * sizeof(PageTableEntry*));

    if (!pageTables) {
        perror("Erro de alocação de memória para pageTables");
        fclose(file);
        exit(1);
    }
    
    for (int i = 0; i < processCount; i++) {
        int pid, virtualSize;
        if (fscanf(file, "%d %d", &pid, &virtualSize) != 2) {
             printf("Erro ao ler dados do processo %d no arquivo de configuração.\n", i);
             fclose(file);
             exit(1);
        }
        
        int pageCount = (virtualSize + pageSize - 1) / pageSize;
        
        pageTables[i] = malloc(pageCount * sizeof(PageTableEntry));
        
        if (pageTables[i] == NULL) {
            perror("Erro de alocação de memória para Tabela de Páginas");
            fclose(file);
            exit(1);
        }

        for (int j = 0; j < pageCount; j++) {
            pageTables[i][j].frame = -1;
            pageTables[i][j].valid = false;
            pageTables[i][j].referenced = false;
        }
    }

    fclose(file);
}

void initializeSimulation() {
    frames = malloc(totalFrames * sizeof(PhysicalFrame));
    //freeFrames = malloc(totalFrames * sizeof(int));
    
    if (!frames) {
        perror("Erro de alocação de memória para frames ou freeFrames");
        exit(1);
    }

    for (int i = 0; i < totalFrames; i++) {
        frames[i].pid = -1;
        frames[i].page = -1;
        frames[i].referenced = false;
        //freeFrames[i] = i;
    }
    
    freeFrameCount = totalFrames;
    clockPointer = 0;
    totalAccesses = 0;
    totalPageFaults = 0;
}

void processAccesses(const char *filename, const char *algorithm) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro: Não foi possível abrir o arquivo de acessos %s\n", filename);
        exit(1);
    }

    int pid, address;
    while (fscanf(file, "%d %d", &pid, &address) == 2) {
        if (pid < 0 || pid >= processCount) {
             printf("Erro: PID %d inválido, ignorando acesso.\n", pid);
             continue;
        }

        int max_pages = 0;
        
        handleAccess(pid, address, algorithm);
    }

    fclose(file);
}

void handleAccess(int pid, int address, const char *algorithm) {
    totalAccesses++;
    
    int page = address / pageSize;
    int offset = address % pageSize;
    
    if (pageTables[pid][page].valid) {
        int frame = pageTables[pid][page].frame;
        
        pageTables[pid][page].referenced = true;
        frames[frame].referenced = true;
        
        printHit(pid, address, page, offset, frame);
    } else {
        totalPageFaults++;
        
        int oldPid = -1, oldPage = -1;
        
        int frame = handlePageFault(pid, page, algorithm, &oldPid, &oldPage);

        if (oldPid != -1) {
            printPageFaultReplace(pid, address, page, offset, oldPid, oldPage, frame);
        } else {
            printPageFaultFree(pid, address, page, offset, frame);
        }
    }
}

int handlePageFault(int pid, int page, const char *algorithm, int *oldPid, int *oldPage) {
    int frame;

    *oldPid = -1;
    *oldPage = -1;
    
    if (freeFrameCount > 0) {
        frame = clockPointer;
        clockPointer = (clockPointer + 1) % totalFrames; 
        freeFrameCount--;
    } else {
        if (strcmp(algorithm, "fifo") == 0) {
            frame = fifoReplacement();
        } else {
            frame = clockReplacement();
        }
        
        *oldPid = frames[frame].pid;
        *oldPage = frames[frame].page;
        
        pageTables[*oldPid][*oldPage].valid = false;
        pageTables[*oldPid][*oldPage].referenced = false;
    }
    
    updatePageTable(pid, page, frame, true);
    
    return frame;
}

int fifoReplacement() {
    int victimFrame = clockPointer;
    clockPointer = (clockPointer + 1) % totalFrames;
    
    return victimFrame;
}

int clockReplacement() {
    while (true) {
        if (!frames[clockPointer].referenced) {
            int victim = clockPointer;
            clockPointer = (clockPointer + 1) % totalFrames;
            return victim;
        } else {
            frames[clockPointer].referenced = false;
            pageTables[frames[clockPointer].pid][frames[clockPointer].page].referenced = false; 

            clockPointer = (clockPointer + 1) % totalFrames;
        }
    }
}

void updatePageTable(int pid, int page, int frame, bool referenced) {
    pageTables[pid][page].frame = frame;
    pageTables[pid][page].valid = true;
    pageTables[pid][page].referenced = referenced;
    
    frames[frame].pid = pid;
    frames[frame].page = page;
    frames[frame].referenced = referenced;
}

void printHit(int pid, int address, int page, int offset, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> HIT: Página %d (PID %d) já está no Frame %d\n", 
           pid, address, page, offset, page, pid, frame);
}

void printPageFaultFree(int pid, int address, int page, int offset, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Página %d (PID %d) alocada no Frame livre %d\n", 
           pid, address, page, offset, page, pid, frame);
}

void printPageFaultReplace(int pid, int address, int page, int offset, int oldPid, int oldPage, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. -> Página %d (PID %d) alocada no Frame %d\n",
           pid, address, page, offset, oldPage, oldPid, frame, page, pid, frame);
}
