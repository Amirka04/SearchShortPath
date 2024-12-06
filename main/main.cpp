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



// Генерация соединений
// void GenerateLink(std::map<int, std::vector<int>>& graph, std::vector<Point>& points) {
//     // Инициализация графа
//     for (int i = 0; i < points.size(); ++i) {
//         std::vector<int> neighbors;
//         graph[i] = neighbors;
//     }

//     // Найдём соединения относительно расстояния других точек
//     for (int i = 0; i < points.size(); ++i) {
//         std::vector<std::pair<float, int>> distances;
//         for (int j = 0; j < points.size(); ++j)
//             if (i != j)
//                 distances.push_back({distance(points[i].circle, points[j].circle), j});

//         std::sort(distances.begin(), distances.end());
        
//         int count = graph[i].size();
//         // Сначало ищем максимально близкие к нам точки
//         for(auto j : distances) {
//             // соединение вершин всегда взаимное, по этому при добавлений мы должны проверить на добавлую вершину, и уже от этого исходить, а так же учесть кол-во точек
//             if(std::distance(graph[j.second].begin(), std::find(graph[j.second].begin(), graph[j.second].end(), i)) != graph[j.second].size()) {
//                 if(count <= 6) break;

//                 graph[i].push_back(j.second);
//                 graph[j.second].push_back(i);
//                 ++count;
//             }
//         }

//         // гарантируем минимум 2 соединения
//         while (graph[i].size() < 2 && distances.size() > 1){
//             graph[i].push_back(distances[graph[i].size()].second);
//         }
//     }
// }


std::map<int, std::vector<int>> createGraph(const std::vector<Point>& points, int minConnections) {
    std::map<int, std::vector<int>> graph;
    std::random_device rd;
    std::mt19937 gen(rd());

    if (points.empty()) return graph; // Обработка пустого вектора

    for (size_t i = 0; i < points.size(); ++i) {
        std::vector<int> connected_points;
        int num_connections = 0;

        while (num_connections < minConnections) {
            int connected_point_index;
            do {
              connected_point_index = gen() % points.size(); //Более эффективная генерация случайных чисел
            } while (connected_point_index == i || std::find(connected_points.begin(), connected_points.end(), connected_point_index) != connected_points.end());
            connected_points.push_back(connected_point_index);
            graph[i].push_back(connected_point_index);
            graph[connected_point_index].push_back(i); // Неориентированный граф
            num_connections++;
        }


        //Добавим случайное количество дополнительных соединений, но не более 6
        int max_additional = std::min(6-minConnections, (int)points.size() -1 - minConnections);
        int additional_connections = gen() % (max_additional + 1);
        for (int j = 0; j < additional_connections; ++j) {
            int connected_point_index;
            do {
                connected_point_index = gen() % points.size();
            } while (connected_point_index == i || std::find(connected_points.begin(), connected_points.end(), connected_point_index) != connected_points.end());
            connected_points.push_back(connected_point_index);
            graph[i].push_back(connected_point_index);
            graph[connected_point_index].push_back(i);
        }

    }
    return graph;
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
    std::map<int, std::vector<int>> graph = createGraph(points, 2);
    
    // Короткий путь
    std::vector<int> shortPath;
    

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
            graph = createGraph(points, 2);
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
