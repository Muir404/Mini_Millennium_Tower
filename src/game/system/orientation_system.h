#pragma once
#include "entt/entity/fwd.hpp"

namespace game::system
{
    class OrientationSystem
    {
    public:
        void update(entt::registry &registry);

    private:
        void updateHasTarget(entt::registry &registry);
        void updateBlocked(entt::registry &registry);
        void updateMoving(entt::registry &registry);
    };
}