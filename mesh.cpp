#include "mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>

Mesh Mesh::LoadFromObj(const std::string& filename) {
    Mesh mesh;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file " << filename << std::endl;
        return mesh;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            // Читаем вершину: v 1.0 -1.0 0.5
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
        else if (prefix == "f") {
            // Читаем грань: f 1 2 3
            // ВАЖНО: В OBJ индексы начинаются с 1, а в C++ с 0. Нужно вычитать 1.
            Mesh::Face f;
            int idx[3];
            // Простой парсер считает, что формат "f v1 v2 v3" (без текстур и нормалей через слэш)
            // Если будут слэши (f 1/1/1), этот код нужно усложнить.
            // Пока рассчитываем на чистую геометрию.
            
            // Пытаемся прочитать обычные числа
            // Для полноценного парсера (f 1//2 2//3...) нужен более сложный код,
            // но для старта сделаем упрощенный.
            
            // Хак для чтения формата f 1/2/3: заменяем слэши на пробелы
             for (char& c : line) if (c == '/') c = ' ';
             std::stringstream ss2(line);
             std::string temp; 
             ss2 >> temp; // Пропускаем 'f'

             // Считываем только первые индексы (позиции), остальное (текстуры/нормали) игнорируем
             // Формат может быть: f v1/vt1/vn1 v2/...
             // Нам нужно вычленить v1, v2, v3
             
             // Самый простой способ для начала: используем sscanf или просто пропускаем мусор
             // Но давай напишем чуть надежнее для простых файлов:
             
             for(int i=0; i<3; i++) {
                 ss2 >> f.v[i];
                 f.v[i]--; // OBJ index 1-based -> C++ 0-based
                 
                 // Если после числа идет мусор (из-за замены слэшей), поток его прожует при следующем чтении числа
                 // Но лучше использовать dummy переменные, если формат сложный.
                 // Пока предположим, что файл простой: f 1 2 3
             }
             mesh.faces.push_back(f);
        }
    }
    
    std::cout << "Loaded " << filename << ": " << mesh.vertices.size() << " verts, " << mesh.faces.size() << " faces." << std::endl;
    return mesh;
}
