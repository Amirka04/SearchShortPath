#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <set>


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

// уведомление об ошибке
bool isError = false;
char errMsg[1024];


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



// Функция для восстановления пути от стартовой вершины до целевой
std::vector<int> reconstructPath(const std::map<int, int>& parents, int startVertex, int endVertex) {
    std::vector<int> path;
    int currentVertex = endVertex;
    while (currentVertex != startVertex) {
        path.push_back(currentVertex);
        currentVertex = parents.at(currentVertex);
    }
    path.push_back(startVertex);
    std::reverse(path.begin(), path.end());
    return path;
}


// Алгоритм нахождения кратчайшего пути между 2 различными точками
void SearchShortPath(std::map<int, std::vector<int>> graph, std::vector<Point> points, std::vector<int>& ShortPath, Point begin, Point end, float& lenght) {
    std::map<int, float> distances;
    std::map<int, int> parents;
    std::set<int> unvisited;

    for(const auto& entry : graph) {
        int vertex = entry.first;
        distances[vertex] = std::numeric_limits<int>::max();
        parents[vertex] = -1;
        unvisited.insert(vertex);
    }

    distances[begin.index] = 0;
    parents[begin.index] = begin.index;

    while(!unvisited.empty()) {
        int currentVertex = *unvisited.begin();
        for(const auto& vertex : unvisited)
            if(distances[vertex] < distances[currentVertex])
                currentVertex = vertex;
        unvisited.erase(currentVertex);
        if(currentVertex == end.index)
            break;

        for(const auto& neighborVertex : graph.at(currentVertex)) {
            float newDistance = distances[currentVertex] + distance(points[currentVertex].circle, points[neighborVertex].circle);
            if(newDistance < distances[neighborVertex]) {
                distances[neighborVertex] = newDistance;
                parents[neighborVertex] = currentVertex;
            }
        }
    }

    ShortPath = reconstructPath(parents, begin.index, end.index);
    for(int i = 0; i < ShortPath.size() - 1; ++i)
        lenght += distance(points[ShortPath[i]].circle, points[ShortPath[i + 1]].circle);
}


int main() {
    srand(time(0));

    // Размеры окна и его инициализация
    float screenWidth = 1400;
    float screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Поиск кратчайшего пути. ТЗ MAD DEVS");
    SetTargetFPS(60);

    Font customFont = LoadFont("TerminusBlackSsiBold.ttf");

    GuiSetFont(customFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    

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
    float lenght = 0;
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

    Rectangle SSPathBound = entryEndPoint;
    SSPathBound.y = entryEndPoint.y + SSPathBound.height + 10.0f;

    Rectangle ResultLabel = SSPathBound;
    ResultLabel.y = SSPathBound.y + ResultLabel.height + 10.0f;

    int generatedButtonPressed = 0;
    int searchButtonPressed = 0;
    
    int begin = -1, end = -1;
    int lastBegin = begin, lastEnd = end;

    int i;

    bool beginSpinnerEditMode = false;
    bool endSpinnerEditMode = false;

    bool isSearchPath = false;


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
            if(abs(begin - lastBegin) > 0 || abs(end - lastEnd) > 0) {
                isSearchPath = false;
                shortPath.clear();
            }
        }
        searchButtonPressed = GuiButton(SSPathBound, "Search and Show Short Path");
        if(searchButtonPressed) {
            if(begin == -1 || end == -1) {
                isError = true;
                strcpy(errMsg, "The starting or ending vertices are not selected");
            }
            else {
                SearchShortPath(graph, points, shortPath, points[begin], points[end], lenght);
                isSearchPath = true;
            }
        }
        GuiLabel(endLabelBound, "End");
        if(generatedButtonPressed){
            Generate(points, circle);
            GenerateGraph(graph, points);
            isSearchPath = false;
            shortPath.clear();
        }
        GuiLabel(ResultLabel, ("Result: " + std::to_string(lenght * 10) + " USD").c_str());


        GuiGroupBox(VisualizeGroupBounds, "Visualization");
        DrawCircleLines(circle.x, circle.y, circle.r, circle.color);

        // Draw lines
        for(i = 0; i < points.size(); ++i)
            for(int j = 0; j < graph[i].size(); ++j)
                DrawLine(points[i].circle.x, points[i].circle.y, points[graph[i][j]].circle.x, points[graph[i][j]].circle.y, BEIGE);
        
        if(isSearchPath)
            for(i = 0; i < shortPath.size() - 1; ++i)
                DrawLine(points[shortPath[i]].circle.x, points[shortPath[i]].circle.y, points[shortPath[i + 1]].circle.x, points[shortPath[i + 1]].circle.y, BLUE);

        // Draw pixels
        for(i = 0; i < points.size(); ++i) {
            if(i == begin)
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r + 1.5, RED);
            else if(i == end)
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r + 1.5, GREEN);
            else
                DrawCircle(points[i].circle.x, points[i].circle.y, points[i].circle.r, points[i].circle.color);
        }

        if(isError) {
            int result = GuiMessageBox((Rectangle){ GetScreenWidth() / 2.0f - 500 / 2.0f, GetScreenHeight() / 2.0f - 200 / 2.0f, 500, 200 },
                    "#191#Error Box", errMsg, "Ok");

            if (result >= 0) isError = false;
        }

        EndDrawing();
    }


    UnloadFont(customFont);

    CloseWindow();

    return 0;
}
