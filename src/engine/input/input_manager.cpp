#include "input_manager.h"
#include "../core/config.h"
#include <stdexcept>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>

namespace engine::input
{
    /**
     * @brief 输入管理器构造函数。
     * @param sdl_renderer SDL 渲染器指针，用于获取输入事件。
     * @param config 配置对象指针，用于获取输入映射配置。
     * @throws std::runtime_error 如果 sdl_renderer_ 为空指针。
     */
    InputManager::InputManager(SDL_Renderer *sdl_renderer, const engine::core::Config *config) : sdl_renderer_(sdl_renderer)
    {
        if (!sdl_renderer_)
        {
            spdlog::error("[输入管理器] SDL_Renderer 为空指针");
            throw std::runtime_error("[输入管理器] SDL_Renderer 为空指针");
        }
        initializeMappings(config);
        // 获取初始鼠标位置
        float x, y;
        SDL_GetMouseState(&x, &y);
        mouse_position_ = {x, y};
        spdlog::trace("初始鼠标位置: ({}, {})", mouse_position_.x, mouse_position_.y);
    }

    /**
     * @brief 注册或获取指定动作名称和状态的信号槽连接。
     * @param action_name 动作名称（例如 "jump"）。
     * @param action_state 动作状态（例如 PRESSSED, RELEASED）。
     * @return 信号槽连接对象，用于触发回调。
     */
    entt::sink<entt::sigh<void()>> InputManager::onAction(std::string_view action_name, ActionState action_state)
    {
        // 如果action_name不存在，自动创建一个std::array<...>
        // .at() 会安全检查
        return actions_to_func_[std::string(action_name)].at(static_cast<size_t>(action_state));
    }

    /**
     * @brief 更新输入状态，处理所有待处理的 SDL 事件。
     * 此方法应在每个游戏循环迭代中调用，以更新输入状态并触发相应的回调。
     */
    void InputManager::update()
    {
        // 1. 根据上一帧的值更新默认的动作状态
        for (auto &[action_name, state] : action_states_)
        {
            if (state == ActionState::PRESSED)
            {
                state = ActionState::HELD; // 当某个键按下不动时，并不会生成SDL_Event。
            }
            else if (state == ActionState::RELEASED)
            {
                state = ActionState::INACTIVE;
            }
        }

        // 2. 处理所有待处理的 SDL 事件 (这将设定 action_states_ 的值)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            processEvent(event);
        }

