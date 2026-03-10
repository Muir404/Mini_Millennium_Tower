#include "remove_dead_system.h"
#include "../defs/tags.h"
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>
namespace game::system
{
    void RemoveDeadSystem::update(entt::registry &registry)
    {
        // 标签为空，所以和组件一样处理使用
        auto view = registry.view<game::defs::DeadTag>();
        for (auto entity : view)
        {
            registry.destroy(entity);
            spdlog::info("[RemoveDeadSystem:update] 删除死亡实体 {} 成功", entt::to_integral(entity));
        }
    }
} // namespace game::system
