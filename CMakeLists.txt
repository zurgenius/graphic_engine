cmake_minimum_required(VERSION 3.15)
project(MyEngine VERSION 1.0.0 LANGUAGES CXX C)

# 1. Стандарт C++ (аналог -std=c++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Опции компилятора (аналог -Wall -g -Wno-deprecated)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -g -Wno-deprecated)
endif()

# 3. Где искать заголовочные файлы (аналог -I)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/include)

# 4. Находим исходники (аналог *.cpp и glad.c)
# file(GLOB ...) ищет файлы автоматически. 
# Для продакшена лучше перечислять файлы вручную, но для обучения GLOB удобнее.
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")
set(GLAD_SOURCE "${CMAKE_SOURCE_DIR}/glad.c")

# 5. Создаем исполняемый файл "app"
add_executable(app ${SOURCES} ${GLAD_SOURCE})

# 6. Подключаем библиотеки (аналог -L и флагов)

# Путь к твоей dylib библиотеке GLFW
set(GLFW_LIB "${CMAKE_SOURCE_DIR}/dependencies/library/libglfw.3.4.dylib")

target_link_libraries(app PRIVATE ${GLFW_LIB})

# 7. Подключаем macOS фреймворки
if(APPLE)
    target_link_libraries(app PRIVATE "-framework OpenGL")
    target_link_libraries(app PRIVATE "-framework Cocoa")
    target_link_libraries(app PRIVATE "-framework IOKit")
    target_link_libraries(app PRIVATE "-framework CoreVideo")
    target_link_libraries(app PRIVATE "-framework CoreFoundation")
endif()
