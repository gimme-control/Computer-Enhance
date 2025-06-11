#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

std::unordered_map<unsigned char, std::string> reg_opcode =
{
    {0b100010, "mov"}, // Move register/memory to/from register 
    {0b1011, "mov"}, // Move immediate to register 
    {0b1100011, "mov"}, // Move immediate to register/memory 

    {0b000000, "add"}, // Add reg/memory with register to either 
    {0b100000, "add"}, // Add immediate to register/memory 
    {0b000001, "add"}, // Add immediate to accumulator 

    {0b001010, "sub"}, // Sub reg/memory with register to either 
    {0b100000, "sub"}, // Sub immediate to register/memory (mod = 101)
    {0b001011, "sub"}, // Sub immediate to accumulator 

    {0b001110, "cmp"}, // Add reg/memory with register to either 
    {0b100000, "cmp"}, // Add immediate to register/memory (mod = 111)
    {0b001111, "cmp"}, // Add immediate to accumulator 
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

        int sign = 0;
}; 

// Helper Functions
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

void print_disp(int disp)
{
    if(disp)
        std::cout << " + " <<  disp << "]";
    else 
        std::cout << "]";
}

// Real Logic
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
        unsigned char currByte = buffer[byteIndex]; 
        decoded.lowData = extractBits(currByte, 7, 0);
    }

    if(extractBits(currByte, 7, 2) == 0b000001)
    {
        decoded.opcode = 0b000001;
        decoded.direction = 0; 
        decoded.wordSize = extractBits(currByte, 0,0);
        
        ++byteIndex; 
        unsigned char currByte = buffer[byteIndex]; 
        decoded.lowData = extractBits(currByte, 7, 0);
    }

    else
    {
        decoded.opcode = extractBits(currByte, 7, 2); 

        if(decoded.opcode == 0b100000)
            decoded.sign = extractBits(currByte, 1, 1);
        else
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
        decoded.lowData = extractBits(currByte, 7, 0);
        if(data == 2)
        {
            decoded.highData = extractBits(currByte, 7, 0);
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
        else if (decoded.opcode == 0b110001 || decoded.opcode == 0b100000)
        {
            data = 1; 
            if(decoded.wordSize && !decoded.sign)
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
    std::cout << "bits 16" << '\n';

    for(instruction tmp : ins)
    {
        int disp = (int)((static_cast<int>(tmp.highDisp) << 8) | tmp.lowDisp);
        std::cout << reg_opcode[tmp.opcode] << ' '; 
        if(((tmp.mod == 0 && tmp.rm != 6) || tmp.mod == 1 || tmp.mod == 2) && tmp.opcode != 0b1011)
        {
            tmp.reg = (tmp.wordSize == 0 ? ((tmp.reg << 1) | 0) : ((tmp.reg << 1) | 1)); 

            if(tmp.direction)
            {
                std::cout << reg_map[tmp.reg] << ", ";
            }


            switch (tmp.rm)
            {
                case 0: 
                    std::cout << "[bx + si";
                    print_disp(disp);
                    break;
                case 1:
                    std::cout << "[bx + di";
                    print_disp(disp);
                    break;
                case 2:
                    std::cout << "[bp + si";
                    print_disp(disp);
                    break;
                case 3:
                    std::cout << "[bp + di";
                    print_disp(disp);
                    break;
                case 4:
                    std::cout << "[si";
                    print_disp(disp);
                    break;
                case 5:
                    std::cout << "[di";
                    print_disp(disp);
                    break;
                case 6:
                    if(tmp.mod != 0)
                    {
                        std::cout << "[bp";
                        print_disp(disp);
                    }
                    break; 
                case 7: 
                    std::cout << "[bx";
                    print_disp(disp);
                    break;
            }

            if(!tmp.direction)
            {
                std::cout << ", " << reg_map[tmp.reg];
            }
        }
        else if(tmp.lowData)
        {
            int data = (int)((static_cast<int>(tmp.highData) << 8) | tmp.lowData);
            tmp.reg = (tmp.wordSize == 0 ? ((tmp.reg << 1) | 0) : ((tmp.reg << 1) | 1)); 
            std::cout << reg_map[tmp.reg] << ", ";
            std::cout << data << ' ';
        }

        else
        {
            tmp.rm = (tmp.wordSize == 0 ? ((tmp.rm << 1) | 0) : ((tmp.rm << 1) | 1)); 
            tmp.reg = (tmp.wordSize == 0 ? ((tmp.reg << 1) | 0) : ((tmp.reg << 1) | 1)); 
            if(tmp.direction)
            {
                std::cout << reg_map[tmp.reg] << ", ";
                std::cout << reg_map[tmp.rm] << ' ';
            }
            else
            {
                std::cout << reg_map[tmp.rm] << ", ";
                std::cout << reg_map[tmp.reg] << ' ';
            }  
        }

        std::cout << '\n';
    }
}

int main()
{
    std::vector<unsigned char> buffer = getFile("add-sub-cmp-jnz.bin"); 
    std::vector<instruction> all_instructions = getInstructions(buffer); 
    print_instructions(all_instructions); 

	return 0; 
}

