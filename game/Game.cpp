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
        render();
    }
}

// ==================================================
// ===================== INIT =====================
// ==================================================
void Game::init()
{
    // подгрузка файлов
    window.create(sf::VideoMode(1280, 720), "history", sf::Style::Close);
    window.setFramerateLimit(76);


    if (!backgroundImage.loadFromFile("images/test1.png"))
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

    endTurnButton = sf::FloatRect(900.f, 20.f, 150.f, 40.f);
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

            if (targetZoom > maxZoom) targetZoom = maxZoom;
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
                if (state == Idle)
                {
                    for (auto &army : armyMgr.armies)
                    {
                        if (army.currentCityId == city.id) // && army.owner == 0 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
                    if (cityMgr.canMoveBetweenCities(fromCity, toCity) &&
                        armyMgr.getById(selectedArmyId)->currentCityId != city.id &&
                        commandMgr.getPendingCount() < maxCommands)
                        {
                            if (!commandMgr.isDuplicateInTurn(selectedArmyId, fromCity, toCity)) // делать ли наслоение
                            {
                                int turnsForCommnad = cityMgr.distance(fromCity, toCity);
                                // команда (owner, army, from, to, units, remaning turns, all turns, active, offset, in battle, battle turn)
                                commandMgr.commands.push_back({armyMgr.getById(selectedArmyId)->owner, selectedArmyId, fromCity, toCity, 0,turnsForCommnad, turnsForCommnad});
                                //пересмотр offset для кривой
                                commandMgr.updateOffsets(fromCity, toCity);
                                selectedArmyId = -1;
                                state = Idle;
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
                        commandMgr.retreat(cmd, cmd.units/4);// вся команда отсупает
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

                        if (cmd.state == CommandState::InBattle) // редактируем команду (!оступления!)
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

        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape)
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
            if (endTurnButton.contains(mouse))
            {
                SimulationSystem::makeAITurns(commandMgr, armyMgr, cityMgr, animMgr);    // ход наполеончика

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


        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W)
        {
            animMgr.spawnWin({600.f, 400.f});
        }
    }
}

    // ===================== UPDATE =====================
void Game::update(float dt)
{
    // ===================== ЕСТЬ ЛИ МЫШЬ НАД ГОРОДОМ =====================
    hoveredCity = nullptr;

    sf::Vector2f mouseWorldPos = mouseWorld;

    for (auto &city : cityMgr.cities)
    {
        float dx = mouseWorldPos.x - city.position.x;
        float dy = mouseWorldPos.y - city.position.y;

        float distSq = dx * dx + dy * dy;
        float radius = city.marker.getRadius();

        if (distSq < radius * radius)
        {
            hoveredCity = &city;
            break;
        }
    }

    // ===================== ДВИЖЕНИЕ КАМЕРЫ =====================
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
    // ====================== Задержки при анимациях (при появлении и удалении команд =========================
    if (waitingForAnimationAppear)
    {
        waitTimer += dt;

        if (waitTimer >= animationTime)
        {
            SimulationSystem::endTurn(commandMgr, armyMgr, cityMgr, animMgr);
            waitingForAnimationAppear = false;
        }
    }

    // ========================= АНИМАЦИИ??? ================
    animMgr.update(dt);
}
    // ==================================================
    // ===================== RENDER =====================
    // ==================================================
void Game::render()
{
    //====================================================
    // ================= ВСЕ ЧТО В МИРЕ ==================
    //====================================================
    window.clear(sf::Color(200, 200, 200));
    window.setView(view);
    window.draw(backgroundSprite);

    // ===================================================
    // ===================== ГОРОДА  =====================
    for (auto &c : cityMgr.cities)
    {
        // ===================== цвет =====================
        if (c.owner == 0)
            c.marker.setFillColor(sf::Color::Green);
        else if (c.owner == 1)
            c.marker.setFillColor(sf::Color::Red);
        else c.marker.setFillColor(sf::Color::Yellow);

        // ======= подсветка городов при создании команды ======
        if (selectedArmyId != -1)
        {
            Army* army = armyMgr.getById(selectedArmyId);

            if (army)
            {
                int fromCity = army->currentCityId;

                if (c.id == fromCity) c.marker.setFillColor(sf::Color::Blue);
                else if (cityMgr.canMoveBetweenCities(fromCity, c.id)) c.marker.setFillColor(sf::Color::Cyan);
            }
        }

        window.draw(c.marker);
        window.draw(c.label);

        // ===================== армии на городах  =====================
        auto armies = armyMgr.getAllInCity(c.id);

        int playerUnits = 0;
        int enemyUnits = 0;

        for (auto *army : armies)
        {
            if (army->owner == 0)
                playerUnits += army->soldiers;
            else
                enemyUnits += army->soldiers;
        }

        // справа — игрок
        if (playerUnits > 0)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(25);
            text.setFillColor(sf::Color::Black);

            text.setString(std::to_string(playerUnits));
            text.setPosition(c.position.x + 25, c.position.y - 10);

            window.draw(text);
        }

        // слева — враг
        if (enemyUnits > 0)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(25);
            text.setFillColor(sf::Color::Yellow);

            text.setString(std::to_string(enemyUnits));
            text.setPosition(c.position.x - 100, c.position.y - 10);

            window.draw(text);
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

    //=====================================================
    // ===================== КОМАНДЫ ======================

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

        auto commandCurve = cityMgr.buildCurve(a->position, b->position, cmd.offset);

        sf::Vector2f battlePoint = cityMgr.getPointOnCurve(commandCurve, cmd.battleDot);

        auto curve = cityMgr.buildCurve(battlePoint, mouseWorld, 0);

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

        if (cmd.state == CommandState::InRetreat)
        {
            sf::Vector2f battlePoint = cityMgr.getPointOnCurve(curve, cmd.battleDot);
            curve = cityMgr.buildCurve(battlePoint, b->position, cmd.offset);
        }

        sf::VertexArray line(sf::LineStrip, curve.size());

        for (size_t i = 0; i < curve.size(); i++)
        {
            line[i].position = curve[i];
            line[i].color = color;
        }
        window.draw(line);


        // ================ стрелка на конце команды ===========
        sf::Vector2f p2 = curve.back();
        sf::Vector2f p1 = curve[curve.size() - 2];

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

        // ============== текст юнитов на команде ============
        sf::Vector2f textPos = curve[curve.size() / 2];

        sf::Text unitsText;
        unitsText.setFont(font);
        unitsText.setCharacterSize(14);
        unitsText.setFillColor(sf::Color::White);

        // ========== текст прогресса ходов на команде
        int progress = cmd.totalTurns - cmd.remainingTurns;

        std::string label =
            std::to_string(cmd.units) +
            " (" +
            std::to_string(progress) +
            "/" +
            std::to_string(cmd.totalTurns) +
            ")";

        unitsText.setString(label);
        unitsText.setPosition(textPos + sf::Vector2f(5.f, -5.f));
        window.draw(unitsText);
    }

    // ===================== АНИМАЦИИ??? ============================
    animMgr.draw(window);


    //====================================================
    //====================== UI ==========================
    //====================================================
    window.setView(window.getDefaultView());

    // ===================== POPUP =====================
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
    // ===================== END TURN BUTTON ==================
    sf::RectangleShape btn({150, 40});
    btn.setPosition(900, 20);
    btn.setFillColor(sf::Color(120,120,120));

    window.draw(btn);

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    text.setString("End Turn");
    text.setPosition(920, 30);
    window.draw(text);

    window.display();
}
