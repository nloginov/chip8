#include <ios>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <random>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>

#include <thread>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Window/Context.hpp>
#include "utils/bitutils.h"
#include "chip8.h"
#include "opcode/types.h"
#include "opcode/parser.h"

namespace {
    std::mutex videoMemoryAccess;
    std::mutex delayTimerAccess;
    std::mutex speakerTimerAccess;
    std::mutex executionLock;

    std::condition_variable keyEvent;

    void DelayTimer(const sf::RenderWindow& screen, volatile uint8_t& timer) {
        while(screen.isOpen()) {
            {
                std::lock_guard<std::mutex> lock(delayTimerAccess);
                if (timer > 0) {
                    timer--;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
        }
    }

    void Speaker(const sf::RenderWindow& screen, volatile uint8_t& speaker) {
        sf::SoundBuffer buffer;
        buffer.loadFromFile("/Users/lognick/Downloads/440Hz_44100Hz_16bit_05sec.wav");
        sf::Sound sound;
        sound.setBuffer(buffer);
        sound.setLoop(true);

        while(screen.isOpen()) {
            {
                std::lock_guard<std::mutex> lock(speakerTimerAccess);
                if (speaker > 0) {
                    sound.play();
                    speaker--;
                }
                else {
                    sound.stop();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
        }
    }

    void Render(sf::RenderWindow& screen, const TChip8Machine::TVideoMemory& videoMemory) {
        while (screen.isOpen()) {
            screen.clear();
            for (size_t x = 0; x < videoMemory.size(); ++x) {
                for (size_t y = 0; y < videoMemory.at(x).size(); ++y) {
                    bool needLightPixel;
                    {
                        std::lock_guard<std::mutex> lock(videoMemoryAccess);
                        needLightPixel = videoMemory.at(x).at(y);
                    }

                    if(needLightPixel) {
                        sf::RectangleShape pixel({10, 10});
                        pixel.setPosition(x * 10, y * 10);
                        pixel.setFillColor(sf::Color::White);
                        screen.draw(pixel);
                    }
                }
            }
            screen.display();

            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
        }
    }

    std::string PrintLikeHex(const uint16_t word) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << word;
        return ss.str();
    }
}


TChip8Machine::TChip8Machine()
    : Screen(sf::VideoMode(640, 320), "CHIP-8", sf::Style::Close)
    {
        ResetState();
    }


void TChip8Machine::LoadGame(const std::string filePath)
{
    std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
    const auto fileSize = ifs.tellg();

    if (fileSize > State.Memory.size() - 0x200) {
        throw std::overflow_error("File is too big for CHIP8 available memory");
    }

    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(&State.Memory.at(0x200)), fileSize);
}

void TChip8Machine::Execute() {
    sf::Shader::isAvailable();

    std::thread renderingThread([this](){
        Render(this->Screen, this->State.VideoMemory);
    });

    std::thread delayTimerThread([this]() {
        DelayTimer(this->Screen, this->State.DT);
    });

    std::thread speakerThread([this]() {
        Speaker(this->Screen, this->State.ST);
    });

    TCPU CpuTask(State);
    std::thread executionThread(CpuTask);

    delayTimerThread.detach();
    speakerThread.detach();
    renderingThread.detach();
    executionThread.detach();


    static std::map<sf::Keyboard::Key, uint8_t> buttons {
            {sf::Keyboard::Key::Num1, 1 },
            {sf::Keyboard::Key::Num2, 2 },
            {sf::Keyboard::Key::Num3, 3 },
            {sf::Keyboard::Key::Num4, 0xC },
            {sf::Keyboard::Key::Q, 4 },
            {sf::Keyboard::Key::W, 5 },
            {sf::Keyboard::Key::E, 6 },
            {sf::Keyboard::Key::R, 0xD },
            {sf::Keyboard::Key::A, 7 },
            {sf::Keyboard::Key::S, 8 },
            {sf::Keyboard::Key::D, 9 },
            {sf::Keyboard::Key::F, 0xE },
            {sf::Keyboard::Key::Z, 0xA },
            {sf::Keyboard::Key::X, 0 },
            {sf::Keyboard::Key::C, 0xD },
            {sf::Keyboard::Key::V, 0xF },
    };
    while(Screen.isOpen())
    {
        sf::Event event;
        while (Screen.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                Screen.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (buttons.find(event.key.code) != buttons.end()) {
                    State.PressedKeys = {};
                    State.PressedKeys.push(buttons.at(event.key.code));
                    keyEvent.notify_one();
                }


            }
        }
    }
}

void TChip8Machine::ResetState() {
    State.PC = 0;
    State.I = 0;
    State.Memory.fill(0x0);
    State.V.fill(0x0);
    for (auto& arr: State.VideoMemory) {
        arr.fill(0x0);
    }


    const unsigned char sprites[] = {
         0xf0, 0x90, 0x90, 0x90, 0xf0,
         0x20, 0x60, 0x20, 0x20, 0x70,
         0xf0, 0x10, 0xf0, 0x80, 0xf0,
         0xf0, 0x10, 0xf0, 0x10, 0xf0,
         0x90, 0x90, 0xf0, 0x10, 0x10,
         0xf0, 0x80, 0xf0, 0x10, 0xf0,
         0xf0, 0x80, 0xf0, 0x90, 0xf0,
         0xf0, 0x10, 0x20, 0x40, 0x40,
         0xf0, 0x90, 0xf0, 0x90, 0xf0,
         0xf0, 0x90, 0xf0, 0x10, 0xf0,
         0xf0, 0x90, 0xf0, 0x90, 0x90,
         0xe0, 0x90, 0xe0, 0x90, 0xe0,
         0xf0, 0x80, 0x80, 0x80, 0xf0,
         0xe0, 0x90, 0x90, 0x90, 0xe0,
         0xf0, 0x80, 0xf0, 0x80, 0xf0,
         0xf0, 0x80, 0xf0, 0x80, 0x80
    };

    std::copy(std::begin(sprites), std::end(sprites), State.Memory.begin() + State.GetSpriteAddr(0));
}

void TChip8Machine::TCPU::operator()()
{
    State.PC = 0x200;

    typedef void (TChip8Machine::TCPU::*TMemberFunc)(const TOpcode&);

    static std::map<EOperationType , TMemberFunc> instructions = {
        {EOperationType::CLS      , &TChip8Machine::TCPU::ClearScreen},
        {EOperationType::RET      , &TChip8Machine::TCPU::Return},
        {EOperationType::JUMP     , &TChip8Machine::TCPU::Jump},
        {EOperationType::CALL     , &TChip8Machine::TCPU::Call},
        {EOperationType::SE_CONST , &TChip8Machine::TCPU::SkipIfEqualToConst},
        {EOperationType::SE_VAR , &TChip8Machine::TCPU::SkipIfEqualToVar},
        {EOperationType::SNE_VAR , &TChip8Machine::TCPU::SkipIfNotEqualToVar},
        {EOperationType::SE_KEY   , &TChip8Machine::TCPU::SkipIfEqualToKey},
        {EOperationType::SNE_KEY   , &TChip8Machine::TCPU::SkipIfNotEqualToKey},
        {EOperationType::SNE_CONST, &TChip8Machine::TCPU::SkipIfNotEqualToConst},
        {EOperationType::LD_CONST , &TChip8Machine::TCPU::LoadConst},
        {EOperationType::ADD_CONST, &TChip8Machine::TCPU::AddConst},
        {EOperationType::LD_VAR   , &TChip8Machine::TCPU::LoadVar},
        {EOperationType::AND_VAR  , &TChip8Machine::TCPU::AndWithVar},
        {EOperationType::OR_VAR  , &TChip8Machine::TCPU::OrWithVar},
        {EOperationType::XOR_VAR  , &TChip8Machine::TCPU::XorWithVar},
        {EOperationType::ADD_VAR  , &TChip8Machine::TCPU::AddWithVar},
        {EOperationType::SHR_VAR  , &TChip8Machine::TCPU::ShrWithVar},
        {EOperationType::SHL_VAR  , &TChip8Machine::TCPU::ShlWithVar},
        {EOperationType::SUB_VAR  , &TChip8Machine::TCPU::SubWithVar},
        {EOperationType::SUBN_VAR  , &TChip8Machine::TCPU::SubnWithVar},
        {EOperationType::LD_ADDR  , &TChip8Machine::TCPU::LoadAddr},
        {EOperationType::RND      , &TChip8Machine::TCPU::Random},
        {EOperationType::DRAW     , &TChip8Machine::TCPU::Draw},
        {EOperationType::LD_KEY   , &TChip8Machine::TCPU::LoadKey},
        {EOperationType::ADD_ADDR , &TChip8Machine::TCPU::AddWithAddr},
        {EOperationType::STORE_MEM, &TChip8Machine::TCPU::StoreMemory},
        {EOperationType::STORE_DT , &TChip8Machine::TCPU::StoreDelayTimer},
        {EOperationType::STORE_ST , &TChip8Machine::TCPU::StoreSpeakerTimer},
        {EOperationType::LD_DT    , &TChip8Machine::TCPU::LoadDelayTimer},
        {EOperationType::LD_ST    , &TChip8Machine::TCPU::LoadSpeakerTimer},
        {EOperationType::LD_SPRITE, &TChip8Machine::TCPU::LoadSprite},
        {EOperationType::STORE_BCD_VAR, &TChip8Machine::TCPU::StoreBCDVar},
        {EOperationType::LD_MEM   , &TChip8Machine::TCPU::LoadMemory},
    };


    while (true) {
        const auto opcodeWord = EatWord();

        const auto& opcode = TOpcodeParser::Parse(opcodeWord);
        auto it = instructions.find(opcode.GetOperationType());
        if (it != instructions.end()) {
            auto instr = it->second;
            (this->*instr)(opcode);
        } else {
            std::stringstream ss;
            ss << "Not implemented opcode: " << PrintLikeHex(opcodeWord);
            throw std::logic_error(ss.str());
        }
    }
}

uint16_t TChip8Machine::TCPU::EatWord()
{
    uint16_t word = (State.Memory.at(State.PC) << 8) | State.Memory.at(State.PC + 1);
    State.PC += 2;
    return word;
}

void TChip8Machine::TCPU::LoadAddr(const TOpcode& opcode) {
    uint16_t loadWhat = opcode.GetArgs<TAddress>().Value;
    std::cout << "LD I, " << PrintLikeHex(loadWhat) << '\n';
    State.I = loadWhat;
}

void TChip8Machine::TCPU::Random(const TOpcode& opcode) {
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    uint8_t mean = uniform_dist(e1);

    const auto& args = opcode.GetArgs<TVarWithConst>();
    uint16_t andWith = args.Const;

    std::cout << "RND V" << PrintLikeHex(args.X) << ", " << PrintLikeHex(andWith) << '\n';
    State.V.at(args.X) = static_cast<uint8_t>(mean & andWith);
}

void TChip8Machine::TCPU::SkipIfEqualToConst(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TVarWithConst>();

    uint8_t compareWith = args.Const;

    std::cout << "SE V" << PrintLikeHex(args.X) << ", " << PrintLikeHex(compareWith) << '\n';

    if (State.V.at(args.X) == compareWith) {
        State.PC += 2;
    }
}

void TChip8Machine::TCPU::SkipIfEqualToVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    std::cout << "SE V" << PrintLikeHex(args.X) << ", V" << PrintLikeHex(args.Y) << '\n';

