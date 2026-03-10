#pragma once
#include "../data/waypoint_node.h"
#include <entt/entt.hpp>
#include <unordered_map>

namespace game::system
{
    class FollowPathSystem
    {
    public:
        void update(entt::registry &registry,
                    entt::dispatcher &dispatcher,
                    std::unordered_map<int, game::data::WaypointNode> &waypoint_nodes);
    };

} // namespace game::system