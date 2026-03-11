#include "blueprint_manager.h"
#include "../../engine/resource/resource_manager.h"
#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>
#include <entt/entt.hpp>

namespace game::factory
{
    BlueprintManager::BlueprintManager(engine::resource::ResourceManager &resource_manager)
        : resource_manager_(resource_manager)
    {
    }
    bool BlueprintManager::loadEnemyClassBlueprints(std::string_view enemy_json_path)
    {
        auto path = std::filesystem::path(enemy_json_path);
        std::ifstream file(path);
        nlohmann::json json;
        file >> json;
        file.close();

        // --- 蓝图解析 ---
        try
        {
            for (auto &[class_name, data_json] : json.items())
            {
                entt::id_type class_id = entt::hashed_string(class_name.c_str());

                // 解析 stats
                data::StatsBlueprint stats = parseStats(data_json);

                // 解析 sprite
                data::SpriteBlueprint sprite = parseSprite(data_json);

                // 解析 animations
                std::unordered_map<entt::id_type, data::AnimationBlueprint> animations = parseAnimationsMap(data_json);

                // 解析 sound
                data::SoundBlueprint sounds = parseSound(data_json);

                // 解析 enemy
                data::EnemyBlueprint enemy = parseEnemy(data_json);

                // 解析 display_info
                data::DisplayInfoBlueprint display_info = parseDisplayInfo(data_json);

                enemy_class_blurprints_.emplace(class_id, data::EnemyClassBlueprint{class_id,
                                                                                    class_name,
                                                                                    std::move(stats),
                                                                                    std::move(enemy),
                                                                                    std::move(sounds),
                                                                                    std::move(sprite),
                                                                                    std::move(display_info),
                                                                                    std::move(animations)});
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("[BlueprintManager] 加载敌人类蓝图失败: {}", e.what());
            return false;
        }
        return true;
    }
    const data::EnemyClassBlueprint &BlueprintManager::getEnemyClassBlueprint(entt::id_type id) const
    {
        if (auto it = enemy_class_blurprints_.find(id); it != enemy_class_blurprints_.end())
        {
            return it->second;
        }
        spdlog::error("[BlueprintManager] 未找到对应ID敌人类蓝图: {}", id);
        return enemy_class_blurprints_.begin()->second;
    }

    data::StatsBlueprint BlueprintManager::parseStats(const nlohmann::json &json)
    {
        float hp = json["hp"].get<float>();
        float atk = json["atk"].get<float>();
        float def = json["def"].get<float>();
        float range = json["range"].get<float>();
        float atk_interval = json["atk_interval"].get<float>();

        return data::StatsBlueprint{hp,
                                    atk,
                                    def,
                                    range,
                                    atk_interval};
    }

    data::SpriteBlueprint BlueprintManager::parseSprite(const nlohmann::json &json)
    {
        auto width = json["width"].get<float>();
        auto height = json["height"].get<float>();
        auto path_str = json["sprite_sheet"].get<std::string>();
        auto path_id = entt::hashed_string(path_str.c_str());

        return data::SpriteBlueprint{path_id,
                                     path_str,
                                     engine::utils::Rect{glm::vec2(json.value("x", 0.0f),
                                                                   json.value("y", 0.0f)),
                                                         glm::vec2(width, height)},
                                     glm::vec2(json.value("size_x", width),
                                               json.value("size_y", height)),
                                     glm::vec2(json.value("offset_x", 0.0f),
                                               json.value("offset_y", 0.0f)),
                                     json.value("face_right", true)};
    }

    std::unordered_map<entt::id_type, data::AnimationBlueprint> BlueprintManager::parseAnimationsMap(const nlohmann::json &json)
    {
        std::unordered_map<entt::id_type, data::AnimationBlueprint> animations;
        for (auto &[anim_name, anim_data] : json["animation"].items())
        {
            // 解析具体的动画
            auto anim_name_id = entt::hashed_string(anim_name.c_str());
            // 动画帧信息
            std::vector<int> frames = anim_data["frames"].get<std::vector<int>>();
            // 创建单个动画蓝图并插入容器
            data::AnimationBlueprint animation{anim_data.value("duration", 100.0f),
                                               anim_data.value("row", 0),
                                               std::move(frames)};
            animations.emplace(anim_name_id, animation);
        }
        return animations;
    }

    data::SoundBlueprint BlueprintManager::parseSound(const nlohmann::json &json)
    {
        data::SoundBlueprint sounds;
        if (json.contains("sounds"))
        { // 如果包含音效
            // 遍历所有音效键值对
            for (auto &[sound_key, sound_value] : json["sounds"].items())
            {
                // 先把 sound_value 看成是音效路径并通过资源管理器加载
                std::string sound_path = sound_value.get<std::string>();
                entt::id_type sound_id = entt::hashed_string(sound_path.c_str());
                resource_manager_.loadSound(sound_id, sound_path);
                // 将音效键值对转换为音效ID并插入到声音蓝图中
                sounds.sounds_.emplace(entt::hashed_string(sound_key.c_str()), sound_id);
            }
        }
        return sounds;
    }

    data::EnemyBlueprint BlueprintManager::parseEnemy(const nlohmann::json &json)
    {
        // 敌人组件蓝图只包含“是否远程”和“移动速度”
        return data::EnemyBlueprint{json["ranged"].get<bool>(), json["speed"].get<float>()};
    }

    data::DisplayInfoBlueprint BlueprintManager::parseDisplayInfo(const nlohmann::json &json)
    {
        // 显示信息蓝图只包含“名称”和“描述”
        return data::DisplayInfoBlueprint{json.value("name", ""), json.value("description", "")};
    }
}