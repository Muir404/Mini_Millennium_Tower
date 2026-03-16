// 实现文件
#include "audio_manager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource
{
    AudioManager::AudioManager()
    {
        // 初始化 SDL_mixer 子系统
        if (!MIX_Init())
        {
            throw std::runtime_error("[AudioManager] 错误: MIX_Init 失败: " + std::string(SDL_GetError()));
        }

        mixer_ = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (!mixer_)
        {
            MIX_Quit();
            throw std::runtime_error("[AudioManager] 错误: Mixer创建失败" + std::string(SDL_GetError()));
        }

        spdlog::trace("[AudioManager] 构造成功");
    }

    AudioManager::~AudioManager()
    {
        MIX_StopAllTracks(mixer_, 0);
        clearAudio();
        MIX_DestroyMixer(mixer_);
        MIX_Quit();

        spdlog::trace("[AudioManager] 析构完成");
    }

    MIX_Audio *AudioManager::loadSound(entt::id_type id, std::string_view file_path)
    {
        // 检查缓存，避免重复加载
        // 负责实际从文件加载音效，并存入缓存
        auto it = sounds_.find(id);
        if (it != sounds_.end())
        {
            return it->second.get();
        }

        // 加载音效文件（predecode=false：不预解码，播放时实时解码，节省内存）
        spdlog::debug("[AudioManager] 开始加载音效:{}", id);
        MIX_Audio *audio = MIX_LoadAudio(mixer_, file_path.data(), true);
        if (!audio)
        {
            spdlog::error("[AudioManager] 加载音效失败: ID: {}, 路径: {} | 错误信息: {}", id, file_path, SDL_GetError());
            return nullptr;
        }

        // 存入缓存（使用自定义删除器管理生命周期）
        sounds_.emplace(id, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>(audio));
        spdlog::debug("[AudioManager] 音效加载并缓存成功: ID: {}, 路径: {}", id, file_path);

        return audio; // 返回一个音频载体，待绑定播放
    }

    MIX_Audio *AudioManager::loadSound(entt::hashed_string str_hs)
    {
        return loadSound(str_hs.value(), str_hs.data());
    }

    MIX_Audio *AudioManager::getSound(entt::id_type id, std::string_view file_path)
    {
        // 优先从缓存获取
        // 负责对外提供音效，优先查缓存，无则调用加载器
        auto it = sounds_.find(id);
        if (it != sounds_.end())
        {
            return it->second.get();
        }
        // 如果未找到，判断是否提供了file_path
        if (file_path.empty())
        {
            spdlog::error("音效 '{}' 未找到缓存，且未提供文件路径，返回nullptr。", id);
            return nullptr;
        }
        spdlog::warn("[AudioManager] 音效缓存未命中，尝试动态加载: ID: {}, 路径: {}", id, file_path);
        return loadSound(id, file_path);
    }

    MIX_Audio *AudioManager::getSound(entt::hashed_string str_hs)
    {
        return getSound(str_hs.value(), str_hs.data());
    }

    void AudioManager::unloadSound(entt::id_type id)
    {
        auto it = sounds_.find(id);
        if (it != sounds_.end())
        {
            spdlog::debug("[AudioManager] 卸载音效资源: ID: {}", id);
            sounds_.erase(it); // unique_ptr 自动释放资源
        }
        else
        {
            spdlog::warn("[AudioManager] 尝试卸载不存在的音效资源: ID: {}", id);
        }
    }

    void AudioManager::clearSounds()
    {
        if (!sounds_.empty())
        {
            spdlog::debug("[AudioManager] 清空所有音效缓存，数量: {}", sounds_.size());
            sounds_.clear(); // unique_ptr 自动释放所有音效资源
        }
    }

    MIX_Audio *AudioManager::loadMusic(entt::id_type id, std::string_view file_path)
    {
        // 检查缓存，避免重复加载
        auto it = music_.find(id);
        if (it != music_.end())
        {
            spdlog::trace("[AudioManager] 音乐已缓存，直接返回: ID: {}, 路径: {}", id, file_path);
            return it->second.get();
        }

        // 加载音乐文件（predecode=true：预解码为 PCM 数据，适合长音频流式播放）
        spdlog::debug("[AudioManager] 开始加载音乐: ID: {}, 路径: {}", id, file_path);
        MIX_Audio *audio = MIX_LoadAudio(mixer_, file_path.data(), true);
        if (!audio)
        {
            spdlog::error("[AudioManager] 加载音乐失败: ID: {}, 路径: {} | 错误信息: {}", id, file_path, SDL_GetError());
            return nullptr;
        }

        // 存入缓存（使用自定义删除器管理生命周期）
        music_.emplace(id, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>(audio));
        spdlog::debug("[AudioManager] 音乐加载并缓存成功: ID: {}, 路径: {}", id, file_path);

        return audio;
    }

    MIX_Audio *AudioManager::loadMusic(entt::hashed_string str_hs)
    {
        return loadMusic(str_hs.value(), str_hs.data());
    }

    MIX_Audio *AudioManager::getMusic(entt::id_type id, std::string_view file_path)
    {
        // 优先从缓存获取
        auto it = music_.find(id);
        if (it != music_.end())
        {
            return it->second.get();
        }

        // 如果未找到，判断是否提供了file_path
        if (file_path.empty())
        {
            spdlog::error("音乐 '{}' 未找到缓存，且未提供文件路径，返回nullptr。", id);
            return nullptr;
        }

        // 缓存未命中，尝试加载
        spdlog::warn("[AudioManager] 音乐缓存未命中，尝试动态加载: ID: {}, 路径: {}", id, file_path);
        return loadMusic(id, file_path);
    }

    MIX_Audio *AudioManager::getMusic(entt::hashed_string str_hs)
    {
        return getMusic(str_hs.value(), str_hs.data());
    }

    void AudioManager::unloadMusic(entt::id_type id)
    {
        auto it = music_.find(id);
        if (it != music_.end())
        {
            spdlog::debug("[AudioManager] 卸载音乐资源: ID: {}", id);
            music_.erase(it); // unique_ptr 自动释放资源
        }
        else
        {
            spdlog::warn("[AudioManager] 尝试卸载不存在的音乐资源: ID: {}", id);
        }
    }

    void AudioManager::clearMusic()
    {
        if (!music_.empty())
        {
            spdlog::debug("[AudioManager] 清空所有音乐缓存，数量: {}", music_.size());
            music_.clear(); // unique_ptr 自动释放所有音乐资源
        }
    }

    void AudioManager::clearAudio()
    {
        clearSounds(); // 清理音效
        clearMusic();  // 清理音乐
        spdlog::debug("[AudioManager] 所有音频资源已清空");
    }

} // namespace engine::resource
