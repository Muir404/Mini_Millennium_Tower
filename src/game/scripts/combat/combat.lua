local CombatConfig = {
    CRIT_RATE = 0.6,       -- 60% 暴击率
    CRIT_MULTIPLIER = 1.5, -- 1.5倍暴击伤害
    MIN_DAMAGE = 20        -- 最低保底伤害
}

function calculateEffectiveDamage(attacker_atk, target_def)
    -- 安全处理：防止传入 nil 导致计算报错
    attacker_atk = attacker_atk or 0
    target_def = target_def or 0

    -- 计算基础伤害：攻击力 - 防御力的20%
    local damage = attacker_atk - (target_def * 0.25)

    -- 判定暴击
    local is_crit = false
    if math.random() <= CombatConfig.CRIT_RATE then
        damage = damage * CombatConfig.CRIT_MULTIPLIER
        is_crit = true
        -- print("Critical Damage Occurred! -- Damage:", damage)
    end

    -- 保底伤害检查
    damage = math.max(damage, CombatConfig.MIN_DAMAGE)

    -- 建议：返回伤害值的同时，可以返回是否暴击，方便 C++ 端弹出“暴击”字样
    -- return damage, is_crit
    return damage
end
