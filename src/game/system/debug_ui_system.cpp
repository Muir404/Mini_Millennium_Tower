#include "debug_ui_system.h"
#include "../component/stats_component.h"
#include "../component/class_name_component.h"
#include "../component/blocker_component.h"
#include "../../engine/component/name_component.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/render/renderer.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace game::system
{

    DebugUISystem::DebugUISystem(entt::registry &registry, engine::core::Context &context) : registry_(registry),
                                                                                             context_(context)
    {
    }

    void DebugUISystem::update()
    {
        beginFrame();
        renderHoveredUnit();
        renderSelectedUnit();
        endFrame();
    }

    void DebugUISystem::beginFrame()
    {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();

        // --- 核心修正：同步 ImGui 与 SDL 的逻辑空间 ---
        ImGuiIO &io = ImGui::GetIO();
        int lw, lh;
        // 获取 SDL 当前定义的逻辑分辨率（即你在 initSDL 中设置的那个“虚拟画布”大小）
        SDL_GetRenderLogicalPresentation(context_.getRenderer().getSDLRenderer(), &lw, &lh, NULL);

        // 1. 告诉 ImGui：你的“世界”只有这么大。
        // 这确保了 ImGui 的窗口、按钮、文字布局是基于逻辑坐标生成的。
        // 即使物理窗口被拉伸，ImGui 内部计算依然保持稳定。
        io.DisplaySize = ImVec2((float)lw, (float)lh);

        // 2. 修正鼠标坐标映射
        // SDL3 默认传给 ImGui 的鼠标位置可能是基于物理窗口像素的（例如鼠标在窗口右下角可能是 2560, 1440）。
        // 但 ImGui 现在的 DisplaySize 只有 640x360，它无法理解 2560 这么大的数值。
        float mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        // 调用 SDL3 专门的转换函数：将“窗口物理坐标”映射回“渲染器逻辑坐标”。
        // 它会自动处理 Letterbox（黑边）导致的偏移和缩放比例。
        // 比如物理坐标 (2560, 1440) 经过此函数可能会变成逻辑坐标 (640, 360)。
        float logical_mouse_x, logical_mouse_y;
        SDL_RenderCoordinatesFromWindow(context_.getRenderer().getSDLRenderer(),
                                        mouse_x, mouse_y,
                                        &logical_mouse_x, &logical_mouse_y);

        // 把转换后的“逻辑鼠标位置”手动喂给 ImGui。
        io.AddMousePosEvent(logical_mouse_x, logical_mouse_y);
        // -------------------

        ImGui::NewFrame();
    }

    void DebugUISystem::endFrame()
    {
        ImGui::Render();
        SDL_Renderer *renderer = context_.getRenderer().getSDLRenderer();

        // --- 核心修正：绕过渲染器的二次缩放 ---

        // 当 SDL 开启逻辑分辨率时，它内部会自动设置一个 Scale（缩放倍数）。
        // 比如从 640 映射到 2560，Scale 可能是 4.0。
        float old_scale_x, old_scale_y;
        SDL_GetRenderScale(renderer, &old_scale_x, &old_scale_y);

        // 重点：我们将渲染器的 Scale 临时重置为 1.0。
        // 为什么？因为 ImGui 的顶点数据已经是基于逻辑坐标 (640, 360) 生成的了。
        // 如果渲染器 Scale 还是 4.0，ImGui 会被画得巨大无比（4倍大）。
        // 我们在这里“裸画”逻辑坐标，SDL 最终在 Present() 时会整体把这层画布拉伸。
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);

        // 执行渲染：此时 ImGui 以逻辑坐标 1:1 的比例绘制在虚拟画布上。
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        // 渲染完成后还原缩放，保证下一帧其他游戏物体的渲染（可能依赖这个 Scale）不受影响。
        SDL_SetRenderScale(renderer, old_scale_x, old_scale_y);
    }

    /*
    void DebugUISystem::beginFrame()
    {
        // 开始新帧
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 关闭逻辑分辨率 (ImGui目前对于SDL逻辑分辨率支持不好，所以使用时先关闭)
        if (!context_.getGameState().disableLogicalPresentation())
        {
            spdlog::error("关闭逻辑分辨率失败");
        }
    }

    void DebugUISystem::endFrame()
    {
        // ImGui: 渲染
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), context_.getRenderer().getSDLRenderer());

        // 渲染完成后，打开(恢复)逻辑分辨率
        if (!context_.getGameState().enableLogicalPresentation())
        {
            spdlog::error("启用逻辑分辨率失败");
        }
    }
    */

    void DebugUISystem::renderHoveredUnit()
    {
        // 确定鼠标悬浮的单位存在
        auto &entity = registry_.ctx().get<entt::entity &>("hovered_unit"_hs);
        if (entity == entt::null || !registry_.valid(entity))
            return;

        // Tooltip 是悬浮在鼠标上的小窗口，可以显示单位信息
        if (!ImGui::BeginTooltip())
        {
            ImGui::EndTooltip();
            spdlog::error("鼠标悬浮单位窗口打开失败");
            return;
        }
        // 获取必要信息并显示
        const auto &stats = registry_.get<game::component::StatsComponent>(entity);
        const auto &class_name = registry_.get<game::component::ClassNameComponent>(entity);
        // 只有玩家单位才有姓名，所以需要尝试获取
        if (auto name = registry_.try_get<engine::component::NameComponent>(entity); name)
        {
            ImGui::Text("%s  ", name->name_.c_str());
            ImGui::SameLine();
        }
        ImGui::Text("%s", class_name.class_name_.c_str());
        ImGui::Text("等级: %d", stats.level_);
        ImGui::SameLine();
        ImGui::Text("稀有度: %d", stats.rarity_);
        ImGui::Text("生命值: %d/%d", static_cast<int>(std::round(stats.hp_)), static_cast<int>(std::round(stats.max_hp_)));
        ImGui::Text("攻击力: %d", static_cast<int>(std::round(stats.atk_)));
        ImGui::Text("防御力: %d", static_cast<int>(std::round(stats.def_)));
        ImGui::Text("攻击范围: %d", static_cast<int>(std::round(stats.range_)));
        ImGui::Text("攻击间隔: %.2f", stats.atk_interval_);
        ImGui::EndTooltip();
    }

    void DebugUISystem::renderSelectedUnit()
    {
        // 确定选中的单位存在
        auto &entity = registry_.ctx().get<entt::entity &>("selected_unit"_hs);
        if (entity == entt::null || !registry_.valid(entity))
            return;

        // 设置窗口位置在左上角
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

        if (!ImGui::Begin("角色状态", nullptr, ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::End();
            spdlog::error("角色状态窗口打开失败");
            return;
        }
        // 获取必要信息并显示
        const auto &stats = registry_.get<game::component::StatsComponent>(entity);
        const auto &class_name = registry_.get<game::component::ClassNameComponent>(entity);
        const auto blocker = registry_.try_get<game::component::BlockerComponent>(entity);
        if (auto name = registry_.try_get<engine::component::NameComponent>(entity); name)
        {
            ImGui::Text("%s  ", name->name_.c_str());
            ImGui::SameLine();
        }
        ImGui::Text("%s", class_name.class_name_.c_str());
        ImGui::Text("等级: %d", stats.level_);
        ImGui::SameLine();
        ImGui::Text("稀有度: %d", stats.rarity_);
        ImGui::Text("生命值: %d/%d", static_cast<int>(std::round(stats.hp_)), static_cast<int>(std::round(stats.max_hp_)));
        ImGui::Text("攻击力: %d", static_cast<int>(std::round(stats.atk_)));
        ImGui::SameLine();
        ImGui::Text("防御力: %d", static_cast<int>(std::round(stats.def_)));
        ImGui::Text("攻击范围: %d", static_cast<int>(std::round(stats.range_)));
        ImGui::SameLine();
        ImGui::Text("攻击间隔: %.2f", stats.atk_interval_);
        if (blocker)
        {
            ImGui::Text("阻挡数量: %d/%d", blocker->current_count_, blocker->max_count_);
        }
        // TODO: 技能相关按钮与信息
        ImGui::End();
    }
} // namespace game::system