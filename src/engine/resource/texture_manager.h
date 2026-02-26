#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace engine::resource
{

    /**
     * @brief 纹理管理器类，负责加载、缓存和管理SDL纹理资源
     */
    class TextureManager
    {
        // 仅允许ResourceManager访问私有接口，封装资源管理逻辑
        friend class ResourceManager;

    private:
        /**
         * @brief 自定义删除器，用于SDL_Texture智能指针，确保在智能指针析构时自动调用SDL_DestroyTexture
         * @param texture SDL_Texture指针，指向要删除的纹理资源
         */
        struct SDLTextureDeleter
        {
            void operator()(SDL_Texture *texture) const
            {
                if (texture)
                {
                    SDL_DestroyTexture(texture);
                }
            }
        };

        // 私有成员变量
        std::unordered_map<entt::id_type, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_; ///< 纹理缓存映射表：文件路径 → 带自定义删除器的SDL_Texture智能指针

        SDL_Renderer *renderer_ = nullptr; ///< SDL渲染器指针（非拥有权，由外部ResourceManager传入并保证生命周期）

    public:
        explicit TextureManager(SDL_Renderer *renderer);

        // 禁用拷贝/移动语义
        // 确保纹理管理器实例唯一，避免资源重复释放
        TextureManager(const TextureManager &) = delete;
        TextureManager &operator=(const TextureManager &) = delete;
        TextureManager(TextureManager &&) = delete;
        TextureManager &operator=(TextureManager &&) = delete;

    private:
        SDL_Texture *loadTexture(entt::id_type id, std::string_view file_path);      ///< 加载纹理 实际干活的函数
        SDL_Texture *loadTexture(entt::hashed_string str_hs);                        ///< 加载纹理 实际调用的函数
        SDL_Texture *getTexture(entt::id_type id, std::string_view file_path = "");  ///< 获取纹理 实际干活的函数
        SDL_Texture *getTexture(entt::hashed_string str_hs);                         ///< 获取纹理 实际调用的函数
        glm::vec2 getTextureSize(entt::id_type id, std::string_view file_path = ""); ///< 获取纹理尺寸 实际干活的函数
        glm::vec2 getTextureSize(entt::hashed_string str_hs);                        ///< 获取纹理尺寸 实际调用的函数
        void unloadTexture(entt::id_type id);                                        ///< 卸载纹理 实际干活的函数+实际调用的函数
        void clearTextures();                                                        ///< 清除所有纹理 实际干活的函数+实际调用的函数
    };
}
