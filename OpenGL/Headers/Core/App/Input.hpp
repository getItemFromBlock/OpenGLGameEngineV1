#pragma once

#include <GLFW/glfw3.h>

#include "Core/Maths/Maths.hpp"

namespace Core::App
{
    class Inputs
    {
    public:
        Core::Maths::Vec2D deltaMouse = Core::Maths::Vec2D();
        Core::Maths::Vec2D MousePos = Core::Maths::Vec2D();
        Core::Maths::Int2D ScreenSize = Core::Maths::Vec2D();
        bool leftMouse = false;
        bool rightMouse = false;
        bool mouseCaptured = false;
        float scroll = 0;
        bool forward = false;
        bool backward = false;
        bool up = false;
        bool down = false;
        bool right = false;
        bool left = false;
        bool shift = false;
        bool control = false;

        void UpdateInputs(GLFWwindow* window);
        void InitInputs(GLFWwindow* window, Core::Maths::Int2D defaultSize);
        int GetLastKey();
        void ClearLastKey();
    };
}