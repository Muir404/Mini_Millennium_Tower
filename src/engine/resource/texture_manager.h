#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>

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
        std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_; ///< 纹理缓存映射表：文件路径 → 带自定义删除器的SDL_Texture智能指针

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
        /**
         * @brief 加载SDL纹理资源，支持缓存复用
         * @param file_path 纹理文件路径（支持相对路径）
         * @return SDL_Texture* 加载的SDL纹理指针（已缓存）
         */
        SDL_Texture *loadTexture(std::string_view file_path);

        /**
         * @brief 获取已加载的SDL纹理指针（若不存在则加载）
         * @param file_path 纹理文件路径（支持相对路径）
         * @return SDL_Texture* 已加载的SDL纹理指针（已缓存）
         */
        SDL_Texture *getTexture(std::string_view file_path);

        /**
         * @brief 卸载指定路径的SDL纹理资源（若存在）
         * @param file_path 纹理文件路径（支持相对路径）
         */
        void unloadTexture(std::string_view file_path);

        /**
         * @brief 获取指定路径纹理的尺寸（若不存在则加载）
         * @param file_path 纹理文件路径（支持相对路径）
         * @return glm::vec2 纹理尺寸（宽度、高度）
         */
        glm::vec2 getTextureSize(std::string_view file_path);

        /**
         * @brief 清除所有已加载的SDL纹理资源（谨慎使用，可能导致内存泄漏）
         */
        void clearTextures();
    };
}
