#pragma once
#include <vector>
#include <cstdint>
#include <glad/glad.h> 

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    // Очистка экрана цветом (формат 0xRRGGBBAA)
    void Clear(uint32_t color);

    // Установка конкретного пикселя (главная функция движка)
    void PutPixel(int x, int y, uint32_t color);

    // НОВОЕ: Рисование линии алгоритмом Брезенхема
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
    
    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    // Отправка буфера на видеокарту и отрисовка
    void DrawBuffer();

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    int m_width;
    int m_height;
    
    // Наш буфер пикселей в оперативной памяти
    std::vector<uint32_t> m_buffer;

    // OpenGL идентификаторы
    GLuint m_textureID;
    GLuint m_shaderProgram;
    GLuint m_VAO, m_VBO;

    void InitOpenGL();
    GLuint CreateShader(const char* vertexSrc, const char* fragmentSrc);
};
