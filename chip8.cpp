#include <ios>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <random>
#include "chip8.h"

namespace {

    template <size_t TFrom, size_t TTo>
    uint16_t GetOctetsRange(uint16_t word)
    {
        static_assert(TFrom <= TTo, "TFrom must be not greater than TTo");
        static_assert((1 <= TFrom && TFrom <= 4) || (1 <= TTo && TTo <= 4), "Octet number must be in range from 1 to 4");

        uint16_t mask = 0x0F;
        for(size_t i = TFrom; i < TTo; ++i) {
            mask = (mask << 4) | 0x0F;
        }

        mask = mask << 4*(TFrom - 1);
        return (word & mask) >> 4*(TFrom - 1);
    }

    template <size_t TNumber>
    uint16_t GetOctetAt(uint16_t word)
    {
        return GetOctetsRange<TNumber, TNumber>(word);
    }

    uint16_t GetLastOctet(uint16_t word)
    {
        return GetOctetAt<4>(word);
    }


}


TChip8Machine::TChip8Machine()
    : PC(0)
    , I(0)
{
    Memory.fill(0x0);
    V.fill(0x0);
}

void TChip8Machine::LoadGame(const std::string filePath)
{
    std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
    const auto fileSize = ifs.tellg();

    if (fileSize > Memory.size() - 0x200) {
        throw std::overflow_error("File is too big for CHIP8 available memory");
    }

    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(&Memory.at(0x200)), fileSize);
}

void TChip8Machine::Execute()
{
    PC = 0x200;

    typedef void (TChip8Machine::*TMemberFunc)(uint16_t);

    static std::map<uint8_t, TMemberFunc> instructions = {
        {0xA, &TChip8Machine::LoadAddr},
        {0xC, &TChip8Machine::Rnd}
    };


    while(true)
    {
        auto opcode = EatWord();

        auto it = instructions.find(GetLastOctet(opcode));
        if (it != instructions.end()) {
            auto instr = it->second;
            (this->*instr)(opcode);
        } else {
            std::stringstream ss;
            ss << "Not implemented opcode: "  << std::hex << std::uppercase << opcode;
            throw std::logic_error(ss.str());
        }
    }
}

uint16_t TChip8Machine::EatWord()
{
    uint16_t word = (Memory.at(PC) << 8) | Memory.at(PC + 1);
    PC += 2;
    return word;
}

void TChip8Machine::LoadAddr(uint16_t opcode) {
    std::cout << "LD I, " << std::hex << std::uppercase << GetOctetsRange<1,3>(opcode) << std::endl;
    I = 0x123;
}

void TChip8Machine::Rnd(uint16_t opcode) {
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    uint8_t mean = uniform_dist(e1);

    uint16_t registerNumber = GetOctetAt<3>(opcode);
    uint16_t andWith = GetOctetsRange<1,2>(opcode);
    std::cout << "RND V" << std::hex << std::uppercase << registerNumber << ", " << std::hex << std::uppercase << andWith << std::endl;
    V.at(registerNumber) = static_cast<uint8_t>(mean & andWith);
}

