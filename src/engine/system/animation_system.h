#pragma once
#include <entt/entity/fwd.hpp>

namespace engine::system
{

    class AnimationSystem
    {
    public:
        void update(entt::registry &registry, float dt);
    };

} // namespace engine::system