#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>

// ===================== City =====================
struct City {
    std::string name;
    sf::Vector2f position;
    sf::CircleShape marker;
    bool isSelected = false;
};

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

// ===================== Game =====================
struct Game {
    sf::RenderWindow window;
    sf::View view;

    int screenW = 1080;
    int screenH = 720;

    float borderX = 0.f;
    float borderY = 0.f;

    bool isDrag = false;
    sf::Vector2i beginDragPos;

    sf::Vector2f targetCenter;
    float targetZoom = 1.f;
    float currentZoom = 1.f;

    const float smoothFactor = 0.2f;

    // MAP
    sf::Image backgroundImage;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    std::vector<City> cities;

    std::vector<std::vector<int>> provinceIDMap;
    std::unordered_map<sf::Color, int, ColorHash, ColorEqual> colorToID;

    int hoveredProvince = -1;

    // ===================== INIT =====================
    void init() {
        window.create(sf::VideoMode(screenW, screenH), "history", sf::Style::Close);
        window.setFramerateLimit(76);

        if (!backgroundImage.loadFromFile("images/test.png")) {
            std::cout << "Failed to load map\n";
        }

        backgroundTexture.loadFromImage(backgroundImage);
        backgroundSprite.setTexture(backgroundTexture);

        borderX = backgroundImage.getSize().x / 2.f;
        borderY = backgroundImage.getSize().y / 2.f;

        view = window.getDefaultView();
        targetCenter = {borderX, borderY};
        view.setCenter(targetCenter);

        initCities();
        //buildProvinceMap();
    }

