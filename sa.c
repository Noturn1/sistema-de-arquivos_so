#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Define os valores de acordo com a especificação 
#define BYTES_PER_SECTOR 512
#define SECTORS_PER_BLOCK 8
#define RESERVED_SECTORS 1
#define DIR_SECTORS 2
#define BITMAP_SECTORS 1
#define DATA_SECTORS 196
#define TOTAL_SECTORS (RESERVED_SECTORS + DIR_SECTORS + BITMAP_SECTORS + DATA_SECTORS)

// Define a estrutura do boot record e entrada do diretório
typedef struct {
    unsigned short bytes_por_sector;   // 2 bytes
    unsigned char sectors_per_block;   // 1 byte 
    unsigned short reserved_sectors;   // 2 bytes
    unsigned short dir_sectors;        // 2 bytes
    unsigned short data_sectors;       // 2 bytes
    unsigned short bitmap_sectors;     // 2 bytes
    unsigned short total_sectors;      // 2 bytes
    unsigned int file_count;           // 4 bytes
    unsigned int first_free_sector;    // 4 bytes
    char reserved[10];                 // 10 bytes 
} BootRecord;

typedef struct __attribute__((packed)) {
    unsigned char status;      // 1 byte: 0x00 para válido, 0xFF para deletado
    char filename[12];         // 12 bytes para o nome do arquivo
    char extension[4];         // 4 bytes para a extensão
    unsigned char attributes;  // 1 byte para atributos
    unsigned int first_sector; // 4 bytes: setor inicial do arquivo
    unsigned int file_size;    // 4 bytes: tamanho do arquivo em bytes
    char reserved[6];          // 6 bytes reservados para futuras expansões
} DirEntry;

void formatar_disco(const char *disk_filename) {
    FILE *disk = fopen(disk_filename, "wb");
    
    if(!disk) {
        perror("Erro ao abrir o arquivo...");
        return;
    }

    BootRecord br;
    br.bytes_por_sector = BYTES_PER_SECTOR;
    br.sectors_per_block = SECTORS_PER_BLOCK;
    br.reserved_sectors = RESERVED_SECTORS;
    br.dir_sectors = DIR_SECTORS;
    br.bitmap_sectors = BITMAP_SECTORS;
    br.data_sectors = DATA_SECTORS;
    br.total_sectors = TOTAL_SECTORS;
    br.file_count = 0;
    br.first_free_sector = RESERVED_SECTORS + DIR_SECTORS + BITMAP_SECTORS; // 1 + 1 + 2 = 4
    memset(br.reserved, 0, sizeof(br.reserved));

    // Escreve o boot record no disco
    fwrite(&br, sizeof(BootRecord), 1, disk);

    // Preenche setores reservados com zeros ###
    size_t boot_record_size = sizeof(BootRecord);
    size_t padding_size = BYTES_PER_SECTOR - boot_record_size;
    unsigned char padding[BYTES_PER_SECTOR];
    memset(padding, 0, BYTES_PER_SECTOR);
    fwrite(padding, padding_size, 1,  disk);

    // Inicializa e escreve entradas do diretório	
    DirEntry empty_entry;
    memset(&empty_entry, 0, sizeof(DirEntry));
    empty_entry.status = 0xFF; // Marcar como livre
    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry);

    for(int i = 0; i < dir_entries; i++) {
        size_t written = fwrite(&empty_entry, sizeof(DirEntry), 1, disk);
        if (written != 1) {
        printf("Erro ao escrever entrada %d do diretório, fwrite returned %zu, errno: %s\n", 
               i, written, strerror(errno));
        }
    }

    // Inicializa e escreve o bitmap
    int bitmap_size = BITMAP_SECTORS * BYTES_PER_SECTOR;
    unsigned char *bitmap = (unsigned char *)calloc(bitmap_size, sizeof(char));
    fwrite(bitmap, sizeof(unsigned char), bitmap_size, disk);
    free(bitmap);

    // Inicializa área de dados
    int data_size = DATA_SECTORS * BYTES_PER_SECTOR;
    unsigned char *data = (unsigned char *)calloc(data_size, sizeof(char));
    fwrite(data, sizeof(unsigned char), data_size, disk);
    free(data);

    fclose(disk);
    printf("Disco formatado com sucesso!\n");
}

