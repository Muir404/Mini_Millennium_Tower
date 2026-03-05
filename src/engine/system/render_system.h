#pragma once
#include <entt/entt.hpp>

namespace engine::render
{
    class Renderer;
    class Camera;
}

namespace engine::system
{

    class RenderSystem
    {
    public:
        void update(entt::registry &registry, render::Renderer &renderer, const render::Camera &camera);
    };

} // namespace engine::system