#include "Renderer.h"

#include <iostream>
#include <execution>

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
    delete[] AcccumulationData_;
    AcccumulationData_ = new glm::vec4[width * height];

    HorizontalIterator_.resize(width);
    VerticalIteractor_.resize(height);
    for(uint32_t i = 0; i < width; i++)
        HorizontalIterator_[i] = i;

    for(uint32_t i = 0; i < height; i++)
        VerticalIteractor_[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
    ActiveScene_ = &scene;
    ActiveCamera_ = &camera;
    if(FrameIndex_ == 1)
        memset(AcccumulationData_, 0, (size_t)FinalImage_->GetWidth() * (size_t)FinalImage_->GetHeight() * sizeof(glm::vec4));
#define MT 1
#if MT
    std::for_each(std::execution::par, VerticalIteractor_.begin(), VerticalIteractor_.end(),
        [this](uint32_t y)
    {
        std::for_each(std::execution::par, HorizontalIterator_.begin(), HorizontalIterator_.end(),
            [this, y](uint32_t x)
        {
            glm::vec4 color = PerPixel(x, y);
            AcccumulationData_[x + y * FinalImage_->GetWidth()] += color;

            glm::vec4 accumulatedColor = AcccumulationData_[x + y * FinalImage_->GetWidth()];
            accumulatedColor /= (float)FrameIndex_;
    
            accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
            ImageData_[x + y * FinalImage_->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
        });
    });
#else 
    for (uint32_t y = 0; y < FinalImage_->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < FinalImage_->GetWidth(); x++)
        {
            glm::vec4 color = PerPixel(x, y);
            AcccumulationData_[x + y * FinalImage_->GetWidth()] += color;

            glm::vec4 accumulatedColor = AcccumulationData_[x + y * FinalImage_->GetWidth()];
            accumulatedColor /= (float)FrameIndex_;
            
            accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
            ImageData_[x + y * FinalImage_->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
        }
    }
#endif
    
    FinalImage_->SetData(ImageData_);

    if(Settings_.Accumulate)
        FrameIndex_++;
    else
        FrameIndex_ = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
    Ray ray;
    ray.Origin = ActiveCamera_->GetPosition();
    ray.Direction = ActiveCamera_->GetRayDirections()[x + y * FinalImage_->GetWidth()];

    glm::vec3 color(0.0f);
    float multiplier = 1.0f;
    int bounces = 5;
    for (int i = 0; i < bounces; i++)
    {
        Renderer::HitPayload payload = TraceRay(ray);
        if(payload.HitDistance < 0.0f)
        {
            glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
            color += skyColor * multiplier;
            break;
        }
    
        glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
        float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);

        const Sphere& sphere = ActiveScene_->Spheres[payload.ObjectIndex];
        const Material& mat = ActiveScene_->Materials[sphere.MaterialIndex];
        glm::vec3 sphereColor = mat.Albedo;
        sphereColor *= lightIntensity;
        color += sphereColor * multiplier;

        multiplier *= 0.5f;

        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
        ray.Direction = glm::reflect(ray.Direction,
            payload.WorldNormal + mat.Roughness * Haketon::Random::Vec3(-0.5f, 0.5f));
    }
    
    return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
    // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    // where
    // a = ray origin
    // b = ray direction
    // r = sphere radius
    // t = hit distance

    int closestSphere = -1;
    float hitDistance = FLT_MAX;
    for (size_t i = 0; i < ActiveScene_->Spheres.size(); i++)
    {
        const Sphere& sphere = ActiveScene_->Spheres[i];
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
        
        if(closestT > 0.0f && closestT < hitDistance)
        {
            hitDistance = closestT;
            closestSphere = (int)i;
        }
    }

    if(closestSphere < 0)
        return Miss(ray);

    return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
    Renderer::HitPayload payload;
    payload.HitDistance = hitDistance;
    payload.ObjectIndex = objectIndex;
    
    const Sphere& closestSphere = ActiveScene_->Spheres[objectIndex];
    glm::vec3 origin = ray.Origin - closestSphere.Position;
    payload.WorldPosition = origin + ray.Direction * hitDistance;
    payload.WorldNormal = glm::normalize(payload.WorldPosition);

    payload.WorldPosition += closestSphere.Position;

    return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
    Renderer::HitPayload payload;
    payload.HitDistance = -1.0f;
    return payload;
}

