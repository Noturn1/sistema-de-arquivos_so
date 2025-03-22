# Sistema de Arquivos - Trabalho de Sistemas Operacionais

Este projeto implementa um sistema de arquivos simples em C, desenvolvido como parte de um trabalho para a disciplina de Sistemas Operacionais. O sistema de arquivos simula operações básicas de gerenciamento de arquivos em um disco virtual.

## Estrutura do Projeto

- **`sistema-de-arquivos.c`**: Contém a implementação principal do sistema de arquivos, incluindo as funções para formatação do disco, manipulação de arquivos e o menu interativo.
- **`disco.img`**: Arquivo que simula o disco virtual (gerado durante a execução do programa).
- **`LICENSE`**: Arquivo de licença do projeto.
- **`README.md`**: Este arquivo, contendo informações sobre o projeto.

## Funcionalidades Implementadas

1. **Formatação do Disco**:
   - Cria um disco virtual (`disco.img`) com as seguintes características:
     - Setores de 512 bytes.
     - 8 setores por bloco.
     - 1 setor reservado.
     - 1 setor para o diretório.
     - 2 setores para o bitmap.
     - 196 setores para dados.
   - Inicializa o boot record, o diretório, o bitmap e a área de dados.

2. **Menu Interativo**:
   - O programa apresenta um menu com as seguintes opções:
     1. Formatar disco.
     2. Copiar arquivo do disco para o sistema (não implementado).
     3. Copiar arquivo do sistema para o disco (não implementado).
     4. Listar arquivos no disco (não implementado).
     5. Remover arquivo do disco (não implementado).
     0. Sair.

3. **Estruturas de Dados**:
   - **Boot Record**:
     - Contém informações sobre o sistema de arquivos, como tamanho dos setores, número de setores reservados, bitmap, diretório e dados.
   - **Entrada de Diretório**:
     - Representa os arquivos armazenados no disco, com informações como nome, extensão, tamanho e setor inicial.

## Como Executar

1. Compile o programa:
   ```bash
   gcc sistema-de-arquivos.c -o sistema-de-arquivos

2. Execute o programa:
    ```bash
    ./sistema-de-arquivos