#pragma once
#include "../../engine/utils/math.h"
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <unordered_map>
#include <vector>

namespace engine::component
{

    /**
     * @brief 单一动画
     */
    struct AnimationFrame
    {
        engine::utils::Rect src_rect_{}; ///< 帧源矩形
        float duration_ms_{100.0f};      ///< 帧间隔（毫秒）

        AnimationFrame(engine::utils::Rect src_rect,
                       float duration_ms = 100.0f) : src_rect_(std::move(src_rect)),
                                                     duration_ms_(duration_ms)
        {
            // 构造函数
        }
    };

    /**
     * @brief 动画map
     */
    struct Animation
    {
        std::vector<AnimationFrame> frames_; ///< 动画帧
        float total_duration_ms_{};          ///< 动画总时长（毫秒）
        bool loop_{true};                    ///< 是否循环

        Animation(std::vector<AnimationFrame> frames,
                  bool loop = true) : frames_(std::move(frames)),
                                      loop_(loop)
        {
            // 计算动画总时长 (总时长 = 所有帧时长之和)
            total_duration_ms_ = 0.0f;
            for (const auto &frame : frames_)
            {
                total_duration_ms_ += frame.duration_ms_;
            }
        }
    };

    /**
     * @brief AnimationComponent
     */
    struct AnimationComponent
    {
        std::unordered_map<entt::id_type, Animation> animations_; ///< 动画集合
        entt::id_type current_animation_id_{entt::null};          ///< 当前播放的动画名称
        size_t current_frame_index_{};                            ///< 当前播放的帧索引
        float current_time_ms_{};                                 ///< 当前播放时间（毫秒）
        float speed_{1.0f};                                       ///< 播放速度

        AnimationComponent(std::unordered_map<entt::id_type, Animation> animations,
                           entt::id_type current_animation_id,
                           size_t current_frame_index = 0,
                           float current_time_ms = 0.0f,
                           float speed = 1.0f) : animations_(std::move(animations)),
                                                 current_animation_id_(current_animation_id),
                                                 current_frame_index_(current_frame_index),
                                                 current_time_ms_(current_time_ms),
                                                 speed_(speed) {}
    };
} // namespace engine::component