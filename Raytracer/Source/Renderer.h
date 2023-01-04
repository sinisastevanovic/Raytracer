#pragma once

#include "Haketon/Image.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render();
    
    std::shared_ptr<Haketon::Image> GetFinalImage() { return FinalImage_; }

private:
    glm::vec4 PerPixel(glm::vec2 coord);
private:
    std::shared_ptr<Haketon::Image> FinalImage_;
    uint32_t* ImageData_ = nullptr;
};
