#pragma once
#include "image.h"
#include "../component/sprite_component.h"
#include "../utils/math.h"
#include <optional>

struct SDL_Renderer;
struct SDL_FRect;
struct SDL_FColor;

namespace engine::resource
{
    class ResourceManager;
}

namespace engine::render
{
    class Camera;

    class Renderer final
    {
    private:
        SDL_Renderer *renderer_ = nullptr;
        engine::resource::ResourceManager *resource_manager_ = nullptr; // 借用的指针

        engine::utils::FColor background_color_ = {0.0f, 0.0f, 0.0f, 1.0f}; // 默认背景颜色

    public:
        Renderer(SDL_Renderer *sdl_renderer,
                 engine::resource::ResourceManager *resource_manager);

        // 禁用拷贝和移动语义
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void drawSprite(const Camera &camera,
                        const component::Sprite &sprite,
                        const glm::vec2 &position,
                        const glm::vec2 &size,
                        const float rotation = 0.0f,
                        const engine::utils::FColor &color = engine::utils::FColor::WHITE());

        void drawUIImage(const Image &image,
                         const glm::vec2 &position,
                         const std::optional<glm::vec2> &size = std::nullopt);

        void drawUIFilledRect(const engine::utils::Rect &rect,
                              const engine::utils::FColor &color);

        void drawFilledRect(const Camera &camera,
                            const glm::vec2 &position,
                            const glm::vec2 &size,
                            const engine::utils::FColor &color);

        void drawRect(const Camera &camera,
                      const glm::vec2 &position,
                      const glm::vec2 &size,
                      const engine::utils::FColor &color, const int thickness = 1);

        void drawFilledCircle(const Camera &camera,
                              const glm::vec2 &position,
                              const float radius,
                              const engine::utils::FColor &color = engine::utils::FColor::WHITE());

        void present();     // 更新屏幕，包装SDL_RenderPresent
        void clearScreen(); // 清屏，包装SDL_RenderClear

        SDL_Renderer *getSDLRenderer() const { return renderer_; } ///< 获取底层的 SDL_Renderer 指针

        void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);       // 包装SDL_SetRenderDrawColor
        void setDrawColorFloat(float r, float g, float b, float a = 1.0f); // 包装SDL_SetRenderDrawColorFloat

        void setBackgroundColor(float r, float g, float b, float a = 1.0f); // 设置背景颜色

    private:
        std::optional<SDL_FRect> getImageSrcRect(const Image &image);       // 获取图片源矩形，用于绘制
        bool isRectInViewport(const Camera &camera, const SDL_FRect &rect); // 判断矩形是否在视口内
    };
}
