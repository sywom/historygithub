#include "AnimationManager.h"
#include <iostream>

// =======================================================
// СПАВН WIN / LOSE
// =======================================================

void AnimationManager::spawnWin(sf::Vector2f pos)
{
    Effect e;
    e.pos = pos;
    e.isWin = true;
    e.timer = 0.f;
    e.duration = 0.5f;
    e.scale = 0.f;
    e.alpha = 255.f;
    e.finished = false;

    effects.push_back(e);
}

void AnimationManager::spawnLose(sf::Vector2f pos)
{
    Effect e;
    e.pos = pos;
    e.isWin = false;
    e.timer = 0.f;
    e.duration = 0.5f;
    e.scale = 0.f;
    e.alpha = 255.f;
    e.finished = false;

    effects.push_back(e);
}




// =======================================================
// ЛИНИИ (команды)
// =======================================================

void AnimationManager::spawnLineAppear(const std::vector<sf::Vector2f>& curve, sf::Color color)
{
    CommandEffect e;
    e.curve = curve;
    e.t = 0.f;
    e.speed = 3.f;
    e.finished = false;
    e.color = color;

    std::cout << "запрос на анимацию создан\n";

    commands.push_back(e);
}

// =======================================================
// UPDATE
// =======================================================

void AnimationManager::update(float dt)
{
    // =========================
    // WIN / LOSE эффекты
    // =========================
    for (auto &e : effects)
    {
        e.timer += dt;

        float t = e.timer / e.duration;

        if (t >= 1.f)
        {
            e.finished = true;
            continue;
        }

        float fade;

        if (t < 0.5f)
            fade = t / 0.5f;                 // появление
        else
            fade = 1.f - (t - 0.5f) / 0.5f;  // исчезновение

        e.scale = fade;
        e.alpha = 255.f * fade;
    }

    effects.erase(
        std::remove_if(effects.begin(), effects.end(),
            [](const Effect& e)
            {
                return e.finished;
            }),
        effects.end()
    );

    // =========================
    // ЛИНИИ
    // =========================
    for (auto &c : commands)
    {
        c.t += dt * c.speed;
        if (c.t > 1.f) c.t = 1.f;
    }
    commands.erase(
        std::remove_if(commands.begin(), commands.end(),
            [](const CommandEffect& c)
            {
                return c.finished;
            }),
        commands.end()
    );
}

// =======================================================
// DRAW
// =======================================================

void AnimationManager::draw(sf::RenderWindow& window)
{
    // =========================
    // ЛИНИИ
    // =========================
    for (auto &l : commands)
    {
        if (l.curve.size() < 2) continue;

        // сколько точек показываем
        float exact = (l.curve.size() - 1) * l.t;
        int count = (int)exact + 1;

        if (count < 2) count = 2;
        if (count > (int)l.curve.size())
            count = l.curve.size();

        sf::VertexArray va(sf::LineStrip, count);

        for (int i = 0; i < count; i++)
        {
            va[i].position = l.curve[i];
            va[i].color = l.color;
        }

        window.draw(va);
    }

    // =========================
    // WIN / LOSE эффекты
    // =========================
    for (auto &e : effects)
    {
        sf::RectangleShape rect(sf::Vector2f(30.f, 30.f));
        rect.setOrigin(15.f, 15.f);
        rect.setPosition(e.pos);

        rect.setScale(e.scale, e.scale);

        sf::Color c = e.isWin ? sf::Color::Green : sf::Color::Red;
        c.a = (sf::Uint8)e.alpha;

        rect.setFillColor(c);

        window.draw(rect);
    }
}
