#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
    Camera(float verticalFOV, float nearClip, float farClip);

    bool OnUpdate(float deltaSeconds);
    void OnResize(uint32_t width, uint32_t height);

    const glm::mat4& GetProjection() const { return Projection_; }
    const glm::mat4& GetInverseProjection() const { return InverseProjection_; }
    const glm::mat4& GetView() const { return View_; }
    const glm::mat4& GetInverseView() const { return InverseView_; }

    const glm::vec3& GetPosition() const { return Position_; }
    const glm::vec3& GetDirection() const { return ForwardDirection_; }

    const std::vector<glm::vec3>& GetRayDirections() const { return RayDirections_; }

    float GetRotationSpeed();

private:
    void RecalculateProjection();
    void RecalculateView();
    void RecalculateRayDirections();

private:
    glm::mat4 Projection_{1.0f};
    glm::mat4 View_{1.0f};
    glm::mat4 InverseProjection_{1.0f};
    glm::mat4 InverseView_{1.0f};

    float VerticalFOV_ = 45.0f;
    float NearClip_ = 0.1f;
    float FarClip_ = 100.0f;

    glm::vec3 Position_{0.0f};
    glm::vec3 ForwardDirection_{0.0f};

    // Cached ray directions
    std::vector<glm::vec3> RayDirections_;

    glm::vec2 LastMousePosition_{0.0f};

    uint32_t ViewportWidth_ = 0, ViewPortHeight_ = 0;
};
