#include "Camera.h"
#include "Renderer.h"
#include "Haketon/Application.h"
#include "Haketon/EntryPoint.h"
#include "Haketon/Timer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Haketon;

class ExampleLayer : public Layer
{
public:
    ExampleLayer()
        : Camera_(45.0f, 0.1f, 100.0f)
    {
        Scene_.Spheres.push_back({{0.0f, 0.0f, 0.0f}, 0.5f, {1.0f, 0.0f, 1.0f}});
        Scene_.Spheres.push_back({{1.0f, 0.0f, -5.0f}, 1.5f, {0.2f, 0.3f, 1.0f}});
    }
    
    virtual void OnUpdate(float deltaSeconds) override
    {
        Camera_.OnUpdate(deltaSeconds);
    }
    
    virtual void OnUIRender() override
    {
        ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", LastRenderTime_);
        ImGui::End();

        ImGui::Begin("Scene");
        for(size_t i = 0; i < Scene_.Spheres.size(); i++)
        {
            ImGui::PushID(i);
            
            Sphere& sphere = Scene_.Spheres[i];
            ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
            ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
            ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo));

            ImGui::Separator();
            
            ImGui::PopID();
        }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        ViewportWidth_ = ImGui::GetContentRegionAvail().x;
        ViewportHeight_ = ImGui::GetContentRegionAvail().y;

        auto image = Renderer_.GetFinalImage();
        if(image)
            ImGui::Image(image->GetDescriptorSet(), {(float)image->GetWidth(), (float)image->GetHeight()},
            ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
        ImGui::PopStyleVar();

        Render();

        //ImGui::ShowDemoWindow();
    }

    void Render()
    {
        Timer timer;

        Renderer_.OnResize(ViewportWidth_, ViewportHeight_);
        Camera_.OnResize(ViewportWidth_, ViewportHeight_);
        Renderer_.Render(Scene_, Camera_);
        
        LastRenderTime_ = timer.ElapsedMs();
    }

private:
    Renderer Renderer_;
    Camera Camera_;
    Scene Scene_;
    uint32_t ViewportWidth_ = 0;
    uint32_t ViewportHeight_ = 0;

    float LastRenderTime_ = 0.0f;
};

Application* Haketon::CreateApplication(int argc, char** argv)
{
    ApplicationSpecification spec;
    spec.Name = "Raytracer";

    Application* app = new Application(spec);
    app->PushLayer<ExampleLayer>();
    app->SetMenubarCallback([app]()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                app->Close();
            }

            ImGui::EndMenu();
        }
    });

    return app;
}
