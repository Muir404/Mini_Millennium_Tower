-- ==============================================
-- 🔥 战斗公式配置文件
-- 修改这里的代码，游戏会自动热更新，不用重启、不用编译
-- ==============================================

--- 计算有效伤害
--- @param attacker_atk number 攻击者攻击力
--- @param target_def number 目标防御力
--- @return number 最终造成的伤害
function calculateEffectiveDamage(attacker_atk, target_def)
    -- ======================
    -- 在这里修改你的伤害公式
    -- ======================

    -- 原版公式：伤害=攻击力-防御力，最低为攻击力的10%
    local damage = attacker_atk - target_def
    damage = math.max(damage, 0.1 * attacker_atk)

    -- 示例1：改成带暴击的公式
    local crit_rate = 1    -- 60%暴击率
    local crit_multi = 100 -- 1.5倍暴击伤害
    local damage = attacker_atk - target_def * 0.8
    if math.random() <= crit_rate then
        damage = damage * crit_multi
        print("💥 暴击了！伤害为：" .. damage)
    end
    damage = math.max(damage, 5) -- 最低伤害5点

    return damage
end
