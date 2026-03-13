#pragma once
#include "ui_interactive.h"
#include <functional>
#include <utility>

using namespace entt::literals;

namespace engine::render
{
    class Image;
} // namespace engine::render

namespace engine::core
{
    class Context;
} // namespace engine::core

namespace engine::ui
{
    /**
     * @brief 按钮UI元素
     *
     * 继承自UIInteractive，用于创建可交互的按钮。
     * 支持三种状态：正常、悬停、按下。
     * 支持回调函数，当按钮被点击时调用。
     */
    class UIButton final : public UIInteractive
    {
    private:
        std::function<void()> callback_;             ///< @brief 可自定义的函数（函数包装器）
        std::function<void()> hover_enter_callback_; ///< @brief 悬停进入回调函数（函数包装器）
        std::function<void()> hover_leave_callback_; ///< @brief 悬停离开回调函数（函数包装器）

    public:
        UIButton(engine::core::Context &context,
                 engine::render::Image normal_image,
                 engine::render::Image hover_image,
                 engine::render::Image pressed_image,
                 glm::vec2 position = {0.0f, 0.0f},
                 glm::vec2 size = {0.0f, 0.0f},
                 std::function<void()> callback = nullptr,
                 std::function<void()> hover_enter_callback = nullptr,
                 std::function<void()> hover_leave_callback = nullptr);

        ~UIButton() override = default;

        void clicked() override; ///< @brief 重写基类方法，当按钮被点击时调用回调函数
        void hover_enter() override
        {
            if (hover_enter_callback_)
                hover_enter_callback_();
        }; ///< @brief 重写基类方法，当鼠标悬停进入按钮时调用悬停进入回调函数
        void hover_leave() override
        {
            if (hover_leave_callback_)
                hover_leave_callback_();
        }; ///< @brief 重写基类方法，当鼠标悬停离开按钮时调用悬停离开回调函数

        void setCallback(std::function<void()> callback) { callback_ = std::move(callback); }                                               ///< @brief 设置点击回调函数
        void setHoverEnterCallback(std::function<void()> hover_enter_callback) { hover_enter_callback_ = std::move(hover_enter_callback); } ///< @brief 设置悬停进入回调函数
        void setHoverLeaveCallback(std::function<void()> hover_leave_callback) { hover_leave_callback_ = std::move(hover_leave_callback); } ///< @brief 设置悬停离开回调函数

        std::function<void()> getCallback() const { return callback_; }                       ///< @brief 获取点击回调函数
        std::function<void()> getHoverEnterCallback() const { return hover_enter_callback_; } ///< @brief 获取悬停进入回调函数
        std::function<void()> getHoverLeaveCallback() const { return hover_leave_callback_; } ///< @brief 获取悬停离开回调函数
    };

} // namespace engine::ui