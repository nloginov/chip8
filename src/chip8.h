#pragma once

#include <string>
#include <array>
#include <SFML/Graphics/RenderWindow.hpp>
#include <stack>
#include <queue>

class TOpcode;


class TChip8Machine {
public:
    using TVideoMemory = std::array<std::array<bool, 32>, 64>;

private:
    struct TState {
        std::array<uint8_t, 0xFFF> Memory;
        TVideoMemory VideoMemory;

        uint16_t PC;
        std::array<uint8_t, 16> V;
        uint16_t I;

        volatile uint8_t DT;
        volatile uint8_t ST;

        std::stack<uint16_t> Stack;
        std::queue<uint8_t> PressedKeys;

        uint16_t GetSpriteAddr(size_t num) {
            const int spritesCount = 16;
            const int spriteSize = 5;
            return Memory.size() - spriteSize * (spritesCount - num);
        }
    };


    class TCPU {
    public:
        TCPU(TState& state)
            : State(state)
        {};

        void operator() ();

    private:
        TState& State;
    private:
        uint16_t EatWord();

        void ClearScreen(const TOpcode&);
        void Draw(const TOpcode& opcode);

        void AddConst(const TOpcode& opcode);
        void AndWithVar(const TOpcode& opcode);
        void OrWithVar(const TOpcode& opcode);
        void AddWithVar(const TOpcode& opcode);
        void SubWithVar(const TOpcode& opcode);
        void SubnWithVar(const TOpcode& opcode);
        void XorWithVar(const TOpcode& opcode);
        void ShrWithVar(const TOpcode& opcode);
        void ShlWithVar(const TOpcode& opcode);
        void AddWithAddr(const TOpcode& opcode);
        void Random(const TOpcode& opcode);

        void Jump(const TOpcode& opcode);
        void Call(const TOpcode& opcode);
        void Return(const TOpcode& opcode);
        void SkipIfEqualToConst(const TOpcode& opcode);
        void SkipIfEqualToVar(const TOpcode& opcode);
        void SkipIfNotEqualToVar(const TOpcode& opcode);
        void SkipIfEqualToKey(const TOpcode& opcode);
        void SkipIfNotEqualToKey(const TOpcode& opcode);
        void SkipIfNotEqualToConst(const TOpcode& opcode);

        void LoadConst(const TOpcode& opcode);
        void LoadKey(const TOpcode& opcode);
        void LoadVar(const TOpcode& opcode);
        void LoadMemory(const TOpcode& opcode);
        void LoadAddr(const TOpcode& opcode);

        void StoreMemory(const TOpcode& opcode);
        void StoreDelayTimer(const TOpcode& opcode);
        void StoreSpeakerTimer(const TOpcode& opcode);
        void LoadDelayTimer(const TOpcode& opcode);
        void LoadSpeakerTimer(const TOpcode& opcode);
        void StoreBCDVar(const TOpcode& opcode);
        void LoadSprite(const TOpcode& opcode);
    };

public:
    TChip8Machine();

    void LoadGame(const std::string filePath);
    void Execute();

private:
    TState State;
    sf::RenderWindow Screen;
private:
    void ResetState();

};

