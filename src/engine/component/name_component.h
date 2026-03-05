#pragma once
#include <string>
#include <entt/entity/entity.hpp>

namespace engine::component
{
    /**
     * @brief 名称组件
     * @note 包含实体的名称ID和名称字符串。
     */
    struct NameComponent
    {
        entt::id_type name_id_{entt::null}; ///< 名称ID
        std::string name_;                  ///< 名称
    };
} // namespace engine::component
