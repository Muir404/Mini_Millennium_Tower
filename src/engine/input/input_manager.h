#pragma once
#include <string>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <SDL3/SDL_render.h>
#include <glm/vec2.hpp>
#include <entt/entt.hpp>

namespace engine::core
{
    class Config;
}

namespace engine::input
{
    /**
     * @brief 输入管理器，负责处理输入事件和动作状态管理。
     */
    enum class ActionState
    {
        PRESSED,  ///< brief 动作在本帧刚刚被按下
        HELD,     ///< brief 动作被持续按下
        RELEASED, ///< brief 动作在本帧刚刚被释放
        INACTIVE, ///< brief 动作未激活
    };

    /**
     * @brief 输入管理器，负责处理输入事件和动作状态管理。
     */
    class InputManager final
    {
    private:
        SDL_Renderer *sdl_renderer_; ///< brief 用于获取逻辑坐标的 SDL_Renderer 指针

        std::unordered_map<std::string, std::array<entt::sigh<void()>, 3>> actions_to_func_;                ///< brief 存储动作名称到函数指针的映射
        std::unordered_map<std::variant<Uint32, SDL_Scancode>, std::vector<std::string>> input_to_actions_; ///< brief 从输入到关联的动作名称列表
        std::unordered_map<std::string, ActionState> action_states_;                                        ///< brief 存储每个动作的当前状态

        bool should_quit_ = false; ///< brief 退出标志
        glm::vec2 mouse_position_; ///< brief 鼠标位置 (针对屏幕坐标)

    public:
        /**
         * @brief 构造函数，初始化输入管理器。
         * @param sdl_renderer 用于获取逻辑坐标的 SDL_Renderer 指针。
         * @param config 指向配置对象的指针，用于初始化输入映射。
         */
        InputManager(SDL_Renderer *sdl_renderer, const engine::core::Config *config);

        /**
         * @brief 注册动作回调函数。
         * @param action_name 动作名称。
         * @param action_state 动作状态，默认值为 ActionState::PRESSED。
         * @return entt::sink<entt::sigh<void()>> 用于连接回调函数的 sink 对象。
         */
        entt::sink<entt::sigh<void()>> onAction(std::string_view action_name, ActionState action_state = ActionState::PRESSED); ///< brief 注册动作回调函数

        void update(); // 更新输入状态，每轮循环最先调用

        // 动作状态检查
        bool isActionDown(std::string_view action_name) const;     ///< brief 动作当前是否触发 (持续按下或本帧按下)
        bool isActionPressed(std::string_view action_name) const;  ///< brief 动作是否在本帧刚刚按下
        bool isActionReleased(std::string_view action_name) const; ///< brief 动作是否在本帧刚刚释放

        bool shouldQuit() const;              ///< brief 查询退出状态
        void setShouldQuit(bool should_quit); ///< brief 设置退出状态

        glm::vec2 getMousePosition() const;        ///< brief 获取鼠标位置 （屏幕坐标）
        glm::vec2 getLogicalMousePosition() const; ///< brief 获取鼠标位置 （逻辑坐标）

    private:
        void processEvent(const SDL_Event &event);                   ///< brief 处理 SDL 事件（将按键转换为动作状态）
        void initializeMappings(const engine::core::Config *config); ///< brief 根据 Config配置初始化映射表

        void updateActionState(std::string_view action_name, bool is_input_active, bool is_repeat_event); ///< brief 辅助更新动作状态
        SDL_Scancode scancodeFromString(std::string_view key_name);                                       ///< brief 将字符串键名转换为 SDL_Scancode
        Uint32 mouseButtonUint32FromString(std::string_view button_name);                                 ///< brief 将字符串按钮名转换为 SDL_Button
    };

} // namespace engine::input