-- -- scripts/damage_formula.lua
-- -- 可热修改的伤害计算公式
-- function calculate_effective_damage(attacker_atk, target_def)
--     -- 基础公式：攻击力 - 防御力
--     local base_damage = attacker_atk - target_def
--     -- 最小伤害为攻击力的10%
--     local min_damage = attacker_atk * 0.1
--     -- 可自定义加成（比如暴击、属性克制等）
--     local final_damage = math.max(base_damage, min_damage)
    
--     -- 示例：添加随机暴击（10%概率翻倍）
--     local crit_chance = 0.1
--     if math.random() < crit_chance then
--         final_damage = final_damage * 2
--         log_info("Critical hit! Damage doubled")
--     end
    
--     return final_damage
-- end

-- -- 可选：暴露配置参数，方便调整
-- DAMAGE_CONFIG = {
--     min_damage_ratio = 0.1,
--     crit_chance = 0.1,
--     crit_multiplier = 2.0
-- }

