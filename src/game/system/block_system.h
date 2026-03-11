#pragma once

#include <entt/entt.hpp>

namespace game::system
{
    class BlockSystem
    {
    public:
        void update(entt::registry &registry, entt::dispatcher &dispatcher);
    };
} // namespace game::system
