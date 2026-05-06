#include "AnimationManager.h"


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
    e.speed = 2.f;
    e.finished = false;
    e.color = color;

    e.appearing = true;

    commands.push_back(e);
}

void AnimationManager::spawnLineDisappear(const std::vector<sf::Vector2f>& curve, sf::Color color)
{
    CommandEffect e;
    e.curve = curve;
    e.t = 1.f;          // начинаем с полной линии
    e.speed = 2.f;
    e.finished = false;
    e.color = color;

    e.appearing = false;

    commands.push_back(e);
}

//============================
// spawn number
//============================
void AnimationManager::spawnNumber(sf::Vector2f pos, int value, float xOffset)
{
    NumberEffect e;
    e.pos = pos;
    e.value = value;

    e.timer = 0.f;
    e.duration = 0.8f;

    e.xOffset = xOffset;
    e.yOffset = 0.f;
    e.alpha = 255.f;

    e.finished = false;

    numbers.push_back(e);
}


//===================== небольщая пауза для отрисовки анимаций ========= (помог иишка)
void AnimationManager::delay(float seconds, std::function<void()> action)
{
    delayTimer = seconds;
    delayedAction = action;
}

// =======================================================
// UPDATE
// =======================================================

void AnimationManager::update(float dt)
{
    // delay
    if (delayTimer > 0.f)
    {
        delayTimer -= dt;

        if (delayTimer <= 0.f && delayedAction)
        {
            delayedAction();
            delayedAction = nullptr;
        }
    }

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

        if (c.appearing)
        {
            c.t += dt * c.speed;
            if (c.t > 1.f)
            {
                c.t = 1.f;
                c.finished = true;
            }
        }
        else
        {
            c.t -= dt * c.speed;
            if (c.t <= 0.f)
            {
                c.t = 0.f;
                c.finished = true;
            }
        }

    }
    commands.erase(
        std::remove_if(commands.begin(), commands.end(),
            [](const CommandEffect& c)
            {
                return c.finished;
            }),
        commands.end()
    );

    //=======================================
    //  числа
    //=======================================
    for (auto &n : numbers)
    {
        n.timer += dt;

        float t = n.timer / n.duration;

        if (t >= 1.f)
        {
            n.finished = true;
            continue;
        }

        // поднимается вверх
        n.yOffset = -30.f * t;

        // плавное исчезновение
        n.alpha = 255.f * (1.f - t);
    }

    numbers.erase(
        std::remove_if(numbers.begin(), numbers.end(),
            [](const NumberEffect& n)
            {
                return n.finished;
            }),
        numbers.end()
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

    //===============================
    // numbers
    //================================
    for (auto &n : numbers)
    {
        sf::Font font;
        font.loadFromFile("fonts/DejaVuSans.ttf");

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(20);

        text.setString("-" + std::to_string(n.value));

        text.setPosition(n.pos + sf::Vector2f(n.xOffset, n.yOffset));

        sf::Color c = sf::Color::Black;
        c.a = (sf::Uint8)n.alpha;

        text.setFillColor(c);

        window.draw(text);
    }

}
