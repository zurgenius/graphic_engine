#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "renderer.h"
#include "mesh.h"
#include "math_3d.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

float rotationAngle = 0.0f;

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Engine v1.0", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

    // ЗАГРУЗКА МОДЕЛИ
    // Убедись, что файл лежит рядом с app или укажи полный путь!
    Mesh myMesh = Mesh::LoadFromObj("../assets/torus.obj"); 
    
    // Если файл не нашелся, создадим один треугольник вручную для теста
    if (myMesh.faces.empty()) {
        std::cout << "Warning: Loading fallback model" << std::endl;
        myMesh.vertices = {{-1,-1,0}, {0,1,0}, {1,-1,0}};
        myMesh.faces = {{0, 1, 2}}; // Обрати внимание: CCW порядок (0->1->2)
    }

    while (!glfwWindowShouldClose(window)) {
        renderer.Clear(MakeColor(40, 40, 40));
        rotationAngle += 0.02f;

        // МАТРИЦЫ
        Mat4 matRotZ = Mat4::RotateZ(rotationAngle);
        Mat4 matRotX = Mat4::RotateX(rotationAngle * 0.5f);
        Mat4 matTrans = Mat4::Translate(0.0f, 0.0f, 5.0f); // Чуть дальше

        float aspect = (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH;
        Mat4 matProj = Mat4::Projection(1.57f, aspect, 0.1f, 100.0f);

        Mat4 matWorld = Mat4::Identity();
        matWorld = matRotZ * matWorld;
        matWorld = matRotX * matWorld;
        matWorld = matTrans * matWorld;

        // ОТРИСОВКА MESH
        for (const auto& face : myMesh.faces) {
            // 1. Transform
            Vec3 v0 = MultiplyMatrixVector(myMesh.vertices[face.v[0]], matWorld);
            Vec3 v1 = MultiplyMatrixVector(myMesh.vertices[face.v[1]], matWorld);
            Vec3 v2 = MultiplyMatrixVector(myMesh.vertices[face.v[2]], matWorld);

            // 2. Calculate Normal (Standard CCW: Edge1 x Edge2)
            Vec3 edge1 = v1 - v0;
            Vec3 edge2 = v2 - v0;
            Vec3 normal = CrossProduct(edge1, edge2).Normalize();

            // 3. Backface Culling
            // Вектор от точки К камере. Камера в (0,0,0).
            // v0 - точка на поверхности.
            // ViewDir = CameraPos - v0 = (0,0,0) - v0 = -v0.
            Vec3 viewDir = (v0 * -1.0f).Normalize();

            // Если нормаль смотрит НА нас (угол < 90), то Dot > 0.
            // Это условие "честного" Culling-а для CCW треугольников.
            if (DotProduct(normal, viewDir) > 0.0f) {
                
                // 4. Lighting
                
                // НОВОЕ: Свет падает Справа-Сверху-Спереди (как Солнце)
                // (X=0.5, Y=1.0, Z=1.0)
                Vec3 lightDir = Vec3(0.5f, 1.0f, -1.0f).Normalize(); 
                
                // Считаем освещенность
                float dot = DotProduct(normal, lightDir);
                
                // std::max(0.0f, dot) убирает свет с обратной стороны
                float intensity = std::max(0.0f, dot);
                
                // Добавляем "фоновый свет" (Ambient), чтобы тени не были черными
                // Было 0.2, сделаем поменьше (0.1), чтобы контраст был выше
                intensity = 0.1f + (0.9f * intensity);

                // Ограничиваем максимум (на всякий случай)
                if (intensity > 1.0f) intensity = 1.0f;

                // НОВОЕ: Оранжевый цвет (R=255, G=165, B=0)
                // Умножаем каждый канал на интенсивность света
                uint8_t r = (uint8_t)(255 * intensity);
                uint8_t g = (uint8_t)(165 * intensity);
                uint8_t b = (uint8_t)(0   * intensity); // Синего нет

                uint32_t color = MakeColor(r, g, b);
                // 5. Projection & Viewport
                Vec3 p0 = MultiplyMatrixVector(v0, matProj);
                Vec3 p1 = MultiplyMatrixVector(v1, matProj);
                Vec3 p2 = MultiplyMatrixVector(v2, matProj);
                
                auto ToScreen = [&](Vec3& p) {
                    p.x = (p.x + 1.0f) * 0.5f * WINDOW_WIDTH;
                    p.y = (p.y + 1.0f) * 0.5f * WINDOW_HEIGHT;
                };
                ToScreen(p0); ToScreen(p1); ToScreen(p2);

                // 6. Draw (Без смены p1/p2 - стандартный порядок)
                renderer.DrawTriangle(
                    (int)p0.x, (int)p0.y, 
                    (int)p1.x, (int)p1.y, 
                    (int)p2.x, (int)p2.y, 
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
