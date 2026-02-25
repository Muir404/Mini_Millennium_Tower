#pragma once

// 标准库头文件（按字母序排列）
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// 第三方库头文件
#include <SDL3_mixer/SDL_mixer.h>

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
        std::unordered_map<std::string, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>> sounds_; ///< 短音效缓存 (路径 -> 音频资源)
        std::unordered_map<std::string, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>> music_;  ///< 长音乐缓存 (路径 -> 音频资源)
        MIX_Mixer *mixer_;

    public:
        AudioManager();
        ~AudioManager();

        // 禁用拷贝和移动语义（单例语义）
        AudioManager(const AudioManager &) = delete;
        AudioManager &operator=(const AudioManager &) = delete;
        AudioManager(AudioManager &&) = delete;
        AudioManager &operator=(AudioManager &&) = delete;

    private:
        // getter and setter
        MIX_Mixer *getMixer()
        {
            return mixer_;
        }

        /**
         * @brief 加载短音效
         *
         * 加载指定路径的音效文件到缓存中。
         *
         * @param file_path 音效文件路径
         * @return MIX_Audio* 加载的音效资源指针
         */
        MIX_Audio *loadSound(std::string_view file_path);

        /**
         * @brief 获取短音效
         *
         * 从缓存中获取指定路径的音效资源。如果资源不存在，尝试加载它。
         *
         * @param file_path 音效文件路径
         * @return MIX_Audio* 音效资源指针
         */
        MIX_Audio *getSound(std::string_view file_path);

        /**
         * @brief 卸载短音效
         *
         * 从缓存中移除指定路径的音效资源。
         *
         * @param file_path 音效文件路径
         */
        void unloadSound(std::string_view file_path);

        /**
         * @brief 清除所有短音效
         *
         * 从缓存中移除所有已加载的短音效资源。
         */
        void clearSounds();

        /**
         * @brief 加载长音乐
         *
         * 加载指定路径的音乐文件到缓存中。
         *
         * @param file_path 音乐文件路径
         * @return MIX_Audio* 加载的音乐资源指针
         */
        MIX_Audio *loadMusic(std::string_view file_path);

        /**
         * @brief 获取长音乐
         *
         * 从缓存中获取指定路径的音乐资源。如果资源不存在，尝试加载它。
         *
         * @param file_path 音乐文件路径
         * @return MIX_Audio* 音乐资源指针
         */
        MIX_Audio *getMusic(std::string_view file_path);

        /**
         * @brief 卸载长音乐
         *
         * 从缓存中移除指定路径的音乐资源。
         *
         * @param file_path 音乐文件路径
         */
        void unloadMusic(std::string_view file_path);

        /**
         * @brief 清除所有长音乐
         *
         * 从缓存中移除所有已加载的长音乐资源。
         */
        void clearMusic();

        /**
         * @brief 清除所有音频资源
         *
         * 从缓存中移除所有已加载的短音效和长音乐资源。
         */
        void clearAudio();
    };

} // namespace engine::resource