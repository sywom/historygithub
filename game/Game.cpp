#include "Game.h"
#include "SimulationSystem.h"
#include <cmath>
#include <iostream>


// пока не знаю в какую структурубрать (подсчет расстояния до кривой)
bool isNearCurve(sf::Vector2f mouse, const std::vector<sf::Vector2f>& pts, float thickness)
{
    float r2 = thickness * thickness;

    for (size_t i = 1; i < pts.size(); i++)
    {
        sf::Vector2f a = pts[i - 1];
        sf::Vector2f b = pts[i];

        sf::Vector2f ab = b - a;
        sf::Vector2f am = mouse - a;

        float abLen2 = ab.x * ab.x + ab.y * ab.y;
        if (abLen2 == 0) continue;

        float t = (ab.x * am.x + ab.y * am.y) / abLen2;
        t = std::max(0.f, std::min(1.f, t));

        sf::Vector2f closest = a + ab * t;
        sf::Vector2f d = mouse - closest;

        if (d.x * d.x + d.y * d.y <= r2)
            return true;
    }
    return false;
}


void Game::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        processEvents();
        update(dt);
        render(dt);
    }
}

// ==================================================
// ===================== INIT =====================
// ==================================================

void Game::initMenu()
{
    // =====================================================
    // BACKGROUND
    // =====================================================

    menuBg.setFillColor(
        sf::Color(20,20,20)
    );

    // =====================================================
    // TITLE
    // =====================================================

    title.setFont(font);

    title.setString(
        L"there is nothing i can do"
    );

    title.setCharacterSize(40);

    // =====================================================
    // START BUTTON
    // =====================================================

    startButton.setFont(font);

    startButton.setString(
        L"Начать игру"
    );

    startButton.setCharacterSize(24);

    // =====================================================
    // RESOLUTION BUTTON
    // =====================================================

    resolutionButton.setFont(font);

    resolutionButton.setCharacterSize(24);

    // =====================================================
    // FULLSCREEN BUTTON
    // =====================================================

    fullscreenButton.setFont(font);

    fullscreenButton.setCharacterSize(24);

    // =====================================================
    // EXIT BUTTON
    // =====================================================

    exitButton.setFont(font);

    exitButton.setString(
        L"Выход"
    );

    exitButton.setCharacterSize(24);
}

void Game::initPauseMenu()
{
    // =====================================================
    // BACKGROUND
    // =====================================================

    pauseBg.setFillColor(
        sf::Color(0,0,0,180)
    );

    // =====================================================
    // TITLE
    // =====================================================

    pauseTitle.setFont(font);

    pauseTitle.setString(
        L"Пауза"
    );

    pauseTitle.setCharacterSize(42);

    // =====================================================
    // CONTINUE BUTTON
    // =====================================================

    continueButton.setFont(font);

    continueButton.setString(
        L"Продолжить"
    );

    continueButton.setCharacterSize(24);

    // =====================================================
    // RESOLUTION BUTTON
    // =====================================================

    pauseResolutionButton.setFont(font);

    pauseResolutionButton.setCharacterSize(24);

    // =====================================================
    // FULLSCREEN BUTTON
    // =====================================================

    pauseFullscreenButton.setFont(font);

    pauseFullscreenButton.setCharacterSize(24);

    // =====================================================
    // BACK TO MENU BUTTON
    // =====================================================

    backToMenuButton.setFont(font);

    backToMenuButton.setString(
        L"Главное меню"
    );

    backToMenuButton.setCharacterSize(24);
}

void Game::initEndGameUI()
{
    endGameBg.setFillColor(sf::Color(0, 0, 0, 200));

    endGameTitle.setFont(font);
    endGameTitle.setCharacterSize(42);

    endGameButton.setFont(font);
    endGameButton.setString(L"В меню");
    endGameButton.setCharacterSize(24);
}

void Game::applyVideoSettings()
{
    sf::VideoMode mode;

    sf::Uint32 style;

    // =====================================================
    // VIDEO MODE
    // =====================================================

    if (settings.fullscreen)
    {
        mode = sf::VideoMode::getDesktopMode();

        style = sf::Style::Fullscreen;
    }
    else
    {
        mode = sf::VideoMode(
            settings.resolution.x,
            settings.resolution.y
        );

        style = sf::Style::Close;
    }

    // =====================================================
    // CREATE WINDOW
    // =====================================================

    window.create(
        mode,
        "History",
        style
    );

    window.setFramerateLimit(75);

    // =====================================================
    // RESET VIEW
    // =====================================================

    view = window.getDefaultView();

    // =====================================================
    // CAMERA CLAMP AFTER RESOLUTION CHANGE
    // =====================================================

    sf::Vector2f size = view.getSize();

    float halfW = size.x / 2.f;
    float halfH = size.y / 2.f;

    // ограничение targetCenter в пределах карты
    if (targetCenter.x - halfW < 0)
        targetCenter.x = halfW;

    if (targetCenter.y - halfH < 0)
        targetCenter.y = halfH;

    if (targetCenter.x + halfW > 2 * borderX)
        targetCenter.x = 2 * borderX - halfW;

    if (targetCenter.y + halfH > 2 * borderY)
        targetCenter.y = 2 * borderY - halfH;

    // синхронизация камеры
    view.setCenter(targetCenter);

    // =====================================================
    // UPDATE UI LAYOUTS
    // =====================================================

    updateMenuLayout();

    updatePauseMenuLayout();

    updateHUDLayout();
}

