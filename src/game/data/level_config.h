#pragma once
#include <string_view>
#include "../data/level_data.h"

namespace game::data
{

    /**
     * @brief 关卡配置类
     * @note 负责载入json配置文件，并从中获取各类关卡数据
     */
    class LevelConfig
    {
        std::vector<game::data::LevelData> level_data_; ///< @brief 关卡数据（每关对应一个LevelData）

    public:
        bool loadFromFile(std::string_view level_json_path = "assets/data/level_config.json"); ///< @brief 加载关卡配置文件

        // --- getters （获取指定关卡编号的对应数据） --- （关卡编号从1开始，数组角标从0开始，因此每次获取时需要减1）
        [[nodiscard]] game::data::LevelData &getLevelData(int level_number) { return level_data_[level_number - 1]; } ///< @brief 获取指定关卡编号的关卡数据

        [[nodiscard]] game::data::Waves &getWavesData(int level_number) { return level_data_[level_number - 1].waves_data_; }     ///< @brief 获取指定关卡编号的波次数据
        [[nodiscard]] int getLevelCount() const { return level_data_.size(); }                                                    ///< @brief 获取关卡总数
        [[nodiscard]] std::string_view getMapPath(int level_number) const { return level_data_[level_number - 1].map_path_; }     ///< @brief 获取指定关卡编号的地图路径
        [[nodiscard]] int getTotalEnemyCount(int level_number) const { return level_data_[level_number - 1].total_enemy_count_; } ///< @brief 获取指定关卡编号的总敌人数量
        [[nodiscard]] bool isFinalLevel(int level_number) const { return level_number == getLevelCount(); }                       ///< @brief 判断指定关卡编号是否为最后一关
        [[nodiscard]] int getEnemyLevel(int level_number) const { return level_data_[level_number - 1].enemy_level_; }            ///< @brief 获取指定关卡编号的敌人等级
        [[nodiscard]] int getEnemyRarity(int level_number) const { return level_data_[level_number - 1].enemy_rarity_; }          ///< @brief 获取指定关卡编号的敌人稀有度
    };

} // namespace game::data