#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <functional>

class AnimationManager
{
public:

    // =========================
    // ВСПЛЫВАЮЩИЕ ЭФФЕКТЫ (галочка / крестик)
    // =========================
    struct Effect
    {
        sf::Vector2f pos;    // позиция эффекта
        bool isWin;          // true = победа, false = поражение
        float timer;         // текущее время жизни
        float duration;      // длительность эффекта
        float scale;         // масштаб появления
        float alpha;         // прозрачность
        bool finished;
    };

    // =========================
    // АНИМАЦИЯ ЛИНИИ (команда)
    // =========================
    struct CommandEffect
    {
        std::vector<sf::Vector2f> curve; // путь
        float t;          // прогресс 0..1
        float speed;      // скорость анимации
        bool appearing;     // true = появление, false = исчезновение
        bool finished;
        sf::Color color;
    };

    // =========================
    // SPAWN EFFECTS
    // =========================
    void spawnWin(sf::Vector2f pos);
    void spawnLose(sf::Vector2f pos);

    void spawnLineAppear(const std::vector<sf::Vector2f>& curve, sf::Color color);
    void spawnLineDisappear(const std::vector<sf::Vector2f>& curve, sf::Color color);

    // =========================
    // UPDATE / DRAW
    // =========================
    void delay(float seconds, std::function<void()> action);

    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    std::vector<Effect> effects;
    std::vector<CommandEffect> commands;

    float delayTimer = 0.f;
    std::function<void()> delayedAction;
};