void Game::init()
{

    applyVideoSettings();
    // подгрузка файлов
    //window.create(sf::VideoMode(900, 600), "history", sf::Style::Close);
    //window.create(sf::VideoMode(1920, 1080), "history", sf::Style::Close);


    if (!backgroundImage.loadFromFile("images/map.png"))
    {
            std::cout << "Failed to load map\n";
    }
    else std::cout << "map ok\n";
    if (!font.loadFromFile("fonts/DejaVuSans.ttf"))
    {
            std::cout << "Failed to load font\n";
    }
    else std::cout << "font ok\n";


    backgroundTexture.loadFromImage(backgroundImage);
    backgroundSprite.setTexture(backgroundTexture);

    borderX = backgroundImage.getSize().x / 2.f;
    borderY = backgroundImage.getSize().y / 2.f;

    view = window.getDefaultView();
    targetCenter = {borderX, borderY};


    cityMgr.loadFromFile("other/cities.csv", font);
    cityMgr.loadConnections("other/connectionsCities.txt");
    armyMgr.init();


    endTurnText.setFont(font);
    centerText.setFont(font);

    initMenu();
    initPauseMenu();
    initEndGameUI();
}


// ==================================================
// ===================== СОБЫТИЯ ====================
// ==================================================
void Game::processEvents()
{
    // если нужны координаты мыши в кадре, брать отсюда
    mouseScreen = sf::Mouse::getPosition(window);
    mouseWorld = window.mapPixelToCoords(mouseScreen, view);

    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();


        switch (globalState)
        {
            case GameOver:

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mouse =
                        window.mapPixelToCoords(mouseScreen, window.getDefaultView());

                    if (endGameButtonRect.contains(mouse))
                    {
                        globalState = Menu;
                    }
                }
                break;

            case Paused:

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mouse =
                        window.mapPixelToCoords(
                            mouseScreen,
                            window.getDefaultView()
                        );

                    // =================================================
                    // CONTINUE
                    // =================================================

                    if (continueButtonRect.contains(mouse))
                    {
                        globalState = Playing;
                    }

                    // =================================================
                    // RESOLUTION
                    // =================================================

                    if (pauseResolutionButtonRect.contains(mouse))
                    {
                        currentResolutionIndex++;

                        if (currentResolutionIndex >= resolutions.size())
                            currentResolutionIndex = 0;

                        settings.resolution =
                            resolutions[currentResolutionIndex];

                        if (!settings.fullscreen)
                        {
                            applyVideoSettings();
                        }
                    }

                    // =================================================
                    // FULLSCREEN
                    // =================================================

                    if (pauseFullscreenButtonRect.contains(mouse))
                    {
                        settings.fullscreen =
                            !settings.fullscreen;

                        applyVideoSettings();
                    }

                    // =================================================
                    // BACK TO MENU
                    // =================================================

                    if (backToMenuButtonRect.contains(mouse))
                    {
                        globalState = Menu;
                    }
                }
                break;

            case Menu:

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mouse =
                        window.mapPixelToCoords(
                            mouseScreen,
                            window.getDefaultView()
                        );

                    // ================= START =================

                    if (startButtonRect.contains(mouse))
                    {
                        globalState = Playing;
                    }

                    // ================= RESOLUTION =================

                    if (resolutionButtonRect.contains(mouse))
                    {
                        currentResolutionIndex++;

                        if (currentResolutionIndex >= resolutions.size())
                            currentResolutionIndex = 0;

                        settings.resolution =
                            resolutions[currentResolutionIndex];

                        applyVideoSettings();
                    }

                    // ================= FULLSCREEN =================

                    if (fullscreenButtonRect.contains(mouse))
                    {
                        settings.fullscreen =
                            !settings.fullscreen;

                        applyVideoSettings();
                    }

                    // ================= EXIT =================

                    if (exitButtonRect.contains(mouse))
                    {
                        window.close();
                    }
                }
                break;

            case Playing:

                // ================== ЗУМ ====================
                if (event.type == sf::Event::MouseWheelScrolled)
                {
                    if (event.mouseWheelScroll.delta > 0)
                        targetZoom *= 0.9f;
                    else
                        targetZoom *= 1.1f;

                    float maxZoomX = (2.f * borderX) / window.getDefaultView().getSize().x * 0.9f;
                    float maxZoomY = (2.f * borderY) / window.getDefaultView().getSize().y * 0.9f;
                    float maxZoom = std::min(maxZoomX, maxZoomY);

                    float minZoom = (2.f * borderX) / window.getDefaultView().getSize().x * 0.3f;

                    if (targetZoom > maxZoom) targetZoom = maxZoom;
                    if (targetZoom < minZoom) targetZoom = minZoom;
                    if (targetZoom < 0.1f) targetZoom = 0.1f;
                }

                // ================= ПЕРЕТАСКИВАНИЕ КАМЕРЫ =================
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left)
                {
                    isDrag = true;
                    beginDragPos = mouseScreen;
                }

                if (event.type == sf::Event::MouseButtonReleased &&
                    event.mouseButton.button == sf::Mouse::Left)
                {
                    isDrag = false;
                }

                // ==================== КЛИКИ ==========================
                if (event.type == sf::Event::MouseButtonPressed)
                {
                    bool clickHandled = false;  // сначала обрабаываем клик по городу (пробуем)

                    // ========================= команды (СОЗДАНИЕ) =============================
                    if (event.mouseButton.button == sf::Mouse::Left &&
                        hoveredCity != nullptr)
                    {
                        City& city = *hoveredCity;
                        city.isSelected = !city.isSelected;

                        //====================== первая точка для команды =======================
                        if (state == Idle && turn!=0)
                        {
                            for (auto &army : armyMgr.armies)
                            {
                                if (army.currentCityId == city.id && army.owner == 0)  //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                {
                                    selectedArmyId = army.id;
                                    state = SelectingTarget;
                                    break;
                                }
                            }
                        }
                        // ======================== вторая точка ============================
                        else if (state == SelectingTarget)
                        {
                            int fromCity = armyMgr.getById(selectedArmyId)->currentCityId;
                            int toCity = city.id;
                            // движение только по правильным соседям, точка from!=to, максимальное кол-во команд = 3
                            if (
                                cityMgr.canMoveBetweenCities(fromCity, toCity, 0) && // 0 - игрок
                                armyMgr.getById(selectedArmyId)->currentCityId != city.id &&
                                commandMgr.getPendingCount() < maxCommands)
                                {
                                    if (!commandMgr.isDuplicateInTurn(selectedArmyId, fromCity, toCity)) // делать ли наслоение
                                    {
                                        int turnsForCommnad = cityMgr.distance(fromCity, toCity);

                                        Command newCommand;
                                        newCommand.owner = armyMgr.getById(selectedArmyId)->owner;
                                        newCommand.armyId = selectedArmyId;
                                        newCommand.fromCity = fromCity;
                                        newCommand.toCity = toCity;
                                        newCommand.units = 0;
                                        newCommand.remainingTurns = turnsForCommnad;
                                        newCommand.totalTurns = turnsForCommnad;
                                        newCommand.battleDot = 1;
                                        newCommand.morale = armyMgr.getById(selectedArmyId)->morale;
                                        newCommand.offset = 0.01f;

                                        commandMgr.commands.push_back(newCommand);
                                        //пересмотр offset для кривой
                                        commandMgr.updateOffsets(fromCity, toCity);
                                        selectedArmyId = -1;
                                        state = Idle;

                                        // вызываем редактиврование
                                        state = EditingCommand;
                                        selectedCommandIndex = (int)commandMgr.commands.size() - 1;
                                        showPopup = true;
                                        inputUnits = commandMgr.commands.back().units;


                                    }
                                }
                        }
                        // ===================== город для отсутпления с фронта ==============
                        else if (state == SelectingRetreat)
                        {
                            Command& cmd = commandMgr.commands[selectedCommandIndex];

                            if (city.id == cmd.fromCity)
                            {
                                //cmd.state = CommandState::InRetreat;
                                commandMgr.retreat(cmd, cmd.units);// вся команда отсупает
                            }
                            state = Idle;
                        }

                        clickHandled = true;
                    }
                    // ========================= coords click (debug) ===============
                    //if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) std::cout << mouseWorld.x << " " << mouseWorld.y << "\n";

                    //========================= КОМАНДЫ (EDIT) ==================
                    // теперь можно попробовать обработать команду
                    if (!clickHandled && state == Idle)
                    {
                        if (event.mouseButton.button == sf::Mouse::Left)
                        {
                            sf::Vector2f mouse = mouseWorld;

                            for (int i = 0; i < (int)commandMgr.commands.size(); i++)
                            {
                                auto &cmd = commandMgr.commands[i];

                                if ((cmd.state == CommandState::InBattle || cmd.state == CommandState::Activated)  && cmd.owner == 0) // отсутпление (отмена)
                                {

                                    if (!commandMgr.isRetreatBlocked(cmd.fromCity, cmd.toCity, 0))// можно ли отступить игроку
                                    {
                                        City* a = cityMgr.findById(cmd.fromCity);
                                        City* b = cityMgr.findById(cmd.toCity);

                                        if (!a || !b) continue;

                                        auto curve = cityMgr.buildCurve(a->position, b->position, cmd.offset);
                                        if (isNearCurve(mouse, curve, 10.f))
                                        {
                                            state = SelectingRetreat;
                                            selectedCommandIndex = i;
                                            break;
                                        }
                                    }
                                }
                                else if (cmd.state == CommandState::Created)// команда этого хода
                                {
                                    City* a = cityMgr.findById(cmd.fromCity);
                                    City* b = cityMgr.findById(cmd.toCity);

                                    if (!a || !b) continue;
                                    // EDIT
                                    auto curve = cityMgr.buildCurve(a->position, b->position, cmd.offset);
                                    if (isNearCurve(mouse, curve, 10.f))
                                    {
                                        state = EditingCommand;
                                        selectedCommandIndex = i;
                                        showPopup = true;
                                        inputUnits = cmd.units;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    // ===========================КОМАНДЫ (УДАЛЕНИЕ) ======================
                    if (event.mouseButton.button == sf::Mouse::Right)
                    {
                        sf::Vector2f mouse = mouseWorld;

                        for (int i = 0; i < (int)commandMgr.commands.size(); i++)
                        {
                            auto &cmd = commandMgr.commands[i];

                            if (!(cmd.state == CommandState::Created)) continue;

                            City* a = cityMgr.findById(cmd.fromCity);
                            City* b = cityMgr.findById(cmd.toCity);

                            if (!a || !b) continue;
                            // DELETE
                            auto curve = cityMgr.buildCurve(a->position, b->position, cmd.offset);
                            if (isNearCurve(mouse, curve, 10.f))
                            {
                                commandMgr.commands.erase(commandMgr.commands.begin() + i);
                                break;
                            }
                        }
                    }
                }
                // ========================= ОСТАЛЬНЫЕ ДЕЙСТВИЯ ===================
                // ===== POPUP  =====
                if (state == EditingCommand)
                {
                    // ===================== TEXT ====================
                    if (showPopup && event.type == sf::Event::TextEntered)
                    {
                        if (event.text.unicode >= '0' && event.text.unicode <= '9')
                            inputUnits = inputUnits * 10 + (event.text.unicode - '0');
                    }
                    // ==================== ПОДТВЕРЖДЕНИЕ РЕДАКТИРОВАНИЯ КОМАНДЫ ==============
                    if (showPopup && event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Enter)
                        {
                            int armyId = commandMgr.commands[selectedCommandIndex].armyId;
                            int maxUnits = commandMgr.getAvailableUnits(armyId, armyMgr) + commandMgr.commands[selectedCommandIndex].units;

                            if (inputUnits > maxUnits) inputUnits = maxUnits;

                            commandMgr.commands[selectedCommandIndex].units = inputUnits;


                            state = Idle;
                            showPopup = false;
                            selectedCommandIndex = -1;
                            inputUnits = 0;
                        }

                        if (event.key.code == sf::Keyboard::BackSpace)
                            inputUnits /= 10;
                    }
                }

                // ====================== СВЯЗИ МЕЖДУ ГОРОДАМИ==================
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C)
                {
                    showConnections = !showConnections;
                }

                if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                ||  (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right))
                {
                    state = Idle;
                    selectedArmyId = -1;
                    showPopup = false;
                    selectedCommandIndex = -1;
                }
                // ======================ЗАКОНЧИТЬ ХОД=====================
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mouse = window.mapPixelToCoords(mouseScreen, window.getDefaultView());
                    if (endTurnButton.contains(mouse) && !waitingForAnimationAppear)
                    {
                        // AI
                        SimulationSystem::makeAITurns(commandMgr, armyMgr, cityMgr, animMgr, turn);    // ход наполеончика

                        waitingForAnimationAppear = true; // чтобы вообще понять что ии сделал, я 3 секунды даю на посмотреть на команды ии
                        waitTimer = 0.f;

                        // end turn перенесен в update
                        selectedArmyId = -1;
                        selectedCommandIndex = -1;
                        showPopup = false;
                        inputUnits = 0;

                        state = Idle;
                    }
                }

                // ================================== ПАУЗА ====================
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape && state ==Idle)
                {
                    globalState = Paused;
                }

                break;
        }
    }
}



