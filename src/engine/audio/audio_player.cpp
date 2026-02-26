#include "audio_player.h"
#include "../resource/resource_manager.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL_properties.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <optional>
#include <string>

namespace engine::audio
{
    AudioPlayer::AudioPlayer(engine::resource::ResourceManager *resource_manager)
        : resource_manager_(resource_manager)
    {
        // 校验资源管理器有效性
        if (!resource_manager_)
        {
            throw std::runtime_error("[AudioPlayer] ResourceManager 指针为空，初始化失败");
        }

        mixer_ = resource_manager_->getMixer(); // 获取资源管理器提供的 mixer

        // 初始化 sound_tracks_ 数组
        for (auto &track_ptr : sound_tracks_)
        {
            track_ptr = MIX_CreateTrack(mixer_);
            if (!track_ptr)
            {
                spdlog::error("[AudioPlayer] 创建音效轨道失败");
            }
        }
        auto raw_music_track = MIX_CreateTrack(mixer_);
        music_track_.reset(raw_music_track);
        spdlog::trace("[AudioPlayer] 初始化成功");
    }

    AudioPlayer::~AudioPlayer()
    {
        for (auto &track_ptr : sound_tracks_)
        {
            if (track_ptr != nullptr)
            {
                // 先停止轨道播放（可选，但推荐，避免销毁时音频还在播放）
                MIX_StopTrack(track_ptr, 0);
                // 销毁轨道资源（SDL_mixer的核心释放函数）
                MIX_DestroyTrack(track_ptr);
                // 2. 重置为nullptr，避免野指针
                track_ptr = nullptr;
            }
        }
        if (music_track_.get() != nullptr)
        {
            // 先停止轨道播放（可选，但推荐，避免销毁时音频还在播放）
            MIX_StopTrack(music_track_.get(), 0);
            // 销毁轨道资源（SDL_mixer的核心释放函数）
            MIX_DestroyTrack(music_track_.get());
            // 2. 重置为nullptr，避免野指针
            music_track_.reset(nullptr);
        }
    }

    bool AudioPlayer::playSound(entt::id_type id)
    {
        MIX_Audio *sound = resource_manager_->getSound(id);
        if (!sound)
        {
            spdlog::error("[AudioPlayer] 音效资源加载失败，ID: {}", id);
            return false;
        }

        // 查找可用的轨道
        for (auto &track_ptr : sound_tracks_)
        {
            if (track_ptr)
            {
                // 检查轨道当前是否空闲
                if (!MIX_TrackPlaying(track_ptr))
                {
                    // 绑定音效到轨道
                    if (!MIX_SetTrackAudio(track_ptr, sound)) // 如果失败
                    {
                        spdlog::error("[AudioPlayer] 音效轨道绑定资源失败，ID: {}, 错误: {}", id, SDL_GetError());
                        continue; // 尝试下一个轨道
                    }

                    // 播放
                    if (!MIX_PlayTrack(track_ptr, 0)) // 如果失败
                    {
                        spdlog::error("[AudioPlayer] 播放音效,ID: {} 失败，错误: {}", id, SDL_GetError());
                        continue; // 尝试下一个轨道
                    }
                    spdlog::trace("[AudioPlayer] 播放音效,ID: {} 成功", id);
                    return true;
                }
            }
        }

        spdlog::error("[AudioPlayer] 无可用音效轨道，ID: {}", id);
        return false;
    }

