# Sistema de Arquivos - Trabalho SO

Este projeto implementa um sistema de arquivos simples em C, simulando um disco armazenado em um arquivo (disco.img).

## Estrutura do Disco

- **Boot Record:** Contém informações sobre o layout do disco, incluindo bytes por setor, setores por bloco, quantidade de setores reservados, diretório, bitmap e dados.  
- **Diretório:** Array de entradas (DirEntry) armazenando metadados de arquivos (nome, extensão, status, setor inicial, tamanho, etc).
- **Bitmap:** Gerencia a alocação dos setores de dados.
- **Área de Dados:** Espaço onde os arquivos são armazenados.

## Funcionalidades

- **Formatar Disco:** Cria a imagem do disco (`disco.img`), inicializando o Boot Record, diretório, bitmap e área de dados.
- **Copiar Arquivo do Sistema para o Disco:** Lê um arquivo fonte e o armazena no disco, atualizando o diretório e o bitmap.
- **Copiar Arquivo do Disco para o Sistema:** Lê um arquivo presente no disco a partir do diretório e o salva no sistema.
- **Listar Arquivos:** Exibe as entradas do diretório, mostrando informações dos arquivos armazenados.
- **Remover Arquivo:** Remove um arquivo do disco, liberando os setores correspondentes no bitmap e atualizando o diretório e o Boot Record.
- **Exibir Disco:** Exibe o conteúdo completo do disco, incluindo Boot Record, diretório e bitmap.

## Uso

Execute o programa e escolha a opção desejada no menu interativo:

1. Formatar disco  
2. Copiar arquivo do disco para o sistema  
3. Copiar arquivo do sistema para o disco  
4. Listar arquivos  
5. Remover arquivo  
6. Exibir disco  
0. Sair

O projeto utiliza funções da biblioteca padrão C para manipulação de arquivos, com tratamento básico de erros e mensagens informativas.

## Compilação

```bash
gcc sa.c -o sa
```

## Execução

```bash
./sa
```

## Licença

Este projeto é licenciado sob a [MIT License](LICENSE).