// ==================================================
// ===================== UPDATE =====================
// ==================================================
void Game::updateCamera(float dt)
{
    if (isDrag)
    {
        sf::Vector2i cur = mouseScreen;
        sf::Vector2i delta = cur - beginDragPos;

        targetCenter -= sf::Vector2f(delta.x, delta.y) * currentZoom;
        beginDragPos = cur;
    }

    // ===================== ЗУМ КАМЕРЫ =====================
    currentZoom += (targetZoom - currentZoom) * smoothFactor;

    sf::View newView = view;

    float zoom = currentZoom;

    newView.setSize(
        window.getDefaultView().getSize().x * zoom,
        window.getDefaultView().getSize().y * zoom
    );

    sf::Vector2f size = newView.getSize();
    float halfW = size.x / 2.f;
    float halfH = size.y / 2.f;

    if (targetCenter.x - halfW < 0) targetCenter.x = halfW;
    if (targetCenter.y - halfH < 0) targetCenter.y = halfH;
    if (targetCenter.x + halfW > 2 * borderX) targetCenter.x = 2 * borderX - halfW;
    if (targetCenter.y + halfH > 2 * borderY) targetCenter.y = 2 * borderY - halfH;

    sf::Vector2f center = newView.getCenter();
    newView.setCenter(center + (targetCenter - center) * smoothFactor);

    view = newView;
}