    bool AudioPlayer::playSound(entt::hashed_string str_hs)
    {
        MIX_Audio *sound = resource_manager_->getSound(str_hs.value(), str_hs.data());
        if (!sound)
        {
            spdlog::error("[AudioPlayer] 音效资源加载失败，ID: {}", str_hs.data());
            return false;
        }

        // 查找可用的轨道
        for (auto &track_ptr : sound_tracks_)
        {
            if (track_ptr)
            {
                // 检查轨道当前是否空闲
                if (!MIX_TrackPlaying(track_ptr))
                {
                    // 绑定音效到轨道
                    if (!MIX_SetTrackAudio(track_ptr, sound)) // 如果失败
                    {
                        spdlog::error("[AudioPlayer] 音效轨道绑定资源失败，ID: {}, 错误: {}", str_hs.data(), SDL_GetError());
                        continue; // 尝试下一个轨道
                    }

                    // 播放
                    if (!MIX_PlayTrack(track_ptr, 0)) // 如果失败
                    {
                        spdlog::error("[AudioPlayer] 播放音效,ID: {} 失败，错误: {}", str_hs.data(), SDL_GetError());
                        continue; // 尝试下一个轨道
                    }
                    spdlog::trace("[AudioPlayer] 播放音效,ID: {} 成功", str_hs.data());
                    return true;
                }
            }
        }

        spdlog::error("[AudioPlayer] 无可用音效轨道，ID: {}", str_hs.data());
        return false;
    }

    bool AudioPlayer::playMusic(entt::id_type id, int loops, int fade_in_ms)
    {
        // 1. 判重：当前已在播放该音乐，直接返回成功
        if (id == current_music_)
        {
            spdlog::trace("[AudioPlayer] 背景音乐 {} 已在播放，无需重复播放", id);
            return true;
        }

        // 2. stop music 平滑停止当前背景音乐
        if (MIX_TrackPlaying(music_track_.get()))
        {
            MIX_StopTrack(music_track_.get(), MIX_TrackMSToFrames(music_track_.get(), fade_in_ms));
        }

        // 3. 通过 ResourceManager 加载/获取音乐资源
        // 向 ResourceManager 要 音频资源
        MIX_Audio *music = resource_manager_->getMusic(id);

        if (!music)
        {
            spdlog::error("[AudioPlayer] 背景音乐资源加载失败，ID: {}", id);
            return false;
        }
        // 4. 更新当前播放标记
        current_music_ = id;

        // 5. 绑定音乐资源到背景音乐轨道
        if (!MIX_SetTrackAudio(music_track_.get(), music))
        {
            spdlog::error("[AudioPlayer] 背景音乐轨道绑定资源失败, ID: {}, 错误: {}", id, SDL_GetError());
            return false;
        }

        // 6. 配置播放属性（SDL3_mixer 标准属性）
        SDL_PropertiesID props = SDL_CreateProperties();
        if (!props)
        {
            spdlog::error("[AudioPlayer] 创建背景音乐播放属性失败: {}", SDL_GetError());
            return false;
        }

        // 7. 配置循环次数（SDL3_mixer 标准属性：MIX_PROP_PLAY_LOOPS_NUMBER）
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        spdlog::trace("[AudioPlayer] 背景音乐 ID:{} 循环次数设置为: {}", id, loops);

        // 8. 启动播放
        if (!MIX_PlayTrack(music_track_.get(), props))
        {
            spdlog::error("[AudioPlayer] 播放背景音乐 ID:{} 失败: {}", id, SDL_GetError());
        }
        else
        {
            spdlog::info("[AudioPlayer] 播放背景音乐 ID:{} 成功（循环：{}）", id, loops);
        }

        // 9. 销毁属性对象（避免内存泄漏）
        SDL_DestroyProperties(props);

        return true;
    }

