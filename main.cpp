#include <iostream>
#include <SFML/Graphics.hpp>
#include "chip8.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Need a game filepath as an argument";
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode(640, 320), "My window");
    window.clear(sf::Color::Black);
    window.display();

    TChip8Machine chip8Machine(window);
    chip8Machine.LoadGame(argv[1]);
    chip8Machine.Execute();
    return 0;
}