void Game::hoverCity()
{
    hoveredCity = nullptr;

    sf::Vector2f mouseWorldPos = mouseWorld;

    for (auto &city : cityMgr.cities)
    {
        float dx = mouseWorldPos.x - city.position.x;
        float dy = mouseWorldPos.y - city.position.y;

        float distSq = dx * dx + dy * dy;
        float radius = 20.f;

        if (distSq < radius * radius)
        {
            hoveredCity = &city;
            break;
        }
    }
}

void Game::updateHud()
{
    // =============================== HUD =========================================
    sf::Vector2u winSize = window.getSize();

    float windowWidth  = static_cast<float>(winSize.x);
    float windowHeight = static_cast<float>(winSize.y);

    // =====================================
    // HUD
    // =====================================

    float hudHeight = windowHeight * 0.14f;
    float hudY = windowHeight - hudHeight;

    hudBackground.setSize({windowWidth, hudHeight});
    hudBackground.setPosition(0.f, hudY);
    hudBackground.setFillColor(sf::Color(30,30,30,220));

    // =====================================
    // END TURN BUTTON
    // =====================================

    float buttonWidth  = windowWidth * 0.12f;
    float buttonHeight = hudHeight * 0.65f;

    float buttonX = windowWidth - buttonWidth - windowWidth * 0.02f;
    float buttonY = hudY + (hudHeight - buttonHeight) / 2.f;

    endTurnButton = sf::FloatRect(buttonX, buttonY, buttonWidth, buttonHeight);

    endTurnShape.setSize({buttonWidth, buttonHeight});
    endTurnShape.setPosition(buttonX, buttonY);
    endTurnShape.setFillColor(sf::Color(120,120,120));

    if (Game::turn==0) endTurnText.setString(L"Начать игру");
    else endTurnText.setString(L"Закончить ход");


    // =====================================
    // BUTTON TEXT
    // =====================================

    endTurnText.setCharacterSize(25);
    sf::FloatRect textBounds =endTurnText.getLocalBounds();

    endTurnText.setPosition(
        buttonX +
        (buttonWidth - textBounds.width) / 2.f,

        buttonY +
        (buttonHeight - textBounds.height) / 2.f - 5.f
    );

    // =====================================
    // CENTER PANEL
    // =====================================

    float centerWidth  = windowWidth * 0.35f;
    float centerHeight = hudHeight * 0.75f;

    float centerX = (windowWidth - centerWidth) / 2.f;
    float centerY = hudY + (hudHeight - centerHeight) / 2.f;

    centerPanel.setSize({centerWidth, centerHeight});
    centerPanel.setPosition(centerX, centerY);
    centerPanel.setFillColor(sf::Color(60,60,60));

    playerTotalUnits = 0;
    enemyTotalUnits = 0;

    for (auto &c : cityMgr.cities)
    {
        auto armies = armyMgr.getAllInCity(c.id);

        for (auto *army : armies)
        {
            if (army->owner == 0)
                playerTotalUnits += army->soldiers;
            else
                enemyTotalUnits += army->soldiers;
        }
    }
    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.state == CommandState::Animating || cmd.state == CommandState::Created) continue;

        if (cmd.owner == 0)
            playerTotalUnits += cmd.units;
        else
            enemyTotalUnits += cmd.units;
    }

    centerText.setCharacterSize(18);
    centerText.setFillColor(sf::Color::White);

    centerText.setString(
    L"Войска игрока: " +
    std::to_wstring(playerTotalUnits) +
    L"\nВражеские войска: " +
    std::to_wstring(enemyTotalUnits)
);

    sf::Vector2f pos = centerPanel.getPosition();
    sf::Vector2f size = centerPanel.getSize();

    centerText.setPosition(pos.x + size.x * 0.1f, pos.y + size.y * 0.2f);
}

