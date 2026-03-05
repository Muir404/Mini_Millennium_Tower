#pragma once
#include <entt/entity/fwd.hpp>
#include <unordered_map>

namespace engine::component
{
    /**
     * @brief 组件-音频组件
     * @note 包含实体的音效集合，名称(哈希) -> 音效ID。
     */
    struct AudioComponent
    {
        std::unordered_map<entt::id_type, entt::id_type> sounds_; ///< 音效集合，名称(哈希) -> 音效ID
    };
} // namespace engine::component