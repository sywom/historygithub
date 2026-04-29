#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// Структура города
struct City {
    std::string name;
    sf::Vector2f position;
    sf::CircleShape marker;
    bool isSelected = false;
};

// Основная игра
struct Game {
    sf::RenderWindow window;
    sf::View view;
    int screenW = 1920;
    int screenH = 1080;

    float borderX = 0.f;
    float borderY = 0.f;

    bool isDrag = false;
    sf::Vector2i beginDragPos;
    sf::Vector2f targetCenter;
    float targetZoom = 1.f;
    float currentZoom = 1.f;

    const float smoothFactor = 0.2f;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    std::vector<City> cities;

    // Инициализация игры
    void init() {
        window.create(sf::VideoMode(screenW, screenH), "history", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);

        // Загружаем фон
        if (!backgroundTexture.loadFromFile("images/gamemap_resized.png")) {
            std::cout << "Failed to load background" << std::endl;
        }
        backgroundTexture.setSmooth(false);
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(0,0);

        borderX = backgroundTexture.getSize().x / 2.f;
        borderY = backgroundTexture.getSize().y / 2.f;

        view = window.getView();
        targetCenter = sf::Vector2f(borderX, borderY);
        view.setCenter(targetCenter);

        initCities();
    }

    // Создание городов - сделано ии
    void initCities() {
        cities.push_back({"City A", sf::Vector2f(500.f, 400.f), sf::CircleShape(20.f)});
        cities.push_back({"City B", sf::Vector2f(1200.f, 300.f), sf::CircleShape(20.f)});
        cities.push_back({"City C", sf::Vector2f(800.f, 800.f), sf::CircleShape(20.f)});

        for (auto &city : cities) {
            city.marker.setOrigin(city.marker.getRadius(), city.marker.getRadius());
            city.marker.setPosition(city.position);
            city.marker.setFillColor(sf::Color::Red);
        }
    }

    // Обработка событий
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

    // Обновление логики
    void update() {
        // Перетаскивание камеры
        if (isDrag) {
            sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
            sf::Vector2f offset = window.mapPixelToCoords(currentMousePos) - window.mapPixelToCoords(beginDragPos);
            targetCenter -= offset;
            beginDragPos = currentMousePos;
        }

        currentZoom += (targetZoom - currentZoom) * smoothFactor;

        sf::View zoomedView = view;
        zoomedView.setSize(window.getDefaultView().getSize() * currentZoom);

        sf::Vector2f size = zoomedView.getSize();
        float halfW = size.x / 2.f;
        float halfH = size.y / 2.f;

        if (targetCenter.x - halfW < 0) targetCenter.x = halfW;
        if (targetCenter.x + halfW > 2 * borderX) targetCenter.x = 2 * borderX - halfW;
        if (targetCenter.y - halfH < 0) targetCenter.y = halfH;
        if (targetCenter.y + halfH > 2 * borderY) targetCenter.y = 2 * borderY - halfH;

        sf::Vector2f currentCenter = zoomedView.getCenter();
        sf::Vector2f newCenter = currentCenter + (targetCenter - currentCenter) * smoothFactor;
        zoomedView.setCenter(newCenter);

        view = zoomedView;
    }

    // Отрисовка
    void render() {
        window.clear(sf::Color(209, 209, 209));
        window.setView(view);
        window.draw(backgroundSprite);

        for (auto &city : cities) { // ии
            window.draw(city.marker);
        }

        window.display();
    }

    // Игровой цикл
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }
};

int main() {
    Game game;
    game.init();
    game.run();
    return 0;
}
