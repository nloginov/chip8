#include <iostream>
#include "chip8.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Need a game filepath as an argument";
        return 1;
    }

    TChip8Machine chip8Machine;
    chip8Machine.LoadGame(argv[1]);
    chip8Machine.Execute();
    return 0;
}