void Game::updateMenu()
{
    sf::Vector2u r;

    if (settings.fullscreen)
    {
        sf::VideoMode desktop =
            sf::VideoMode::getDesktopMode();

        r.x = desktop.width;
        r.y = desktop.height;
    }
    else
    {
        r = resolutions[currentResolutionIndex];
    }

    // =====================================================
    // RESOLUTION TEXT
    // =====================================================

    resolutionButton.setString(
        L"Разрешение: " +
        std::to_wstring(r.x) +
        L"x" +
        std::to_wstring(r.y)
    );

    // =====================================================
    // FULLSCREEN TEXT
    // =====================================================

    fullscreenButton.setString(
        settings.fullscreen
        ? L"Режим: Fullscreen"
        : L"Режим: Windowed"
    );

    updateMenuLayout();
}

void Game::updatePauseMenu()
{
    sf::Vector2u r;

    if (settings.fullscreen)
    {
        sf::VideoMode desktop =
            sf::VideoMode::getDesktopMode();

        r.x = desktop.width;
        r.y = desktop.height;
    }
    else
    {
        r = resolutions[currentResolutionIndex];
    }

    // =====================================================
    // RESOLUTION TEXT
    // =====================================================

    pauseResolutionButton.setString(
        L"Разрешение: " +
        std::to_wstring(r.x) +
        L"x" +
        std::to_wstring(r.y)
    );

    // =====================================================
    // FULLSCREEN TEXT
    // =====================================================

    pauseFullscreenButton.setString(
        settings.fullscreen
        ? L"Режим: Fullscreen"
        : L"Режим: Windowed"
    );

    updatePauseMenuLayout();
}

void Game::updateMenuLayout()
{
    sf::Vector2u win = window.getSize();

    float centerX = win.x * 0.5f;

    // =====================================================
    // BACKGROUND
    // =====================================================

    menuBg.setSize(
        sf::Vector2f(win)
    );

    // =====================================================
    // TITLE
    // =====================================================

    sf::FloatRect titleBounds =
        title.getLocalBounds();

    title.setPosition(
        centerX - titleBounds.width / 2.f,
        win.y * 0.18f
    );

    // =====================================================
    // START BUTTON
    // =====================================================

    startButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.38f,
        300.f,
        50.f
    );

    sf::FloatRect startBounds =
        startButton.getLocalBounds();

    startButton.setPosition(
        centerX - startBounds.width / 2.f,
        startButtonRect.top + 10.f
    );

    // =====================================================
    // RESOLUTION BUTTON
    // =====================================================

    resolutionButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.48f,
        300.f,
        50.f
    );

    sf::FloatRect rBounds =
        resolutionButton.getLocalBounds();

    resolutionButton.setPosition(
        centerX - rBounds.width / 2.f,
        resolutionButtonRect.top + 10.f
    );

    // =====================================================
    // FULLSCREEN BUTTON
    // =====================================================

    fullscreenButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.58f,
        300.f,
        50.f
    );

    sf::FloatRect fBounds =
        fullscreenButton.getLocalBounds();

    fullscreenButton.setPosition(
        centerX - fBounds.width / 2.f,
        fullscreenButtonRect.top + 10.f
    );

    // =====================================================
    // EXIT BUTTON
    // =====================================================

    exitButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.68f,
        300.f,
        50.f
    );

    sf::FloatRect exitBounds =
        exitButton.getLocalBounds();

    exitButton.setPosition(
        centerX - exitBounds.width / 2.f,
        exitButtonRect.top + 10.f
    );
}

void Game::updatePauseMenuLayout()
{
    sf::Vector2u win = window.getSize();

    float centerX = win.x * 0.5f;

    // =====================================================
    // BACKGROUND
    // =====================================================

    pauseBg.setSize(
        sf::Vector2f(win)
    );

    // =====================================================
    // TITLE
    // =====================================================

    sf::FloatRect titleBounds =
        pauseTitle.getLocalBounds();

    pauseTitle.setPosition(
        centerX - titleBounds.width / 2.f,
        win.y * 0.18f
    );

    // =====================================================
    // CONTINUE BUTTON
    // =====================================================

    continueButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.38f,
        300.f,
        50.f
    );

    sf::FloatRect continueBounds =
        continueButton.getLocalBounds();

    continueButton.setPosition(
        centerX - continueBounds.width / 2.f,
        continueButtonRect.top + 10.f
    );

    // =====================================================
    // RESOLUTION BUTTON
    // =====================================================

    pauseResolutionButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.48f,
        300.f,
        50.f
    );

    sf::FloatRect rBounds =
        pauseResolutionButton.getLocalBounds();

    pauseResolutionButton.setPosition(
        centerX - rBounds.width / 2.f,
        pauseResolutionButtonRect.top + 10.f
    );

    // =====================================================
    // FULLSCREEN BUTTON
    // =====================================================

    pauseFullscreenButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.58f,
        300.f,
        50.f
    );

    sf::FloatRect fBounds =
        pauseFullscreenButton.getLocalBounds();

    pauseFullscreenButton.setPosition(
        centerX - fBounds.width / 2.f,
        pauseFullscreenButtonRect.top + 10.f
    );

    // =====================================================
    // BACK TO MENU BUTTON
    // =====================================================

    backToMenuButtonRect = sf::FloatRect(
        centerX - 150.f,
        win.y * 0.68f,
        300.f,
        50.f
    );

    sf::FloatRect backBounds =
        backToMenuButton.getLocalBounds();

    backToMenuButton.setPosition(
        centerX - backBounds.width / 2.f,
        backToMenuButtonRect.top + 10.f
    );
}

