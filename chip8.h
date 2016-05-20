#pragma once

#include <string>
#include <array>

class TChip8Machine {
public:
    TChip8Machine();

    void LoadGame(const std::string filePath);
    void Execute();

private:
    uint16_t EatWord();


    void LoadAddr(uint16_t opcode);
    void Rnd(uint16_t opcode);

private:
    std::array<uint8_t, 0xFFF> Memory;

    uint16_t PC;
    std::array<uint8_t, 16> V;
    uint16_t I;

};

