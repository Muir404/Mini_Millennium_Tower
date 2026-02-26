#include <stdexcept>
#include <string>
#include "font_manager.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <spdlog/spdlog.h>

namespace engine::resource
{
    FontManager::FontManager()
    {
        if (!TTF_WasInit() && !TTF_Init())
        {
            throw std::runtime_error("[FontManager] 构造失败：" + std::string(SDL_GetError()));
        }
        spdlog::trace("[FontManager] 构造成功");
    }

    FontManager::~FontManager()
    {
        if (!fonts_.empty())
        {
            spdlog::debug("[FontManager] 缓存不为空，执行字体清理逻辑");
            clearFonts(); // 调用 clearFonts 处理清理逻辑
        }
        TTF_Quit();
        spdlog::trace("[FontManager] 析构成功");
    }

    TTF_Font *FontManager::loadFont(entt::id_type id, int point_size, std::string_view file_path)
    {
        // 检查点大小是否有效
        if (point_size <= 0)
        {
            spdlog::error("[FontManager] 无法加载字体 ID: {}, 路径: {}, 字号: {}", id, file_path, point_size);
            return nullptr;
        }

        // 创建映射表的键
        FontKey key = {id, point_size};

        // 首先检查缓存
        auto it = fonts_.find(key);
        if (it != fonts_.end())
        {
            return it->second.get();
        }

        // 缓存中不存在，则加载字体
        spdlog::debug("[FontManager] 正在加载字体 ID: {}, 路径: {}, 字号: {}", id, file_path, point_size);
        TTF_Font *raw_font = TTF_OpenFont(file_path.data(), point_size);
        if (!raw_font)
        {
            spdlog::error("[FontManager] 加载字体 ID: {}, 路径: {}, 字号: {} 失败：{}", id, file_path, point_size, SDL_GetError());
            return nullptr;
        }

        // 使用 unique_ptr 存储到缓存中
        fonts_.emplace(key, std::unique_ptr<TTF_Font, SDLFontDeleter>(raw_font));
        spdlog::debug("[FontManager] 成功加载并缓存字体 ID: {}, 路径: {}, 字号: {}", id, file_path, point_size);
        return raw_font;
    }

    TTF_Font *FontManager::loadFont(entt::hashed_string str_hs, int point_size)
    {
        return loadFont(str_hs.value(), point_size, str_hs.data());
    }

    TTF_Font *FontManager::getFont(entt::id_type id, int point_size, std::string_view file_path)
    {
        FontKey key = {id, point_size};
        auto it = fonts_.find(key);
        if (it != fonts_.end())
        {
            return it->second.get();
        }

        spdlog::warn("[FontManager] 字体 ID: {}, 路径: {}, 字号: {} 不在缓存中，尝试加载", id, file_path, point_size);
        return loadFont(id, point_size, file_path);
    }

    TTF_Font *FontManager::getFont(entt::hashed_string str_hs, int point_size)
    {
        return getFont(str_hs.value(), point_size, str_hs.data());
    }

    void FontManager::unloadFont(entt::id_type id, int point_size)
    {
        FontKey key = {id, point_size};
        auto it = fonts_.find(key);
        if (it != fonts_.end())
        {
            spdlog::debug("[FontManager] 卸载字体 ID: {}, 字号: {}", id, point_size);
            fonts_.erase(it); // unique_ptr 会处理 TTF_CloseFont
        }
        else
        {
            spdlog::warn("[FontManager] 尝试卸载不存在的字体 ID: {}, 字号: {}", id, point_size);
        }
    }

    void FontManager::clearFonts()
    {
        if (!fonts_.empty())
        {
            spdlog::debug("[FontManager] 正在清理所有 {} 个缓存的字体", fonts_.size());
            fonts_.clear(); // unique_ptr 会处理删除
        }
    }
}
