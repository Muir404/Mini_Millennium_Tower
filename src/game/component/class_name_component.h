#pragma once
#include <string>
#include <entt/entt.hpp>

namespace game::component
{
    /**
     * 职业名称组件, 用于存储玩家职业的名称
     */
    struct ClassNameComponent
    {
        entt::id_type class_id_{entt::null}; ///< 职业ID
        std::string class_name_{""};         ///< 职业名称
    };
}