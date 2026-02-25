#pragma once
#include <functional>         // 用于 std::hash
#include <memory>             // 用于 std::unique_ptr
#include <stdexcept>          // 用于 std::runtime_error
#include <string>             // 用于 std::string
#include <string_view>        // 用于 const std::string&
#include <unordered_map>      // 用于 std::unordered_map
#include <utility>            // 用于 std::pair
#include <SDL3_ttf/SDL_ttf.h> // SDL_ttf 主头文件

namespace engine::resource
{

    using FontKey = std::pair<std::string, int>; // 字体键（文件路径+字号）

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
            std::hash<std::string> string_hasher;
            std::hash<int> int_hasher;
            return string_hasher(key.first) ^ int_hasher(key.second);
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
        /**
         * @brief 加载字体资源
         *
         * @param file_path 字体文件路径
         * @param point_size 字体字号
         * @return TTF_Font* 字体指针
         */
        TTF_Font *loadFont(std::string_view file_path, int point_size);

        /**
         * @brief 获取字体资源
         *
         * @param file_path 字体文件路径
         * @param point_size 字体字号
         * @return TTF_Font* 字体指针
         */
        TTF_Font *getFont(std::string_view file_path, int point_size);

        /**
         * @brief 卸载字体资源
         *
         * @param file_path 字体文件路径
         * @param point_size 字体字号
         */
        void unloadFont(std::string_view file_path, int point_size);

        /**
         * @brief 清空所有字体资源
         */
        void clearFonts();
    };
};
