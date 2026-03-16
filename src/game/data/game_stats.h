#pragma once

namespace game::data
{
    /**
     * @brief 游戏统计数据(关卡内数据)
     *
     */
    struct GameStats
    {
        float cost_{10.0f};               ///< 单位生成成本
        float cost_gen_per_second_{2.0f}; ///< 每秒生成成本
        int home_hp_{100};                ///< 家的健康值
        int enemy_count_{0};              ///< 敌人数量
        int enemy_arrived_count_{0};      ///< 敌人到达数量
        int enemy_killed_count_{0};       ///< 敌人被击杀数量
    };
}