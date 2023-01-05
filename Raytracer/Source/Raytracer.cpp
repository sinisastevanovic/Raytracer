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
        {
            Material mat;
            mat.Albedo = {1.0f, 0.0f, 1.0f};
            mat.Roughness = 0.0f;
            mat.Metallic = 1.0f;
            Scene_.Materials.push_back(mat);
        }

        {
            Material mat;
            mat.Albedo = {0.2f, 0.3f, 1.0f};
            mat.Roughness = 0.2f;
            mat.Metallic = 0.0f;
            Scene_.Materials.push_back(mat);
        }
        
        {
            Sphere sphere;
            sphere.Position = {0.0f, 0.0f, 0.0f};
            sphere.Radius = 1.0f;
            sphere.MaterialIndex = 0;
            Scene_.Spheres.push_back(sphere);
        }

        {
            Sphere sphere;
            sphere.Position = {0.0f, -101.0f, 0.0f};
            sphere.Radius = 100.0f;
            sphere.MaterialIndex = 1;
            Scene_.Spheres.push_back(sphere);
        }
    }

    virtual void OnUpdate(float deltaSeconds) override
    {
        if(Camera_.OnUpdate(deltaSeconds))
        {
            Renderer_.ResetFrameIndex();
        }
    }

    virtual void OnUIRender() override
    {
        ImGui::Begin("Settings");
        ImGui::Text("Frametime: %.3fms", LastRenderTime_);
        ImGui::Checkbox("Accumulate", &Renderer_.GetSettings().Accumulate);
        if(ImGui::Button("Reset"))
        {
            Renderer_.ResetFrameIndex();
        }
        ImGui::End();

        ImGui::Begin("Scene");
        for (size_t i = 0; i < Scene_.Spheres.size(); i++)
        {
            ImGui::PushID((int)i);

            Sphere& sphere = Scene_.Spheres[i];
            ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
            ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
            ImGui::DragInt("Material", &sphere.MaterialIndex, 0.1f, 0, (int)Scene_.Materials.size() - 1);

            ImGui::Separator();

            ImGui::PopID();
        }
    
        for (size_t i = 0; i < Scene_.Materials.size(); i++)
        {
            ImGui::PushID((int)i);

            Material& mat = Scene_.Materials[i];
            ImGui::ColorEdit3("Albedo", glm::value_ptr(mat.Albedo));
            ImGui::DragFloat("Roughness", &mat.Roughness, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Metallic", &mat.Metallic, 0.01f, 0.0f, 1.0f);

            ImGui::Separator();
            
            ImGui::PopID();
        }
        
        
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        ViewportWidth_ = (int)ImGui::GetContentRegionAvail().x;
        ViewportHeight_ = (int)ImGui::GetContentRegionAvail().y;

        auto image = Renderer_.GetFinalImage();
        if (image)
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
