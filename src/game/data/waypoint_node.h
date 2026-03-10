#pragma once
#include <glm/vec2.hpp>
#include <vector>

namespace game::data
{
    struct WaypointNode
    {
        int idJ_;                        // 节点在网格中的列索引
        glm::vec2 position_;             // 节点在世界空间中的位置
        std::vector<int> next_node_ids_; // 连接到的下一个节点的ID的列表
    };
} // namespace game::data
