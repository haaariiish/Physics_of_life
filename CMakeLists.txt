cmake_minimum_required(VERSION 3.16)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)


find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(Physics_of_life
    src/main.cpp
    src/maths/Vec2.cpp
    src/physics/Body.cpp
    src/physics/World.cpp
    src/entities/Entity.cpp
    src/entities/Particule.cpp
    src/entities/BallCells.cpp
)

target_include_directories(Physics_of_life PRIVATE src)

# Lie SFML à ton exécutable
target_link_libraries(Physics_of_life PRIVATE SFML::Graphics SFML::Window SFML::System)