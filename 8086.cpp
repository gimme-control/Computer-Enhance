#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <unordered_map>
// #include <bitset>

std::unordered_map<unsigned char, std::string> reg_opcode =
{
    {0b100010, "mov"}, 
    {0b1011, "mov"}, 
    {0b1100011, "mov"}, 
}; 

std::unordered_map<unsigned char, std::string> reg_map = 
{
    {0b0000, "al"},
    {0b0001, "ax"},
    {0b0010, "cl"},
    {0b0011, "cx"},
    {0b0100, "dl"},
    {0b0101, "dx"},
    {0b0110, "bl"},
    {0b0111, "bx"},
    {0b1000, "ah"},
    {0b1001, "sp"},
    {0b1010, "ch"},
    {0b1011, "bp"},
    {0b1100, "dh"},
    {0b1101, "si"},
    {0b1110, "bh"},
    {0b1111, "di"},
}; 

struct instruction {
        unsigned char opcode = 0; 
        int direction = 0;  
        int wordSize = 0;  
        unsigned char mod = 0;
        unsigned char reg = 0;  
        unsigned char rm = 0;  

        unsigned char lowDisp = 0;  
        unsigned char highDisp = 0;  

        unsigned char lowData = 0;  
        unsigned char highData = 0; 
}; 

// File opener
std::vector<unsigned char> getFile(std::string fileName)
{
	std::ifstream input; 
	input.open(fileName, std::ios::binary | std::ios::in); 

	std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(input)), {});
	input.close(); 

    return buffer; 
}

unsigned char extractBits(unsigned char curr, int startBit, int endBit)
{
    unsigned char mask = ((1 << (startBit - endBit + 1)) - 1) << endBit; 
    return (curr & mask) >> endBit; 
}

instruction decodeInstruction(std::vector<unsigned char>& buffer, int byteIndex)
{
    instruction decoded; 

    unsigned char currByte = buffer[byteIndex]; 

    if(extractBits(currByte, 7, 4) == 0b1011)
    {
        decoded.opcode = 0b1011;
        decoded.wordSize = extractBits(currByte, 3, 3);
        decoded.reg = extractBits(currByte, 2, 0);
        
        ++byteIndex; 

        currByte = buffer[byteIndex]; 
        decoded.lowData = extractBits(currByte, 7, 0);

        if(decoded.wordSize)
        {
            ++byteIndex; 
            currByte = buffer[byteIndex];
            decoded.highData = extractBits(currByte, 7, 0);
        }
    }

    else
    {
        decoded.opcode = extractBits(currByte, 7, 2); 
        decoded.direction = extractBits(currByte, 1, 1); 
        decoded.wordSize = extractBits(currByte, 0, 0); 

        byteIndex++; 
        currByte = buffer[byteIndex]; 
        decoded.mod = extractBits(currByte, 7, 6); 
        decoded.reg = extractBits(currByte, 5, 3); 
        decoded.rm = extractBits(currByte, 2, 0); 
    }

    return decoded; 
}

void decodeAdditional(std::vector<unsigned char>& buffer, instruction &decoded, 
        int index, int disp, int data) 
{
    if(disp)
    {
        unsigned char currByte = buffer[index];
        decoded.lowDisp = extractBits(currByte, 7, 0);

        if(disp == 2)
        {
            ++index; 
            currByte = buffer[index]; 
            decoded.highDisp = extractBits(currByte, 7, 0);
        }
    }

    index += disp; 

    if(data)
    {
        unsigned char currByte = buffer[index];
        decoded.highData = extractBits(currByte, 7, 0);
        if(data == 2)
        {
            decoded.lowData = extractBits(currByte, 7, 0);
            ++index; 
            currByte= buffer[index];
            decoded.highData = extractBits(currByte, 7, 0);
        }
    }
}

std::vector<instruction> getInstructions(std::vector<unsigned char> buffer)
{
    std::vector<instruction> ins; 

    for(int i = 0; i < buffer.size(); i += 2)
    {
        int disp = 0; 
        int data = 0; 

        instruction decoded = decodeInstruction(buffer, i);

        switch(decoded.mod)
        {
            case 0b00: 
                if(decoded.rm == 0b110)
                    disp = 2;  
                else
                    disp = 0; 
                break; 
            case 0b01: 
                disp = 1; 
                break; 
            case 0b10: 
                disp = 2; 
                break; 
            default: 
                disp = 0; 
        }

        if(decoded.opcode == 0b1011 && decoded.wordSize) // check debugger 
        {
            data = 1;
            disp = 0; 
        } 
        else if (decoded.opcode == 0b110001)
        {
            data = 1; 
            if(decoded.wordSize)
                ++data; 
        }

        if(disp || data)
            decodeAdditional(buffer, decoded, i+2, disp, data); // i+2? because i isnt updated

        i += disp + data; 

        ins.push_back(decoded);
    }

    return ins; 
}

void print_instructions(std::vector<instruction> ins)
{
    for(instruction tmp : ins)
    {
        if(tmp.lowData)
        {
            std::cout << reg_opcode[tmp.opcode] << ' '; 
            tmp.reg = (tmp.wordSize == 0 ? ((tmp.reg << 1) | 0) : ((tmp.reg << 1) | 1)); 
            std::cout << reg_map[tmp.reg] << ' ';
            std::cout << (int)((static_cast<int>(tmp.highData) << 8) | tmp.lowData) << ' ';
            std::cout << '\n';
        }
        else
        {
            std::cout << reg_opcode[tmp.opcode] << ' '; 
            tmp.rm = (tmp.wordSize == 0 ? ((tmp.rm << 1) | 0) : ((tmp.rm << 1) | 1)); 
            tmp.reg = (tmp.wordSize == 0 ? ((tmp.reg << 1) | 0) : ((tmp.reg << 1) | 1)); 
            std::cout << reg_map[tmp.rm] << ' ';
            std::cout << reg_map[tmp.reg] << ' ';
            std::cout << '\n';
        }
    }
}

int main()
{
    std::vector<unsigned char> buffer = getFile("manymov.bin"); 
    std::vector<instruction> all_instructions = getInstructions(buffer);
    print_instructions(all_instructions);

	return 0; 
}

