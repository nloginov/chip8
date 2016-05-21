#pragma once

#include <string>
#include <array>
#include <SFML/Graphics/RenderWindow.hpp>

class TChip8Machine {
public:
    TChip8Machine(sf::RenderWindow& screen);

    void LoadGame(const std::string filePath);
    void Execute();

private:
    uint16_t EatWord();


    void LoadAddr(uint16_t opcode);
    void Rnd(uint16_t opcode);
    void SkipIf(uint16_t opcode);
    void Draw(uint16_t opcode);

    void DrawPixel(size_t x, size_t y, bool isEnabled);

private:
    sf::RenderWindow& Screen;
    std::array<uint8_t, 0xFFF> Memory;

    uint16_t PC;
    std::array<uint8_t, 16> V;
    uint16_t I;

};

