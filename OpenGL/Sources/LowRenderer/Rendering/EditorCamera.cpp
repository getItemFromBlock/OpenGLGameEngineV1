#include "LowRenderer/Rendering/EditorCamera.hpp"

#include <ImGUI/imgui.h>

using namespace Core::Maths;

bool active = false;
float gTime = 0;
float posY = 0;
float posY2 = 0;
Vec2D ballPos = Vec2D();
Vec2D ballVel = Vec2D();
void LowRenderer::Rendering::EditorCamera::RenderGUI()
{
    Camera::RenderGUI();
    ImGui::DragFloat3("Camera Rotation", &rotation.x, 0.1f);
    ImGui::DragFloat("Camera Distance", &distance, 0.1f);
    ImGui::DragFloat("Movement Speed", &MovementSpeed, 0.1f);
    ImGui::DragFloat("Rotation Speed", &RotationSpeed, 0.1f);
    if (ImGui::Button("Special Mode"))
    {
        active = true;
        posY = Resolution.y / 2.0f;
        posY2 = Resolution.y / 2.0f;
        ballPos = Vec2D(Resolution.x / 2.0f, Resolution.y / 2.0f);
        ballVel = Vec2D(-2.0f, -1.0f).unitVector();
    }
    if (active)
    {
        ImGui::SetNextWindowSize(ImVec2((float)Resolution.x, (float)Resolution.y));
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowFocus();
        ImGui::Begin("Pong", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
        ImGui::SetWindowFontScale(4.0f);
        ImGui::SetCursorPos(ImVec2(15, posY));
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "D\nD");
        ImGui::SetCursorPos(ImVec2((float)Resolution.x-45, posY2));
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "D\nD");
        ImGui::SetCursorPos(ImVec2(ballPos.x, ballPos.y));
        Vec3D color;
        ImGui::ColorConvertHSVtoRGB(Util::mod(gTime,1.0f),1.0f, 1.0f, color.x, color.y, color.z);
        ImGui::TextColored(ImVec4(color.x, color.y, color.z, 1.0f), "/-\\\n\\-/");
        ImGui::End();
    }
}

void LowRenderer::Rendering::EditorCamera::Update(const Core::App::Inputs& inputs, const float deltaTime)
{
    gTime += deltaTime;
#ifndef GAME
    if (active)
    {
        posY -= (inputs.up | inputs.forward) * deltaTime * Resolution.y * 0.4f;
        posY = Util::cut(posY, 0.0f, Resolution.y - 90.0f);
        posY += (inputs.down | inputs.backward) * deltaTime * Resolution.y * 0.4f;
        if (posY2 > ballPos.y)
        {
            posY2 -= deltaTime * Resolution.y * 0.3f;
        }
        if (posY2 < ballPos.y)
        {
            posY2 += deltaTime * Resolution.y * 0.3f;
        }
        ballPos = ballPos + ballVel * Resolution * deltaTime * 0.45f;
        if (ballPos.x < 0)
        {
            active = false;
            rotation = rotation * 3.157f + position * 21.0f + focus * 18.7856f;
            rotation = Vec3D(Util::mod(rotation.x, 360), Util::mod(rotation.y, 360), Util::mod(rotation.z, 360));
        }
        if (ballPos.x > Resolution.x - 70) active = false;
        if (ballPos.y < 0 || ballPos.y > Resolution.y - 90)
        {
            ballPos.y = Util::cut(ballPos.y, 0, (float)Resolution.y - 90);
            ballVel.y = -ballVel.y;
        }
        if (ballPos.x < 45 && ballPos.y > posY - 45 && ballPos.y < posY + 45)
        {
            ballVel = Vec2D(1.0, (ballPos.y - posY) / 12.0f).unitVector();
        }
        if (ballPos.x > Resolution.x - 135 && ballPos.y > posY2 - 45 && ballPos.y < posY2 + 45)
        {
            ballVel = Vec2D(-1.0, (ballPos.y - posY2) / 12.0f).unitVector();
        }
    }
    if (inputs.mouseCaptured)
    {
        distance = Util::cut(distance - inputs.scroll / 3, 0.001f, 100.0f);
        // Update rotation.
        rotation = rotation + Vec3D(-inputs.deltaMouse.x, inputs.deltaMouse.y, 0) * RotationSpeed;
        rotation = Vec3D(Util::mod(rotation.x, 360), Util::mod(rotation.y, 360), Util::mod(rotation.z, 360));
        // Update focus.
        float dSpeed = deltaTime * MovementSpeed * (inputs.shift ? 5.0f : (inputs.control ? 0.2f : 1.0f));
        Vec3D delta = Vec3D(dSpeed * inputs.right - dSpeed * inputs.left, dSpeed * inputs.up - dSpeed * inputs.down, dSpeed * inputs.backward - dSpeed * inputs.forward) / 20;
        focus = focus + Vec3D(cosf(Util::toRadians(rotation.x)) * delta.x + sinf(Util::toRadians(rotation.x)) * delta.z, delta.y, -sinf(Util::toRadians(rotation.x)) * delta.x + cosf(Util::toRadians(rotation.x)) * delta.z);
        // Update Position.
    }
#endif // _DEBUG
    // Update distance.
    Mat4D Rot = Core::Maths::Mat4D::CreateRotationMatrix(Vec3D(-rotation.y, rotation.x, rotation.z));
    position = focus + (Rot * Core::Maths::Vec3D(0, 0, distance)).getVector();//Vec3D(sinf(Util::toRadians(rotation.x)) * cosf(Util::toRadians(rotation.y)), sinf(Util::toRadians(rotation.y)), cosf(Util::toRadians(rotation.x)) * cosf(Util::toRadians(rotation.y)))* (distance == 0.0f ? 0.001f : distance);
    aspect_ratio = inputs.ScreenSize.x * 1.0f / inputs.ScreenSize.y;
    Resolution = inputs.ScreenSize;
    deltaUp = (Rot * up).getVector();
}