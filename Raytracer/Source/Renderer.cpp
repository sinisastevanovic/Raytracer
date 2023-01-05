﻿#include "Renderer.h"

#include <iostream>

#include "Haketon/Random.h"

namespace Utils
{
    static uint32_t ConvertToRGBA(const glm::vec4& color)
    {
        uint8_t r = (uint8_t)(color.r * 255.0f);
        uint8_t g = (uint8_t)(color.g * 255.0f);
        uint8_t b = (uint8_t)(color.b * 255.0f);
        uint8_t a = (uint8_t)(color.a * 255.0f);

        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        return result;
    }
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    if(FinalImage_)
    {
        if(FinalImage_->GetWidth() == width && FinalImage_->GetHeight() == height)
            return;
        
        FinalImage_->Resize(width, height);
    }
    else
    {
        FinalImage_ = std::make_shared<Haketon::Image>(width, height, Haketon::ImageFormat::RGBA);
    }

    delete[] ImageData_;
    ImageData_ = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
    Ray ray;
    ray.Origin = camera.GetPosition();
    
    for (uint32_t y = 0; y < FinalImage_->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < FinalImage_->GetWidth(); x++)
        {
            ray.Direction = camera.GetRayDirections()[x + y * FinalImage_->GetWidth()];
            glm::vec4 color = TraceRay(scene, ray);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            ImageData_[x + y * FinalImage_->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }

    FinalImage_->SetData(ImageData_);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
    // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    // where
    // a = ray origin
    // b = ray direction
    // r = sphere radius
    // t = hit distance

    if(scene.Spheres.size() == 0)
        return glm::vec4(0, 0, 0, 1);

    const Sphere* closestSphere = nullptr;
    float hitDistance = FLT_MAX;
    for (const Sphere& sphere : scene.Spheres)
    {
        glm::vec3 origin = ray.Origin - sphere.Position;
        
        float a = glm::dot(ray.Direction, ray.Direction);
        float b = 2.0f * glm::dot(origin, ray.Direction);
        float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

        // Quadratic formula discriminant:
        // b^2 - 4ac

        float discriminant = b * b - 4.0f * a * c;

        if(discriminant < 0.0f)
            continue;

        //float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
        float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
        
        if(closestT < hitDistance)
        {
            hitDistance = closestT;
            closestSphere = &sphere;
        }
    }

    if(closestSphere == nullptr)
        return glm::vec4(0, 0, 0, 1);

    glm::vec3 origin = ray.Origin - closestSphere->Position;
    glm::vec3 hitPosition = origin + ray.Direction * hitDistance;
    glm::vec3 normal = glm::normalize(hitPosition);

    glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

    float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.0f);

    glm::vec3 sphereColor = closestSphere->Albedo;
    sphereColor *= lightIntensity;
    
    return glm::vec4(sphereColor, 1.0f);
    
}
