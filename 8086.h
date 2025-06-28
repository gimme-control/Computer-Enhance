#pragma once
#include <iostream> 
#include <unordered_map>
#include <unordered_set>
#include <array>

struct instruction {

        // Info Locations 
        std::array<int, 3> modLoc{};
        std::array<int, 3> regLoc{};
        std::array<int, 3> rmLoc{};
        std::array<int, 3> lDispLoc{};
        std::array<int, 3> hDispLoc{};
        std::array<int, 3> directionLoc{};
        std::array<int, 3> wordSizeLoc{};
        std::array<int, 3> lDataLoc{};
        std::array<int, 3> hDataLoc{};
        std::array<int, 3> signLoc{};

        // Printables
        std::string operation = "no-op";

        // Fields
        unsigned char opcode = 0; 
        
        unsigned char direction = 0;  
        unsigned char wordSize = 0;  
    
        unsigned char mod = 0;
        unsigned char reg = 0;  
        unsigned char rm = 0;  

        unsigned char lowDisp = 0;  
        unsigned char highDisp = 0;  

        unsigned char lowData = 0;  
        unsigned char highData = 0; 

        unsigned char sign = 0;

        instruction
        (
            unsigned char op, 
            std::string operation, 
            std::array<int, 3> directionLoc, 
            std::array<int, 3> wordSizeLoc, 
            std::array<int, 3> modLoc, 
            std::array<int, 3> regLoc, 
            std::array<int, 3> rmLoc, 
            std::array<int, 3> lDataLoc,
            std::array<int, 3> signLoc
        )
        : 
        opcode(op), 
        operation(operation), 
        directionLoc(directionLoc),
        wordSizeLoc(wordSizeLoc),
        modLoc(modLoc),
        regLoc(regLoc),
        rmLoc(rmLoc),
        lDataLoc(lDataLoc),
        signLoc(signLoc)
        {}

        instruction() = default; 
}; 

std::unordered_map<unsigned char, instruction> insInfo =
{
    {0b100010, 
    instruction(0b100010, "mov", {0,1,1}, {0,0,0}, {1,7,6}, {1,5,3}, {1,2,0}, {-1,-1,-1},
            {-1,-1,-1})
    }
    , 
    {0b110001, 
    instruction(0b110001, "mov", {0,1,1}, {0,0,0}, {1,7,6}, {1,5,3}, {1,2,0}, {-1,-1,-1}, {-1,-1,-1})
    }
    , 
    {0b1011, 
    instruction(0b1011, "mov", {-1,-1,-1}, {0,3,3}, {-1,-1,-1}, {0,2,0}, {-1,-1,-1}, {1,7,0}, {-1,-1,-1})
    }
    , 
    {0b000000, 
    instruction(0b000000, "add", {0,1,1}, {0,0,0}, {1,7,6}, {1,5,3}, {1,2,0}, {-1,-1,-1}, {-1,-1,-1})
    }
    , 
    {0b100000, 
    instruction(0b100000, "add", {-1,-1,-1}, {0,0,0}, {1,7,6}, {1,5,3}, {1,2,0}, {-1,-1,-1}, {0,1,1})
    }
}; 


inline std::unordered_map<unsigned char, int> data_addr_instructions= 
{ 
    {0b110001, 2}, 
    {0b1011, 1}, 
    {0b101000, 1}, 
    {0b101000, 1}, 
    {0b100000, 2}, 
    {0b000001, 1}
};

inline std::unordered_map<unsigned char, std::string> effective_addr = 
{
    {0, "[bx + si"},
    {1, "[bx + di"},
    {2, "[bp + si"},
    {3, "[bp + di"},
    {4, "[si"},
    {5, "[di"},
    {6, "[bp"},
    {7, "[bx"}
};

inline std::unordered_map<unsigned char, std::string> reg_map =  
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
