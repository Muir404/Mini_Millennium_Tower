#pragma once

namespace game::component
{
    struct EnemyComponent
    {
        int target_waypoint_id_{}; ///< 目标路径节点索引
        float speed_{};            ///< 移动速度
    };
} // namespace game::component
