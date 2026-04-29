#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <fstream>
#include <sstream>


// какая то хрень чтобы проверить как далеко клик от линии команды
bool isNearLine(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b)
{
    float length = std::sqrt((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
    float t = ((p.x-a.x)*(b.x-a.x) + (p.y-a.y)*(b.y-a.y)) / (length*length);

    t = std::max(0.f, std::min(1.f, t));

    sf::Vector2f proj = {
        a.x + t*(b.x-a.x),
        a.y + t*(b.y-a.y)
    };

    float dist = std::sqrt((p.x-proj.x)*(p.x-proj.x) + (p.y-proj.y)*(p.y-proj.y));
    std::cout << "dist: " << dist << "\n";

    return dist < 15.f; // толщина клика
}





// ===================== Color Hash ===================== (хз ваше че это - ии (для отделения стран друг от друга))
struct ColorHash {
    size_t operator()(const sf::Color& c) const {
        return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;
    }
};

struct ColorEqual {
    bool operator()(const sf::Color& a, const sf::Color& b) const {
        return a.r == b.r && a.g == b.g &&
               a.b == b.b && a.a == b.a;
    }
};
// ===================== City =====================
struct City {
    int id;
    int owner;

    std::string name;
    sf::Text label;
    sf::Vector2f position;
    sf::CircleShape marker;

    bool isSelected = false;
};
// ===================== Army =====================
struct Army {
    int owner;
    int soldiers;
    int currentCityId;
};
// ===================== First Command - move ======
struct Command {
    int armyIndex;
    int fromCity;
    int toCity;
    int units;
};
// ===================== Game =====================
struct Game {
    // window
    sf::RenderWindow window;
    sf::View view;

    int screenW = 1080;
    int screenH = 720;

    float borderX = 0.f;
    float borderY = 0.f;
    // mouse
    bool isDrag = false;
    sf::Vector2i beginDragPos;

    sf::Vector2f targetCenter;
    float targetZoom = 1.f;
    float currentZoom = 1.f;

    const float smoothFactor = 0.2f;

    // map
    sf::Image backgroundImage;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    std::vector<City> cities;

    std::vector<std::vector<int>> provinceIDMap;
    std::unordered_map<sf::Color, int, ColorHash, ColorEqual> colorToID;

    int hoveredProvince = -1;

    sf::Font font;
    // armies
    std::vector<Army> armies;

    std::vector<Command> playerCommands;
    std::vector<Command> franceCommands;

    int selectedArmy = -1;
    int selectedCommandIndex = -1;

    int maxCommands = 3; // по 3 действия за ход
    int inputUnits = 0;

    // всплываюшее окошко
    bool showPopup = false;
    // города
    City* hoveredCity = NULL;

    // ===================== INIT =====================
    void init() {
        window.create(sf::VideoMode(screenW, screenH), "history", sf::Style::Close);
        window.setFramerateLimit(76);

        if (!backgroundImage.loadFromFile("images/test1.png")) {
            std::cout << "Failed to load map\n";
        }
        else std::cout << "map ok\n";
        if (!font.loadFromFile("fonts/DejaVuSans.ttf")) {
            std::cout << "Failed to load font\n";
        }
        else std::cout << "font ok\n";

        backgroundTexture.loadFromImage(backgroundImage);
        backgroundSprite.setTexture(backgroundTexture);

        borderX = backgroundImage.getSize().x / 2.f;
        borderY = backgroundImage.getSize().y / 2.f;

        view = window.getDefaultView();
        targetCenter = {borderX, borderY};
        view.setCenter(targetCenter);

        loadCitiesFromFile("other/cities.csv");
        //buildProvinceMap();
        initArmies();
    }

    // ===================== Cities =====================
    void loadCitiesFromFile(const std::string& filename)
    {
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cout << "Failed to open cities file\n";
            return;
        }

        std::string line;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);

            std::string idStr, name, xStr, yStr, ownerStr;

            std::getline(ss, idStr, ';');
            std::getline(ss, name, ';');
            std::getline(ss, xStr, ';');
            std::getline(ss, yStr, ';');
            std::getline(ss, ownerStr, ';');

            float x = std::stof(xStr);
            float y = std::stof(yStr);

            City c;
            c.id = std::stoi(idStr);
            c.name = name;
            c.position = {x, y};
            c.owner = std::stoi(ownerStr);
            c.marker = sf::CircleShape(20.f);

            c.marker.setOrigin(20.f, 20.f);
            c.marker.setPosition(c.position);


            if (c.owner == 0) c.marker.setFillColor(sf::Color::Green);
            else c.marker.setFillColor(sf::Color::Red);


            c.label.setFont(font);
            c.label.setCharacterSize(18);
            c.label.setFillColor(sf::Color::Black);
            c.label.setString(sf::String::fromUtf8(c.name.begin(), c.name.end()));

            // центрируем текст
            sf::FloatRect bounds = c.label.getLocalBounds();
            c.label.setOrigin(bounds.width / 2, bounds.height / 2);

            // чуть выше города
            c.label.setPosition(c.position.x, c.position.y - 25.f);

            cities.push_back(c);
        }
    }

    // ===================== PROVINCES =====================
    void buildProvinceMap() {
        std::cout << "Building provinces...\n";

        int w = backgroundImage.getSize().x;
        int h = backgroundImage.getSize().y;

        provinceIDMap.assign(w, std::vector<int>(h, -1));

        const sf::Uint8* pixels = backgroundImage.getPixelsPtr();

        int nextID = 0;

        for (int x = 0; x < w; x++) {
            for (int y = 0; y < h; y++) {

                int index = (x + y * w) * 4;

                sf::Color c(
                    pixels[index],
                    pixels[index + 1],
                    pixels[index + 2],
                    pixels[index + 3]
                );

                if (!colorToID.count(c))
                    colorToID[c] = nextID++;

                provinceIDMap[x][y] = colorToID[c];
            }
        }

        std::cout << "Provinces found: " << nextID << std::endl;
    }
    // ===================== ARMIES ===================== (потом лучше в файл как с городами)
    void initArmies()
    {
        // игрок
        armies.push_back({0, 100, 15}); // owner, soldiers, cityID
        armies.push_back({0,100,2});
        // франция
        armies.push_back({1, 100, 0});
    }
    // ===================== EVENTS =====================
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Зум колесиком мыши
            if (event.type == sf::Event::MouseWheelScrolled) {
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

            // Перетаскивание камеры
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                isDrag = true;
                beginDragPos = sf::Mouse::getPosition(window);
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                isDrag = false;
            }

            // работа с командами (создание и управление)
            if (event.type == sf::Event::MouseButtonPressed)
            {
                bool clickHandled = false; // приоритет на города

                sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                // сначала обрабатываем клик на город
                if (event.mouseButton.button == sf::Mouse::Left && hoveredCity != NULL)
                {
                    City& city = *hoveredCity;

                    city.isSelected = !city.isSelected;
                    std::cout << "Clicked on city: " << city.name << " at (" << city.position.x << ", " << city.position.y << ")\n";

                    // вторая точка для выбора команды (стоит в коде раньше, чтобы сначала сработал блок с выбором)
                    if (selectedArmy != -1 && armies[selectedArmy].currentCityId != city.id && playerCommands.size() < maxCommands)
                    {
                        // создание команды
                        playerCommands.push_back({selectedArmy, armies[selectedArmy].currentCityId, city.id, 0});
                        selectedArmy = -1; // "обнуляем значение для след. команды"
                        std::cout << "Command: move to " << city.name << "\n";
                    }

                    // первая точка для команды
                    for (int i = 0; i < armies.size(); i++)
                    {
                        if (armies[i].currentCityId == city.id && armies[i].owner == 0)
                        {
                            selectedArmy = i;
                            std::cout << "Selected army in " << city.name << " sol.: " << armies[selectedArmy].soldiers << "\n";
                        }
                    }
                    clickHandled = true; // клик на город обработан
                }

                // если клик по городу не обработан, значит клика по городу не было - пробуем обработать команду
                if (!clickHandled)
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {

                        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                        for (int i = 0; i < playerCommands.size(); i++)
                        {
                            auto &cmd = playerCommands[i];

                            sf::Vector2f a = cities[cmd.fromCity].position;
                            sf::Vector2f b = cities[cmd.toCity].position;

                            if (isNearLine(mouse, a, b))
                            {
                                selectedCommandIndex = i;
                                std::cout << "find command. to : " << playerCommands[selectedCommandIndex].toCity << " units: " << playerCommands[selectedCommandIndex].units << "\n";
                                showPopup = true;
                                inputUnits = playerCommands[selectedCommandIndex].units;
                                break;
                            }
                        }
                    }
                    if (event.mouseButton.button == sf::Mouse::Right)
                    {

                        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                        for (int i = 0; i < playerCommands.size(); i++)
                        {
                            auto &cmd = playerCommands[i];

                            sf::Vector2f a = cities[cmd.fromCity].position;
                            sf::Vector2f b = cities[cmd.toCity].position;

                            if (isNearLine(mouse, a, b))
                            {
                                selectedCommandIndex = i;
                                std::cout << "find and delete command: " << playerCommands[selectedCommandIndex].armyIndex << "\n";
                                playerCommands.erase(playerCommands.begin() + selectedCommandIndex);
                                break;
                            }
                        }
                    }
                }
            }




            // работа с окном для ввода
            // ввод
            if (showPopup && event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode >= '0' && event.text.unicode <= '9')
                {
                    inputUnits = inputUnits * 10 + (event.text.unicode - '0');
                }
            }
            if (showPopup && event.type == sf::Event::KeyPressed)
            {
                // сохранения введенного числа и выход
                if (event.key.code == sf::Keyboard::Enter)
                {
                    playerCommands[selectedCommandIndex].units = inputUnits;
                    showPopup = false;
                    selectedCommandIndex = -1;
                    inputUnits = 0;
                }
                // стереть число
                if (event.key.code == sf::Keyboard::BackSpace)
                {
                    inputUnits /= 10;
                    // убираем последнюю цифру (123 → 12 → 1 → 0)
                }
            }
        }
    }

    // ===================== UPDATE =====================
    void update() {
        // hovering cities
        hoveredCity = NULL;
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (auto &city : cities)
        {
            float dx = mouseWorldPos.x - city.position.x;
            float dy = mouseWorldPos.y - city.position.y;
            float distSq = dx * dx + dy * dy;
            float radius = city.marker.getRadius();

            if (distSq < radius * radius)
            {
                hoveredCity = &city;
                break;
                //city.marker.setFillColor(sf::Color::Green);
            }
        }


        // camera move
        if (isDrag) {
            sf::Vector2i cur = sf::Mouse::getPosition(window);

            sf::Vector2f offset =
                window.mapPixelToCoords(cur) -
                window.mapPixelToCoords(beginDragPos);

            targetCenter -= offset;
            beginDragPos = cur;
        }
        // camera zoom
        currentZoom += (targetZoom - currentZoom) * smoothFactor;
        sf::View newView = view;
        float zoom = currentZoom;
        newView.setSize(window.getDefaultView().getSize().x * zoom, window.getDefaultView().getSize().y * zoom);

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

    // ===================== RENDER =====================
    void render() {
        window.clear(sf::Color(200, 200, 200));

        // camera move
        window.setView(view);

        // world render
        window.draw(backgroundSprite);
        for (auto &c : cities)
        {
            window.draw(c.marker);
            window.draw(c.label);
        }
        for (auto &cmd : playerCommands)
        {
            sf::Vector2f a = cities[cmd.fromCity].position;
            sf::Vector2f b = cities[cmd.toCity].position;

            sf::Vertex line[] =
            {
                sf::Vertex(a, sf::Color::Yellow),
                sf::Vertex(b, sf::Color::Yellow)
            };

            window.draw(line, 2, sf::Lines);
            // центр линии
            sf::Vector2f mid;
            mid.x = (a.x + b.x) / 2.f;
            mid.y = (a.y + b.y) / 2.f;

            sf::Text unitsText;
            unitsText.setFont(font);
            unitsText.setCharacterSize(14);
            unitsText.setFillColor(sf::Color::White);

            unitsText.setString(std::to_string(cmd.units));
            // показываем сколько юнитов отправлено по этой стрелке

            unitsText.setPosition(mid);
            window.draw(unitsText);
        }

        //default view
        //window.setView(window.getDefaultView()); - здесь баг :(   (исправляется разделением мыши на мировую и окна) возможно полное изменение структуры кода бля
        // ui render
        if (showPopup)
        {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);

            sf::RectangleShape box({250, 150});
            box.setFillColor(sf::Color(0, 0, 0, 200));

            // небольшой отступ, чтобы не перекрывать курсор
            box.setPosition(mousePixel.x + 10, mousePixel.y + 10);

            window.draw(box);

            sf::Text t;
            t.setFont(font);
            t.setCharacterSize(16);
            t.setFillColor(sf::Color::White);
            t.setString("Units: " + std::to_string(inputUnits));

            t.setPosition(mousePixel.x + 20, mousePixel.y + 20);

            window.draw(t);
        }

        window.display();
    }

    // ===================== LOOP =====================
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }
};

// ===================== MAIN =====================
int main() {
    Game game;
    game.init();
    game.run();
    return 0;
}
