#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <ctime>
#include <algorithm>
#include <numeric>


// графические библиотеки которые я использовал для визуализации результата
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"
#include "raymath.h"



struct Circle {
    float x, y, r;
    Color color;
};

struct Point {
    int index;
    Circle circle;
};



// Генерация случайных чисел с плавающей точкой
float floatRand() { return static_cast<double>(rand()) / RAND_MAX; }

// Генерация точек в окружности
void Generate(std::vector<Point>& points, Circle d) {
    for(int i = 0; i < points.size(); ++i) {
        float r = (d.r - 2) * sqrt(floatRand());
        float theta = 2 * M_PI * floatRand();

        // Не забудем что я использую не структуру, а структуру данных с изменённым типом данных
        points[i].index = i;
        points[i].circle = (Circle){d.x + r * std::cos(theta), d.y + r * std::sin(theta), 3, ORANGE};
    }
}

float distance(Circle c1, Circle c2) {
    return sqrt((c1.x - c2.x)*(c1.x - c2.x) + (c1.y - c2.y)*(c1.y - c2.y));
}

float mean(const std::vector<float>& values) {
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

float standardDeviation(const std::vector<float>& values, float mean) {
    float sum = 0.0;
    for (float value : values) {
        sum += (value - mean) * (value - mean);
    }
    return std::sqrt(sum / values.size());
}




int main() {
    srand(time(0));

    // Размеры окна и его инициализация
    float screenWidth = 1400;
    float screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Поиск кратчайшего пути. ТЗ MAD DEVS");
    SetTargetFPS(60);


    // Формы для оформления
    // форма блока с визуализацией результата, генерацией и нахождении кратчайшего маршрута между двумя любыми точками
    Rectangle VisualizeGroupBounds;
    VisualizeGroupBounds.x = 0 + 10.0f;
    VisualizeGroupBounds.y = 0 + 10.0f;
    VisualizeGroupBounds.width = GetScreenWidth() / 2.0f - 20.0f;
    VisualizeGroupBounds.height = GetScreenHeight() - 20.0f;

    // форма блока с визуализацией UI элементов
    Rectangle UIGroupBounds;
    UIGroupBounds.x = GetScreenWidth() / 2.0f + 20.0f;
    UIGroupBounds.y = 0 + 10.0f;
    UIGroupBounds.width = GetScreenWidth() - 20.0f - UIGroupBounds.x;
    UIGroupBounds.height = GetScreenHeight() - 20.0f;

    
    // Круг внутри которого будут помещаться точки
    float radius = VisualizeGroupBounds.width / 2.0f - 15.0f; // Уменьшаем радиус, чтобы круг не выходил за границы формы
    // Круг
    Circle circle;
    circle.x = VisualizeGroupBounds.x + VisualizeGroupBounds.width / 2.0f;
    circle.y = VisualizeGroupBounds.y + VisualizeGroupBounds.height / 2.0f;
    circle.r = VisualizeGroupBounds.width / 2.0f - 20.0f;
    circle.color = BLACK;

    
    // точки которые мы генерируем внутри окружности
    std::vector<Point> points(9);
    Generate(points, circle);


    // Это граф, его структура это словарь, ключами которого являются целые числа (id точек на окружности)
    // а значения это id точек с которыми он смежен
    std::map<int, std::vector<int>> graph;


    // Кнопка
    Rectangle generateButtonBounds;
    generateButtonBounds.x = UIGroupBounds.x + 10.0f;
    generateButtonBounds.y = UIGroupBounds.y + 10.0f;
    generateButtonBounds.width = UIGroupBounds.width - 20.0f;
    generateButtonBounds.height = UIGroupBounds.height / 10.0f;

    int generatedButtonPressed = 0;

    int i;

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        GuiGroupBox(UIGroupBounds, "UI");
        generatedButtonPressed = GuiButton(generateButtonBounds, "Generate");
        if(generatedButtonPressed){
            Generate(points, circle);
        }

        GuiGroupBox(VisualizeGroupBounds, "Visualization");
        DrawCircleLines(circle.x, circle.y, circle.r, circle.color);
        
        // Draw lines
        for(i = 0; i < points.size(); ++i)
            for(int j = 0; j < graph[i].size(); ++j)
                DrawLine(points[i].circle.x, points[i].circle.y, points[graph[i][j]].circle.x, points[graph[i][j]].circle.y, BEIGE);
        
        // for(i = 1; i < points.size(); ++i)
        //         DrawLine(points[i-1].circle.x, points[i-1].circle.y, points[i].circle.x, points[i].circle.y, RED);
        
        // Draw pixels
        for(i = 0; i < points.size(); ++i) {
            if(i == 0)
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, RED);
            else if(i == (points.size() - 1))
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, GREEN);
            else
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, points[i].circle.color);
        }

        EndDrawing();
    }


    CloseWindow();

    return 0;
}
