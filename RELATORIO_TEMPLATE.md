# Relatório do Projeto 2: Simulador de Memória Virtual

**Disciplina:** Sistemas Operacionais
**Professor:** Lucas Figueiredo
**Data:** 24/11/2025

## Integrantes do Grupo

- Geovana Bomfim Rodrigues - 10410514
- Yating Zheng - 10439511

---

## 1. Instruções de Compilação e Execução

### 1.1 Compilação

Descreva EXATAMENTE como compilar seu projeto. Inclua todos os comandos necessários.

Para compilar o nosso simulador e só usar: gcc -o simulador simulador.c

### 1.2 Execução

Forneça exemplos completos de como executar o simulador.

./simulador fifo tests/config_1.txt tests/acessos_1.txt > output.txt
diff output.txt tests/expected_fifo_1.txt

./simulador clock tests/config_1.txt tests/acessos_1.txt > output.txt
diff output.txt tests/expected_clock_1.txt
—-----------------------------

./simulador fifo tests/config_2.txt tests/acessos_2.txt > output.txt
diff output.txt tests/expected_fifo_2.txt

./simulador clock tests/config_2.txt tests/acessos_2.txt > output.txt
diff output.txt tests/expected_clock_2.txt

—----------------
./simulador fifo tests/config_3.txt tests/acessos_3.txt > output.txt
diff output.txt tests/expected_fifo_3.txt

./simulador clock tests/config_3.txt tests/acessos_3.txt > output.txt
diff output.txt tests/expected_clock_3.txt


—-------------------------
./simulador fifo tests/config_4.txt tests/acessos_4.txt > output.txt
diff output.txt tests/expected_fifo_4.txt

./simulador clock tests/config_4.txt tests/acessos_4.txt > output.txt
diff output.txt tests/expected_clock_4.txt

—-------------------------------

./simulador fifo tests/config_5.txt tests/acessos_5.txt > output.txt
diff output.txt tests/expected_fifo_5.txt

./simulador clock tests/config_5.txt tests/acessos_5.txt > output.txt
diff output.txt tests/expected_clock_5.txt


—------------------------
./simulador fifo tests/config_6.txt tests/acessos_6.txt > output.txt
diff output.txt tests/expected_fifo_6.txt

./simulador clock tests/config_6.txt tests/acessos_6.txt > output.txt
diff output.txt tests/expected_clock_6.txt
--------------------------
./simulador fifo tests/config_proprio.txt tests/acessos_proprio.txt > output.txt

./simulador clock tests/config_proprio.txt tests/acessos_proprio.txt > output.txt



## 2. Decisões de Design

### 2.1 Estruturas de Dados

Descreva as estruturas de dados que você escolheu para representar:

**Tabela de Páginas:**
- Qual estrutura usou? (array, lista, hash map, etc.)
Decidimos usar uma matriz.

- Quais informações armazena para cada página?
Para cada página armazenamos o valor inteiro que representa o frame, um valor booleano que representa o valor de validade e outro valor booleano que representa um bit de referencia.

- Como organizou para múltiplos processos?
Array onde o primeiro índice é o PID e o segundo é o número da página

- **Justificativa:** Por que escolheu essa abordagem?
Acesso direto por índice é eficiente e simples de implementar

**Frames Físicos:**
- Como representou os frames da memória física?
Array de structs

- Quais informações armazena para cada frame?
Um valor inteiro que representa o PID, outro valor inteiro que representa a página e o bit de referencia.


- Como rastreia frames livres vs ocupados?
Usamos o proprio ponteiro para acompanhar o frames livres, pois começamos no inicio e vamos avançando, eventualmente acabamos com o livres e usamos o algoritmo escolhido para a substituição das paginas.

- **Justificativa:** Por que escolheu essa abordagem?
A principio, estavamos usando um vetor que guardava os frames livres e uma função que procurava frames livres, porém a saida estava dando diferença das saidas esperadas, sendo assim, fomos testando pequenas mudanças, e concluimos que "fazer" uma copia de informações pode gerar alguns erros no caminho, causando irregulidades nas informações. Sendo assim, decidimos por tirar isso, e adaptar para que não seja mais necessario seu uso.

**Estrutura para FIFO:**
- Como mantém a ordem de chegada das páginas?
Ponteiro ponteiroClock que avança circularmente

- Como identifica a página mais antiga?
Sempre seleciona o frame apontado por ponteiroClock

- **Justificativa:** Por que escolheu essa abordagem?
Implementação simples e eficiente para o algoritmo FIFO

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

· main() - Coordena execução e gerencia memória
· lerConfiguracao() - Processa arquivo de configuração
· inicializarSimulacao() - Inicializa estruturas de dados
· processarAcessos() - Loop principal de simulação
· tratarAcesso() - Processa cada acesso individual
· tratarPageFault() - Trata faltas de página
· substituicaoFIFO() - Algoritmo FIFO
· substituicaoClock() - Algoritmo Clock
· atualizarTabelaPaginas() - Atualiza tabela de páginas
· imprimirHit() - imprime os dados caso entrote a pagina
· imprimirPageFaultLivre() - imprime os dados caso entre um frame livre
· imprimirPageFaultSubstituicao() - imprime caso tenha que realocar uma pagina

### 2.3 Algoritmo FIFO

Explique **como** implementou a lógica FIFO:

- Como mantém o controle da ordem de chegada?
- Como seleciona a página vítima?
- Quais passos executa ao substituir uma página?

