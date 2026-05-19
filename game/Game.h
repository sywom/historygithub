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

    void initMenu();
    void initPauseMenu();

    void updateCamera(float dt);
    void hoverCity();
    void updateHud();
    void updateMenu();
    void updatePauseMenu();

    void updatePauseMenuLayout();
    void updateMenuLayout();
    void updateHUDLayout();

    void renderCities();
    void renderCommands();
    void renderPopUp();
    void renderFps(float dt);
    void renderMenu();
    void renderPauseMenu();

    int playerTotalUnits = 0;
    int enemyTotalUnits = 0;

    // время - фильм такой есть, смешной
    bool waitingForAnimationAppear = false;
    bool waitingForAnimationDisappear = false;
    float waitTimer = 0.f;
    float animationTime = 0.5f;



    // =========== настройки разрешения ===============
    void applyVideoSettings();
    struct VideoSettings
    {
        sf::Vector2u resolution = {1280, 720};

        bool fullscreen = false;
    };

    VideoSettings settings;

    std::vector<sf::Vector2u> resolutions =
    {
        {900, 600},
        {1280, 720},
        {1366, 768},
        {1600, 900},
        {1920, 1080}
    };

    int currentResolutionIndex = 1;

    // ==================================================
    // MENU UI
    // ==================================================


    sf::Text titleText;
    sf::FloatRect endTurnButton;

    sf::RectangleShape hudBackground;
    sf::RectangleShape endTurnShape;
    sf::RectangleShape centerPanel;

    sf::Text endTurnText;
    sf::Text centerText;

    // ===================== MAIN MENU =====================

    sf::RectangleShape menuBg;

    sf::Text title;

    sf::Text startButton;
    sf::Text resolutionButton;
    sf::Text fullscreenButton;
    sf::Text exitButton;

    sf::FloatRect startButtonRect;
    sf::FloatRect resolutionButtonRect;
    sf::FloatRect fullscreenButtonRect;
    sf::FloatRect exitButtonRect;

    // ===================== PAUSE MENU =====================

    sf::RectangleShape pauseBg;

    sf::Text pauseTitle;

    sf::Text continueButton;
    sf::Text pauseResolutionButton;
    sf::Text pauseFullscreenButton;
    sf::Text backToMenuButton;

    sf::FloatRect continueButtonRect;
    sf::FloatRect pauseResolutionButtonRect;
    sf::FloatRect pauseFullscreenButtonRect;
    sf::FloatRect backToMenuButtonRect;
};


