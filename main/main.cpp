#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <cstring>


// графические библиотеки которые я использовал для визуализации результата
#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
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
        points[i].circle = (Circle){d.x + r * std::cos(theta), d.y + r * std::sin(theta), 4, ORANGE};
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


void GenerateGraph(std::map<int, std::vector<int>>& graph, std::vector<Point> points) {
    int i;
    float minDistance = 200;
    
    // инициализирую граф
    for(i = 0; i < points.size(); ++i) {
        std::vector<int> vertexes;
        graph[i] = vertexes;
    }

    for(i = 0; i < points.size(); ++i) {
        // массив расстояний до каждой точки
        std::vector<std::pair<float, int>> distances;
        for(int j = 0; j < points.size(); ++j)
            if(i != j) 
                distances.push_back(std::make_pair(distance(points[i].circle, points[j].circle), j));
        // Сортируем точки с минимальным расстоянием были у нас спереди
        std::sort(distances.begin(), distances.end());
        
        // При добавлении учту что у нас не ориентированный граф, при соединении одной вершины к другой нужно их взаимно добавить друг другу
        for(int j = 0; j < distances.size(); ++j) {
            if(graph[i].size() >= 6) break;
            graph[i].push_back(distances[j].second);
            graph[distances[j].second].push_back(i);
        }
    }
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
    std::vector<Point> points(100);
    Generate(points, circle);


    // Это граф, его структура это словарь, ключами которого являются целые числа (id точек на окружности)
    // а значения это id точек с которыми он смежен
    std::map<int, std::vector<int>> graph;
    GenerateGraph(graph, points);


    // Короткий путь
    std::vector<int> shortPath;
    

    // Кнопка
    Rectangle generateButtonBounds;
    generateButtonBounds.x = UIGroupBounds.x + 10.0f;
    generateButtonBounds.y = UIGroupBounds.y + 10.0f;
    generateButtonBounds.width = UIGroupBounds.width - 20.0f;
    generateButtonBounds.height = UIGroupBounds.height / 15.0f;

    Rectangle beginLabelBound = generateButtonBounds;
    beginLabelBound.y = generateButtonBounds.y + beginLabelBound.height + 20.0f;
    float startY = generateButtonBounds.y;

    Rectangle entryBeginPoint = beginLabelBound;
    entryBeginPoint.y = beginLabelBound.y + entryBeginPoint.height + 10.0f;
    
    Rectangle endLabelBound = beginLabelBound;
    endLabelBound.y = beginLabelBound.y + endLabelBound.height * 2 + 20.0f;
    
    Rectangle entryEndPoint = endLabelBound;
    entryEndPoint.y = endLabelBound.y + entryEndPoint.height + 10.0f;

    int generatedButtonPressed = 0;
    
    int begin = -1, end = -1;

    int i;

    bool beginSpinnerEditMode = false;
    bool endSpinnerEditMode = false;

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        GuiGroupBox(UIGroupBounds, "UI");
        generatedButtonPressed = GuiButton(generateButtonBounds, "Generate");
        GuiLabel(beginLabelBound, "Begin");
        GuiSpinner(entryBeginPoint, "", &begin, -1, points.size() - 1, beginSpinnerEditMode);
        GuiSpinner(entryEndPoint, "", &end, -1, points.size() - 1, endSpinnerEditMode);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, entryBeginPoint)) {
                beginSpinnerEditMode = true;
                endSpinnerEditMode = false;
            } else if (CheckCollisionPointRec(mousePos, entryEndPoint)) {
                beginSpinnerEditMode = false;
                endSpinnerEditMode = true;
            } else {
                beginSpinnerEditMode = false;
                endSpinnerEditMode = false;
            }
        }


        GuiLabel(endLabelBound, "End");
        if(generatedButtonPressed){
            Generate(points, circle);
            GenerateGraph(graph, points);
        }

        GuiGroupBox(VisualizeGroupBounds, "Visualization");
        DrawCircleLines(circle.x, circle.y, circle.r, circle.color);
        
        // Draw lines
        for(i = 0; i < points.size(); ++i)
            for(int j = 0; j < graph[i].size(); ++j)
                DrawLine(points[i].circle.x, points[i].circle.y, points[graph[i][j]].circle.x, points[graph[i][j]].circle.y, BEIGE);
        
        // Draw pixels
        for(i = 0; i < points.size(); ++i) {
            if(i == begin)
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, RED);
            else if(i == end)
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, GREEN);
            else
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, points[i].circle.color);
        }

        EndDrawing();
    }


    CloseWindow();

    return 0;
}
