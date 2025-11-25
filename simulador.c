#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Estrutura para entrada da tabela de páginas
typedef struct {
    int frame;        // Número do frame físico
    bool valido;       // Bit de validade
    bool referenciado; // Bit de referência (R-bit)
} EntradaTabelaPaginas;
// Estrutura para quadro físico
typedef struct {
    int pid;           // ID do processo que ocupa o frame
    int pagina;        // Número da página virtual
    bool referenciado; // Bit de referência (R-bit)
} FrameFisico;
// Variáveis globais
FrameFisico *frames;
EntradaTabelaPaginas **tabelasPaginas;
int contadorFrameLivres;
int totalFrames;
int tamanhoPagina;
int numeroProcessos;
int ponteiroClock;
int totalAcessos;
int totalPageFaults;

// Declarações de funções
void lerConfiguracao(const char *nomeArquivo);
void inicializarSimulacao();
void processarAcessos(const char *nomeArquivo, const char *algoritmo);
void tratarAcesso(int pid, int endereco, const char *algoritmo);
int tratarPageFault(int pid, int pagina, const char *algoritmo, int *pidAntigo, int *paginaAntiga);
int substituicaoFIFO();
int substituicaoClock();
void atualizarTabelaPaginas(int pid, int pagina, int frame, bool referenciado);
void imprimirHit(int pid, int endereco, int pagina, int deslocamento, int frame);
void imprimirPageFaultLivre(int pid, int endereco, int pagina, int deslocamento, int frame);
void imprimirPageFaultSubstituicao(int pid, int endereco, int pagina, int deslocamento, 
                                   int pidAntigo, int paginaAntiga, int frame);
                                   
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <fifo|clock> <arquivo_config> <arquivo_acessos>\n", argv[0]);
        return 1;
    }
    const char *algoritmo = argv[1];
    const char *arquivoConfig = argv[2];
    const char *arquivoAcessos = argv[3];
    if (strcmp(algoritmo, "fifo") != 0 && strcmp(algoritmo, "clock") != 0) {
        printf("Erro: Algoritmo deve ser 'fifo' ou 'clock'\n");
        return 1;
    }

    lerConfiguracao(arquivoConfig);
    inicializarSimulacao();
    processarAcessos(arquivoAcessos, algoritmo);
    printf("--- Simulação Finalizada (Algoritmo: %s)\n", algoritmo);
    printf("Total de Acessos: %d\n", totalAcessos);
    printf("Total de Page Faults: %d\n", totalPageFaults);
    // Liberar memória
    for (int i = 0; i < numeroProcessos; i++) {
        if (tabelasPaginas[i] != NULL) {
             free(tabelasPaginas[i]);
        }
    }
    free(tabelasPaginas);
    free(frames);
    return 0;
}

void lerConfiguracao(const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro: Não foi possível abrir o arquivo de configuração %s\n", nomeArquivo);
        exit(1);
    }
    if (fscanf(arquivo, "%d", &totalFrames) != 1 ||
        fscanf(arquivo, "%d", &tamanhoPagina) != 1 ||
        fscanf(arquivo, "%d", &numeroProcessos) != 1) {
        printf("Erro ao ler parâmetros iniciais do arquivo de configuração.\n");
        fclose(arquivo);
        exit(1);
    }
    tabelasPaginas = malloc(numeroProcessos * sizeof(EntradaTabelaPaginas*));
    if (!tabelasPaginas) {
        perror("Erro de alocação de memória para pageTables");
        fclose(arquivo);
        exit(1);
    }
    
    for (int i = 0; i < numeroProcessos; i++) {
        int pid, tamanhoVirtual;
        if (fscanf(arquivo, "%d %d", &pid, &tamanhoVirtual) != 2) {
             printf("Erro ao ler dados do processo %d no arquivo de configuração.\n", i);
             fclose(arquivo);
             exit(1);
        }
        int numeroPaginas = (tamanhoVirtual + tamanhoPagina - 1) / tamanhoPagina;
        tabelasPaginas[i] = malloc(numeroPaginas * sizeof(EntradaTabelaPaginas));
        if (tabelasPaginas[i] == NULL) {
            perror("Erro de alocação de memória para Tabela de Páginas");
            fclose(arquivo);
            exit(1);
        }
// Inicializar todas as entradas da tabela de páginas
        for (int j = 0; j < numeroPaginas; j++) {
            tabelasPaginas[i][j].frame = -1;
            tabelasPaginas[i][j].valido = false;
            tabelasPaginas[i][j].referenciado  = false;
        }
    }
    fclose(arquivo);
}

void inicializarSimulacao() {
    frames = malloc(totalFrames * sizeof(FrameFisico));
    
    if (!frames) {
        perror("Erro de alocação de memória para frames ou freeFrames");
        exit(1);
    }
// Inicializar todos os frame como livres
    for (int i = 0; i < totalFrames; i++) {
        frames[i].pid = -1;
        frames[i].pagina = -1;
        frames[i].referenciado = false;
    }
    
    contadorFrameLivres = totalFrames;
    ponteiroClock = 0;
    totalAcessos = 0;
    totalPageFaults = 0;
}

