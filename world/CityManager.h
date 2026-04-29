#pragma once
#include "City.h"
#include <vector>

class CityManager {
public:
    std::vector<City> cities;

    City* findById(int id); // найти по id
    City* getHovered(sf::Vector2f mouse); // найти под мышкой
    bool isNeighbor(int a, int b); // сосед ли?
    bool canMoveBetweenCities(int fromId, int toId); // могу ли сделать команду туда?
    int distance(int a, int b); // расстояние между городами

    std::vector<sf::Vector2f> buildCurve(int fromCity,int toCity,float offset); // построить кривую между городами
    sf::Vector2f getPointOnCurve(std::vector<sf::Vector2f>& curve, float t);    // найти точку

    // загрузка всех значений
    void loadFromFile(const std::string& filename, sf::Font& font);
    void loadConnections(const std::string& filename);
};
