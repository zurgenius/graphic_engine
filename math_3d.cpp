#include "math_3d.h"
#include <cmath>

Mat4 Mat4::RotateX(float theta) {
    Mat4 mat = Mat4::Identity();
    mat.m[1][1] = cosf(theta); mat.m[1][2] = -sinf(theta);
    mat.m[2][1] = sinf(theta); mat.m[2][2] = cosf(theta);
    return mat;
}

Mat4 Mat4::RotateY(float theta) {
    Mat4 mat = Mat4::Identity();
    mat.m[0][0] = cosf(theta); mat.m[0][2] = sinf(theta);
    mat.m[2][0] = -sinf(theta); mat.m[2][2] = cosf(theta);
    return mat;
}

Mat4 Mat4::RotateZ(float theta) {
    Mat4 mat = Mat4::Identity();
    mat.m[0][0] = cosf(theta); mat.m[0][1] = -sinf(theta);
    mat.m[1][0] = sinf(theta); mat.m[1][1] = cosf(theta);
    return mat;
}

Mat4 Mat4::Translate(float x, float y, float z) {
    Mat4 mat = Mat4::Identity();
    mat.m[0][3] = x;
    mat.m[1][3] = y;
    mat.m[2][3] = z;
    return mat;
}

Mat4 Mat4::Projection(float fov, float aspectRatio, float zNear, float zFar) {
    Mat4 mat; // Нулевая матрица
    float f = 1.0f / tanf(fov * 0.5f); // Фокусное расстояние
    float q = zFar / (zFar - zNear);

    mat.m[0][0] = aspectRatio * f;
    mat.m[1][1] = f;
    mat.m[2][2] = q;
    mat.m[2][3] = -zNear * q; // Смещение Z
    mat.m[3][2] = 1.0f;       // Важно! Здесь мы сохраняем Z в компоненту W для деления
    return mat;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 res;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                res.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return res;
}

Vec3 MultiplyMatrixVector(const Vec3& i, const Mat4& m) {
    Vec3 v;
    // Умножаем вектор как (x, y, z, 1)
    float x = i.x * m.m[0][0] + i.y * m.m[0][1] + i.z * m.m[0][2] + m.m[0][3];
    float y = i.x * m.m[1][0] + i.y * m.m[1][1] + i.z * m.m[1][2] + m.m[1][3];
    float z = i.x * m.m[2][0] + i.y * m.m[2][1] + i.z * m.m[2][2] + m.m[2][3];
    float w = i.x * m.m[3][0] + i.y * m.m[3][1] + i.z * m.m[3][2] + m.m[3][3];

    // Perspective Divide (деление на W)
    // Это превращает усеченную пирамиду в куб (-1..1)
    if (w != 0.0f) {
        v.x = x / w;
        v.y = y / w;
        v.z = z / w;
    }
    return v;
}

// Скалярное произведение (косинус угла)
float DotProduct(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Векторное произведение (находит перпендикуляр к двум векторам - нормаль)
Vec3 CrossProduct(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}