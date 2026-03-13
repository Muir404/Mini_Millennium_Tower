#pragma once

// 标准库头文件
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// 第三方库头文件
#include <entt/core/hashed_string.hpp>

// 项目内部头文件
#include "ui_element.h"
#include "state/ui_state.h"
#include "../render/image.h" // 需要引入头文件而非前置声明（map容器创建时检查析构定义）

namespace engine::core
{
    class Context;
}

namespace engine::ui
{
    /**
     * @brief 可交互UI元素的基类,继承自UIElement
     *
     * 定义了可交互UI元素的通用属性和行为。
     * 管理UI状态的切换和交互逻辑。
     * 提供事件处理、更新和渲染的虚方法。
     */
    class UIInteractive : public UIElement
    {
        // ========================== 受保护成员变量 ==========================
    protected:
        engine::core::Context &context_;                                  ///< 引擎上下文引用
        std::unique_ptr<engine::ui::state::UIState> state_;               ///< 当前UI状态
        std::unique_ptr<engine::ui::state::UIState> next_state_;          ///< 下一个要切换的UI状态
        std::unordered_map<entt::id_type, engine::render::Image> images_; ///< 图片资源映射（ID -> 图片）
        std::unordered_map<entt::id_type, entt::id_type> sounds_;         ///< 音效资源映射（自定义ID -> 路径哈希ID）
        entt::id_type current_image_id_;                                  ///< 当前显示图片的ID
        bool interactive_ = true;                                         ///< 是否可交互标识

        // ========================== 公共接口 ==========================
    public:
        // -------------------------- 构造/析构 --------------------------
        UIInteractive(engine::core::Context &context, glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f});
        ~UIInteractive() override;

        // -------------------------- 交互事件处理 --------------------------
        virtual void clicked() {}     ///< 点击事件（派生类重写实现自定义逻辑）
        virtual void hover_enter() {} ///< 悬停进入事件（派生类重写实现自定义逻辑）
        virtual void hover_leave() {} ///< 悬停退出事件（派生类重写实现自定义逻辑）

        // -------------------------- 资源管理 --------------------------
        void addImage(entt::id_type id, engine::render::Image image); ///< 添加图片资源
        void setCurrentImage(entt::id_type id);                       ///< 设置当前显示的图片
        void setHoverSound(entt::id_type id, std::string_view path);  ///< 设置悬停音效（ID + 路径）
        void setClickSound(entt::id_type id, std::string_view path);  ///< 设置点击音效（ID + 路径）
        void playSound(entt::id_type id);                             ///< 播放指定ID的音效

        // -------------------------- 状态管理 --------------------------
        void setState(std::unique_ptr<engine::ui::state::UIState> state);     ///< 设置当前UI状态
        void setNextState(std::unique_ptr<engine::ui::state::UIState> state); ///< 设置下一个要切换的UI状态
        engine::ui::state::UIState *getState() const { return state_.get(); } ///< 获取当前UI状态

        void setInteractive(bool interactive) { interactive_ = interactive; } ///< 设置是否可交互
        bool isInteractive() const { return interactive_; }                   ///< 获取是否可交互状态

        // -------------------------- 上下文访问 --------------------------
        engine::core::Context &getContext() const { return context_; } ///< 获取引擎上下文引用

        // -------------------------- 核心生命周期 --------------------------
        void update(float delta_time, engine::core::Context &context) override; ///< 更新UI状态（每帧调用）
        void render(engine::core::Context &context) override;                   ///< 渲染UI（每帧调用）
    };

} // namespace engine::ui
