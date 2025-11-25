# Relatório do Projeto 2: Simulador de Memória Virtual

**Disciplina:** Sistemas Operacionais
**Professor:** Lucas Figueiredo
**Data:**

## Integrantes do Grupo

- Geovana Bomfim Rodrigues - 10410514
- Yating Zheng - 10439511

---

## 1. Instruções de Compilação e Execução

### 1.1 Compilação

Descreva EXATAMENTE como compilar seu projeto. Inclua todos os comandos necessários.

**Exemplo:**
```bash
gcc -o simulador simulador.c
```

ou

```bash
make
```

Para compilar o nosso simulador e só usar:
gcc -o simulador simulador.c

### 1.2 Execução

Forneça exemplos completos de como executar o simulador.

**Exemplo com FIFO:**
```bash
./simulador fifo tests/config_1.txt tests/acessos_1.txt
```

**Exemplo com Clock:**
```bash
./simulador clock tests/config_1.txt tests/acessos_1.txt
```
./simulador fifo tests/config_1.txt tests/acessos_1.txt
./simulador clock tests/config_1.txt tests/acessos_1.txt
./simulador fifo tests/config_2.txt tests/acessos_2.txt
./simulador clock tests/config_2.txt tests/acessos_2.txt
---

## 2. Decisões de Design

### 2.1 Estruturas de Dados

Descreva as estruturas de dados que você escolheu para representar:

**Tabela de Páginas:**
- Qual estrutura usou? (array, lista, hash map, etc.)
Decidimos usar uma matriz.
- Quais informações armazena para cada página?
Para cada página armazenamos o valor inteiro que representa o frame, um valor booleano que representa o valor de validade e outro valor booleano que representa um bit de referencia.
- Como organizou para múltiplos processos?

- **Justificativa:** Por que escolheu essa abordagem?


**Frames Físicos:**
- Como representou os frames da memória física?

- Quais informações armazena para cada frame?
Um valor inteiro que representa o PID, outro valor inteiro que representa a página e o bit de referencia.
- Como rastreia frames livres vs ocupados?
Usamos o proprio ponteiro para acompanhar o frames livres, pois começamos no inicio e vamos avançando, eventualmente acabamos com o livres e usamos o algoritmo escolhido para a substituição das paginas.
- **Justificativa:** Por que escolheu essa abordagem?
A principio, estavamos usando um vetor que guardava os frames livres e uma função que procurava frames livres, porém a saida estava dando diferença das saidas esperadas, sendo assim, fomos testando pequenas mudanças, e concluimos que "fazer" uma copia de informações pode gerar alguns erros no caminho, causando irregulidades nas informações. Sendo assim, decidimos por tirar isso, e adaptar para que não seja mais necessario seu uso. 

**Estrutura para FIFO:**
- Como mantém a ordem de chegada das páginas?
- Como identifica a página mais antiga?
- **Justificativa:** Por que escolheu essa abordagem?

**Estrutura para Clock:**
- Como implementou o ponteiro circular?
Implemento usando aritmetica, basicamente usamos a logica de uma lista circular.
- Como armazena e atualiza os R-bits?
Armazenamos em um variavel da struct, sendo assim, vamos analizando onde o ponteiro aponta, e vamos atualizando esse bit de acordo com a logica, se for 1 mudamos pra zero e ser for zero, escolhemos essa oagina como vitima.
- **Justificativa:** Por que escolheu essa abordagem?
Escolhemos essa abordagem, pois é a que fez mais sentido pra gente, pois, a logica desse algoritmo é o "metodo do relogio", que vai andando até encontrar uma pagina que ja teve uma chance.

### 2.2 Organização do Código

Descreva como organizou seu código:

- Quantos arquivos/módulos criou?
Criamos 10 modulos.
- Qual a responsabilidade de cada arquivo/módulo?
Temos modulos de processamento de arquivo, modulo de inicialização, modulos de alocação de paginas e modulos de realocação de paginas.
- Quais são as principais funções e o que cada uma faz?


**Exemplo:**
```
simulador.c
├── main() - lê argumentos e coordena execução
├── ler_config() - processa arquivo de configuração
├── processar_acessos() - loop principal de simulação
├── traduzir_endereco() - calcula página e deslocamento
├── consultar_tabela() - verifica se página está na memória
├── tratar_page_fault() - lida com page faults
├── algoritmo_fifo() - seleciona vítima usando FIFO
└── algoritmo_clock() - seleciona vítima usando Clock
```

```
simulador.c
├── main() - lê argumentos, verifica a validade dos argumentos e coordena execução
├── readConfig() - processa arquivo de configuração
├── initializeSimulation() - inicia a paginação, dividindo o tamanho da paginas
├── processAccesses() - loop principal de simulação
├── handleAccess() - verifica se a pagina esta alocada
├── handlePageFault() - tenta alocar a pagina
├── updatePageTable() - atualiza as informações da tabela
├── printHit() - imprime os dados caso entrote a pagina
├── printPageFaultFree() - impreime os dados caso entre um frame livre
├── printPageFaultReplace() - imprime caso tenha que realocar uma pagina
├── algoritmo_fifo() - seleciona vítima usando FIFO
└── algoritmo_clock() - seleciona vítima usando Clock
```

