#include "texture_manager.h"
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
namespace engine::resource
{
    TextureManager::TextureManager(SDL_Renderer *renderer)
        : renderer_(renderer)
    {
        if (!renderer_)
        {
            throw std::runtime_error("[TextureManager] 构造失败：渲染器指针为空");
        }
        spdlog::info("[TextureManager] 构造成功");
    }

    SDL_Texture *TextureManager::loadTexture(entt::id_type id, std::string_view file_path)
    {
        // 检查是否已经加载过该纹理
        auto it = textures_.find(id);
        if (it != textures_.end())
        {
            return it->second.get();
        }

        // 如果没有加载则尝试加载
        SDL_Texture *raw_texture = IMG_LoadTexture(renderer_, file_path.data());

        if (!SDL_SetTextureScaleMode(raw_texture, SDL_SCALEMODE_NEAREST)) // 最邻近插值优化画面
        {
            spdlog::warn("[TextureManager] 无法设置纹理模式为最邻近插值");
        }

        if (!raw_texture)
        {
            spdlog::error("[TextureManager] 加载纹理失败：ID: {}, 路径: {}, 错误原因: {}", id, file_path, SDL_GetError());
            return nullptr;
        }
        textures_.emplace(id, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_texture));
        spdlog::debug("[TextureManager] 成功载入并缓存纹理：ID: {}", id);

        return raw_texture;
    }

    SDL_Texture *TextureManager::loadTexture(entt::hashed_string str_hs)
    {
        return loadTexture(str_hs.value(), str_hs.data());
    }

    SDL_Texture *TextureManager::getTexture(entt::id_type id, std::string_view file_path)
    {
        auto it = textures_.find(id);
        if (it != textures_.end())
        {
            return it->second.get();
        }

        if (file_path.empty())
        {
            spdlog::error("[TextureManager] 尝试获取空路径的纹理：ID: {}", id);
            return nullptr;
        }
        spdlog::warn("[TextureManager] 未缓存纹理ID: {}, 路径: {}，尝试加载它", id, file_path);

        return loadTexture(id, file_path);
    }

    SDL_Texture *TextureManager::getTexture(entt::hashed_string str_hs)
    {
        return getTexture(str_hs.value(), str_hs.data());
    }

    glm::vec2 TextureManager::getTextureSize(entt::id_type id, std::string_view file_path)
    {
        SDL_Texture *texture = getTexture(id, file_path);
        if (!texture)
        {
            spdlog::error("[TextureManager] 无法获取纹理：ID: {}, 路径: {}, 错误原因: {}", id, file_path, SDL_GetError());
            return glm::vec2(0);
        }

        glm::vec2 size; // 完全保留原代码的size变量定义
        if (!SDL_GetTextureSize(texture, &size.x, &size.y))
        {
            spdlog::error("[TextureManager] 无法查询纹理尺寸：ID: {}, 路径: {}, 错误原因: {}", id, file_path, SDL_GetError());
            return glm::vec2(0);
        }
        return size;
    }

    glm::vec2 TextureManager::getTextureSize(entt::hashed_string str_hs)
    {
        return getTextureSize(str_hs.value(), str_hs.data());
    }

    void TextureManager::unloadTexture(entt::id_type id)
    {
        auto it = textures_.find(id);
        if (it != textures_.end())
        {
            spdlog::debug("[TextureManager] 卸载纹理：ID: {}", id);
            textures_.erase(it); // 借助删除器删除
        }
        else
        {
            spdlog::warn("[TextureManager] 尝试删除不存在的纹理：ID: {}", id);
        }
    }

    void TextureManager::clearTextures()
    {
        textures_.clear();
        spdlog::info("[TextureManager] 所有纹理已清空");
    }

} // namespace engine::resource
