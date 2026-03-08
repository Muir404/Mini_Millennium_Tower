#include "ysort_system.h"
#include "../component/render_component.h"
#include "../component/transform_component.h"
#include <entt/entt.hpp>

namespace engine::system
{
    void YSortSystem::update(entt::registry &registry)
    {
        auto view = registry.view<component::RenderComponent, const component::TransformComponent>();
        for (auto entity : view)
        {
            auto &render = view.get<component::RenderComponent>(entity);                   // 获取渲染组件
            const auto &transform = view.get<const component::TransformComponent>(entity); // 获取变换组件
            render.depth_ = transform.position_.y;                                         // 按 y 坐标排序
        }
    }
}