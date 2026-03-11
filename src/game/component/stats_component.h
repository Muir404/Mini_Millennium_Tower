#pragma once

namespace game::component
{
    /**
     * @brief 属性组件, 包含实体的基础属性
     */
    struct StatsComponent
    {
        float hp_{0};           ///< 当前生命值
        float max_hp_{0};       ///< 最大生命值
        float atk_{0};          ///< 攻击值
        float def_{0};          ///< 防御值
        float range_{0};        ///< 攻击范围
        float atk_interval_{0}; ///< 攻击间隔
        float atk_timer_{0};    ///< 攻击定时器
        int level_{1};          ///< 等级
        int rarity_{1};         ///< 稀有度, 1-5
    };
} // namespace game::component