void processarAcessos(const char *nomeArquivo, const char *algoritmo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro: Não foi possível abrir o arquivo de acessos %s\n", nomeArquivo);
        exit(1);
    }

    int pid, endereco;
    while (fscanf(arquivo, "%d %d", &pid, &endereco) == 2) {
        if (pid < 0 || pid >= numeroProcessos) {
             printf("Erro: PID %d inválido, ignorando acesso.\n", pid);
             continue;
        }

        int max_pages = 0;
        
        tratarAcesso(pid, endereco, algoritmo);
    }

    fclose(arquivo);
}

void tratarAcesso(int pid, int endereco, const char *algoritmo) {
    totalAcessos++;
    // Traduzir endereço virtual em número de página e deslocamento
    int pagina = endereco / tamanhoPagina;
    int deslocamento = endereco % tamanhoPagina;
    // Verificar se a página está na memória
    if (tabelasPaginas[pid][pagina].valido) {
        // HIT - Página já está na memória
        int frame = tabelasPaginas[pid][pagina].frame;
        
        // Atualizar bit de referência
        tabelasPaginas[pid][pagina].referenciado = true;
        frames[frame].referenciado = true;
        
        imprimirHit(pid, endereco, pagina, deslocamento, frame);
    } else {
        totalPageFaults++;
        
        int pidAntigo = -1, paginaAntiga = -1;
        
        int frame = tratarPageFault(pid, pagina, algoritmo, &pidAntigo, &paginaAntiga);

        if (pidAntigo != -1) { // Substituição de página
            imprimirPageFaultSubstituicao(pid, endereco, pagina, deslocamento, pidAntigo, paginaAntiga, frame);
        } else { // Alocação em frame livre
            imprimirPageFaultLivre(pid, endereco, pagina, deslocamento, frame);
        }
    }
}

int tratarPageFault(int pid, int pagina, const char *algoritmo, int *pidAntigo, int *paginaAntiga) {
    int frame;

    *pidAntigo = -1;
    *paginaAntiga = -1;
    
    if (contadorFrameLivres > 0) { 
        frame = ponteiroClock;
        ponteiroClock = (ponteiroClock + 1) % totalFrames; 
        contadorFrameLivres--;
    } else { // Memória cheia, aplicar algoritmo de substituição
        if (strcmp(algoritmo, "fifo") == 0) {
            frame = substituicaoFIFO();
        } else {
            frame = substituicaoClock();
        }
    
        // Registrar informações da página substituída
        *pidAntigo = frames[frame].pid;
        *paginaAntiga = frames[frame].pagina;
        
        tabelasPaginas[*pidAntigo][*paginaAntiga].valido = false;
        tabelasPaginas[*pidAntigo][*paginaAntiga].referenciado = false;
    }
    // Atualizar tabela de páginas com a nova página
    atualizarTabelaPaginas(pid, pagina, frame, true);
    
    return frame;
}

int substituicaoFIFO() {
    int vitima = ponteiroClock;
    ponteiroClock = (ponteiroClock + 1) % totalFrames;
    return vitima;
}

int substituicaoClock() {
    while (true) {
        if (!frames[ponteiroClock].referenciado) {
        	// Encontrou página com R-bit = 0
            int vitima  = ponteiroClock;
            ponteiroClock = (ponteiroClock + 1) % totalFrames;
            return vitima ;
        } else {  // Dar segunda chance: zerar R-bit e continuar
            frames[ponteiroClock].referenciado = false;
            tabelasPaginas[frames[ponteiroClock].pid][frames[ponteiroClock].pagina ].referenciado = false; 

            ponteiroClock = (ponteiroClock + 1) % totalFrames;
        }
    }
}

void atualizarTabelaPaginas(int pid, int pagina, int frame, bool referenciado) {
    // Atualizar entrada na tabela de páginas do processo
	tabelasPaginas[pid][pagina].frame = frame;
    tabelasPaginas[pid][pagina].valido  = true;
    tabelasPaginas[pid][pagina].referenciado = referenciado;
    // Atualizar informações do frame físico
    frames[frame].pid = pid;
    frames[frame].pagina = pagina;
    frames[frame].referenciado = referenciado;
}

void imprimirHit(int pid, int endereco, int pagina, int deslocamento, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> HIT: Página %d (PID %d) já está no Frame %d\n", 
           pid, endereco, pagina, deslocamento, pagina, pid, frame);
}

void imprimirPageFaultLivre(int pid, int endereco, int pagina, int deslocamento, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Página %d (PID %d) alocada no Frame livre %d\n", 
           pid, endereco, pagina, deslocamento, pagina, pid, frame);
}

void imprimirPageFaultSubstituicao(int pid, int endereco, int pagina, int deslocamento, int pidAntigo, int paginaAntiga, int frame) {
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. -> Página %d (PID %d) alocada no Frame %d\n",
           pid, endereco, pagina, deslocamento, paginaAntiga, pidAntigo, frame, pagina, pid, frame);
}