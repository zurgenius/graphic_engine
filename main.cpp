#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <filesystem>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "renderer.h"
#include "mesh.h"
#include "math_3d.h"

namespace fs = std::filesystem;

// глобальные константы
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const std::string ASSETS_DIR = "../assets/";

float cameraZoom = 5.0f;
float rotX = 0.0f;
float rotY = 0.0f;
bool autoRotate = true;
char importPathBuffer[512] = "";

uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void ReloadMesh(Mesh& mesh, const std::string& filename) {
    std::string fullPath = ASSETS_DIR + filename;
    if (fs::exists(fullPath)) {
        Mesh temp = Mesh::LoadFromObj(fullPath);
        if (!temp.faces.empty()) {
            mesh = temp;
            std::cout << "Loaded: " << filename << std::endl;
        }
    } else {
        std::cerr << "File not found: " << fullPath << std::endl;
    }
}

void ImportAndLoad(Mesh& mesh, const std::string& sourcePath) {
    if (!fs::exists(sourcePath)) {
        std::cerr << "Import failed: Source file doesn't exist." << std::endl;
        return;
    }
    
    try {
        fs::path src(sourcePath);
        std::string filename = src.filename().string();
        std::string destPath = ASSETS_DIR + filename;
        
        if (!fs::exists(ASSETS_DIR)) fs::create_directory(ASSETS_DIR);

        if (fs::absolute(src) != fs::absolute(destPath)) {
            fs::copy_file(src, destPath, fs::copy_options::overwrite_existing);
        }
        
        ReloadMesh(mesh, filename);
    } catch (const std::exception& e) {
        std::cerr << "Import error: " << e.what() << std::endl;
    }
}

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

    Mesh myMesh;
    ReloadMesh(myMesh, "cube.obj");
    
    if (myMesh.faces.empty()) {
         myMesh.vertices = {{-1,-1,0}, {0,1,0}, {1,-1,0}};
         myMesh.faces = {{0, 1, 2}};
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //обрабатываем вращение мыши через imGUI
        if (!io.WantCaptureMouse && !autoRotate) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                rotY += io.MouseDelta.x * 0.01f;
                rotX += io.MouseDelta.y * 0.01f;
            }
        }
        

        if (autoRotate) {
            rotX += 0.01f;
            rotY += 0.02f;
        }

        // --- UI ---
        ImGui::SetNextWindowPos(ImVec2(0, WINDOW_HEIGHT - 140));
        ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, 140));
        ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("Models:");
        if (ImGui::Button("Cube")) ReloadMesh(myMesh, "cube.obj");
        ImGui::SameLine();
        if (ImGui::Button("Pyramid")) ReloadMesh(myMesh, "pyramid.obj");
        ImGui::SameLine();
        if (ImGui::Button("Sphere")) ReloadMesh(myMesh, "sphere.obj");
        ImGui::SameLine();
        if (ImGui::Button("Torus")) ReloadMesh(myMesh, "torus.obj");

        ImGui::Separator();
        
        ImGui::Text("Camera:");
        ImGui::SliderFloat("Zoom", &cameraZoom, 2.0f, 20.0f);
        ImGui::Checkbox("Auto Rotate", &autoRotate);
        
        ImGui::Separator();
        ImGui::Text("Import Custom .OBJ:");
        ImGui::PushItemWidth(400);
        ImGui::InputText("##path", importPathBuffer, sizeof(importPathBuffer));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Load File")) {
            ImportAndLoad(myMesh, std::string(importPathBuffer));
        }

        ImGui::End();
        // --- END IMGUI UPDATE ---

        renderer.Clear(MakeColor(40, 40, 40));


        Mat4 matRotY = Mat4::RotateY(rotY);
        Mat4 matRotX = Mat4::RotateX(rotX); 
        
        Mat4 matTrans = Mat4::Translate(0.0f, 0.0f, cameraZoom); 

        float aspect = (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH;
        Mat4 matProj = Mat4::Projection(1.57f, aspect, 0.1f, 100.0f);

        Mat4 matWorld = Mat4::Identity();
        matWorld = matRotY * matWorld;
        matWorld = matRotX * matWorld;
        matWorld = matTrans * matWorld;

        for (const auto& face : myMesh.faces) {
            if (face.v[0] >= myMesh.vertices.size() || 
                face.v[1] >= myMesh.vertices.size() || 
                face.v[2] >= myMesh.vertices.size()) continue;

            // 1. Transform
            Vec3 v0 = MultiplyMatrixVector(myMesh.vertices[face.v[0]], matWorld);
            Vec3 v1 = MultiplyMatrixVector(myMesh.vertices[face.v[1]], matWorld);
            Vec3 v2 = MultiplyMatrixVector(myMesh.vertices[face.v[2]], matWorld);

            // 2. Calculate Normal
            Vec3 edge1 = v1 - v0;
            Vec3 edge2 = v2 - v0;
            Vec3 normal = CrossProduct(edge1, edge2).Normalize();

            // 3. Backface Culling
            Vec3 viewDir = (v0 * -1.0f).Normalize();

            if (DotProduct(normal, viewDir) > 0.0f) {
                
                // 4. Lighting
                Vec3 lightDir = Vec3(0.5f, 1.0f, -1.0f).Normalize(); 
                float dot = DotProduct(normal, lightDir);
                float intensity = std::max(0.0f, dot);
                intensity = 0.1f + (0.9f * intensity);
                if (intensity > 1.0f) intensity = 1.0f;

                uint8_t r = (uint8_t)(255 * intensity);
                uint8_t g = (uint8_t)(165 * intensity);
                uint8_t b = (uint8_t)(0   * intensity);

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

                // 6. Draw
                renderer.DrawTriangle(
                    (int)p0.x, (int)p0.y, 
                    (int)p1.x, (int)p1.y, 
                    (int)p2.x, (int)p2.y, 
                    color
                );
            }
        }

        renderer.DrawBuffer();

        // Рендерим интерфейс поверх всего
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