    bool AudioPlayer::playMusic(entt::hashed_string str_hs, int loops, int fade_in_ms)
    {
        // 1. 判重：当前已在播放该音乐，直接返回成功
        if (str_hs.value() == current_music_)
        {
            spdlog::trace("[AudioPlayer] 背景音乐 {} 已在播放，无需重复播放", str_hs.data());
            return true;
        }

        // 2. stop music 平滑停止当前背景音乐
        if (MIX_TrackPlaying(music_track_.get()))
        {
            MIX_StopTrack(music_track_.get(), MIX_TrackMSToFrames(music_track_.get(), fade_in_ms));
        }

        // 3. 通过 ResourceManager 加载/获取音乐资源
        // 向 ResourceManager 要 音频资源
        MIX_Audio *music = resource_manager_->getMusic(str_hs.value(), str_hs.data());

        if (!music)
        {
            spdlog::error("[AudioPlayer] 背景音乐资源加载失败，ID: {}", str_hs.data());
            return false;
        }
        // 4. 更新当前播放标记
        current_music_ = str_hs.value();

        // 5. 绑定音乐资源到背景音乐轨道
        if (!MIX_SetTrackAudio(music_track_.get(), music))
        {
            spdlog::error("[AudioPlayer] 背景音乐轨道绑定资源失败, ID: {}, 错误: {}", str_hs.data(), SDL_GetError());
            return false;
        }

        // 6. 配置播放属性（SDL3_mixer 标准属性）
        SDL_PropertiesID props = SDL_CreateProperties();
        if (!props)
        {
            spdlog::error("[AudioPlayer] 创建背景音乐播放属性失败: {}", SDL_GetError());
            return false;
        }

        // 7. 配置循环次数（SDL3_mixer 标准属性：MIX_PROP_PLAY_LOOPS_NUMBER）
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        spdlog::trace("[AudioPlayer] 背景音乐 ID:{} 循环次数设置为: {}", str_hs.data(), loops);

        // 8. 启动播放
        if (!MIX_PlayTrack(music_track_.get(), props))
        {
            spdlog::error("[AudioPlayer] 播放背景音乐 ID:{} 失败: {}", str_hs.data(), SDL_GetError());
        }
        else
        {
            spdlog::info("[AudioPlayer] 播放背景音乐 ID:{} 成功（循环：{}）", str_hs.data(), loops);
        }

        // 9. 销毁属性对象（避免内存泄漏）
        SDL_DestroyProperties(props);

        return true;
    }

    void AudioPlayer::stopMusic(int fade_out_ms)
    {
        if (!(music_track_.get()))
        {
            return;
        }
        auto time = fade_out_ms > 0 ? MIX_TrackMSToFrames(music_track_.get(), fade_out_ms) : MIX_TrackMSToFrames(music_track_.get(), 0);
        // 淡出时长
        MIX_StopTrack(music_track_.get(), time);
        spdlog::trace("[AudioPlayer] 停止背景音乐（淡出时长：{}ms）", fade_out_ms);
    }

    void AudioPlayer::pauseMusic()
    {

        MIX_PauseTrack(music_track_.get());
        spdlog::trace("[AudioPlayer] 暂停背景音乐播放");
    }

    void AudioPlayer::resumeMusic()
    {

        MIX_ResumeTrack(music_track_.get());
        spdlog::trace("[AudioPlayer] 恢复背景音乐播放");
    }

    void AudioPlayer::setSoundVolume(float volume)
    {
        auto sound_volume = glm::clamp(volume, 0.0f, 1.0f);
        // 批量更新所有音效轨道音量（通过 ResourceManager 按标签控制）
        for (auto &track_ptr : sound_tracks_)
        {
            MIX_SetTrackGain(track_ptr, volume);
        }
        spdlog::info("[AudioPlayer] 全局音效音量设置为: {}", sound_volume);
    }

    void AudioPlayer::setMusicVolume(float volume)
    {
        auto music_volume = glm::clamp(volume, 0.0f, 1.0f);
        MIX_SetTrackGain(music_track_.get(), music_volume);
        spdlog::info("[AudioPlayer] 背景音乐音量设置为: {}", music_volume);
    }

    float AudioPlayer::getMusicVolume() const
    {
        auto gain = 0.0f;
        if (!music_track_.get())
        {
            gain = 0.0f;
        }
        else
        {
            gain = static_cast<float>(MIX_GetTrackGain(music_track_.get()) / 1.0f);
        }
        return gain;
    }

    float AudioPlayer::getSoundVolume() const
    {
        auto gain = 0.0f;
        for (auto &track_ptr : sound_tracks_)
        {
            if (!track_ptr)
            {
                continue;
            }
            gain = static_cast<float>(MIX_GetTrackGain(track_ptr) / 1.0f);
        }
        return gain;
    }
} // namespace engine::audio
