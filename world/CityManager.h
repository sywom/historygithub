#pragma once
#include "City.h"
#include <vector>



class CityManager {
public:
    std::vector<City> cities;

    City* findById(int id); // найти по id
    City* getHovered(sf::Vector2f mouse); // найти под мышкой
    bool isNeighbor(int a, int b); // сосед ли?
    bool canMoveBetweenCities(int fromId, int toId, int owner); // могу ли сделать команду туда?
    int distance(int a, int b); // расстояние между городами

    std::vector<sf::Vector2f> buildCurve(sf::Vector2f A, sf::Vector2f B, float offset); // построить кривую между городами
    sf::Vector2f getPointOnCurve(std::vector<sf::Vector2f>& curve, float t);   // найти точку

    sf::Texture ourCityTexture;
    sf::Texture franceCityTexture;
    sf::Texture ciegeCityTexture;
    sf::Texture selectedCityTexture;
    sf::Texture sosedCityTexture;

    // загрузка всех значений
    void setTexture(int cityId, int state);
    void loadFromFile(const std::string& filename, sf::Font& font);
    void loadConnections(const std::string& filename);
};