    // ===================== Cities =====================
    void initCities() {
        // пруссия
        cities.push_back({"Мемель", {568.f, 613.f}, sf::CircleShape(20.f)});
        cities.push_back({"Кёнигсберг", {357.f, 1033.f}, sf::CircleShape(20.f)});
        cities.push_back({"Тильзит", {726.f, 940.f}, sf::CircleShape(20.f)});
        // франция
        cities.push_back({"Сувалки", {945.f, 1345.f}, sf::CircleShape(20.f)});
        cities.push_back({"Ковно", {1197.f, 1013.f}, sf::CircleShape(20.f)});
        cities.push_back({"Люблин", {764.f, 2438.f}, sf::CircleShape(20.f)});
        cities.push_back({"Варшава", {418.f, 2009.f}, sf::CircleShape(20.f)});
        cities.push_back({"Брест-Литовск", {1036.f, 2153.f}, sf::CircleShape(20.f)});
        // россия
        cities.push_back({"Гродно", {1135.f, 1518.f}, sf::CircleShape(20.f)});
        cities.push_back({"Белосток", {975.f, 1721.f}, sf::CircleShape(20.f)});
        cities.push_back({"Вильно", {1502.f, 1136.f}, sf::CircleShape(20.f)});
        cities.push_back({"Ольшаны", {1655.f, 1312.f}, sf::CircleShape(20.f)});
        cities.push_back({"Минск", {2007.f, 1467.f}, sf::CircleShape(20.f)});
        cities.push_back({"Мир", {1706.f, 1594.f}, sf::CircleShape(20.f)});
        cities.push_back({"Слоним", {1467.f, 1763.f}, sf::CircleShape(20.f)});
        cities.push_back({"Рига", {1295.f, 220.f}, sf::CircleShape(20.f)});
        cities.push_back({"Несвиж", {1809.f, 1703.f}, sf::CircleShape(20.f)});
        cities.push_back({"Пинск", {1619.f, 2168.f}, sf::CircleShape(20.f)});
        cities.push_back({"Луцк", {1418.f, 2693.f}, sf::CircleShape(20.f)});
        cities.push_back({"Двинск", {1789.f, 672.f}, sf::CircleShape(20.f)});
        cities.push_back({"Дрисса", {2127.f, 727.f}, sf::CircleShape(20.f)});
        cities.push_back({"Клястицы", {2251.f, 684.f}, sf::CircleShape(20.f)});
        cities.push_back({"Полоцк", {2365.f, 836.f}, sf::CircleShape(20.f)});
        cities.push_back({"Витебск", {2603.f, 938.f}, sf::CircleShape(20.f)});
        cities.push_back({"Орша", {2646.f, 1214.f}, sf::CircleShape(20.f)});
        cities.push_back({"Могилёв", {2632.f, 1459.f}, sf::CircleShape(20.f)});
        cities.push_back({"Борисов", {2232.f, 1334.f}, sf::CircleShape(20.f)});
        cities.push_back({"Салтановка", {2613.f, 1522.f}, sf::CircleShape(20.f)});
        cities.push_back({"Бобруйск", {2380.f, 1754.f}, sf::CircleShape(20.f)});
        cities.push_back({"Смоленск", {3033.f, 1076.f}, sf::CircleShape(20.f)});
        cities.push_back({"Брянск", {3565.f, 1669.f}, sf::CircleShape(20.f)});
        cities.push_back({"Дорогобуж", {3322.f, 1031.f}, sf::CircleShape(20.f)});
        cities.push_back({"Вязьма", {3507.f, 912.f}, sf::CircleShape(20.f)});
        cities.push_back({"Царево-Займище", {3605.f, 819.f}, sf::CircleShape(20.f)});
        cities.push_back({"Гжатск", {3668.f, 768.f}, sf::CircleShape(20.f)});
        cities.push_back({"Бородино", {3828.f, 757.f}, sf::CircleShape(20.f)});
        cities.push_back({"Можайск", {3909.f, 744.f}, sf::CircleShape(20.f)});
        cities.push_back({"Фили", {4176.f, 618.f}, sf::CircleShape(20.f)});
        cities.push_back({"Москва", {4248.f, 624.f}, sf::CircleShape(20.f)});
        cities.push_back({"Тарутино", {4098.f, 884.f}, sf::CircleShape(20.f)});
        cities.push_back({"Малоярославец", {4015.f, 929.f}, sf::CircleShape(20.f)});
        cities.push_back({"Серпухов", {4213.f, 951.f}, sf::CircleShape(20.f)});
        cities.push_back({"Калуга", {4004.f, 1132.f}, sf::CircleShape(20.f)});
        cities.push_back({"Тула", {4314.f, 1225.f}, sf::CircleShape(20.f)});
        cities.push_back({"Коломна", {4513.f, 849.f}, sf::CircleShape(20.f)});
        cities.push_back({"Владимир", {4812.f, 403.f}, sf::CircleShape(20.f)});
        cities.push_back({"Дмитров", {4173.f, 375.f}, sf::CircleShape(20.f)});
        cities.push_back({"Клин", {4029.f, 411.f}, sf::CircleShape(20.f)});
        cities.push_back({"Тверь", {3800.f, 237.f}, sf::CircleShape(20.f)});


        // австрийская империя
        cities.push_back({"Львов", {1081.f, 3010.f}, sf::CircleShape(20.f)});



        for (auto &c : cities) {
            c.marker.setOrigin(20.f, 20.f);
            c.marker.setPosition(c.position);
            c.marker.setFillColor(sf::Color::Red);
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
        }

        // просто клик для отображения координат
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i pixel = sf::Mouse::getPosition(window);
            sf::Vector2f world = window.mapPixelToCoords(pixel);

            int x = (int)world.x;
            int y = (int)world.y;

            std::cout << "====================\n";
            std::cout << "CLICK:\n";
            //std::cout << "Pixel: (" << pixel.x << ", " << pixel.y << ")\n";
            std::cout << "World: (" << world.x << ", " << world.y << ")\n";

            if ( (!provinceIDMap.empty()) && x >= 0 && y >= 0 &&            // проверяем массив IDmap, если не заполняем провинции
                x < (int)backgroundImage.getSize().x &&
                y < (int)backgroundImage.getSize().y)
            {
                int id = provinceIDMap[x][y];
                std::cout << "Province ID: " << id << "\n";
            }

            std::cout << "====================\n";
        }



        // Наведение и клик на города - сделано ии - разобрать что к чему
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (auto &city : cities)
        {

            float dist = std::sqrt(std::pow(mouseWorldPos.x - city.position.x, 2) + std::pow(mouseWorldPos.y - city.position.y, 2));

            // Наведение
            if (dist < city.marker.getRadius())
            {
                city.marker.setFillColor(sf::Color::Green);
                // Клик обрабатываем только при событии MouseButtonPressed
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    city.isSelected = !city.isSelected;
                    std::cout << "Clicked on city: " << city.name << " at (" << city.position.x << ", " << city.position.y << ")\n";
                }
            }
            else
            {
                city.marker.setFillColor(city.isSelected ? sf::Color::Blue : sf::Color::Red);
            }
        }
    }

    // ===================== UPDATE =====================
    void update() {
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

        window.setView(view);

        window.draw(backgroundSprite);

        for (auto &c : cities)
            window.draw(c.marker);

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
