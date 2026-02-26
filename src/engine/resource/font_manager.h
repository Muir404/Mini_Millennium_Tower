#pragma once
#include <functional>         // 用于 std::hash
#include <memory>             // 用于 std::unique_ptr
#include <stdexcept>          // 用于 std::runtime_error
#include <string>             // 用于 std::string
#include <string_view>        // 用于 const std::string&
#include <unordered_map>      // 用于 std::unordered_map
#include <utility>            // 用于 std::pair
#include <SDL3_ttf/SDL_ttf.h> // SDL_ttf 主头文件
#include <entt/entt.hpp>      // entt 主头文件

namespace engine::resource
{

    using FontKey = std::pair<entt::id_type, int>; // 字体键（文件路径+字号）

    /**
     * @brief 字体键哈希函数，用于在 unordered_map 中存储 FontKey
     * 用于将 FontKey 映射到 TTF_Font 指针
     */
    struct FontKeyHash
    {
        /**
         * @brief 计算 FontKey 的哈希值
         *
         * @param key 字体键（文件路径+字号）
         * @return std::size_t 哈希值
         */
        std::size_t operator()(const FontKey &key) const
        {
            std::size_t h1 = std::hash<entt::id_type>{}(key.first);
            std::size_t h2 = std::hash<int>{}(key.second);
            // 混合哈希值，避免相同路径不同字号的字体键哈希冲突 boost::hash_combine
            return h1 ^ (h2 + 0x9e3779b9 + (h2 << 6) + (h2 >> 2));
        }
    };

    /**
     * @brief 字体管理器，负责加载、获取、卸载和清空字体资源
     */
    class FontManager
    {
        // 仅允许ResourceManager访问私有接口，封装资源管理逻辑
        friend class ResourceManager;

    private:
        /**
         * @brief SDL 字体删除器，用于在 unique_ptr 中管理 TTF_Font 指针
         */
        struct SDLFontDeleter
        {
            void operator()(TTF_Font *font) const
            {
                if (font)
                {
                    TTF_CloseFont(font);
                }
            }
        };

        std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHash> fonts_; ///< 字体映射表，键为字体键（文件路径+字号），值为字体指针
    public:
        FontManager();
        ~FontManager();

        // 禁用拷贝/移动语义
        // 确保字体管理器实例唯一，避免资源重复释放
        FontManager(const FontManager &) = delete;
        FontManager &operator=(const FontManager &) = delete;
        FontManager(FontManager &&) = delete;
        FontManager &operator=(FontManager &&) = delete;

    private:
        TTF_Font *loadFont(entt::id_type id, int point_size, std::string_view file_path);     ///< 加载指定哈希字符串的SDL字体资源（若不存在则加载）
        TTF_Font *loadFont(entt::hashed_string str_hs, int point_size);                       ///< 加载指定哈希字符串的SDL字体资源（若不存在则加载）
        TTF_Font *getFont(entt::id_type id, int point_size, std::string_view file_path = ""); ///< 获取指定哈希字符串的SDL字体资源（若不存在则加载）
        TTF_Font *getFont(entt::hashed_string str_hs, int point_size);                        ///< 获取指定哈希字符串的SDL字体资源（若不存在则加载）
        void unloadFont(entt::id_type id, int point_size);                                    ///< 卸载指定哈希字符串的SDL字体资源
        void clearFonts();                                                                    ///< 清空所有已加载的SDL字体资源
    };
};