void Game::updateEndGameLayout(bool isWin)
{
    sf::Vector2u win = window.getSize();

    float cx = win.x * 0.5f;

    endGameBg.setSize(sf::Vector2f(win));

    if (isWin)
        endGameTitle.setString(L"Победа");
    else
        endGameTitle.setString(L"Поражение");

    sf::FloatRect t = endGameTitle.getLocalBounds();

    endGameTitle.setPosition(
        cx - t.width / 2.f,
        win.y * 0.3f
    );

    endGameButtonRect = sf::FloatRect(
        cx - 150.f,
        win.y * 0.55f,
        300.f,
        50.f
    );

    sf::FloatRect b = endGameButton.getLocalBounds();

    endGameButton.setPosition(
        cx - b.width / 2.f,
        endGameButtonRect.top + 10.f
    );
}

void Game::updateHUDLayout()
{
    sf::Vector2u win = window.getSize();

    float windowWidth  = static_cast<float>(win.x);
    float windowHeight = static_cast<float>(win.y);

    // =====================================================
    // HUD BACKGROUND
    // =====================================================

    float hudHeight = windowHeight * 0.14f;
    float hudY = windowHeight - hudHeight;

    hudBackground.setSize({windowWidth, hudHeight});
    hudBackground.setPosition(0.f, hudY);

    // =====================================================
    // CENTER PANEL
    // =====================================================

    float centerWidth  = windowWidth * 0.35f;
    float centerHeight = hudHeight * 0.75f;

    float centerX = (windowWidth - centerWidth) / 2.f;
    float centerY = hudY + (hudHeight - centerHeight) / 2.f;

    centerPanel.setSize({centerWidth, centerHeight});
    centerPanel.setPosition(centerX, centerY);

    // =====================================================
    // CENTER TEXT (если есть)
    // =====================================================

    sf::FloatRect textBounds = centerText.getLocalBounds();

    centerText.setPosition(
        centerX + (centerWidth - textBounds.width) / 2.f,
        centerY + (centerHeight - textBounds.height) / 2.f - 5.f
    );

    // =====================================================
    // END TURN BUTTON
    // =====================================================

    float buttonWidth  = windowWidth * 0.12f;
    float buttonHeight = hudHeight * 0.65f;

    float buttonX = windowWidth - buttonWidth - windowWidth * 0.02f;
    float buttonY = hudY + (hudHeight - buttonHeight) / 2.f;

    endTurnButton = sf::FloatRect(buttonX, buttonY, buttonWidth, buttonHeight);

    endTurnShape.setSize({buttonWidth, buttonHeight});
    endTurnShape.setPosition(buttonX, buttonY);

    // =====================================================
    // END TURN TEXT
    // =====================================================

    sf::FloatRect textRect = endTurnText.getLocalBounds();

    endTurnText.setPosition(
        buttonX + (buttonWidth - textRect.width) / 2.f,
        buttonY + (buttonHeight - textRect.height) / 2.f - 5.f
    );
}

void Game::update(float dt)
{
    switch (globalState)
    {
        case GameOver:
        {
            bool playerWon = playerTotalUnits > enemyTotalUnits;
            updateEndGameLayout(playerWon);
            break;
        }
        case Paused:
            updatePauseMenu();
            break;

        case Menu:
            updateMenu();
            break;

        case Playing:
            // ==================== выйграли ли.==============================
            if (turn!=0)
            {
                if (enemyTotalUnits < 25000 || playerTotalUnits < 25000)
                {
                    globalState = GameOver;
                }
            }



            // ===================== ЕСТЬ ЛИ МЫШЬ НАД ГОРОДОМ =====================
            hoverCity();
            // =====================  КАМЕРА =====================
            updateCamera(dt);
            // ====================== Задержки при анимациях (при появлении и удалении команд =========================
            if (waitingForAnimationAppear)
            {
                waitTimer += dt;

                if (waitTimer >= animationTime)
                {
                    state = Idle;
                    SimulationSystem::endTurn(commandMgr, armyMgr, cityMgr, animMgr, Game::turn++);
                    waitingForAnimationAppear = false;
                }
            }
            // ========================= просчет информации про худ =======
            updateHud();
            // ========================= АНИМАЦИИ??? ====================
            animMgr.update(dt);
            break;
    }
}


// ==================================================
// ===================== RENDER =====================
// ==================================================

