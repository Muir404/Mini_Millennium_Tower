#pragma once
#include <unordered_map>
#include <vector>
#include "../../engine/loader/level_loader.h"  // 确保路径正确
#include "../../engine/core/context.h"
#include "../data/waypoint_node.h"  // 假设 WaypointNode 在这里

namespace game::loader
{
    class EntityBuilderTD final : public engine::loader::BasicEntityBuilder
    {
    private:
        std::unordered_map<int, game::data::WaypointNode> &waypoint_nodes_;
        std::vector<int> &start_points_;

    public:
        EntityBuilderTD(engine::loader::LevelLoader &level_loader,
                        engine::core::Context &context,
                        entt::registry &registry,
                        std::unordered_map<int, game::data::WaypointNode> &waypoint_nodes,
                        std::vector<int> &start_points);

        ~EntityBuilderTD() = default;

        EntityBuilderTD *build() override;

    private:
        void buildPath(); ///< 生成路径节点
    };
}