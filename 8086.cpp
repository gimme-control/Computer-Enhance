#include "8086.h"
#include <fstream>
#include <vector>

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
    if(startBit < 0 || endBit < 0)
        return 0;  
    unsigned char mask = ((1 << (startBit - endBit + 1)) - 1) << endBit; 
    return (curr & mask) >> endBit; 
}

void print_disp(int disp)
{
    if(disp)
        std::cout << " + " << disp << "]";
    else 
        std::cout << "]";
}

// Real Logic
instruction decodeInstruction(std::vector<unsigned char>& buffer, int byteIndex)
{
    instruction decoded; 

    for(int i = 7; i > 1; --i)
    {
        if(insInfo.count(extractBits(buffer[byteIndex], 7, i)) > 0)
        {
            decoded = insInfo[extractBits(buffer[byteIndex], 7, i)]; 
        }
    }

    decoded.wordSize = extractBits(buffer[decoded.wordSizeLoc[0] + byteIndex], decoded.wordSizeLoc[1], decoded.wordSizeLoc[2]);
    decoded.direction = extractBits(buffer[decoded.directionLoc[0] + byteIndex], decoded.directionLoc[1], decoded.directionLoc[2]);
    
    decoded.mod = extractBits
        (buffer[decoded.modLoc[0] + byteIndex], decoded.modLoc[1], decoded.modLoc[2]);
    decoded.reg = extractBits
        (buffer[decoded.regLoc[0] + byteIndex], decoded.regLoc[1], decoded.regLoc[2]);
    decoded.rm = extractBits
        (buffer[decoded.rmLoc[0] + byteIndex], decoded.rmLoc[1], decoded.rmLoc[2]);

    decoded.lowData = extractBits(buffer[decoded.lDataLoc[0] + byteIndex], decoded.lDataLoc[1], decoded.lDataLoc[2]);
    decoded.sign = extractBits(buffer[decoded.signLoc[0] + byteIndex], decoded.signLoc[1], decoded.signLoc[2]);

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
        switch(data)
        {
            case 1: 
            {
                decoded.lowData = extractBits(currByte, 7, 0);
            } break; 
            case 2: 
            {
                decoded.lowData = extractBits(currByte, 7, 0);
                currByte = buffer[index + 1];
                decoded.highData = extractBits(currByte, 7, 0);
            } break; 
            case 3: 
            {
                decoded.highData = extractBits(currByte, 7, 0);
            } break; 
            default:
            {
            } break; 
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
            case 0:  
            {
                if(decoded.rm == 0b110)
                    disp = 2;  
                else
                    disp = 0; 
            } break;
            case 1:  
            {
                disp = 1; 
            } break; 
            case 2:  
            {
                disp = 2; 
            } break; 
            default: 
            {
                disp = 0; 
            }
        }

        // data = 1 means decoded 1 byte in low 
        // data = 2 means decode 2 bytes in low and high 
        // data = 3 means just decoded 1 byte in high

        if(data_addr_instructions.count(decoded.opcode))
        {
            switch(data_addr_instructions[decoded.opcode])
            {
                case 1:
                {
                    if(decoded.wordSize)
                    { 
                        data = 3; 
                        disp = 0; 
                    }
                } break; 
                case 2: 
                {
                    data = 1; 
                    if(decoded.wordSize && !decoded.sign) // sign is true by default 
                        ++data; 
                } break; 
            }
        }

        if(disp || data) decodeAdditional(buffer, decoded, i+2, disp, data); 
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
        std::cout << tmp.operation << ' '; 

        if(((tmp.mod == 0 && tmp.rm != 6) || tmp.mod == 1 || tmp.mod == 2) && tmp.opcode != 0b1011)
        {
            tmp.reg = (tmp.wordSize == 0 ? ((tmp.reg << 1) | 0) : ((tmp.reg << 1) | 1)); 

            if(tmp.direction)
            {
                std::cout << reg_map[tmp.reg] << ", ";
            }


            if(tmp.rm == 6)
            {
                if(tmp.mod != 0)
                {
                    std::cout << effective_addr[tmp.rm]; 
                    print_disp(disp);
                }
            }

            else
            {
                std::cout << effective_addr[tmp.rm]; 
                print_disp(disp);
            }

            if(!tmp.direction)
            {
                std::cout << ", " << reg_map[tmp.reg];
            }
        }
        else if(tmp.lowData)
        {
            int data = (int)((static_cast<int>(tmp.highData) << 8) | tmp.lowData);
            tmp.rm = (tmp.wordSize == 0 ? ((tmp.rm << 1) | 0) : ((tmp.rm << 1) | 1)); 
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

int main(int argc, char *argv[])
{

    if(argc < 1)
    {
        std::cout << "Usage: ./8086.exe filepath" << '\n';
    }

    std::vector<unsigned char> buffer = getFile(argv[1]); 
    std::vector<instruction> all_instructions = getInstructions(buffer); 
    print_instructions(all_instructions); 

	return 0; 
}