        // 3. 触发回调
        for (auto &[action_name, state] : action_states_)
        {
            if (state != ActionState::INACTIVE) // 如果动作不是INACTIVE，说明有回调函数需要触发
            {
                // 绑定回调
                if (auto it = actions_to_func_.find(std::string(action_name)); it != actions_to_func_.end())
                {
                    it->second.at(static_cast<size_t>(state)).publish(); // 触发回调
                }
            }
        }
    }

    /**
     * @brief 检查指定动作名称是否当前正在按下或保持。
     * @param action_name 动作名称（例如 "jump"）。
     * @return 如果动作正在按下或保持，则返回 true；否则返回 false。
     */
    bool InputManager::isActionDown(std::string_view action_name) const
    {
        // C++17 引入的 “带有初始化语句的 if 语句”
        if (auto it = action_states_.find(std::string(action_name)); it != action_states_.end())
        {
            return it->second == ActionState::PRESSED || it->second == ActionState::HELD;
        }
        return false;
    }

    /**
     * @brief 检查指定动作名称是否当前正在按下（而不是保持）。
     * @param action_name 动作名称（例如 "jump"）。
     * @return 如果动作正在按下（而不是保持），则返回 true；否则返回 false。
     */
    bool InputManager::isActionPressed(std::string_view action_name) const
    {
        if (auto it = action_states_.find(std::string(action_name)); it != action_states_.end())
        {
            return it->second == ActionState::PRESSED;
        }
        return false;
    }

    /**
     * @brief 检查指定动作名称是否当前正在释放（而不是保持）。
     * @param action_name 动作名称（例如 "jump"）。
     * @return 如果动作正在释放（而不是保持），则返回 true；否则返回 false。
     */
    bool InputManager::isActionReleased(std::string_view action_name) const
    {
        if (auto it = action_states_.find(std::string(action_name)); it != action_states_.end())
        {
            return it->second == ActionState::RELEASED;
        }
        return false;
    }

    /**
     * @brief 检查是否应该退出游戏。
     * @return 如果应该退出游戏，则返回 true；否则返回 false。
     */
    bool InputManager::shouldQuit() const
    {
        return should_quit_;
    }

    /**
     * @brief 设置是否应该退出游戏。
     * @param should_quit 如果应该退出游戏，则设置为 true；否则设置为 false。
     */
    void InputManager::setShouldQuit(bool should_quit)
    {
        should_quit_ = should_quit;
    }

    /**
     * @brief 获取当前鼠标位置（窗口坐标）。
     * @return 当前鼠标位置的 glm::vec2 向量，包含 x 和 y 坐标。
     */
    glm::vec2 InputManager::getMousePosition() const
    {
        return mouse_position_;
    }

    /**
     * @brief 获取当前鼠标位置（逻辑坐标）。
     * @return 当前鼠标位置的 glm::vec2 向量，包含 x 和 y 坐标。
     */
    glm::vec2 InputManager::getLogicalMousePosition() const
    {
        glm::vec2 logical_pos;
        // 通过窗口坐标获取渲染坐标（逻辑坐标）
        SDL_RenderCoordinatesFromWindow(sdl_renderer_, mouse_position_.x, mouse_position_.y, &logical_pos.x, &logical_pos.y);
        return logical_pos;
    }

    /**
     * @brief 处理 SDL 事件，更新输入状态。
     * @param event 要处理的 SDL_Event 结构体。
     */
    void InputManager::processEvent(const SDL_Event &event)
    {
        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            SDL_Scancode scancode = event.key.scancode; // 获取按键的scancode
            bool is_down = event.key.down;
            bool is_repeat = event.key.repeat;

            auto it = input_to_actions_.find(scancode);
            if (it != input_to_actions_.end())
            { // 如果按键有对应的action
                const std::vector<std::string> &associated_actions = it->second;
                for (const auto &action_name : associated_actions)
                {
                    updateActionState(action_name, is_down, is_repeat); // 更新action状态
                }
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            Uint32 button = event.button.button; // 获取鼠标按钮
            bool is_down = event.button.down;
            auto it = input_to_actions_.find(button);
            if (it != input_to_actions_.end())
            { // 如果鼠标按钮有对应的action
                const std::vector<std::string> &associated_actions = it->second;
                for (const auto &action_name : associated_actions)
                {
                    // 鼠标事件不考虑repeat, 所以第三个参数传false
                    updateActionState(action_name, is_down, false); // 更新action状态
                }
            }
            // 在点击时更新鼠标位置
            mouse_position_ = {event.button.x, event.button.y};
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: // 处理鼠标运动
            mouse_position_ = {event.motion.x, event.motion.y};
            break;
        case SDL_EVENT_QUIT:
            should_quit_ = true;
            break;
        default:
            break;
        }
    }

    /**
     * @brief 初始化输入映射，根据配置文件设置按键到动作的映射。
     * @param config 指向 engine::core::Config 结构体的指针，包含输入映射配置。
     * @details 该函数根据配置文件中的输入映射，初始化按键到动作的映射。
     *          它会遍历配置中的每个动作，将其关联的按键添加到输入映射中。
     *          如果配置中没有定义鼠标按钮动作(通常不需要配置),则添加默认映射, 用于 UI。
     */
    void InputManager::initializeMappings(const engine::core::Config *config)
    {
        spdlog::trace("[输入管理器] 初始化输入映射...");
        if (!config)
        {
            spdlog::error("[输入管理器] Config 为空指针");
            throw std::runtime_error("[输入管理器] Config 为空指针");
        }
        auto actions_to_keyname = config->input_mappings_; // 获取配置中的输入映射（动作 -> 按键名称）
        input_to_actions_.clear();
        action_states_.clear();

        // 如果配置中没有定义鼠标按钮动作(通常不需要配置),则添加默认映射, 用于 UI
        if (actions_to_keyname.find("mouse_left") == actions_to_keyname.end())
        {
            spdlog::debug("[输入管理器] 配置中没有定义 'MouseLeftClick' 动作,添加默认映射到 'MouseLeft'.");
            actions_to_keyname["mouse_left"] = {"MouseLeft"}; // 如果缺失则添加默认映射
        }
        if (actions_to_keyname.find("mouse_right") == actions_to_keyname.end())
        {
            spdlog::debug("[输入管理器] 配置中没有定义 'MouseRightClick' 动作,添加默认映射到 'MouseRight'.");
            actions_to_keyname["mouse_right"] = {"MouseRight"}; // 如果缺失则添加默认映射
        }
        // 遍历 动作 -> 按键名称 的映射
        for (const auto &[action_name, key_names] : actions_to_keyname)
        {
            // 每个动作对应一个动作状态，初始化为 INACTIVE
            action_states_[action_name] = ActionState::INACTIVE;
            spdlog::trace("[输入管理器] 映射动作: {}", action_name);
            // 设置 "按键 -> 动作" 的映射
            for (const auto &key_name : key_names)
            {
                SDL_Scancode scancode = scancodeFromString(key_name);        // 尝试根据按键名称获取scancode
                Uint32 mouse_button = mouseButtonUint32FromString(key_name); // 尝试根据按键名称获取鼠标按钮
                // 未来可添加其它输入类型 ...

                if (scancode != SDL_SCANCODE_UNKNOWN)
                {
                    // 如果scancode有效,则将action添加到scancode_to_actions_map_中
                    input_to_actions_[scancode].push_back(action_name);
                    spdlog::trace("[输入管理器] 映射按键: {} (Scancode: {}) 到动作: {}", key_name, static_cast<int>(scancode), action_name);
                }
                else if (mouse_button != 0)
                {
                    // 如果鼠标按钮有效,则将action添加到mouse_button_to_actions_map_中
                    input_to_actions_[mouse_button].push_back(action_name);
                    spdlog::trace("[输入管理器] 映射鼠标按钮: {} (Button ID: {}) 到动作: {}", key_name, static_cast<int>(mouse_button), action_name);
                    // else if: 未来可添加其它输入类型 ...
                }
                else
                {
                    spdlog::warn("[输入管理器] 映射警告: 未知键或按钮名称 '{}' 用于动作 '{}'.", key_name, action_name);
                }
            }
        }
        spdlog::trace("[输入管理器] 输入映射初始化完成.");
    }

    /**
     * @brief 更新指定动作的状态。
     * @param action_name 要更新状态的动作名称。
     * @param is_input_active 如果输入当前被激活（按下），则设置为 true；否则设置为 false。
     * @param is_repeat_event 如果这是一个重复事件（例如，按住按键），则设置为 true；否则设置为 false。
     * @details 该函数根据输入事件的状态，更新指定动作的状态。
     *          如果输入被激活（按下），则根据是否为重复事件，将动作状态设置为 PRESSED 或 HELD。
     *          如果输入被释放（松开），则将动作状态设置为 RELEASED。
     */
    void InputManager::updateActionState(std::string_view action_name, bool is_input_active, bool is_repeat_event)
    {
        auto it = action_states_.find(std::string(action_name));
        if (it == action_states_.end())
        {
            spdlog::warn("[输入管理器] 尝试更新未注册的动作状态: {}", action_name);
            return;
        }

        if (is_input_active)
        {
            if (is_repeat_event) // 输入被激活 (按下)
            {
                it->second = ActionState::HELD;
            }
            else
            {
                it->second = ActionState::PRESSED; // 非重复的按下事件
            }
        }
        else
        {
            it->second = ActionState::RELEASED; // 输入被释放 (松开)
        }
    }

    /**
     * @brief 将按键名称转换为 SDL_Scancode。
     * @param key_name 按键名称（例如 "W", "A", "MouseLeft" 等）。
     * @return 对应的 SDL_Scancode。如果未找到匹配项，则返回 SDL_SCANCODE_UNKNOWN。
     */
    SDL_Scancode InputManager::scancodeFromString(std::string_view key_name)
    {
        return SDL_GetScancodeFromName(key_name.data());
    }

    /**
     * @brief 将鼠标按钮名称转换为 SDL 定义的 Uint32 按钮值。
     * @param button_name 鼠标按钮名称（例如 "MouseLeft", "MouseMiddle", "MouseRight" 等）。
     * @return 对应的 SDL 按钮值。如果未找到匹配项，则返回 0。
     */
    Uint32 InputManager::mouseButtonUint32FromString(std::string_view button_name)
    {
        if (button_name == "MouseLeft")
        {
            return SDL_BUTTON_LEFT;
        }
        if (button_name == "MouseMiddle")
        {
            return SDL_BUTTON_MIDDLE;
        }
        if (button_name == "MouseRight")
        {
            return SDL_BUTTON_RIGHT;
        }
        // SDL 还定义了 SDL_BUTTON_X1 和 SDL_BUTTON_X2
        if (button_name == "MouseX1")
        {
            return SDL_BUTTON_X1;
        }
        if (button_name == "MouseX2")
        {
            return SDL_BUTTON_X2;
        }
        return 0; // 0 不是有效的按钮值，表示无效
    }
} // namespace engine::input