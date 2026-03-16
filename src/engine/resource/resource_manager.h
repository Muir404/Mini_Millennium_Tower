#pragma once
#include <memory>      // 用于 std::unique_ptr
#include <string>      // 用于 std::string
#include <string_view> // 用于 std::string_view
#include <glm/glm.hpp>
#include <entt/entt.hpp>

// 前向声明 SDL 类型
struct SDL_Renderer;
struct SDL_Texture;
struct MIX_Audio;
struct TTF_Font;
struct MIX_Mixer;

namespace engine::resource
{

    // 前向声明内部管理器
    class TextureManager;
    class AudioManager;
    class FontManager;

    /**
     * @brief 资源管理器类，负责管理游戏中的所有资源（纹理、音效、音乐、字体）
     * 该类负责初始化和管理所有资源加载、缓存和卸载操作。
     * 它负责初始化 SDL 音频和字体子系统，并提供统一的资源访问接口。
     */
    class ResourceManager final
    {
    private:
        // 使用 unique_ptr 确保所有权和自动清理
        std::unique_ptr<TextureManager> texture_manager_;
        std::unique_ptr<AudioManager> audio_manager_;
        std::unique_ptr<FontManager> font_manager_;

    public:
        explicit ResourceManager(SDL_Renderer *renderer); ///< 构造函数，初始化资源管理器并设置渲染器
        ~ResourceManager();                               ///< 显式声明析构函数，这是为了能让智能指针正确管理仅有前向声明的类
        void clear();                                     ///< 清空所有资源

        // 当前设计中，我们只需要一个ResourceManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
        ResourceManager(const ResourceManager &) = delete;
        ResourceManager &operator=(const ResourceManager &) = delete;
        ResourceManager(ResourceManager &&) = delete;
        ResourceManager &operator=(ResourceManager &&) = delete;

        // 加载资源
        void loadResources(std::string_view file_path); ///< 从 JSON 文件加载所有资源

        // --- 统一资源访问接口 ---
        // -- Texture --
        SDL_Texture *loadTexture(entt::id_type id, std::string_view file_path);      ///< 载入纹理资源
        SDL_Texture *loadTexture(entt::hashed_string str_hs);                        ///< 载入纹理资源
        SDL_Texture *getTexture(entt::id_type id, std::string_view file_path = "");  ///< 尝试获取已加载纹理的指针，如果未加载则尝试加载
        SDL_Texture *getTexture(entt::hashed_string str_hs);                         ///< 尝试获取已加载纹理的指针，如果未加载则尝试加载
        void unloadTexture(entt::id_type id);                                        ///< 卸载指定的纹理资源
        glm::vec2 getTextureSize(entt::id_type id, std::string_view file_path = ""); ///< 获取指定纹理的尺寸
        glm::vec2 getTextureSize(entt::hashed_string str_hs);                        ///< 获取指定纹理的尺寸
        void clearTextures();                                                        ///< 清空所有纹理资源

        // -- Sound Effects (Chunks) --
        MIX_Audio *loadSound(entt::id_type id, std::string_view file_path);     ///< 载入音效资源
        MIX_Audio *loadSound(entt::hashed_string str_hs);                       ///< 载入音效资源
        MIX_Audio *getSound(entt::id_type id, std::string_view file_path = ""); ///< 尝试获取已加载音效的指针，如果未加载则尝试加载
        MIX_Audio *getSound(entt::hashed_string str_hs);                        ///< 尝试获取已加载音效的指针，如果未加载则尝试加载
        void unloadSound(entt::id_type id);                                     ///< 卸载指定的音效资源
        void clearSounds();                                                     ///< 清空所有音效资源

        // -- Music --
        MIX_Audio *loadMusic(entt::id_type id, std::string_view file_path);     ///< 载入音乐资源
        MIX_Audio *loadMusic(entt::hashed_string str_hs);                       ///< 载入音乐资源
        MIX_Audio *getMusic(entt::id_type id, std::string_view file_path = ""); ///< 尝试获取已加载音乐的指针，如果未加载则尝试加载
        MIX_Audio *getMusic(entt::hashed_string str_hs);                        ///< 尝试获取已加载音乐的指针，如果未加载则尝试加载
        void unloadMusic(entt::id_type id);                                     ///< 卸载指定的音乐资源
        void clearMusic();                                                      ///< 清空所有音乐资源

        // -- Fonts --
        TTF_Font *loadFont(entt::id_type id, int point_size, std::string_view file_path);     ///< 载入字体资源
        TTF_Font *loadFont(entt::hashed_string str_hs, int point_size);                       ///< 载入字体资源
        TTF_Font *getFont(entt::id_type id, int point_size, std::string_view file_path = ""); ///< 尝试获取已加载字体的指针，如果未加载则尝试加载
        TTF_Font *getFont(entt::hashed_string str_hs, int point_size);                        ///< 尝试获取已加载字体的指针，如果未加载则尝试加载
        void unloadFont(entt::id_type id, int point_size);                                    ///< 卸载指定的字体资源
        void clearFonts();                                                                    ///< 清空所有字体资源

        // Mixer
        MIX_Mixer *getMixer(); ///< 获取 Mixer 实例指针
    };

} // namespace engine::resource