### 2.3 Algoritmo FIFO

Explique **como** implementou a lógica FIFO:

- Como mantém o controle da ordem de chegada?
- Como seleciona a página vítima?
- Quais passos executa ao substituir uma página?

**Não cole código aqui.** Explique a lógica em linguagem natural.

### 2.4 Algoritmo Clock

Explique **como** implementou a lógica Clock:

- Como gerencia o ponteiro circular?
Incrementamos o ponteiro e pegamos o resto da divisão pelo numero de paginas.
- Como implementou a "segunda chance"?
Usando uma variavel booleana e ifs para verificar se uma pagina tem ou não uma segunda chance, sendo assim, se for 1, colocamos o R-bit como 0 e passamos pra proxima pagina.
- Como trata o caso onde todas as páginas têm R=1?
- Como garante que o R-bit é setado em todo acesso?

**Não cole código aqui.** Explique a lógica em linguagem natural.

### 2.5 Tratamento de Page Fault

Explique como seu código distingue e trata os dois cenários:

**Cenário 1: Frame livre disponível**
- Como identifica que há frame livre?
- Quais passos executa para alocar a página?

**Cenário 2: Memória cheia (substituição)**
- Como identifica que a memória está cheia?
- Como decide qual algoritmo usar (FIFO vs Clock)?
- Quais passos executa para substituir uma página?

---

## 3. Análise Comparativa FIFO vs Clock

### 3.1 Resultados dos Testes

Preencha a tabela abaixo com os resultados de pelo menos 3 testes diferentes:

| Descrição do Teste        | Total de Acessos | Page Faults FIFO | Page Faults Clock | Diferença |
|---------------------------|------------------|------------------|-------------------|-----------|
| Teste 1 - Básico          |                  |                  |                   |           |
| Teste 2 - Memória Pequena |                  |                  |                   |           |
| Teste 3 - Simples         |                  |                  |                   |           |
| Teste Próprio 1           |                  |                  |                   |           |

### 3.2 Análise

Com base nos resultados acima, responda:

1. **Qual algoritmo teve melhor desempenho (menos page faults)?**

2. **Por que você acha que isso aconteceu?** Considere:
   - Como cada algoritmo escolhe a vítima
   - O papel do R-bit no Clock
   - O padrão de acesso dos testes

3. **Em que situações Clock é melhor que FIFO?**
   - Dê exemplos de padrões de acesso onde Clock se beneficia

4. **Houve casos onde FIFO e Clock tiveram o mesmo resultado?**
   - Por que isso aconteceu?

5. **Qual algoritmo você escolheria para um sistema real e por quê?**

---

## 4. Desafios e Aprendizados

### 4.1 Maior Desafio Técnico

Descreva o maior desafio técnico que seu grupo enfrentou durante a implementação:

- Qual foi o problema?

- Como identificaram o problema?
- Como resolveram?
- O que aprenderam com isso?

### 4.2 Principal Aprendizado

Descreva o principal aprendizado sobre gerenciamento de memória que vocês tiveram com este projeto:

- O que vocês não entendiam bem antes e agora entendem?
Tinhamos uma dificuldade de entender de fato o funcionamento dos algoritmos, porem como tivemos implemntar, as coisas fizeram mais sentido agora.
- Como este projeto mudou sua compreensão de memória virtual?
A nossa compreensão de como a paginação é importante.
- Que conceito das aulas ficou mais claro após a implementação?
Paginação no geral.

---

## 5. Vídeo de Demonstração

**Link do vídeo:** [Insira aqui o link para YouTube, Google Drive, etc.]

### Conteúdo do vídeo:

Confirme que o vídeo contém:

- [ ] Demonstração da compilação do projeto
- [ ] Execução do simulador com algoritmo FIFO
- [ ] Execução do simulador com algoritmo Clock
- [ ] Explicação da saída produzida
- [ ] Comparação dos resultados FIFO vs Clock
- [ ] Breve explicação de uma decisão de design importante

---

## Checklist de Entrega

Antes de submeter, verifique:

- [X] Código compila sem erros conforme instruções da seção 1.1
- [X] Simulador funciona corretamente com FIFO
- [X] Simulador funciona corretamente com Clock
- [X] Formato de saída segue EXATAMENTE a especificação do ENUNCIADO.md
- [x] Testamos com os casos fornecidos em tests/
- [ ] Todas as seções deste relatório foram preenchidas
- [ ] Análise comparativa foi realizada com dados reais
- [ ] Vídeo de demonstração foi gravado e link está funcionando
- [ ] Todos os integrantes participaram e concordam com a submissão

---
## Referências
Liste aqui quaisquer referências que utilizaram para auxiliar na implementação (livros, artigos, sites, **links para conversas com IAs.**)


---

## Comentários Finais

Use este espaço para quaisquer observações adicionais que julguem relevantes (opcional).

---
