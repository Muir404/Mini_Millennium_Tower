#pragma once
#include <string>                 // 用于存储音乐路径
#include <array>                  // 用于管理音效轨道
#include <memory>                 // 用于管理音效轨道智能指针
#include <unordered_set>          // 用于存储已播放音效 ID，避免重复播放
#include <SDL3_mixer/SDL_mixer.h> // 用于音效和音乐播放
#include <SDL3/SDL_properties.h>  // 用于获取 SDL_Mixer 属性
#include <entt/entt.hpp>          // 用于实体组件系统 (ECS) 中的实体标识

// 前向声明 ResourceManager
namespace engine::resource
{
    class ResourceManager;
}

namespace engine::audio
{

    /**
     * @brief 用于控制音频播放的单例类。
     *
     * 提供播放音效和音乐的方法，使用由 ResourceManager 管理的资源。
     * 必须使用有效的 ResourceManager 实例初始化。
     */
    class AudioPlayer final
    {
    private:
        /**
         * @brief 音效轨道智能指针删除器
         *
         * 用于在智能指针销毁时调用 MIX_DestroyTrack 释放音效轨道资源。
         */
        struct SDLMixTrackDeleter
        {
            void operator()(MIX_Track *track) const
            {
                if (track)
                {
                    MIX_DestroyTrack(track);
                }
            }
        };

    private:
        engine::resource::ResourceManager *resource_manager_{nullptr};        ///<  指向 ResourceManager 的非拥有指针，用于加载和管理音频资源。
        entt::id_type current_music_{entt::null};                             ///<  当前正在播放的音乐路径，用于避免重复播放同一音乐。
        MIX_Mixer *mixer_{nullptr};                                           ///<  非拥有指针，借用 ResourceManager 提供的 mixer
        std::array<MIX_Track *, 16> sound_tracks_{nullptr};                   ///<  音效轨道数组，用于播放多个音效。
        std::unique_ptr<MIX_Track, SDLMixTrackDeleter> music_track_{nullptr}; ///<  背景音乐轨道，用于播放单条音乐。

        // 同帧去重记录
        uint64_t last_frame_time_{0};                         // 记录上次请求的时间戳（ms）
        std::unordered_set<entt::id_type> played_this_frame_; // 本帧已播 ID 集合

        // 强制征用（Voice Stealing）轮询索引
        size_t next_replacement_track_{0};

    public:
        /**
         * @brief 构造函数，使用 ResourceManager 初始化。
         * @param[in] resource_manager 指向 ResourceManager 的指针，用于加载和管理音频资源。
         */
        explicit AudioPlayer(engine::resource::ResourceManager *resource_manager);
        ~AudioPlayer();

        // 删除复制/移动操作
        AudioPlayer(const AudioPlayer &) = delete;
        AudioPlayer &operator=(const AudioPlayer &) = delete;
        AudioPlayer(AudioPlayer &&) = delete;
        AudioPlayer &operator=(AudioPlayer &&) = delete;

        // --- 播放控制方法 ---
        bool playSound(entt::id_type id);           ///< 播放指定ID的音效。适合知道音效ID的场景。
        bool playSound(entt::hashed_string str_hs); ///< 播放指定哈希字符串的音效。适合不知道音效ID的场景。

        bool playMusic(entt::id_type id, int loops = -1, int fade_in_ms = 0);           ///< 播放指定ID的音乐，可选循环次数和淡入时间。适合知道音乐ID的场景。
        bool playMusic(entt::hashed_string str_hs, int loops = -1, int fade_in_ms = 0); ///< 播放指定哈希字符串的音乐，可选循环次数和淡入时间。适合不知道音乐ID的场景。

        void stopMusic(int fade_out_ms = 0); ///< 停止当前播放的音乐，可选淡出效果。
        void pauseMusic();                   ///< 暂停当前播放的音乐。
        void resumeMusic();                  ///< 恢复当前暂停的音乐播放。
        void setSoundVolume(float volume);   ///< 设置音效音量。
        void setMusicVolume(float volume);   ///< 设置背景音乐音量。
        float getMusicVolume() const;        ///< 获取当前背景音乐音量。
        float getSoundVolume() const;        ///< 获取当前音效音量。
    };

} // namespace engine::audio
