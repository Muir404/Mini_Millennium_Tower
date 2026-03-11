#pragma once
#include "../utils/events.h"
#include <entt/entt.hpp>

namespace engine::system
{

    class AnimationSystem
    {
        entt::registry &registry_;
        entt::dispatcher &dispatcher_;

    public:
        AnimationSystem(entt::registry &registry, entt::dispatcher &dispatcher);
        ~AnimationSystem();

        void update(float dt);

    private:
        void onPlayAnimationEvent(const engine::utils::PlayAnimationEvent &event);
    };

} // namespace engine::system