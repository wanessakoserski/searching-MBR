#include <stdio.h> 

#include <stdint.h> 

#include <stdlib.h> 

 

#define MBR_SIZE 512 

#define PARTITION_TABLE_OFFSET 446 

#define PARTITION_ENTRY_SIZE 16 

#define NUM_PARTITIONS 4 

 

// Estrutura para representar uma entrada da tabela de partições 

struct PartitionEntry { 

    uint8_t status;             // Status de boot 

    uint8_t chs_start[3];       // CHS do início da partição (obsoleto) 

    uint8_t partition_type;     // Tipo de partição 

    uint8_t chs_end[3];         // CHS do fim da partição (obsoleto) 

    uint32_t lba_start;         // LBA inicial 

    uint32_t num_sectors;       // Número de setores 

}; 

 

// Função para imprimir o tipo de partição 

const char* partition_type(uint8_t type) { 

    switch (type) { 

        case 0x83: return "Linux"; 

        case 0x07: return "HPFS/NTFS/exFAT"; 

        case 0x0b: return "W95 FAT32"; 

        case 0x0c: return "W95 FAT32 (LBA)"; 

        case 0x0f: return "W95 Ext'd (LBA)"; 

        case 0x82: return "Linux swap"; 

        case 0x00: return "Empty"; 

        default: return "Unknown"; 

    } 

} 

 

int main(int argc, char *argv[]) { 

    if (argc != 2) { 

        printf("Uso: %s <arquivo_mbr>\n", argv[0]); 

        return 1; 

    } 

 

    // Abrir o arquivo que contém o MBR 

    FILE *mbr_file = fopen(argv[1], "rb"); 

    if (!mbr_file) { 

        perror("Erro ao abrir o arquivo MBR"); 

        return 1; 

    } 

 

    // Ler os 512 bytes do arquivo 

    uint8_t mbr[MBR_SIZE]; 

    if (fread(mbr, 1, MBR_SIZE, mbr_file) != MBR_SIZE) { 

        perror("Erro ao ler o arquivo MBR"); 

        fclose(mbr_file); 

        return 1; 

    } 

    fclose(mbr_file); 

 

    // Verificar assinatura do MBR (os dois últimos bytes devem ser 0x55 e 0xAA) 

    if (mbr[510] != 0x55 || mbr[511] != 0xAA) { 

        printf("Assinatura de MBR inválida!\n"); 

        return 1; 

    } 

 

    // Processar as entradas da tabela de partições 

    struct PartitionEntry partitions[NUM_PARTITIONS]; 

 

    printf("Disco: %s: 512 bytes, 512 bytes por setor\n\n", argv[1]); 

    printf("Device Boot   Start      End   Sectors   Size Id Type\n"); 

 

    for (int i = 0; i < NUM_PARTITIONS; i++) { 

        // Ler cada entrada de partição (16 bytes) 

        int offset = PARTITION_TABLE_OFFSET + i * PARTITION_ENTRY_SIZE; 

        struct PartitionEntry *p = (struct PartitionEntry*)&mbr[offset]; 

 

        // Ignorar partições vazias (tipo 0x00) 

        if (p->partition_type == 0x00) { 

            continue; 

        } 

 

        // Calcular o final da partição com base no LBA inicial e o número de setores 

        uint32_t lba_end = p->lba_start + p->num_sectors - 1; 

 

        // Imprimir informações da partição 

        printf("%s%d   %s     %-10u %-10u %-10u %-5u  %02X  %s\n", 

               argv[1], i+1,               // Nome do dispositivo 

               (p->status == 0x80) ? "*" : " ",  // Bootável 

               p->lba_start,                // LBA inicial 

               lba_end,                     // LBA final 

               p->num_sectors,              // Número de setores 

               p->num_sectors / 2048,       // Tamanho em MB (512 bytes por setor) 

               p->partition_type,           // Tipo de partição (ID) 

               partition_type(p->partition_type)); // Tipo de partição (nome) 

    } 

 

    return 0; 

} 