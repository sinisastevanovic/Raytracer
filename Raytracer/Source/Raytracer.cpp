#include "Haketon/Application.h"
#include "Haketon/EntryPoint.h"
#include "Haketon/Image.h"
#include "Haketon/Random.h"
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

        if(Image_)
            ImGui::Image(Image_->GetDescriptorSet(), {(float)Image_->GetWidth(), (float)Image_->GetHeight()});

        ImGui::End();
        ImGui::PopStyleVar();

        Render();

        //ImGui::ShowDemoWindow();
    }

    void Render()
    {
        Timer timer;
        
        if (!Image_ || ViewportWidth_ != Image_->GetWidth() || ViewportHeight_ != Image_->GetHeight())
        {
            Image_ = std::make_shared<Image>(ViewportWidth_, ViewportHeight_, ImageFormat::RGBA);
            delete[] ImageData_;
            ImageData_ = new uint32_t[ViewportWidth_ * ViewportHeight_];
        }

        for (uint32_t i = 0; i < ViewportWidth_ * ViewportHeight_; i++)
        {
            ImageData_[i] = Random::UInt();
            ImageData_[i] |= 0xff000000;
        }

        Image_->SetData(ImageData_);

        LastRenderTime_ = timer.ElapsedMs();
    }

private:
    std::shared_ptr<Image> Image_;
    uint32_t* ImageData_ = nullptr;
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
