#include "CityManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

// получить указатель по id
City* CityManager::findById(int id)
{
    for (auto &c : cities)
        if (c.id == id) return &c;
    return nullptr;
}

// расстояние между городами
int CityManager::distance(int a, int b)
{
    City* A = findById(a);
    City* B = findById(b);

    if (!A || !B) return 1;

    float dx = A->position.x - B->position.x;
    float dy = A->position.y - B->position.y;

    return std::max(1, (int)(sqrt(dx*dx + dy*dy) / 100.f));
}

// получить указатель на город под мышкой
City* CityManager::getHovered(sf::Vector2f mouse)
{
    for (auto &c : cities)
    {
        float dx = mouse.x - c.position.x;
        float dy = mouse.y - c.position.y;

        if (dx*dx + dy*dy < c.marker.getRadius()*c.marker.getRadius())
            return &c;
    }
    return nullptr;
}

// явлется ли город соседом
bool CityManager::isNeighbor(int a, int b)
{
    City* c = findById(a);
    if (!c) return false;

    for (int n : c->neighbors)
        if (n == b) return true;

    return false;
}

// является ли город крутым соседом (нельзя штурмовать не твои города, если стоишь не на своем)
bool CityManager::canMoveBetweenCities(int fromId, int toId)
{
    City* from = findById(fromId);
    City* to   = findById(toId);

    if (!from || !to) return false;

    //  должны быть соседями
    if (!isNeighbor(fromId, toId)) return false;

    // если цель НЕ твоя (щас красным)
    if (to->owner != 0)
    {
        // можно идти только из зелёного города
        if (from->owner != 0)
            return false;
    }

    return true;
}

// загрузка городов
void CityManager::loadFromFile(const std::string& filename, sf::Font& font)
{
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
            std::cout << "Failed to open cities file\n";
            return;
    }


    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string idStr, name, xStr, yStr, ownerStr;

        std::getline(ss, idStr, ';');
        std::getline(ss, name, ';');
        std::getline(ss, xStr, ';');
        std::getline(ss, yStr, ';');
        std::getline(ss, ownerStr, ';');

        City c;
        c.id = std::stoi(idStr);
        c.name = name;
        c.position = {std::stof(xStr), std::stof(yStr)};
        c.owner = std::stoi(ownerStr);

        c.marker = sf::CircleShape(20.f);
        c.marker.setOrigin(20,20);
        c.marker.setPosition(c.position);

        c.label.setFont(font);
        c.label.setCharacterSize(18);
        c.label.setString(name);
        c.label.setPosition(c.position.x, c.position.y - 25);
        c.label.setString(sf::String::fromUtf8(c.name.begin(), c.name.end()));

        cities.push_back(c);
    }
    std::cout << "cities ok\n";

}
// загрузкза информации о сосеоях
void CityManager::loadConnections(const std::string& filename)
{
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
            std::cout << "Failed to open connections file\n";
            return;
    }


    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string idStr, neighborsStr;

        std::getline(ss, idStr, ':');
        std::getline(ss, neighborsStr);

        City* city = findById(std::stoi(idStr));
        if (!city) continue;

        std::stringstream ns(neighborsStr);
        std::string item;

        while (std::getline(ns, item, ','))
        {
            if (!item.empty())
                city->neighbors.push_back(std::stoi(item));
        }
    }
    std::cout << "connections ok\n";
}

// построить кривую, сунул сюда потому что круто, чаще всего по двум точкам работать
std::vector<sf::Vector2f> CityManager::buildCurve(sf::Vector2f A, sf::Vector2f B, float offset)
{
    std::vector<sf::Vector2f> points;

    sf::Vector2f dir = B - A;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0) return points;
    dir /= len;

    // нормаль и все что связано с ней
    sf::Vector2f normal(-dir.y, dir.x);

    if (normal.y > 0) normal = -normal;
    //  куда гнуть
    float side = ((A.x + A.y) < (B.x + B.y)) ? 1.f : -1.f;

    // ===================== ФОРМА =====================
    float curveStrength = len * offset;

    sf::Vector2f mid = (A + B) / 2.f;
    sf::Vector2f control = mid + normal * side * curveStrength;

    // ===================== самая кривая =====================
    const int segments = 20;

    for (int i = 0; i < segments; i++)
    {
        float t = i / (float)(segments - 1);
        float u = 1.f - t;

        sf::Vector2f p =
            u * u * A +
            2.f * u * t * control +
            t * t * B;

        points.push_back(p);
    }

    return points;
}

// найти точку на кривой
sf::Vector2f CityManager::getPointOnCurve(std::vector<sf::Vector2f>& curve, float t)
{
    if (curve.empty()) return {};

    t = std::max(0.f, std::min(1.f, t));

    int index = (int)(t * (curve.size() - 1));
    return curve[index];
}
