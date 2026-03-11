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

}

namespace game::factory
{
    class BlueprintManager
    {
        friend class EntityFactory;

    private:
        engine::resource::ResourceManager &resource_manager_;
        std::unordered_map<entt::id_type, data::EnemyClassBlueprint> enemy_class_blurprints_;

    public:
        BlueprintManager(engine::resource::ResourceManager &resource_manager);

        [[nodiscard]] bool loadEnemyClassBlueprints(std::string_view enemy_json_path);
        const data::EnemyClassBlueprint &getEnemyClassBlueprint(entt::id_type id) const;

    private:
        data::StatsBlueprint parseStats(const nlohmann::json &json);
        data::SpriteBlueprint parseSprite(const nlohmann::json &json);
        std::unordered_map<entt::id_type, data::AnimationBlueprint> parseAnimationsMap(const nlohmann::json &json);
        data::SoundBlueprint parseSound(const nlohmann::json &json);
        data::EnemyBlueprint parseEnemy(const nlohmann::json &json);
        data::DisplayInfoBlueprint parseDisplayInfo(const nlohmann::json &json);
    };

} // namespace game::factory
