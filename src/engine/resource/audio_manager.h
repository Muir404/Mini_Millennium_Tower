#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <SDL3_mixer/SDL_mixer.h>
#include <entt/entt.hpp>

namespace engine::resource
{

    /**
     * @brief 音频资源管理器
     *
     * 负责加载、缓存和管理游戏中的音频资源（音效和音乐）。
     * 支持短音效（如按钮点击）和长音乐（如背景音乐）。
     */
    class AudioManager final
    {
        // 友元声明
        friend class ResourceManager;

    private:
        /**
         * @brief SDL 音频资源删除器
         *
         * 用于自动释放 SDL 音频资源（音效和音乐）。
         */
        struct SDLMixAudioDeleter
        {
            void operator()(MIX_Audio *audio) const
            {
                if (audio)
                {
                    MIX_DestroyAudio(audio);
                }
            }
        };

    private:
        // 资源缓存
        std::unordered_map<entt::id_type, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>> sounds_; ///< 短音效缓存 (ID -> 音频资源)
        std::unordered_map<entt::id_type, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>> music_;  ///< 长音乐缓存 (ID -> 音频资源)
        MIX_Mixer *mixer_{nullptr};                                                                ///< SDL 音频 Mixer 指针

    public:
        AudioManager();
        ~AudioManager();

        // 禁用拷贝和移动语义（单例语义）
        AudioManager(const AudioManager &) = delete;
        AudioManager &operator=(const AudioManager &) = delete;
        AudioManager(AudioManager &&) = delete;
        AudioManager &operator=(AudioManager &&) = delete;

    private:
        MIX_Mixer *getMixer() { return mixer_; } ///< 获取 SDL 音频 Mixer 指针

        MIX_Audio *loadSound(entt::id_type id, std::string_view file_path);     ///< 加载短音效 实际干活的函数
        MIX_Audio *loadSound(entt::hashed_string str_hs);                       ///< 加载短音效 实际调用的函数
        MIX_Audio *getSound(entt::id_type id, std::string_view file_path = ""); ///< 获取短音效 实际干活的函数
        MIX_Audio *getSound(entt::hashed_string str_hs);                        ///< 获取短音效 实际调用的函数
        void unloadSound(entt::id_type id);                                     ///< 卸载短音效 实际干活的函数+实际调用的函数
        void clearSounds();                                                     ///< 清除所有短音效 实际干活的函数+实际调用的函数

        MIX_Audio *loadMusic(entt::id_type id, std::string_view file_path);     ///< 加载长音乐 实际干活的函数
        MIX_Audio *loadMusic(entt::hashed_string str_hs);                       ///< 加载长音乐 实际调用的函数
        MIX_Audio *getMusic(entt::id_type id, std::string_view file_path = ""); ///< 获取长音乐 实际干活的函数
        MIX_Audio *getMusic(entt::hashed_string str_hs);                        ///< 获取长音乐 实际调用的函数
        void unloadMusic(entt::id_type id);                                     ///< 卸载长音乐 实际干活的函数+实际调用的函数
        void clearMusic();                                                      ///< 清除所有长音乐 实际干活的函数+实际调用的函数

        void clearAudio(); ///< 清除所有音频资源 实际干活的函数+实际调用的函数
    };

} // namespace engine::resource