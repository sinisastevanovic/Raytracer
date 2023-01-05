#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Haketon/Input/Input.h"

Camera::Camera(float verticalFOV, float nearClip, float farClip)
    : VerticalFOV_(verticalFOV), NearClip_(nearClip), FarClip_(farClip)
{
    ForwardDirection_ = glm::vec3(0, 0, -1);
    Position_ = glm::vec3(0, 0, 3);
}

void Camera::OnUpdate(float deltaSeconds)
{
    glm::vec2 mousePos = Haketon::Input::GetMousePosition();
    glm::vec2 mousePosDelta = (mousePos - LastMousePosition_) * 0.002f;
    LastMousePosition_ = mousePos;

    if (!Haketon::Input::IsMouseButtonDown(Haketon::MouseButton::Right))
    {
        Haketon::Input::SetCursorMode(Haketon::CursorMode::Normal);
        return;
    }

    Haketon::Input::SetCursorMode(Haketon::CursorMode::Locked);

    bool moved = false;

    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 rightDirection = glm::cross(ForwardDirection_, upDirection);

    float speed = 5.0f;

    // Movement
    if (Haketon::Input::IsKeyDown(Haketon::KeyCode::W))
    {
        Position_ += ForwardDirection_ * speed * deltaSeconds;
        moved = true;
    }
    else if (Haketon::Input::IsKeyDown(Haketon::KeyCode::S))
    {
        Position_ -= ForwardDirection_ * speed * deltaSeconds;
        moved = true;
    }
    else if (Haketon::Input::IsKeyDown(Haketon::KeyCode::A))
    {
        Position_ -= rightDirection * speed * deltaSeconds;
        moved = true;
    }
    else if (Haketon::Input::IsKeyDown(Haketon::KeyCode::D))
    {
        Position_ += rightDirection * speed * deltaSeconds;
        moved = true;
    }
    else if (Haketon::Input::IsKeyDown(Haketon::KeyCode::Q))
    {
        Position_ -= upDirection * speed * deltaSeconds;
        moved = true;
    }
    else if (Haketon::Input::IsKeyDown(Haketon::KeyCode::E))
    {
        Position_ += upDirection * speed * deltaSeconds;
        moved = true;
    }

    // Rotation
    if (mousePosDelta.x != 0.0f || mousePosDelta.y != 0.0f)
    {
        float pitchDelta = mousePosDelta.y * GetRotationSpeed();
        float yawDelta = mousePosDelta.x * GetRotationSpeed();

        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
                                                glm::angleAxis(-yawDelta, glm::vec3(0.0f, 1.0f, 0.0f))));
        ForwardDirection_ = glm::rotate(q, ForwardDirection_);

        moved = true;
    }

    if (moved)
    {
        RecalculateView();
        RecalculateRayDirections();
    }
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
    if (width == ViewportWidth_ && height == ViewPortHeight_)
        return;

    ViewportWidth_ = width;
    ViewPortHeight_ = height;

    RecalculateProjection();
    RecalculateRayDirections();
}

float Camera::GetRotationSpeed()
{
    return 0.3f;
}

void Camera::RecalculateProjection()
{
    Projection_ = glm::perspectiveFov(glm::radians(VerticalFOV_), (float)ViewportWidth_, (float)ViewPortHeight_, NearClip_, FarClip_);
    InverseProjection_ = glm::inverse(Projection_);
}

void Camera::RecalculateView()
{
    View_ = glm::lookAt(Position_, Position_ + ForwardDirection_, glm::vec3(0, 1, 0));
    InverseView_ = glm::inverse(View_);
}

void Camera::RecalculateRayDirections()
{
    RayDirections_.resize(ViewportWidth_ * ViewPortHeight_);

    for(uint32_t y = 0; y < ViewPortHeight_; y++)
    {
        for(uint32_t x = 0; x < ViewportWidth_; x++)
        {
            glm::vec2 coord = { (float)x / (float)ViewportWidth_, (float)y / (float)ViewPortHeight_ };
            coord = coord * 2.0f - 1.0f; // -1 - 1

            glm::vec4 target = InverseProjection_ * glm::vec4(coord.x, coord.y, 1, 1);
            glm::vec3 rayDir = glm::vec3(InverseView_ * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
            RayDirections_[x + y * ViewportWidth_] = rayDir;
        }
    }
}
