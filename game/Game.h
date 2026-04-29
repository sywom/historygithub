#pragma once
#include <SFML/Graphics.hpp>

#include "../world/CityManager.h"
#include "../world/ArmyManager.h"
#include "../world/CommandManager.h"
#include "AnimationManager.h"


// заголовок со всемми начальными значениями для игровых параметров
class Game {
public:
    void init();
    void run();

private:
    sf::RenderWindow window;
    sf::View view;

    // ===== MAP =====
    sf::Image backgroundImage;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    float borderX = 0.f;
    float borderY = 0.f;

    // ===== MOUSE =====
    sf::Vector2f mouseWorld;
    sf::Vector2i mouseScreen;

    // ===== CAMERA =====
    bool isDrag = false;
    sf::Vector2i beginDragPos;

    sf::Vector2f targetCenter;
    float targetZoom = 1.f;
    float currentZoom = 1.f;
    const float smoothFactor = 0.2f;

    // ===== GAME STATE =====
    enum GameState
    {
        Idle,
        SelectingTarget,
        SelectingRetreat,
        EditingCommand
    };
    GameState state = Idle;

    int selectedArmyId = -1;
    int selectedCommandIndex = -1;

    bool showPopup = false;
    int inputUnits = 0;

    bool showConnections = false;

    // ===== HOVER =====
    City* hoveredCity = nullptr;

    // ===== MANAGERS =====
    CityManager cityMgr;
    ArmyManager armyMgr;
    CommandManager commandMgr;
    AnimationManager animMgr;

    // ===== FONT =====
    sf::Font font;

    // ===== SYSTEM =====
    void processEvents();
    void update(float dt);
    void render();
    //
    int maxCommands = 3;
    // потом мооожет быть есои не лень
    void handleMouseClick(sf::Event& event);

    void updateHover();
    void updateCamera();
    void updatePopup();

    void drawWorld();
    void drawUI();

    // логика передвежения армий
    sf::FloatRect endTurnButton;

    // время - фильм такой есть, смешной
    bool waitingForSimulation = false;
    float waitTimer = 0.f;
};
