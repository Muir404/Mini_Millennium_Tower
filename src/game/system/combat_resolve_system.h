#pragma once
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>
#include "../defs/events.h"
#include "../binding/lua_binder.h"
namespace game::system
{

    /**
     * @brief 战斗结算系统，用于处理战斗结算
     *
     * 根据接受到的事件（攻击/治疗），执行相应的结算操作。
     */
    class CombatResolveSystem
    {
        entt::registry &registry_;             ///< 实体注册表引用
        entt::dispatcher &dispatcher_;         ///< 事件分发器引用
        game::binding::LuaBinder &lua_binder_; ///< Lua绑定器引用

    public:
        CombatResolveSystem(entt::registry &registry, entt::dispatcher &dispatcher, game::binding::LuaBinder &lua_binder);
        ~CombatResolveSystem();

    private:
        // 事件回调函数
        void onAttackEvent(const game::defs::AttackEvent &event);
        void onHealEvent(const game::defs::HealEvent &event);
    };

} // namespace game::system