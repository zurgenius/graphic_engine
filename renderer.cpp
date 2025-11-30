#include "renderer.h"
#include <iostream>
#include <algorithm>

// Простейшие шейдеры. Вершинный просто передает координаты, 
// Фрагментный берет цвет из нашей текстуры.
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D screenTexture;
    void main() {
        FragColor = texture(screenTexture, TexCoord);
    }
)";

// Вспомогательная функция: Edge Function
// Если результат >= 0, точка P находится справа от вектора AB
static int EdgeFunction(int x0, int y0, int x1, int y1, int px, int py) {
    return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
}

void Renderer::DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // 1. Находим ограничивающий прямоугольник (Bounding Box)
    int minX = std::min({x0, x1, x2});
    int minY = std::min({y0, y1, y2});
    int maxX = std::max({x0, x1, x2});
    int maxY = std::max({y0, y1, y2});

    // Обрезаем по краям экрана (Clipping)
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, m_width - 1);
    maxY = std::min(maxY, m_height - 1);

    // 2. Пробегаем по всем пикселям прямоугольника
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            // 3. Проверяем, лежит ли пиксель внутри треугольника
            // Пиксель должен быть "справа" от всех трех сторон треугольника
            int w0 = EdgeFunction(x1, y1, x2, y2, x, y);
            int w1 = EdgeFunction(x2, y2, x0, y0, x, y);
            int w2 = EdgeFunction(x0, y0, x1, y1, x, y);

            // Если точка внутри - рисуем
            // (Обратите внимание: >= 0 работает для стандартного обхода вершин против часовой стрелки)
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                PutPixel(x, y, color);
            }
        }
    }
}

Renderer::Renderer(int width, int height) : m_width(width), m_height(height) {
    // Выделяем память под пиксели (width * height)
    m_buffer.resize(width * height); 
    InitOpenGL();
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteTextures(1, &m_textureID);
    glDeleteProgram(m_shaderProgram);
}

void Renderer::Clear(uint32_t color) {
    // Быстрая заливка массива одним значением
    std::fill(m_buffer.begin(), m_buffer.end(), color);
}

void Renderer::PutPixel(int x, int y, uint32_t color) {
    // Защита от выхода за границы массива
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
    
    // Формула перевода 2D координат в 1D индекс массива
    // Мы считаем (0,0) левым верхним углом
    m_buffer[y * m_width + x] = color;
}

void Renderer::DrawBuffer() {
    // 1. Обновляем текстуру данными из нашего массива
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.data());

    // 2. Рисуем квадрат на весь экран с этой текстурой
    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::DrawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    // Вычисляем смещение по осям
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    
    // Определяем направление шага (1 или -1)
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    // Ошибка накопления
    int err = dx - dy;
    
    while (true) {
        // Рисуем пиксель в текущей точке
        PutPixel(x0, y0, color);
        
        // Если дошли до конца — выходим
        if (x0 == x1 && y0 == y1) break;
        
        // Вычисляем следующий шаг на основе ошибки
        int e2 = 2 * err;
        
        // Если ошибка указывает на смещение по X
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        
        // Если ошибка указывает на смещение по Y
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Renderer::InitOpenGL() {
    // Создаем шейдерную программу
    m_shaderProgram = CreateShader(vertexShaderSource, fragmentShaderSource);

    // Координаты квадрата на весь экран (2 треугольника)
    // Format: X, Y, U, V
    float vertices[] = {
        // Первый треугольник
        -1.0f,  1.0f,  0.0f, 0.0f, // Top-left
        -1.0f, -1.0f,  0.0f, 1.0f, // Bottom-left
         1.0f, -1.0f,  1.0f, 1.0f, // Bottom-right
        // Второй треугольник
        -1.0f,  1.0f,  0.0f, 0.0f, // Top-left
         1.0f, -1.0f,  1.0f, 1.0f, // Bottom-right
         1.0f,  1.0f,  1.0f, 0.0f  // Top-right
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Атрибут координат (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Атрибут текстурных координат (U, V)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Создаем текстуру
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    // Параметры: не размывать пиксели (GL_NEAREST), чтобы видеть каждый пиксель четко
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Выделяем память на GPU под текстуру (пока пустую)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

GLuint Renderer::CreateShader(const char* vertexSrc, const char* fragmentSrc) {
    // Стандартная компиляция шейдеров (сокращенно для экономии места)
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSrc, NULL);
    glCompileShader(vertex);
    
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSrc, NULL);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}
