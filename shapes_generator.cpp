#include "shapes_generator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Vertex {
    float x, y, z;
};

namespace ShapesGenerator {

void CreateSmoothSphere(const std::string& filename, float radius, int slices, int stacks) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to create " << filename << std::endl;
        return;
    }

    std::vector<Vertex> vertices;
    
    for (int i = 0; i <= stacks; ++i) {
        float phi = (float)i / stacks * M_PI;
        
        for (int j = 0; j < slices; ++j) {
            float theta = (float)j / slices * 2.0f * M_PI; 
        
            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * sinf(phi) * sinf(theta);
            float z = radius * cosf(phi);
            
            vertices.push_back({x, y, z});
            out << "v " << x << " " << y << " " << z << "\n";
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            
            int currentRow = i * slices;
            int nextRow = (i + 1) * slices;
            
            int p1 = currentRow + j + 1;
            int p2 = currentRow + ((j + 1) % slices) + 1;
            int p3 = nextRow + ((j + 1) % slices) + 1;    
            int p4 = nextRow + j + 1;
            
            out << "f " << p1 << " " << p2 << " " << p3 << " " << p4 << "\n";
        }
    }

    out.close();
    std::cout << "Generated high-poly sphere: " << filename << std::endl;
}

void CreateSmoothTorus(const std::string& filename, float majorRadius, float minorRadius, int majorSegments, int minorSegments) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to create " << filename << std::endl;
        return;
    }

    for (int i = 0; i < majorSegments; ++i) {
        float theta = (float)i / majorSegments * 2.0f * M_PI;
        
        for (int j = 0; j < minorSegments; ++j) {
            float phi = (float)j / minorSegments * 2.0f * M_PI;
            
            
            float x = (majorRadius + minorRadius * cosf(phi)) * cosf(theta);
            float y = (majorRadius + minorRadius * cosf(phi)) * sinf(theta);
            float z = minorRadius * sinf(phi);
            
            out << "v " << x << " " << y << " " << z << "\n";
        }
    }

    for (int i = 0; i < majorSegments; ++i) {
        for (int j = 0; j < minorSegments; ++j) {
            
            int nextI = (i + 1) % majorSegments;
            int nextJ = (j + 1) % minorSegments;

            int a = (i * minorSegments + j) + 1;
            int b = (i * minorSegments + nextJ) + 1;
            int c = (nextI * minorSegments + nextJ) + 1;
            int d = (nextI * minorSegments + j) + 1;
        
            out << "f " << a << " " << d << " " << c << " " << b << "\n";
        }
    }

    out.close();
    std::cout << "Generated high-poly torus: " << filename << std::endl;
}

} 
