#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;

int main()
{
    int screenH = 720, screenW = 1280;
    // для перемещения камеры
    bool isDrag = false; // состояние (есть перемещение или нет)
    sf::Vector2i beginDragPos; // vector2i - точка с двумя координатами x y


    sf::RenderWindow window(sf::VideoMode(screenW, screenH), "history", sf::Style::Titlebar | sf::Style::Close); // само окно
    window.setFramerateLimit(60);   // fps
    sf::View view = window.getView(); // первый "вид"
    sf::Clock clock;       // внутреннее время

    int border = 5000; // границы "поля" (половина т.е. границы - куб 10000x10000)

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            // события

            if (event.type == sf::Event::Closed)
                window.close();

            // Обработка прокрутки колесика мыши для изменения масштаба
            if (event.type == sf::Event::MouseWheelMoved)
            {
                // Масштабирование: при прокрутке колесика меняем zoomFactor
                if (event.mouseWheel.delta > 0) {
                    view.zoom(0.9f);  // Приближаем
                }
                else {
                    view.zoom(1.1f);  // Отдаляем
                }
            }

            // Обработка перемещения камеры при зажатой левой кнопке мыши
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                isDrag = true;
                beginDragPos = sf::Mouse::getPosition(window);
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDrag = false;
            }
        }


        // Если мышь зажата, перемещаем камеру
        if (isDrag)
        {
            // Получаем текущую позицию мыши в окне
            sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
            // Вычисляем смещение в мировых координатах
            sf::Vector2f offset = window.mapPixelToCoords(currentMousePos) - window.mapPixelToCoords(beginDragPos);

            // проверка границ, но с приближение-отдалением пока баги
            sf::Vector2f topLeft = window.mapPixelToCoords(sf::Vector2i(0, 0));// левый верхний угол экрана
            sf::Vector2f downRight = window.mapPixelToCoords(sf::Vector2i(screenW, screenH));//правый нижний угол экрана

            if (topLeft.x < -5000) cout << "out of border" << endl;
            if (topLeft.y < -5000) cout << "out of border" << endl;
            if (downRight.x > 5000) cout << "out of border" << endl;
            if (downRight.y > 5000) cout << "out of border" << endl;

            // Перемещаем камеру
            view.move(-offset);
            // Обновляем позицию мыши для следующего кадра
            beginDragPos = currentMousePos;
        }



        // отрисовка кадра

        // Очищаем экран
        window.clear(sf::Color(209, 209, 209));

        // сетка из точек (для ореинтирования в пространтсве)
        sf::CircleShape point(10);
        point.setFillColor(sf::Color::Red);

        for (int i = -border; i <= border; i += 200)
        {
            for (int j = -border; j <= border; j += 200)
            {
                point.setPosition(i, j);
                window.draw(point);
            }
        }


        // Применяем новый вид (View)
        window.setView(view);
        // Отображаем новый кадр
        window.display();
    }

    return 0;
}
