#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define os valores de acordo com a especificação 
#define BYTES_PER_SECTOR 512
#define SECTORS_PER_BLOCK 8
#define RESERVED_SECTORS 1
#define DIR_SECTORS 1
#define BITMAP_SECTORS 2
#define DATA_SECTORS 196
#define TOTAL_SECTORS (RESERVED_SECTORS + DIR_SECTORS + BITMAP_SECTORS + DATA_SECTORS)

// Define a estrutura do boot record e entrada do diretório
typedef struct {
    unsigned short bytes_por_sector;   // 2 bytes
    unsigned char sectors_per_block;   // 1 byte ###
    unsigned short reserved_sectors;   // 2 bytes
    unsigned short dir_sectors;        // 2 bytes
    unsigned short data_sectors;       // 2 bytes
    unsigned short bitmap_sectors;     // 2 bytes
    unsigned short total_sectors;      // 2 bytes
    unsigned int file_count;           // 4 bytes
    unsigned int first_free_sector;    // 4 bytes
    char reserved[10];                 // 10 bytes 
} BootRecord;

typedef struct {
    unsigned char status;      // 1 byte: 0x00 para válido, 0xFF para deletado
    char filename[12];         // 12 bytes para o nome do arquivo
    char extension[4];         // 4 bytes para a extensão
    unsigned char attributes;  // 1 byte para atributos
    unsigned int first_sector; // 4 bytes: setor inicial do arquivo
    unsigned int file_size;    // 4 bytes: tamanho do arquivo em bytes
    char reserved[6];          // 6 bytes reservados para futuras expansões
} DirEntry;

// Protótipos das funções ## para teste
void formatar_disco(const char *disk_filename);
void copiar_para_sa(const char *disk_filename, const char *source_filename);
void copiar_para_disco(const char *disk_filename, const char *target_filename);
void listar_arquivos(const char *disk_filename);
void remover_arquivo(const char *disk_filename, const char *filename);

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
            case 2: {
                char source_filename[256];
                printf("Informe o nome do arquivo para copiar para o sistema: ");
                scanf("%s", source_filename);
                copiar_para_sistema(disk_filename, source_filename);
                break;
            }
            case 3: {
                char target_filename[256];
                printf("Informe o nome do arquivo para copiar do sistema para o disco: ");
                scanf("%s", target_filename);
                copiar_para_disco(disk_filename, target_filename);
                break;
            }
            case 4:
                listar_arquivos(disk_filename);
                break;
            case 5: {
                char filename[256];
                printf("Informe o nome do arquivo a ser removido: ");
                scanf("%s", filename);
                remover_arquivo(disk_filename, filename);
                break;
            }
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (opcao != 0);
    
    return 0;
}

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

    // Inicializa e escreve entradas do diretório	
    DirEntry empty_entry;
    memset(&empty_entry, 0, sizeof(DirEntry));
    int dir_entries = (DIR_SECTORS * BYTES_PER_SECTOR) / sizeof(DirEntry);

    for(int i = 0; i < dir_entries; i++) {
        fwrite(&empty_entry, sizeof(DirEntry), 1, disk);
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

void copiar_para_sa(const char *disk_filename, const char *source_filename) {
    printf("Funcionalidade não implementada...\n");
}

void copiar_para_disco(const char *disk_filename, const char *target_filename){
    printf("Funcionalidade não implementada...\n");
}

void listar_arquivos(const char *disk_filename){
    printf("Funcionalidade não implementada...\n");
}

void remover_arquivo(const char *disk_filename, const char *filename){
    printf("Funcionalidade não implementada...\n");
}