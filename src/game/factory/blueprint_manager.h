#pragma once
#include <unordered_map>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../data/entity_blueprint.h"

namespace engine::resource
{
    class ResourceManager;
}

namespace game::data
{
    struct EnemyClassBlueprint;
    struct PlayerClassBlueprint;
}

namespace game::factory
{
    class BlueprintManager
    {
        friend class EntityFactory;

    private:
        engine::resource::ResourceManager &resource_manager_; ///< 资源管理器引用

        std::unordered_map<entt::id_type, data::EnemyClassBlueprint> enemy_class_blueprints_;   ///< 敌人职业蓝图映射
        std::unordered_map<entt::id_type, data::PlayerClassBlueprint> player_class_blueprints_; ///< 玩家职业蓝图映射
        std::unordered_map<entt::id_type, data::ProjectileBlueprint> projectile_blueprints_;    ///< 投射物蓝图映射
        std::unordered_map<entt::id_type, data::EffectBlueprint> effect_blueprints_;            ///< 效果蓝图映射

    public:
        BlueprintManager(engine::resource::ResourceManager &resource_manager);

        [[nodiscard]] bool loadEnemyClassBlueprints(std::string_view enemy_json_path);      ///< 加载敌人职业蓝图
        [[nodiscard]] bool loadPlayerClassBlueprints(std::string_view player_json_path);    ///< 加载玩家职业蓝图
        [[nodiscard]] bool loadProjectileBlueprints(std::string_view projectile_json_path); ///< 加载投射物蓝图
        [[nodiscard]] bool loadEffectBlueprints(std::string_view effect_json_path);         ///< 加载效果蓝图

        const data::EnemyClassBlueprint &getEnemyClassBlueprint(entt::id_type id) const;   ///< 获取敌人职业蓝图
        const data::PlayerClassBlueprint &getPlayerClassBlueprint(entt::id_type id) const; ///< 获取玩家职业蓝图
        const data::ProjectileBlueprint &getProjectileBlueprint(entt::id_type id) const;   ///< 获取投射物蓝图
        const data::EffectBlueprint &getEffectBlueprint(entt::id_type id) const;           ///< 获取效果蓝图

    private:
        entt::id_type parseProjectileId(const nlohmann::json &json);                                                ///< 解析投射物ID
        data::StatsBlueprint parseStats(const nlohmann::json &json);                                                ///< 解析统计数据蓝图
        data::SpriteBlueprint parseSprite(const nlohmann::json &json);                                              ///< 解析精灵蓝图
        std::unordered_map<entt::id_type, data::AnimationBlueprint> parseAnimationsMap(const nlohmann::json &json); ///< 解析动画映射
        data::SoundBlueprint parseSound(const nlohmann::json &json);                                                ///< 解析音效蓝图
        data::EnemyBlueprint parseEnemy(const nlohmann::json &json);                                                ///< 解析敌人蓝图
        data::PlayerBlueprint parsePlayer(const nlohmann::json &json);                                              ///< 解析玩家蓝图
        data::DisplayInfoBlueprint parseDisplayInfo(const nlohmann::json &json);                                    ///< 解析显示信息蓝图
        data::AnimationBlueprint parseOneAnimation(const nlohmann::json &json);                                     ///< 解析单一动画蓝图
    };

} // namespace game::factory
