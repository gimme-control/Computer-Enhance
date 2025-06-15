#define _CTR_SECURE_NO_WARNINGS 
#define ArraySize(Array) (sizeof(Array) / sizeof(Array[0])) 
#include <stdlib.h>
#include <stdio.h> 
#include <stdint.h> 

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef enum
{
    MOV, 
    JMP, 
    ADD,
    SUB,
    CMP, 
    UNKNOWN 

} Operation; 

typedef struct
{
    Operation op;   // 6 bits 
    bool direction; // 1 bit 
    bool wordSime;  // 1 bit
    
    unsigned char mod; // 2 bits
    unsigned char reg; // 3 bits
    unsigned char rm;  // 3 bits

    unsigned char lowDisp;  // 1 Byte
    unsigned char highDisp; // 1 Byte

    unsigned char lowData;  // 1 Byte
    unsigned char highData; // 1 Byte
    
    unsigned char lowAddr;  // 1 Byte
    unsigned char highAddr; // 1 Byte

} Instruction; 



size_t readFile(const char* fileName, unsigned char* data, size_t buffer_size)
{
    FILE *ptr = fopen(fileName, "rb");
    size_t bytes_read = fread(data, 1, buffer_size, ptr);
    fclose(ptr);
    return bytes_read;
}

int main(int argc, char *argv[])
{
    unsigned char data[1024*1024]; // 1MB of addressable data on the 8086

    if(argc < 2)
    {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    size_t actual_bytes_read = readFile(argv[1], data, ArraySize(data));

    if(!actual_bytes_read) return EXIT_FAILURE; 

    return 0; 
}
