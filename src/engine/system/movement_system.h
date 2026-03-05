#pragma once
#include <entt/entity/registry.hpp>

namespace engine::system
{

    class MovementSystem
    {
    public:
        void update(entt::registry &registry, float delta_time);
    };
}