void Game::renderCities()
{
    for (auto &c : cityMgr.cities)
    {
        // ===================== цвет =====================
        if (c.owner == 0)
            cityMgr.setTexture(c.id, 0);
        else if (c.owner == 1)
            cityMgr.setTexture(c.id, 1);
        else
            cityMgr.setTexture(c.id, 2);

        // ======= подсветка городов при создании команды ======
        if (selectedArmyId != -1)
        {
            Army* army = armyMgr.getById(selectedArmyId);

            if (army)
            {
                int fromCity = army->currentCityId;

                if (c.id == fromCity) cityMgr.setTexture(c.id, 3);
                else if (cityMgr.canMoveBetweenCities(fromCity, c.id, 0)) cityMgr.setTexture(c.id, 4);
            }
        }

        window.draw(c.marker);
        window.draw(c.label);

        // ===================== армии на городах  =====================
        auto armies = armyMgr.getAllInCity(c.id);

        int playerUnits = 0;
        int enemyUnits = 0;

        float playerMorale = 0;
        float enemyMorale = 0;

        for (auto *army : armies)
        {
            if (army->owner == 0)
            {
                playerUnits += army->soldiers;
                playerMorale = army->morale;
            }
            else
            {
                enemyUnits += army->soldiers;
                enemyMorale = army->morale;
            }
        }

        // справа — игрок 0
        if (playerUnits > 0)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(25);
            text.setFillColor(sf::Color::Black);

            text.setString(std::to_string(playerUnits));
            text.setPosition(c.position.x + 25, c.position.y - 10);

            window.draw(text);

            sf::Text morale;
            morale.setFont(font);
            morale.setCharacterSize(20);
            morale.setFillColor(sf::Color::Black);

            morale.setString(std::to_string(playerMorale));
            morale.setPosition(c.position.x + 25, c.position.y - 25);

            window.draw(morale);
        }

        // слева — враг 1
        if (enemyUnits > 0)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(25);
            text.setFillColor(sf::Color::Yellow);

            text.setString(std::to_string(enemyUnits));
            text.setPosition(c.position.x - 100, c.position.y - 10);

            window.draw(text);


            sf::Text morale;
            morale.setFont(font);
            morale.setCharacterSize(20);
            morale.setFillColor(sf::Color::Black);

            morale.setString(std::to_string(enemyMorale));
            morale.setPosition(c.position.x - 100, c.position.y - 25);

            window.draw(morale);
        }

        //================= подсвтетка связей между городами ===============
        if (showConnections)
        {
            for (int neighborId : c.neighbors)
            {
                City* other = nullptr;

                for (auto &cc : cityMgr.cities)
                {
                    if (cc.id == neighborId)
                    {
                        other = &cc;
                        break;
                    }
                }

                if (!other) continue;

                sf::Vertex line[] =
                {
                    sf::Vertex(c.position, sf::Color::White),
                    sf::Vertex(other->position, sf::Color::White)
                };

                window.draw(line, 2, sf::Lines);
            }
        }
    }
}

void Game::renderCommands()
{
    // ========= отрисовка стрелки при выборе точки для команды ==============
    if (state==SelectingTarget)
    {
        int fromCity = armyMgr.getById(selectedArmyId)->currentCityId;
        City* a = cityMgr.findById(fromCity);


        auto curve = cityMgr.buildCurve(a->position, mouseWorld, 0);
        //if (curve.size() < 2) continue;

        sf::VertexArray line(sf::LineStrip, curve.size());

        for (size_t i = 0; i < curve.size(); i++)
        {
            line[i].position = curve[i];
            line[i].color = sf::Color::White;
        }
        window.draw(line);
    }
    //========== отрисовка стрелки при выборе отступления ================= (как будто бы очень очень запарно доставать всю информацию из индекса
    if (state==SelectingRetreat)                                               // надо будет потом указатель запоминать это будет намного проще)
    {                                                                           // если время будет исправлю пока так сойдет потом
        Command cmd = commandMgr.commands[selectedCommandIndex];
        City* a = cityMgr.findById(cmd.fromCity);
        City* b = cityMgr.findById(cmd.toCity);

        auto curve = cityMgr.buildCurve(a->position, b->position, cmd.offset);

        sf::Vector2f battlePoint = cityMgr.getPointOnCurve(curve, cmd.battleDot);
        curve = cityMgr.buildCurve(battlePoint, mouseWorld, 0);

        sf::VertexArray line(sf::LineStrip, curve.size());

        for (size_t i = 0; i < curve.size(); i++)
        {
            line[i].position = curve[i];
            line[i].color = sf::Color::Green;
        }
        window.draw(line);
    }

    for (auto &cmd : commandMgr.commands)
    {
        sf::Color color;

        if (cmd.state == CommandState::Created) color = sf::Color::White;
        if (cmd.state == CommandState::Activated) color = sf::Color::Yellow;
        if (cmd.state == CommandState::InBattle) color = sf::Color::Red;
        if (cmd.state == CommandState::InRetreat) color = sf::Color::Green;

        if (cmd.owner==1) color = sf::Color::Blue;//команды врага всегда одного цвета (информация о своих командах есть только у нас - логично)

        if (cmd.state == CommandState::Animating) // хз пока, вроде работает, но темка мутная
        {
            //std::cout << "нельзя рисовать (анимация)\n";
            continue;
        }

        City* a = cityMgr.findById(cmd.fromCity);
        City* b = cityMgr.findById(cmd.toCity);

        if (!a || !b) continue;


        // ===================== кривая команды ==================
        auto curve = cityMgr.buildCurve(a->position, b->position, cmd.offset);
        if (curve.size() < 2) continue;

        sf::VertexArray line(sf::LineStrip);

        // --- индексы ---
        size_t battleIndex = std::clamp((size_t)(curve.size() * cmd.battleDot),(size_t)0,curve.size() - 1);

        float moveProgress = 1.0f - (float)cmd.remainingTurns / cmd.totalTurns;
        size_t progressIndex = std::clamp((size_t)(curve.size() * moveProgress),(size_t)0,curve.size() - 1);

        // =====================
        // РЕНДЕР
        // =====================
        if (cmd.state == CommandState::InRetreat)   // я хз может вообще выпилю эту механнику она мне не нравится прям.
        {
            sf::Vector2f beginPos = cityMgr.getPointOnCurve(curve, cmd.battleDot);
             curve = cityMgr.buildCurve(beginPos, b->position, cmd.offset);

            for (size_t i = 0; i < curve.size(); i++) line.append(sf::Vertex(curve[i], color));
        }
        else if (cmd.state == CommandState::InBattle)
        {
            for (size_t i = 0; i < battleIndex; i++) line.append(sf::Vertex(curve[i], color));
        }
        else if (cmd.state == CommandState::Created)
        {
            for (size_t i = 0; i < curve.size(); i++) line.append(sf::Vertex(curve[i], color));
        }
        else if (cmd.state == CommandState::Activated)
        {
            for (size_t i = 0; i < curve.size(); i++) line.append(sf::Vertex(curve[i], color));
        }

        window.draw(line);

        if(cmd.state != CommandState::Created) {
            // ================ стрелка на конце команды ===========
            sf::Vector2f p1, p2;
            if (cmd.state == CommandState::InBattle)
            {
                p2 = curve[battleIndex];
                p1 = curve[battleIndex - 2];
            }
            else
            {
                p2 = curve[progressIndex];
                p1 = curve[progressIndex - 2];
            }

            sf::Vector2f dir = p2 - p1;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len != 0)
            {
                dir /= len;

                sf::Vector2f left(-dir.y, dir.x);
                float size = 10.f;

                sf::Vertex arrow[] =
                {
                    sf::Vertex(p2, sf::Color::Yellow),
                    sf::Vertex(p2 - dir * size + left * size * 0.5f,color),

                    sf::Vertex(p2, sf::Color::Yellow),
                    sf::Vertex(p2 - dir * size - left * size * 0.5f, color),
                };

                window.draw(arrow, 4, sf::Lines);
            }


        }

        // ============== текст юнитов на команде ============

        sf::Vector2f textPos;
        if (cmd.state == CommandState::InBattle)
        {
            textPos = curve[battleIndex];
        }
        else
        {
            textPos = curve[progressIndex];
        }

        sf::Text unitsText;
        unitsText.setFont(font);
        unitsText.setCharacterSize(14);
        unitsText.setFillColor(sf::Color::White);

        // ========== текст прогресса ходов на команде
        int progress = cmd.totalTurns - cmd.remainingTurns;

        std::string label = std::to_string(cmd.units) + " (" + std::to_string(progress) + "/" + std::to_string(cmd.totalTurns) + ") m: " + std::to_string(cmd.morale);

        unitsText.setString(label);
        unitsText.setPosition(textPos + sf::Vector2f(5.f, -5.f));
        window.draw(unitsText);
    }
}

