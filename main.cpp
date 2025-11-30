#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "renderer.h"
#include "math_3d.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Функция создания цвета
uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

struct Triangle {
    int v[3];
};

// Вершины КУБА
Vec3 cubeVertices[8] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
};

// Треугольники (индексы)
Triangle cubeTris[12] = {
    // Front
    {0, 1, 2}, {0, 2, 3},
    // Right
    {1, 5, 6}, {1, 6, 2},
    // Back
    {5, 4, 7}, {5, 7, 6},
    // Left
    {4, 0, 3}, {4, 3, 7},
    // Top
    {3, 2, 6}, {3, 6, 7},
    // Bottom
    {4, 5, 1}, {4, 1, 0}
};

float rotationAngle = 0.0f;

int main() {
    // --- Инициализация OpenGL ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Software Renderer", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD" << std::endl;
        return -1;
    }

    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

    while (!glfwWindowShouldClose(window)) {
        // 1. Очистка
        renderer.Clear(MakeColor(40, 40, 40)); // Серый фон

        // 2. Обновление вращения
        rotationAngle += 0.02f;

        // 3. Матрицы
        // Вращаем куб
        Mat4 matRotZ = Mat4::RotateZ(rotationAngle);
        Mat4 matRotX = Mat4::RotateX(rotationAngle * 0.5f);
        
        // Сдвигаем куб от камеры (по Z)
        // Важно: Camera в 0,0,0. Куб ставим в 0,0,4.
        Mat4 matTrans = Mat4::Translate(0.0f, 0.0f, 4.0f); 

        // Проекция
        float aspect = (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH;
        Mat4 matProj = Mat4::Projection(1.57f, aspect, 0.1f, 100.0f);

        // Итоговая матрица мира
        Mat4 matWorld = Mat4::Identity();
        matWorld = matRotZ * matWorld;
        matWorld = matRotX * matWorld;
        matWorld = matTrans * matWorld;

        // 4. Отрисовка треугольников
        for (int i = 0; i < 12; i++) {
            Triangle t = cubeTris[i];
            
            // A. Трансформация вершин (World Space)
            Vec3 v0 = MultiplyMatrixVector(cubeVertices[t.v[0]], matWorld);
            Vec3 v1 = MultiplyMatrixVector(cubeVertices[t.v[1]], matWorld);
            Vec3 v2 = MultiplyMatrixVector(cubeVertices[t.v[2]], matWorld);

            // B. Расчет Нормали и Луча Взгляда
            Vec3 line1 = v1 - v0;
            Vec3 line2 = v2 - v0;
            // Внимание: CrossProduct(line1, line2)
            Vec3 normal = CrossProduct(line1, line2).Normalize();

            // Вектор от камеры (0,0,0) к поверхности
            Vec3 cameraRay = v0 - Vec3(0,0,0);

            // C. Backface Culling
            // Используем знак '>', так как '<' давал "наизнанку".
            // Это выбирает ПЕРЕДНИЕ грани, которые раньше не рисовались.
            if (DotProduct(normal, cameraRay) > 0.0f) {
                 
                 // D. Освещение (Простое)
                 // Светим прямо вдоль оси Z (от камеры)
                 float dotLight = DotProduct(normal, Vec3(0.0f, 0.0f, 1.0f)); 
                 float intensity = std::abs(dotLight); // Модуль, чтобы грань всегда светилась
                 
                 intensity = 0.2f + (0.8f * intensity); // Ambient light
                 if (intensity > 1.0f) intensity = 1.0f;

                 uint32_t color = MakeColor((int)(255*intensity), (int)(255*intensity), (int)(255*intensity));

                 // E. Проекция в 2D
                 Vec3 p0 = MultiplyMatrixVector(v0, matProj);
                 Vec3 p1 = MultiplyMatrixVector(v1, matProj);
                 Vec3 p2 = MultiplyMatrixVector(v2, matProj);
                 
                 // F. Viewport (Экранные координаты)
                 p0.x = (p0.x + 1.0f) * 0.5f * WINDOW_WIDTH; p0.y = (p0.y + 1.0f) * 0.5f * WINDOW_HEIGHT;
                 p1.x = (p1.x + 1.0f) * 0.5f * WINDOW_WIDTH; p1.y = (p1.y + 1.0f) * 0.5f * WINDOW_HEIGHT;
                 p2.x = (p2.x + 1.0f) * 0.5f * WINDOW_WIDTH; p2.y = (p2.y + 1.0f) * 0.5f * WINDOW_HEIGHT;

                 // G. ОТРИСОВКА (FIX!)
                 // Меняем p1 и p2 местами! 
                 // (p0, p2, p1) вместо (p0, p1, p2)
                 // Это инвертирует порядок обхода вершин для растеризатора.
                 renderer.DrawTriangle(
                    (int)p0.x, (int)p0.y, 
                    (int)p2.x, (int)p2.y, // <-- P2 здесь
                    (int)p1.x, (int)p1.y, // <-- P1 здесь
                    color
                );
            }
        }

        renderer.DrawBuffer();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
