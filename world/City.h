#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct City {
    int id;
    int owner;
    std::vector<int> neighbors;

    std::string name;
    sf::Text label;
    sf::Vector2f position;
    sf::Sprite marker;

    bool isSelected = false;
};
