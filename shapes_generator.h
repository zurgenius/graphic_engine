#pragma once
#include <string>

namespace ShapesGenerator {
    void CreateSmoothSphere(const std::string& filename, float radius, int slices, int stacks);
    void CreateSmoothTorus(const std::string& filename, float majorRadius, float minorRadius, int majorSegments, int minorSegments);
}
