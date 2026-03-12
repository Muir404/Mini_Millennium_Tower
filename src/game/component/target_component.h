#pragma once
#include <entt/entity/entity.hpp>

namespace game::component
{
    /**
     * @brief 目标组件，存储目标实体(攻击对象)
     */
    struct TargetComponent
    {
        entt::entity entity_{entt::null};
    };
} // namespace game::component
