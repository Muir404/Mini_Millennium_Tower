#pragma once
#include "animation_component.h"
#include "sprite_component.h"
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <utility>
#include <optional>
#include <SDL3/SDL_rect.h>
#include <nlohmann/json.hpp>

namespace engine::component
{

    /**
     * @brief 定义瓦片的类型，用于游戏逻辑（例如碰撞）。
     */
    enum class TileType
    {
        EMPTY,  ///< @brief 空白瓦片
        NORMAL, ///< @brief 普通瓦片
        SOLID,  ///< @brief 静止可碰撞瓦片
        HAZARD, ///< @brief 危险瓦片（例如火焰、尖刺等）
        // TODO: 补充其它类型
    };

    /**
     * @brief 组件-瓦片信息组件
     * @note 用于存储瓦片的精灵、类型、动画和属性
     */
    struct TileInfo
    {
        engine::component::Sprite sprite_;                      ///< 精灵
        engine::component::TileType type_;                      ///< 类型
        std::optional<engine::component::Animation> animation_; ///< 动画
        std::optional<nlohmann::json> properties_;              ///< 属性

        TileInfo() = default;

        TileInfo(engine::component::Sprite sprite,
                 engine::component::TileType type,
                 std::optional<engine::component::Animation> animation = std::nullopt,
                 std::optional<nlohmann::json> properties = std::nullopt) : sprite_(std::move(sprite)),
                                                                            type_(type),
                                                                            animation_(std::move(animation)),
                                                                            properties_(std::move(properties)) {}
    };

    /**
     * @brief 组件-瓦片层组件
     * @note 瓦片层是一个二维数组，每个元素对应一个瓦片实体
     */
    struct TileLayerComponent
    {
        glm::ivec2 tile_size_;            ///< 瓦片大小
        glm::ivec2 map_size_;             ///< 地图大小
        std::vector<entt::entity> tiles_; ///< 瓦片实体列表，每个瓦片对应一个实体，按顺序排列

        TileLayerComponent(glm::ivec2 tile_size,
                           glm::ivec2 map_size,
                           std::vector<entt::entity> tiles) : tile_size_(std::move(tile_size)),
                                                              map_size_(std::move(map_size)),
                                                              tiles_(std::move(tiles)) {}
    };
} // namespace engine::component