void Game::renderPopUp()
{
    if (showPopup)
    {
        sf::RectangleShape box({250, 150});
        box.setFillColor(sf::Color(0, 0, 0, 200));
        box.setPosition(400, 400);

        window.draw(box);

        sf::Text unitsText;
        unitsText.setFont(font);
        unitsText.setCharacterSize(16);

        int armyId = commandMgr.commands[selectedCommandIndex].armyId;

        int maxUnits =
            commandMgr.getAvailableUnits(armyId, armyMgr) +
            commandMgr.commands[selectedCommandIndex].units;

        unitsText.setString("Units: " + std::to_string(inputUnits));
        unitsText.setPosition(420, 420);

        if (inputUnits > maxUnits)
            unitsText.setFillColor(sf::Color::Red);
        else
            unitsText.setFillColor(sf::Color::White);

        window.draw(unitsText);

        sf::Text maxText;
        maxText.setFont(font);
        maxText.setCharacterSize(16);
        maxText.setFillColor(sf::Color::White);
        maxText.setString("Max: " + std::to_string(maxUnits));
        maxText.setPosition(420, 450);

        window.draw(maxText);
    }
}

void Game::renderFps(float dt)
{
    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(16);
    fpsText.setPosition(20, 20);

    fpsText.setString("fps: " + std::to_string((int)(1/dt)));

    window.draw(fpsText);
}

void Game::renderMenu()
{
    window.setView(window.getDefaultView());

    window.draw(menuBg);

    window.draw(title);

    window.draw(startButton);
    window.draw(resolutionButton);
    window.draw(fullscreenButton);
    window.draw(exitButton);
}

void Game::renderPauseMenu()
{
    window.setView(window.getDefaultView());

    window.draw(pauseBg);

    window.draw(pauseTitle);

    window.draw(continueButton);

    window.draw(pauseResolutionButton);

    window.draw(pauseFullscreenButton);

    window.draw(backToMenuButton);
}

void Game::renderEndGame()
{
    window.setView(window.getDefaultView());

    window.draw(endGameBg);
    window.draw(endGameTitle);
    window.draw(endGameButton);
}

void Game::render(float dt)
{
    //====================================================
    // ================= ВСЕ ЧТО В МИРЕ ==================
    //====================================================
    window.clear(sf::Color(200, 200, 200));
    window.setView(view);
    window.draw(backgroundSprite);

    switch (globalState)
    {

        case GameOver:
            renderEndGame();
            break;

        case Playing:
            // ===================== ГОРОДА  =====================
            renderCities();
            // ===================== КОМАНДЫ ======================
            renderCommands();
            // ===================== АНИМАЦИИ??? ============================
            animMgr.draw(window);

            //====================================================
            //====================== UI ==========================
            //====================================================
            window.setView(window.getDefaultView());

            // ===================== POPUP =====================
            renderPopUp();
            // ================HUD=====================
            window.draw(hudBackground);

            window.draw(centerPanel);
            window.draw(centerText);

            window.draw(endTurnShape);
            window.draw(endTurnText);

            // ======================= FPS ================================
            renderFps(dt);


            break;

        case Menu:
            renderMenu();
            break;

        case Paused:
            // ===================== ГОРОДА  =====================
            renderCities();
            // ===================== КОМАНДЫ ======================
            renderCommands();
            // ===================== АНИМАЦИИ??? ============================
            animMgr.draw(window);

            //====================================================
            //====================== UI ==========================
            //====================================================
            window.setView(window.getDefaultView());

            // ===================== POPUP =====================
            renderPopUp();
            // ================HUD=====================
            window.draw(hudBackground);

            window.draw(centerPanel);
            window.draw(centerText);

            window.draw(endTurnShape);
            window.draw(endTurnText);

            // ======================= FPS ================================
            renderFps(dt);

            renderPauseMenu();
            break;
    }

    window.display();
}





