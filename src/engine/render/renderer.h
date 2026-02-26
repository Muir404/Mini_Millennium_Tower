#pragma once
#include <string>
#include <optional> // For std::optional
#include "../utils/math.h"
#include "sprite.h"

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

    public:
        Renderer(SDL_Renderer *sdl_renderer,
                 engine::resource::ResourceManager *resource_manager);

        // 禁用拷贝和移动语义
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void drawSprite(const Camera &camera,
                        const Sprite &sprite,
                        const glm::vec2 &position,
                        const glm::vec2 &scale = {1.0f, 1.0f},
                        double angle = 0.0f);

        void drawParallax(
            const Camera &camera,
            const Sprite &sprite,
            const glm::vec2 &position,
            const glm::vec2 &scroll_factor,
            const glm::bvec2 &repeat = {true, true},
            const glm::vec2 &scale = {1.0f, 1.0f});

        void drawUISprite(const Sprite &sprite,
                          const glm::vec2 &position,
                          const std::optional<glm::vec2> &size = std::nullopt);

        void drawUIFilledRect(const engine::utils::Rect &rect,
                              const engine::utils::FColor &color);

        void present();     // 更新屏幕，包装SDL_RenderPresent
        void clearScreen(); // 清屏，包装SDL_RenderClear

        SDL_Renderer *getSDLRenderer() const { return renderer_; } ///< 获取底层的 SDL_Renderer 指针

        void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);       // 包装SDL_SetRenderDrawColor
        void setDrawColorFloat(float r, float g, float b, float a = 1.0f); // 包装SDL_SetRenderDrawColorFloat

    private:
        std::optional<SDL_FRect> getSpriteSrcRect(const Sprite &sprite);    // 获取精灵源矩形，用于绘制
        bool isRectInViewport(const Camera &camera, const SDL_FRect &rect); // 判断矩形是否在视口内
    };
}
