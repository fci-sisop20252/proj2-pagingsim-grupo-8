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
int *freeFrames;            
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
void handlePageFault(int pid, int page, const char *algorithm);
int findFreeFrame();
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
        free(pageTables[i]);
    }
    free(pageTables);
    free(frames);
    free(freeFrames);

    return 0;
}

void readConfig(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro: Não foi possível abrir o arquivo de configuração %s\n", filename);
        exit(1);
    }

    fscanf(file, "%d", &totalFrames);
    fscanf(file, "%d", &pageSize);
    fscanf(file, "%d", &processCount);

    pageTables = malloc(processCount * sizeof(PageTableEntry*));
    
    for (int i = 0; i < processCount; i++) {
        int pid, virtualSize;
        fscanf(file, "%d %d", &pid, &virtualSize);
        
        int pageCount = (virtualSize + pageSize - 1) / pageSize;
        
        pageTables[pid] = malloc(pageCount * sizeof(PageTableEntry));
        
        for (int j = 0; j < pageCount; j++) {
            pageTables[pid][j].frame = -1;
            pageTables[pid][j].valid = false;
            pageTables[pid][j].referenced = false;
        }
    }

    fclose(file);
}

void initializeSimulation() {
    frames = malloc(totalFrames * sizeof(PhysicalFrame));
    freeFrames = malloc(totalFrames * sizeof(int));
    
    for (int i = 0; i < totalFrames; i++) {
        frames[i].pid = -1;
        frames[i].page = -1;
        frames[i].referenced = false;
        freeFrames[i] = i;  
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
        handlePageFault(pid, page, algorithm);
        
        int frame = pageTables[pid][page].frame;
        printPageFaultFree(pid, address, page, offset, frame);
    }
}

void handlePageFault(int pid, int page, const char *algorithm) {
    int frame;
    
    if (freeFrameCount > 0) {
        frame = findFreeFrame();
    } else {
        if (strcmp(algorithm, "fifo") == 0) {
            frame = fifoReplacement();
        } else {
            frame = clockReplacement();
        }
        
        int oldPid = frames[frame].pid;
        int oldPage = frames[frame].page;
        pageTables[oldPid][oldPage].valid = false;
        
        printf(" -> Memória cheia. Página %d (PID %d) (Frame %d) será desalocada.", 
               oldPage, oldPid, frame);
    }
    
    updatePageTable(pid, page, frame, true);
}

int findFreeFrame() {
    if (freeFrameCount == 0) return -1;
    
    int frame = freeFrames[--freeFrameCount];
    return frame;
}

int fifoReplacement() {
    static int fifoPointer = 0;
    fifoPointer = (fifoPointer + 1) % totalFrames;
    int frame = fifoPointer;
    return frame;
}

int clockReplacement() {
    while (true) {
        if (!frames[clockPointer].referenced) {
            int victim = clockPointer;
            clockPointer = (clockPointer + 1) % totalFrames;
            return victim;
        } else {
            frames[clockPointer].referenced = false;
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

void printPageFaultReplace(int pid, int address, int page, int offset, 
                          int oldPid, int oldPage, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. -> Página %d (PID %d) alocada no Frame %d\n",
           pid, address, page, offset, oldPage, oldPid, frame, page, pid, frame);
}
