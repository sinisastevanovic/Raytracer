#include "Renderer.h"
#include "Haketon/Application.h"
#include "Haketon/EntryPoint.h"
#include "Haketon/Timer.h"

using namespace Haketon;

class ExampleLayer : public Layer
{
public:
    virtual void OnUIRender() override
    {
        ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", LastRenderTime_);
        if (ImGui::Button("Render"))
        {
            Render();
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
        Renderer_.Render();
        
        LastRenderTime_ = timer.ElapsedMs();
    }

private:
    Renderer Renderer_;
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