    if (State.V.at(args.X) == State.V.at(args.Y)) {
        State.PC += 2;
    }
}

void TChip8Machine::TCPU::SkipIfNotEqualToVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    std::cout << "SNE V" << PrintLikeHex(args.X) << ", V" << PrintLikeHex(args.Y) << '\n';

    if (State.V.at(args.X) == State.V.at(args.Y)) {
        State.PC += 2;
    }
}

void TChip8Machine::TCPU::SkipIfNotEqualToConst(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TVarWithConst>();

    uint8_t compareWith = args.Const;

    std::cout << "SNE V" << PrintLikeHex(args.X) << ", " << PrintLikeHex(compareWith) << '\n';

    if (State.V.at(args.X) != compareWith) {
        State.PC += 2;
    }
}

void TChip8Machine::TCPU::Draw(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVarsWithConst>();
    uint8_t memSize = args.Const;

    std::cout << "DRW V" << PrintLikeHex(args.X) << ", V"
                         << PrintLikeHex(args.Y) << '\n';

    State.V.at(0xF) = 0;
    for (size_t i = 0; i < memSize; ++i) {
        uint8_t memoryByte = State.Memory.at(State.I + i);
        for (size_t j = 0; j < 8; ++j) {

            uint8_t value = (memoryByte >> (7 - j)) & 0x1;
            size_t x = (State.V.at(args.X) + j) % State.VideoMemory.size();
            size_t y = (State.V.at(args.Y) + i) % State.VideoMemory.at(x).size();

            std::lock_guard<std::mutex> lock(videoMemoryAccess);
            if (static_cast<bool>(value) && State.VideoMemory.at(x).at(y)) {
                State.V.at(0xF) = 1;
                std::cout << "BINGO!!" << std::endl;
            }

            State.VideoMemory.at(x).at(y) = value ^ State.VideoMemory.at(x).at(y);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void TChip8Machine::TCPU::AddConst(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TVarWithConst>();

    uint8_t x = args.X;
    uint8_t addWith = args.Const;;

    std::cout << "ADD V" << PrintLikeHex(x) << ", "
                         << PrintLikeHex(addWith) << "\n";

    uint16_t sum = State.V.at(x) + addWith;
    State.V.at(x) = sum & 0x00FF;
}

void TChip8Machine::TCPU::Jump(const TOpcode& opcode) {
   uint16_t jumpTo = opcode.GetArgs<TAddress>().Value;
   std::cout << "JP " << PrintLikeHex(jumpTo) << '\n';

    State.PC = jumpTo;
}

void TChip8Machine::TCPU::LoadConst(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TVarWithConst>();
    
    uint8_t x = args.X;
    uint8_t loadWhat = args.Const;

    std::cout << "LD V" << PrintLikeHex(x) << ", "
                        << PrintLikeHex(loadWhat) << '\n';

    State.V.at(x) = loadWhat;
}

void TChip8Machine::TCPU::Call(const TOpcode& opcode) {
    uint16_t callTo = opcode.GetArgs<TAddress>().Value;
    State.Stack.push(State.PC);
    State.PC = callTo;

    std::cout << "CALL " << PrintLikeHex(callTo) << '\n';
}

void TChip8Machine::TCPU::Return(const TOpcode& opcode) {
    State.PC = State.Stack.top();
    State.Stack.pop();
    std::cout << "RET " << PrintLikeHex(State.PC) << '\n';
}

void TChip8Machine::TCPU::LoadVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    State.V.at(args.X) = State.V.at(args.Y);
    std::cout << "SET V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::AndWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    State.V.at(args.X) = State.V.at(args.X) & State.V.at(args.Y);
    std::cout << "AND V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::XorWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    State.V.at(args.X) = State.V.at(args.X) ^ State.V.at(args.Y);
    std::cout << "XOR V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}


void TChip8Machine::TCPU::AddWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();
    uint16_t sum = State.V.at(args.X) + State.V.at(args.Y);

    State.V.at(args.X) = static_cast<uint8_t>(sum & 0x00FF);
    State.V.at(0xF) = static_cast<uint8_t>(sum >= std::numeric_limits<uint8_t>::max());
    std::cout << "ADD V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::SubWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    State.V.at(0xF) = static_cast<uint16_t>(State.V.at(args.X) >= State.V.at(args.Y));
    State.V.at(args.X) = State.V.at(args.X) - State.V.at(args.Y);

    std::cout << "SUB V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::SubnWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    State.V.at(0xF) = static_cast<uint16_t>(State.V.at(args.Y) >= State.V.at(args.X));
    State.V.at(args.X) = State.V.at(args.Y) - State.V.at(args.X);

    std::cout << "SUBN V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::AddWithAddr(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    State.I = State.I + State.V.at(x);
    std::cout << "ADD I, V" << PrintLikeHex(x) << '\n';
}

void TChip8Machine::TCPU::LoadKey(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    std::cout << "KEY V" << PrintLikeHex(x) << std::endl;
    if (State.PressedKeys.empty()) {
        std::unique_lock<std::mutex> lock(executionLock);
        keyEvent.wait(lock);
    }

    uint8_t key = State.PressedKeys.front();
    State.PressedKeys.pop();
    State.V.at(x) = key;
}

void TChip8Machine::TCPU::LoadMemory(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    std::cout << "STORE [I] V0..V" << PrintLikeHex(x) << '\n';
    for (size_t i = 0; i <= x; ++i) {
        State.V.at(i) = State.Memory.at(State.I + i);
    }
}

void TChip8Machine::TCPU::StoreMemory(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    std::cout << "LD [I] V0..V" << PrintLikeHex(x) << '\n';
    for (size_t i = 0; i <= x; ++i) {
        State.Memory.at(State.I + i) = State.V.at(i);
    }
}

void TChip8Machine::TCPU::ClearScreen(const TOpcode& opcode) {
    for(auto& arr: State.VideoMemory) {
        arr.fill(0x0);
    }

    std::cout << "CLS\n";
}

void TChip8Machine::TCPU::StoreBCDVar(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    uint8_t var = State.V.at(x);
    State.Memory.at(State.I) = var / 100;
    State.Memory.at(State.I + 1) = (var / 10) % 10;
    State.Memory.at(State.I + 2) = var % 10;
    std::cout << "LD B, V" << PrintLikeHex(x) << '\n';
}

void TChip8Machine::TCPU::StoreDelayTimer(const TOpcode& opcode) {
    auto x = opcode.GetArgs<TVar>().X;
    {
        std::lock_guard<std::mutex> lock(delayTimerAccess);
        State.DT = State.V.at(x);
    }

    std::cout << "LD DT, V" << PrintLikeHex(x) << '\n';
}

void TChip8Machine::TCPU::LoadDelayTimer(const TOpcode& opcode) {
    auto x = opcode.GetArgs<TVar>().X;
    {
        std::lock_guard<std::mutex> lock(delayTimerAccess);
        State.V.at(x) = State.DT;
    }
    std::cout << "LD V" << PrintLikeHex(x) << ", DT\n";
}

void TChip8Machine::TCPU::LoadSpeakerTimer(const TOpcode& opcode) {
    auto x = opcode.GetArgs<TVar>().X;
    {
        std::lock_guard<std::mutex> lock(speakerTimerAccess);
        State.V.at(x) = State.ST;
    }
    std::cout << "LD V" << PrintLikeHex(x) << ", ST\n";
}

void TChip8Machine::TCPU::SkipIfEqualToKey(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    std::cout << "SKP V" << PrintLikeHex(x) << std::endl;

    if (State.PressedKeys.empty()) {
        return;
    }
    uint8_t key = State.PressedKeys.front();

    if(State.V.at(x) == key) {
        State.PressedKeys.pop();
        State.PC += 2;
    }
}

void TChip8Machine::TCPU::LoadSprite(const TOpcode& opcode) {
    const uint8_t x = opcode.GetArgs<TVar>().X;
    uint8_t num = State.V.at(x);
    State.I = State.GetSpriteAddr(num);

    std::cout << "LD F, V" << PrintLikeHex(x) << '\n';
}

void TChip8Machine::TCPU::SkipIfNotEqualToKey(const TOpcode& opcode) {
    uint8_t x = opcode.GetArgs<TVar>().X;
    std::cout << "SKNP V" << PrintLikeHex(x) << std::endl;

    if (State.PressedKeys.empty()) {
        State.PC += 2;
        return;
    }
    uint8_t key = State.PressedKeys.front();
    if(State.V.at(x) != key) {
        State.PC += 2;
    }
    else {
        State.PressedKeys.pop();
    }
}

void TChip8Machine::TCPU::StoreSpeakerTimer(const TOpcode& opcode) {
    auto x = opcode.GetArgs<TVar>().X;
    {
        std::lock_guard<std::mutex> lock(speakerTimerAccess);
        State.ST = State.V.at(x);
    }

    std::cout << "LD ST, V" << PrintLikeHex(x) << '\n';
}

void TChip8Machine::TCPU::ShrWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();
    const uint16_t operand = State.V.at(args.Y);
    State.V.at(args.X) = operand >> 1;
    State.V.at(0xF) = operand & 0x1;
    std::cout << "SHR V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::ShlWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();
    const uint16_t operand = State.V.at(args.Y);
    State.V.at(args.X) = operand << 1;
    State.V.at(0xF) = operand & 0x8000;
    std::cout << "SHL V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}

void TChip8Machine::TCPU::OrWithVar(const TOpcode& opcode) {
    const auto& args = opcode.GetArgs<TTwoVars>();

    State.V.at(args.X) = State.V.at(args.X) | State.V.at(args.Y);
    std::cout << "OR V" << PrintLikeHex(args.X) << " V" << PrintLikeHex(args.Y) << '\n';
}
