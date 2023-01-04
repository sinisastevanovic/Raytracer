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

void Renderer::Render()
{
    for (uint32_t y = 0; y < FinalImage_->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < FinalImage_->GetWidth(); x++)
        {
            glm::vec2 coord = { (float)x / (float)FinalImage_->GetWidth(), (float)y / (float)FinalImage_->GetHeight() };
            coord = coord * 2.0f - 1.0f; // -1 -> 1
            glm::vec4 color = PerPixel(coord);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            ImageData_[x + y * FinalImage_->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }

    FinalImage_->SetData(ImageData_);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
    glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
    glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
    glm::vec3 sphereOrigin(0.0f, 0.0f, 0.0f);
    float radius = 0.5f;
    //rayDirection = glm::normalize(rayDirection);

    // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    // where
    // a = ray origin
    // b = ray direction
    // r = sphere radius
    // t = hit distance

    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(rayOrigin, rayDirection);
    float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

    // Quadratic formula discriminant:
    // b^2 - 4ac

    float discriminant = b * b - 4.0f * a * c;

    if(discriminant < 0.0f)
        return glm::vec4(0, 0, 0, 1);

    glm::vec3 sphereColor(1, 0, 1);
    
    float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
    float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

    glm::vec3 hitPosition = rayOrigin + rayDirection * closestT;
    glm::vec3 normal = glm::normalize(hitPosition - sphereOrigin);

    glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

    float d = glm::max(glm::dot(normal, -lightDir), 0.0f);

    sphereColor *= d;
    
    return glm::vec4(sphereColor, 1.0f);
    
}