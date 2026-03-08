#pragma once
#include "../utils/math.h"
#include "basic_entity_builder.h"
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <entt/entity/registry.hpp>
#include <SDL3/SDL_rect.h>
#include <map>

namespace engine::component
{
    enum class TileType;
    struct TileInfo;
}

namespace engine::scene
{
    class Scene;
}

namespace engine::loader
{
    class LevelLoader final
    {
        friend class BasicEntityBuilder;

    private:
        engine::scene::Scene *scene_;

        std::string map_path_;
        glm::ivec2 map_size_;
        glm::ivec2 tile_size_;

        std::map<int, nlohmann::json> tileset_data_;

        std::unique_ptr<BasicEntityBuilder> entity_builder_;

        int current_layer_ = 0; ///< 渲染顺序

    public:
        LevelLoader() = default;
        ~LevelLoader();

        void setEntityBuilder(std::unique_ptr<BasicEntityBuilder> builder);

        [[nodiscard]] bool loadLevel(std::string_view level_path, engine::scene::Scene *scene);

        const glm::ivec2 &getMapSize() const { return map_size_; }
        const glm::ivec2 &getTileSize() const { return tile_size_; }
        int getCurrentLayer() const { return current_layer_; }

    private:
        void loadImageLayer(const nlohmann::json &layer_json);
        void loadTileLayer(const nlohmann::json &layer_json);
        void loadObjectLayer(const nlohmann::json &layer_json);

        void loadTileset(std::string_view tileset_path, int first_gid);

        template <typename T>
        std::optional<T> getTileProperty(const nlohmann::json &tile_json, std::string_view property_name)
        {
            if (!tile_json.contains("properties"))
                return std::nullopt;
            const auto &properties = tile_json["properties"];
            for (const auto &property : properties)
            {
                if (property.contains("name") && property["name"] == std::string(property_name))
                {
                    if (property.contains("value"))
                    {
                        return property["value"].get<T>();
                    }
                }
            }
            return std::nullopt;
        }

        std::optional<engine::utils::Rect> getColliderRect(const nlohmann::json &tile_json);
        engine::utils::Rect getTextureRect(const nlohmann::json &tileset_json, int local_id);
        engine::component::TileType getTileType(const nlohmann::json &tile_json);
        engine::component::TileType getTileTypeById(const nlohmann::json &tileset_json, int local_id);
        std::optional<engine::component::TileInfo> getTileInfoByGid(int gid);
        std::string resolvePath(std::string_view relative_path, std::string_view file_path);
    };
}