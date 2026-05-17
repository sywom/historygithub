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
    enum GameGlobalState
    {
        Menu,
        Playing,
        Paused
    };
    GameGlobalState globalState = Menu;

    enum GameState
    {
        Idle,
        SelectingTarget,
        SelectingRetreat,
        EditingCommand
    };
    GameState state = Idle;
    int turn=0;

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
    void render(float dt);
    //
    int maxCommands = 3;


    void updateCamera(float dt);
    void hoverCity();
    void updateHud();

    void renderCities();
    void renderCommands();
    void renderPopUp();
    void renderFps(float dt);

    void renderMenu();

    // hud
    sf::RectangleShape menuBg;
    sf::Text title;
    sf::Text startButton;
    sf::FloatRect startButtonRect;


    sf::FloatRect endTurnButton;

    sf::RectangleShape hudBackground;
    sf::RectangleShape endTurnShape;
    sf::RectangleShape centerPanel;

    sf::Text endTurnText;
    sf::Text centerText;

    int playerTotalUnits = 0;
    int enemyTotalUnits = 0;

    // время - фильм такой есть, смешной
    bool waitingForAnimationAppear = false;
    bool waitingForAnimationDisappear = false;
    float waitTimer = 0.f;
    float animationTime = 0.5f;
};