**Não cole código aqui.** Explique a lógica em linguagem natural.
· Mantém um ponteiro fifoPointer que avança circularmente pelos frames
· Seleciona sempre o frame apontado pelo ponteiro como vítima
· Após substituição, avança o ponteiro para o próximo frame
· Não considera padrão de acesso, apenas ordem de chegada

### 2.4 Algoritmo Clock

Explique **como** implementou a lógica Clock:

- Como gerencia o ponteiro circular?
- Como implementou a "segunda chance"?
- Como trata o caso onde todas as páginas têm R=1?
- Como garante que o R-bit é setado em todo acesso?

**Não cole código aqui.** Explique a lógica em linguagem natural.
· Incrementamos o ponteiro e pegamos o resto da divisão pelo numero de paginas.
· Usando uma variavel booleana e ifs para verificar se uma pagina tem ou não uma segunda chance, sendo assim, se for 1, colocamos o R-bit como 0 e passamos pra proxima pagina.
· Atualiza R-bit tanto no frame quanto na tabela de páginas correspondente
· Garante que todas as páginas recebem chance antes de serem substituídas

### 2.5 Tratamento de Page Fault

Explique como seu código distingue e trata os dois cenários:

**Cenário 1: Frame livre disponível**
- Como identifica que há frame livre?
contadorFrameLivres > 0

- Quais passos executa para alocar a página?
Seleciona próximo frame livre, atualiza tabela de páginas, decrementa contador

**Cenário 2: Memória cheia (substituição)**
- Como identifica que a memória está cheia?
contadorFrameLivres == 0

- Como decide qual algoritmo usar (FIFO vs Clock)?
Baseada no parâmetro de linha de comando

- Quais passos executa para substituir uma página?
Executa algoritmo de substituição, remove página vítima, aloca nova página

---

## 3. Análise Comparativa FIFO vs Clock

### 3.1 Resultados dos Testes

Preencha a tabela abaixo com os resultados de pelo menos 3 testes diferentes:

| Descrição do Teste |       Total de Acessos       | Page Faults FIFO | Page Faults Clock | Diferença |
|--------------------|------------------------------|------------------|-------------------|-----------|
| Teste 1 - Básico   |              8               |         5        |           5       |     0     |
| Teste 2 - Memória Pequena |       10              |        10        |          10       |     0     |
| Teste 3 - Simples |               7               |         4        |           4       |     0     |
| Teste Próprio 1   |               14              |         10       |           11      |     -1    |

### 3.2 Análise

Com base nos resultados acima, responda:

1. **Qual algoritmo teve melhor desempenho (menos page faults)?**
Nos testes básicos, ambos tiveram o mesmo desempenho. No teste próprio com padrão repetitivo, Clock teve melhor desempenho com 2 page faults a menos.

2. **Por que você acha que isso aconteceu?** Considere:
   - Como cada algoritmo escolhe a vítima
   FIFO sempre remove a página mais antiga, independentemente do uso

   - O papel do R-bit no Clock
   Clock preserva páginas frequentemente acessadas através do R-bit

   - O padrão de acesso dos testes
   Nos testes básicos, o padrão de acesso não favoreceu significativamente o Clock


3. **Em que situações Clock é melhor que FIFO?**
   - Dê exemplos de padrões de acesso onde Clock se beneficia
Clock é superior em cenários com:
   · Acessos repetitivos às mesmas páginas
   · Padrões de "working set" onde algumas páginas são muito utilizadas
   · Processos com localidade temporal forte

4. **Houve casos onde FIFO e Clock tiveram o mesmo resultado?**
   - Por que isso aconteceu?
   Sim, nos testes 1, 2 e 3 fornecidos. Isso ocorreu porque os padrões de acesso não exploraram a vantagem do R-bit.


5. **Qual algoritmo você escolheria para um sistema real e por quê?**
Clock, pois oferece melhor desempenho em cenários reais com custo computacional similar ao FIFO.

---

## 4. Desafios e Aprendizados

### 4.1 Maior Desafio Técnico

Descreva o maior desafio técnico que seu grupo enfrentou durante a implementação:

- Qual foi o problema?
Gerenciamento correto do R-bit entre frames físicos e tabelas de páginas

- Como identificaram o problema?
Durante testes, notamos inconsistências no comportamento do Clock

- Como resolveram?
Implementamos atualização sincronizada do R-bit tanto na estrutura do frame quanto na entrada correspondente da tabela de páginas

- O que aprenderam com isso?
A importância da consistência entre diferentes estruturas de dados em sistemas de memória virtual


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

**Link do vídeo:** [https://youtu.be/jMpjKbJbCnw?si=v9o3I79Hpc6G3iZ-]

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

- [ ] Código compila sem erros conforme instruções da seção 1.1
- [ ] Simulador funciona corretamente com FIFO
- [ ] Simulador funciona corretamente com Clock
- [ ] Formato de saída segue EXATAMENTE a especificação do ENUNCIADO.md
- [ ] Testamos com os casos fornecidos em tests/
- [ ] Todas as seções deste relatório foram preenchidas
- [ ] Análise comparativa foi realizada com dados reais
- [ ] Vídeo de demonstração foi gravado e link está funcionando
- [ ] Todos os integrantes participaram e concordam com a submissão

---
## Referências
Liste aqui quaisquer referências que utilizaram para auxiliar na implementação (livros, artigos, sites, **links para conversas com IAs.**)

ARPACI-DUSSEAU, R. H.; ARPACI-DUSSEAU, A. C. Operating Systems: Three Easy Pieces. ArpaciDusseau Books, versão 1.10, novembro de 2023. 
---

## Comentários Finais

Use este espaço para quaisquer observações adicionais que julguem relevantes (opcional).

---