void exibir_disco(const char *disk_filename) {
    FILE *disk = fopen(disk_filename, "rb");
    if (!disk) {
        perror("Erro ao abrir o arquivo do disco");
        return;
    }

    printf("\n--- Exibindo Conteúdo do Disco ---\n");

    // Ler e exibir o Boot Record
    BootRecord br;
    fread(&br, sizeof(BootRecord), 1, disk);
    printf("\n[Boot Record]\n");
    printf("Bytes por setor: %d\n", br.bytes_por_sector);
    printf("Setores por bloco: %d\n", br.sectors_per_block);
    printf("Setores reservados: %d\n", br.reserved_sectors);
    printf("Setores do diretório: %d\n", br.dir_sectors);
    printf("Setores do bitmap: %d\n", br.bitmap_sectors);
    printf("Setores de dados: %d\n", br.data_sectors);
    printf("Setores totais: %d\n", br.total_sectors);
    printf("Contagem de arquivos: %d\n", br.file_count);
    printf("Primeiro setor livre: %d\n", br.first_free_sector);

    // Ler e exibir as entradas do diretório
    printf("\n[Entradas do Diretório]\n");
    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry);
    for (int i = 0; i < dir_entries; i++) {
        DirEntry entry;
        fread(&entry, sizeof(DirEntry), 1, disk);
        if (entry.status == 0x00) { // Arquivo válido
            printf("Arquivo %d:\n", i + 1);
            printf("  Nome: %s.%s\n", entry.filename, entry.extension);
            printf("  Atributos: %d\n", entry.attributes);
            printf("  Setor inicial: %d\n", entry.first_sector);
            printf("  Tamanho: %d bytes\n", entry.file_size);
        }
    }

    // Ler e exibir o bitmap
    printf("\n[Bitmap]\n");
    int bitmap_size = BITMAP_SECTORS * BYTES_PER_SECTOR;
    unsigned char *bitmap = (unsigned char *)malloc(bitmap_size);
    fread(bitmap, sizeof(unsigned char), bitmap_size, disk);
    for (int i = 0; i < bitmap_size; i++) {
        printf("%02X ", bitmap[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    free(bitmap);

    fclose(disk);
    printf("\n--- Fim do Conteúdo do Disco ---\n");
}

void copiar_para_sa(const char *disk_filename, const char *source_filename) {
    // Abre o arquivo fonte em modo binário
    FILE *src = fopen(source_filename, "rb");
    if (!src) {
        perror("Erro ao abrir arquivo fonte");
        return;
    }
    
    // Determina o tamanho do arquivo fonte
    fseek(src, 0, SEEK_END);
    long file_size = ftell(src);
    fseek(src, 0, SEEK_SET);
    // Calcula a quantidade de setores necessários (arredondando para cima)
    int sectors_needed = (file_size + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR;
    
    // Abre a imagem do disco em modo leitura/escrita
    FILE *disk = fopen(disk_filename, "rb+");
    if (!disk) {
        perror("Erro ao abrir imagem do disco");
        fclose(src);
        return;
    }

    // Lê o boot record (setor 0)
    BootRecord br;
    fseek(disk, 0, SEEK_SET);

    if (fread(&br, sizeof(BootRecord), 1, disk) != 1) {
        perror("Erro ao ler boot record");
        fclose(src);
        fclose(disk);
        return;
    }

    // Lê área de diretório (setor 1, entrada de 32 bytes)
    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry);
    DirEntry dir[dir_entries];
    fseek(disk, (RESERVED_SECTORS * BYTES_PER_SECTOR) , SEEK_SET);
    if (fread(dir, sizeof(DirEntry), dir_entries, disk) != dir_entries) {
        perror("Erro ao ler diretório");
        fclose(src);
        fclose(disk);
        return;
    }

    // Lê o bitmap (BITMAP_SECTORS setores)
    int bitmap_size = BITMAP_SECTORS * BYTES_PER_SECTOR;
    unsigned char *bitmap = (unsigned char *)malloc(bitmap_size);
    if (!bitmap) {
        perror("Erro ao alocar memória para bitmap");
        fclose(src);
        fclose(disk);
        return;
    }

    fseek(disk, (RESERVED_SECTORS + DIR_SECTORS) * BYTES_PER_SECTOR, SEEK_SET);
    if (fread(bitmap, 1, bitmap_size, disk) != (size_t)bitmap_size) {
        perror("Erro ao ler bitmap");
        free(bitmap);
        fclose(src);
        fclose(disk);
        return;
    }

    // Procura por um espaço contíguo livre na área de dados
    // inicia em br.first_free_sector (RESERVED_SECTORS + DIR_SECTORS + BITMAP_SECTORS)
    int data_start = br.first_free_sector;
    int data_end = data_start + DATA_SECTORS; // limite superior não incluso
    int start_sector = -1, consecutive = 0;
    for (int sector = data_start; sector < data_end; sector++) {
        int byte_index = sector / 8; // índice do byte no bitmap
        int bit_index = sector % 8;  // índice do bit no byte

        // Verifica se setor está livre
        if (!(bitmap[byte_index] & (1 << bit_index))) {
            if (consecutive == 0) {
                start_sector = sector;
            }
            consecutive++;
        } else {
            consecutive = 0;
            start_sector = -1;
        }
    }

    // Se não encontrou espaço contíguo suficiente
    if (consecutive < sectors_needed) {
        printf("Erro: Espaço insuficiente no disco\n");
        free(bitmap);
        fclose(src);
        fclose(disk);
        return;
    }

    // Marca os setores alocados no bitmap
    for (int i = 0; i < sectors_needed; i++) {
        int sector = start_sector + i;
        int byte_index = sector / 8;
        int bit_index = sector % 8;
        bitmap[byte_index] |= (1 << bit_index);
    }

    // Escreve dados do arquivo na área de dados
    long data_offset = start_sector * BYTES_PER_SECTOR;
    fseek(disk, data_offset, SEEK_SET);
    unsigned char buffer[BYTES_PER_SECTOR];
    int bytes_remaining = file_size;
    while (bytes_remaining > 0) {
        int bytes_to_read = (bytes_remaining > BYTES_PER_SECTOR) ? BYTES_PER_SECTOR : bytes_remaining;
        size_t lidos = fread(buffer, 1, bytes_to_read, src);
        if (lidos != (size_t)bytes_to_read) {
            perror("Erro ao ler arquivo fonte");
            free(bitmap);
            fclose(src);
            fclose(disk);
            return;
        }

        // Se não preencher setor completo, preenche com zeros
        if (lidos < BYTES_PER_SECTOR) ///
            memset(buffer + bytes_to_read, 0, BYTES_PER_SECTOR - bytes_to_read);
        fwrite(buffer, 1, BYTES_PER_SECTOR, disk);
        bytes_remaining -= bytes_to_read;
    }

    // Atualiza o diretório: encontra entrada vazia
    int free_entry_index = -1;
    for (int i = 0; i < dir_entries; i++) {
        // Supondo que as entradas livres tem status diferente de 0x00 (válido)
        if (dir[i].status == 0xFF) {
            free_entry_index = i;
            break;
        }
    }

    if (free_entry_index == -1) {
        printf("Erro: Diretório cheio\n");
        free(bitmap);
        fclose(src);
        fclose(disk);
        return;
    }

    // Preenche nova entrada no diretório
    DirEntry new_entry;
    new_entry.status = 0x00; // Válido

    // Extração do nome e extensão do arquivo
    char *dot = strchr(source_filename, '.');
    if (dot) {
        int name_len = dot - source_filename;
        if (name_len > 12) {name_len = 12;}
        strncpy(new_entry.filename, source_filename, name_len);

        if (name_len < 12){
            memset(new_entry.filename + name_len, 0, 12 - name_len);
        }
        
        int ext_len = strlen(dot + 1);
        if (ext_len > 4) {ext_len = 4;}
        strncpy(new_entry.extension, dot + 1, ext_len);

        if (ext_len < 4){
            memset(new_entry.extension + ext_len, 0, 4 - ext_len);
        }
        
    } else {
        // Se não houver extensão, copia nome completo (até 12 bytes)
        int name_len = strlen(source_filename);
        if (name_len > 12) {name_len = 12;}
        strncpy(new_entry.filename, source_filename, name_len);

        if (name_len < 12){
            memset(new_entry.filename + name_len, 0, 12 - name_len);
        }
        memset(new_entry.extension, 0, 4);
    }

    new_entry.attributes = 0; // Atributo padrão
    new_entry.first_sector = start_sector;
    new_entry.file_size = file_size;
    memset(new_entry.reserved, 0, sizeof(new_entry.reserved));

    // Insere nova entrada na posição livre
    dir[free_entry_index] = new_entry;

    // Atualiza o boot record
    br.file_count++;

    // Reescreve o boot record, diretório e bitmap
    // Boot Record (setor 0)
    fseek(disk, 0, SEEK_SET);
    fwrite(&br, sizeof(BootRecord), 1, disk);

    // Diretório (começa em RESERVED_SECTORS)
    fseek(disk, (RESERVED_SECTORS * BYTES_PER_SECTOR), SEEK_SET);
    fwrite(dir, sizeof(DirEntry), dir_entries, disk);

    // Bitmap (começa em RESERVED_SECTORS + DIR_SECTORS)
    fseek(disk, (RESERVED_SECTORS + DIR_SECTORS) * BYTES_PER_SECTOR, SEEK_SET);
    fwrite(bitmap, 1, bitmap_size, disk);

    // Libera recursos e fecha arquivo

    free(bitmap);
    fclose(disk);
    fclose(src);
    
    printf("Arquivo copiado para o sistema de arquivos com sucesso!\n");
}

void copiar_para_disco(const char *disk_filename, const char *target_filename){
    FILE *disk = fopen(disk_filename, "rb+"); // Leitura e escrita (binário)

    if (!disk){
        perror("Erro ao abrir imagem do disco");
        return;
    }

    // Lê Boot Record (não é usado para cópia, mas para garantir que o disco está formatado)
    BootRecord br;
    fseek(disk, 0, SEEK_SET);
    if (fread(&br, sizeof(BootRecord), 1, disk) != 1){
        perror("Erro ao ler boot record");
        fclose(disk);
        return;
    }

    // Calcula número de entradas do diretório
    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry);

    // Aloca array para armazenar entradas do diretório
    DirEntry *directory = (DirEntry *)malloc(dir_entries * sizeof(DirEntry));
    if (!directory){
        perror("Erro ao alocar memória para diretório");
        fclose(disk);
        return;
    }

    // Posiciona ponteiro no começo da área de diretório e lê entradas
    fseek(disk, RESERVED_SECTORS * BYTES_PER_SECTOR, SEEK_SET);

    if (fread(directory, sizeof(DirEntry), dir_entries, disk) != (size_t)dir_entries){
        perror("Erro ao ler entradas do diretório");
        free(directory);
        fclose(disk);
        return;
    }

    // Procura por entrada cujo nome e extensão correspondam ao target_filename
    int found_index = -1;
    char full_name[18]; // 12 (nome) + 1 (ponto) + 4 (extensão) + 1 (terminador)
    for (int i = 0; i < dir_entries; i++){
        if (directory[i].status == 0x00){ // Entrada válida
            // Concatena nome e extensão
            if (strlen(directory[i].extension) > 0){
                snprintf(full_name, sizeof(full_name), "%s.%s", directory[i].filename, directory[i].extension);
            } else {
                snprintf(full_name, sizeof(full_name), "%s", directory[i].filename);
            }
            if (strcmp(full_name, target_filename) == 0){
                found_index = i;
                break;
            }
        }
    }

    if (found_index == -1){
        printf("Arquivo não encontrado no diretório\n");
        free(directory);
        fclose(disk);
        return;
    }

    // Obtém dados do arquivo encontrado
    DirEntry file_entry = directory[found_index];
    free(directory);

    // Abre arquivo de saída com mesmo nome do target_filename
    FILE *out = fopen(target_filename, "wb");
    if (!out){
        perror("Erro ao abrir arquivo de saída");
        fclose(disk);
        return;
    }

    // Calcula quantos setores foram usados para armazenar o arquivo
    int sectors_needed = (file_entry.file_size + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR;

    // Lê os setores do arquivo e escreve no arquivo de saída
    int file_size_remaining = file_entry.file_size;

    for (int s = 0; s < sectors_needed; s++){

        int sector_num = file_entry.first_sector + s;
        long sector_offset = sector_num * BYTES_PER_SECTOR;
        fseek(disk, sector_offset, SEEK_SET);
        unsigned char buffer[BYTES_PER_SECTOR];
        size_t bytes_read = fread(buffer, 1, BYTES_PER_SECTOR, disk);

        if (bytes_read != BYTES_PER_SECTOR){
            // Se chegou ao final do arquivo, significa que foi o último setor
            // então preenche o restante do buffer com zeros
            if (feof(disk)){
                memset(buffer + bytes_read, 0, BYTES_PER_SECTOR - bytes_read);
            } else {
                perror("Erro ao ler setor do arquivo");
                fclose(out);
                fclose(disk);
                return;
            }
        }

        int bytes_to_write = (file_size_remaining > BYTES_PER_SECTOR) ? BYTES_PER_SECTOR : file_size_remaining;
        fwrite(buffer, 1, bytes_to_write, out);
        file_size_remaining -= bytes_to_write;
    }

    fclose(out);
    fclose(disk);
    printf("Arquivo copiado para o sistema com sucesso!\n");
}

void listar_arquivos(const char *disk_filename){
    FILE *disk = fopen(disk_filename, "rb"); // Leitura (binário)

    if (!disk){
        perror("Erro ao abrir imagem do disco");
        return;
    }

    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry); // Calcula quantidade de entradas no diretório

    // Aloca array para armazenar entradas do diretório
    DirEntry *directory = (DirEntry *)malloc(dir_entries * sizeof(DirEntry));

    if (!directory){
        perror("Erro ao alocar memória para diretório");
        fclose(disk);
        return;
    }

    // Posiciona ponteiro no começo da área de diretório
    fseek(disk, RESERVED_SECTORS * BYTES_PER_SECTOR, SEEK_SET);

    // Lê entradas do diretório e armazena no array directory
    if (fread(directory, sizeof(DirEntry), dir_entries, disk) != dir_entries){
        perror("Erro ao ler entradas do diretório");
        free(directory);
        fclose(disk);
        return;
    }

    // Cabeçalho para listagem
    printf("\n--- Listagem de Arquivos ---\n");
    int arquivo_encontrado = 0;

    // Percorrer todas as entradas do diretório 
    for (int i = 0; i < dir_entries; i++){

        if (directory[i].status == 0x00 &&
            directory[i].file_size > 0 &&
            strlen(directory[i].filename) > 0 &&
            strcmp(directory[i].filename, ".") != 0) {

            arquivo_encontrado = 1;
            printf("Arquivo %d:\n", i + 1);
            printf("  Nome: %s.%s\n", directory[i].filename, directory[i].extension);
            printf("  Atributos: %d\n", directory[i].attributes);
            printf("  Setor inicial: %d\n", directory[i].first_sector);
            printf("  Tamanho: %d bytes\n", directory[i].file_size);
        }
    }

    if(!arquivo_encontrado){
        printf("Nenhum arquivo encontrado no diretório\n");
    }
    
    // Libera memória do array directory e fecha arquivo
    free(directory);
    fclose(disk);

    
}

void remover_arquivo(const char *disk_filename, const char *filename){
    FILE *disk = fopen(disk_filename, "rb+"); // Leitura e escrita (binário)

    if (!disk){
        perror("Erro ao abrir imagem do disco");
        return;
    }

    // Lê Boot Record 
    BootRecord br;
    fseek(disk, 0, SEEK_SET);
    if (fread(&br, sizeof(BootRecord), 1, disk) != 1){
        perror("Erro ao ler boot record");
        fclose(disk);
        return;
    }

    // Calcula número de entradas do diretório
    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry);

    // Aloca memória para ler entradas do diretório
    DirEntry *directory = (DirEntry *)malloc(dir_entries * sizeof(DirEntry));
    if (!directory){
        perror("Erro ao alocar memória para diretório");
        fclose(disk);
        return;
    }

    // Posiciona ponteiro no começo da área de diretório e lê entradas
    fseek(disk, RESERVED_SECTORS * BYTES_PER_SECTOR, SEEK_SET);
    if (fread(directory, sizeof(DirEntry), dir_entries, disk) != (size_t)dir_entries){
        perror("Erro ao ler entradas do diretório");
        free(directory);
        fclose(disk);
        return;
    }

    // Procura por entrada cujo nome e extensão correspondam ao filename
    int found_index = -1;
    char full_name[18]; // 12 (nome) + 1 (ponto) + 4 (extensão) + 1 (terminador)

    for (int i = 0; i < dir_entries; i++){
        if (directory[i].status == 0x00){ // Entrada válida
            // Concatena nome e extensão
            if (directory[i].status == 0x00){ // Entrada válida
                // Concatena nome e extensão
                if (strlen(directory[i].extension) > 0){
                    snprintf(full_name, sizeof(full_name), "%s.%s", directory[i].filename, directory[i].extension);
                } else {
                    snprintf(full_name, sizeof(full_name), "%s", directory[i].filename);
                }
                if (strcmp(full_name, filename) == 0){
                    found_index = i;
                    break;
                }
            }
        }
    }

    if (found_index == -1){
        printf("Arquivo não encontrado no diretório\n");
        free(directory);
        fclose(disk);
        return;
    }

    // Obtém entrada correspondente ao arquivo encontrado
    DirEntry file_entry = directory[found_index];

    // Calcula quantidade de setores utilizados (arredonda para cima)
    int sectors_needed = (file_entry.file_size + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR;

    // Lê o Bitmap
    int bitmap_size = BITMAP_SECTORS * BYTES_PER_SECTOR;
    unsigned char *bitmap = (unsigned char *)malloc(bitmap_size);

    if (!bitmap){
        perror("Erro ao alocar memória para bitmap");
        free(directory);
        fclose(disk);
        return;
    }

    fseek(disk, (RESERVED_SECTORS + DIR_SECTORS) * BYTES_PER_SECTOR, SEEK_SET);
    if (fread(bitmap, sizeof(unsigned char), bitmap_size, disk) != (size_t)bitmap_size){
        perror("Erro ao ler bitmap");
        free(bitmap);
        free(directory);
        fclose(disk);
        return;
    }

    // Libera os setores alocados no bitmap, limpando os bits correspondentes
    for (int i = 0; i < sectors_needed; i++){
        int sector = file_entry.first_sector + i;
        int byte_index = sector / 8;
        int bit_index = sector % 8;
        bitmap[byte_index] &= ~(1 << bit_index); // Limpa o bit (define como 0)
    }

    // Marca entrada do diretório como deletada
    directory[found_index].status = 0xFF;
    memset(directory[found_index].filename, 0, sizeof(directory[found_index].filename));
    memset(directory[found_index].extension, 0, sizeof(directory[found_index].extension));
    directory[found_index].attributes = 0;
    directory[found_index].first_sector = 0;
    directory[found_index].file_size = 0;
    memset(directory[found_index].reserved, 0, sizeof(directory[found_index].reserved));

    // Atualiza o boot record
    if (br.file_count > 0){
        br.file_count--;
    }

    // Reescreve o boot record no início do disco
    fseek(disk, 0, SEEK_SET);
    if (fwrite(&br, sizeof(BootRecord), 1, disk) != 1){
        perror("Erro ao atualizar boot record");
        free(bitmap);
        free(directory);
        fclose(disk);
        return;
    }

    // Reescreve o diretório
    fseek(disk, RESERVED_SECTORS * BYTES_PER_SECTOR, SEEK_SET);
    if (fwrite(directory, sizeof(DirEntry), dir_entries, disk) != (size_t)dir_entries){
        perror("Erro ao atualizar diretório");
        free(bitmap);
        free(directory);
        fclose(disk);
        return;
    }

    // Reescreve o bitmap
    fseek(disk, (RESERVED_SECTORS + DIR_SECTORS) * BYTES_PER_SECTOR, SEEK_SET);
    if (fwrite(bitmap, sizeof(unsigned char), bitmap_size, disk) != (size_t)bitmap_size){
        perror("Erro ao atualizar bitmap");
        free(bitmap);
        free(directory);
        fclose(disk);
        return;
    }

    // Libera recursos alocados
    free(bitmap);
    free(directory);
    fclose(disk);

    printf("Arquivo '%s' removido com sucesso!\n", filename);
}

int main() {
    int opcao;
    char disk_filename[256] = "disco.img"; // Arquivo que simula o disco
    
    printf("Sistema de Arquivos - Trabalho SO\n");
    do {
        printf("\nMenu:\n");
        printf("1. Formatar disco\n");
        printf("2. Copiar arquivo do disco para o sistema\n");
        printf("3. Copiar arquivo do sistema para o disco\n");
        printf("4. Listar arquivos\n");
        printf("5. Remover arquivo\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        
        switch(opcao) {
            case 1:
                formatar_disco(disk_filename);
                break;
            case 2: 
                char source_filename[256];
                printf("Informe o nome do arquivo para copiar para o sistema: ");
                scanf("%s", source_filename);
                copiar_para_sa(disk_filename, source_filename);
                break;
            
            case 3: 
                char target_filename[256];
                printf("Informe o nome do arquivo para copiar do sistema para o disco: ");
                scanf("%s", target_filename);
                copiar_para_disco(disk_filename, target_filename);
                break;
            
            case 4:
                listar_arquivos(disk_filename);
                break;
            case 5: 
                char filename[256];
                printf("Informe o nome do arquivo a ser removido: ");
                scanf("%s", filename);
                remover_arquivo(disk_filename, filename);
                break;
            case 6:
                exibir_disco(disk_filename);
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (opcao != 0);
    
    return 0;
}