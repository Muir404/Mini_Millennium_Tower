#include "entity_builder_TD.h"
#include "../../engine/loader/basic_entity_builder.h"
#include "../../engine/component/tilelayer_component.h"
#include "../data/waypoint_node.h"
#include "../defs/tags.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace game::loader
{
    EntityBuilderTD::EntityBuilderTD(engine::loader::LevelLoader &level_loader,
                                     engine::core::Context &context,
                                     entt::registry &registry,
                                     std::unordered_map<int, game::data::WaypointNode> &waypoint_nodes,
                                     std::vector<int> &start_points)
        : BasicEntityBuilder(level_loader, context, registry),
          waypoint_nodes_(waypoint_nodes),
          start_points_(start_points)
    {
    }

    EntityBuilderTD *EntityBuilderTD::build()
    {
        if (object_json_ && !tile_info_)
        {
            buildPath();
        }
        else
        {
            BasicEntityBuilder::build();
            buildPlace();
        }

        return this;
    }

    void EntityBuilderTD::buildPath()
    {
        // 检查有效性
        if (object_json_->value("point", false) != true)
        {
            return;
        }
        if (!object_json_->contains("properties") || !object_json_->at("properties").is_array())
        {
            return;
        }
        auto id = object_json_->value("id", 0);
        if (id == 0)
        {
            return;
        }

        // 解析数据
        auto position = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
        std::vector<int> next_node_ids;
        for (auto &property : object_json_->at("properties"))
        {
            // 如果是对象类型，并且以next开头，则添加到next_node_ids_
            if (property.value("type", "") == "object" && property.value("name", "").starts_with("next")) // C++20 starts_with：检查字符串是否以指定前缀开头
            {
                auto next_node_id = property.value("value", 0);
                if (next_node_id != 0)
                {
                    next_node_ids.push_back(next_node_id);
                }
            }

            // 如果名称是start且值为true，则将id添加到start_points_
            if (property.value("name", "") == "start" && property.value("value", false) == true)
            {
                start_points_.push_back(id);
            }
        }
        // 添加到waypoint_nodes_
        waypoint_nodes_[id] = game::data::WaypointNode{id, std::move(position), std::move(next_node_ids)};
        spdlog::trace("waypoint_nodes_ size: {}", waypoint_nodes_.size());
    }

    void EntityBuilderTD::buildPlace()
    {
        if (tile_info_ && tile_info_->properties_)
        {
            auto &properties = tile_info_->properties_.value();
            for (auto &property : properties)
            {
                if (property.value("name", "") == "place")
                {
                    auto type = property.value("value", "");
                    if (type == "melee")
                    {
                        registry_.emplace<game::defs::MeleePlaceTag>(entity_id_);
                    }
                    else if (type == "ranged")
                    {
                        registry_.emplace<game::defs::RangedPlaceTag>(entity_id_);
                    }
                }
            }
        }
    }
}