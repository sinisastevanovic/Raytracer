#pragma once

#include "Haketon/Image.h"

#include <memory>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

class Renderer
{
public:
    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render(const Scene& scene, const Camera& camera);
    
    std::shared_ptr<Haketon::Image> GetFinalImage() { return FinalImage_; }

private:
    struct HitPayload
    {
        float HitDistance;
        glm::vec3 WorldNormal;
        glm::vec3 WorldPosition;

        int ObjectIndex;
    };
    
    glm::vec4 PerPixel(uint32_t x, uint32_t y);
    
    HitPayload TraceRay(const Ray& ray);
    HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
    HitPayload Miss(const Ray& ray);
private:
    std::shared_ptr<Haketon::Image> FinalImage_;

    const Scene* ActiveScene_ = nullptr;
    const Camera* ActiveCamera_ = nullptr;
    
    uint32_t* ImageData_ = nullptr;
};
