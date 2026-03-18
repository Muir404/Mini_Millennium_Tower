#include "debug_ui_system.h"
#include "../component/stats_component.h"
#include "../component/class_name_component.h"
#include "../component/blocker_component.h"
#include "../component/skill_component.h"

#include "../defs/events.h"
#include "../defs/tags.h"

#include "../data/session_data.h"
#include "../data/game_stats.h"
#include "../data/level_data.h"

#include "../factory/blueprint_manager.h"

#include "../../engine/component/name_component.h"

#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/core/time.h"

#include "../../engine/render/renderer.h"

#include "../../engine/audio/audio_player.h"

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
        renderHoveredPortrait();
        renderHoveredUnit();
        renderSelectedUnit();
        renderInfoUI();
        renderSettingUI();
        renderDebugUI();
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

    void DebugUISystem::renderHoveredPortrait()
    {
        // 确定鼠标悬浮的单位肖像存在
        if (hovered_portrait_ == entt::null)
        {
            return;
        }

        // 角色名称不是一个实体，需要从蓝图中获取数据
        const auto &session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
        const auto &blueprint_mgr = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
        const auto &unit_data = session_data->getUnitData(hovered_portrait_);
        const auto &class_blueprint = blueprint_mgr->getPlayerClassBlueprint(unit_data.class_id_);
        const auto &stats = class_blueprint.stats_;
        // 计算等级和稀有度对属性的影响
        const auto hp = engine::utils::statModify(stats.hp_, unit_data.level_, unit_data.rarity_);
        const auto atk = engine::utils::statModify(stats.atk_, unit_data.level_, unit_data.rarity_);
        const auto def = engine::utils::statModify(stats.def_, unit_data.level_, unit_data.rarity_);
        const auto range = stats.range_;
        std::string_view name = class_blueprint.display_info_.name_;
        // std::string_view description = class_blueprint.display_info_.description_;

        // 显示Tooltip信息
        if (!ImGui::BeginTooltip())
        {
            ImGui::EndTooltip();
            spdlog::error("鼠标悬浮单位肖像窗口打开失败");
            return;
        }
        ImGui::Text("%s", unit_data.name_.c_str());
        ImGui::SameLine();
        ImGui::Text("职业: %s", name.data());
        ImGui::Text("等级: %d", unit_data.level_);
        ImGui::SameLine();
        ImGui::Text("稀有度: %d", unit_data.rarity_);
        ImGui::Text("生命值: %d", static_cast<int>(std::round(hp)));
        ImGui::SameLine();
        ImGui::Text("攻击力: %d", static_cast<int>(std::round(atk)));
        ImGui::Text("防御力: %d", static_cast<int>(std::round(def)));
        ImGui::SameLine();
        ImGui::Text("攻击范围: %d", static_cast<int>(std::round(range)));
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
        // 技能显示与交互
        if (auto skill = registry_.try_get<game::component::SkillComponent>(entity); skill)
        {
            // 如果技能准备就绪，则按钮可用（激活技能），否则按钮不可用
            auto ready = registry_.all_of<game::defs::SkillReadyTag>(entity);
            ImGui::BeginDisabled(!ready);
            // 设置快捷键 S 激活技能
            ImGui::SetNextItemShortcut(ImGuiKey_S, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
            if (ImGui::Button(skill->name_.c_str()))
            {
                // 激活技能
                context_.getDispatcher().enqueue<game::defs::SkillActiveEvent>(entity);
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            // 如果技能激活中，则显示"剩余时间"或"被动技能激活中"
            if (registry_.all_of<game::defs::SkillActiveTag>(entity))
            {
                if (registry_.all_of<game::defs::PassiveSkillTag>(entity))
                {
                    ImGui::Text("被动技能激活中");
                }
                else
                {
                    ImGui::Text("激活中，剩余时间: %.1f 秒", skill->duration_ - skill->duration_timer_);
                }
                // 否则显示冷却时间
            }
            else
            {
                ImGui::Text("快捷键 S: ");
                ImGui::SameLine();
                if (registry_.all_of<game::defs::SkillReadyTag>(entity))
                {
                    ImGui::Text("技能准备就绪");
                }
                else
                {
                    // 用进度条显示冷却时间百分比
                    ImGui::ProgressBar(skill->cooldown_timer_ / skill->cooldown_);
                }
            }
            // 显示技能描述
            ImGui::TextWrapped("%s", skill->description_.c_str());
        }
        ImGui::End();
    }

    void DebugUISystem::renderInfoUI()
    {
        if (!ImGui::Begin("关卡信息", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::End();
            spdlog::error("关卡信息窗口打开失败");
            return;
        }
        // 获取关卡相关数据
        const auto &game_stats = registry_.ctx().get<game::data::GameStats &>();
        const auto &waves = registry_.ctx().get<game::data::Waves &>();
        const auto &session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
        // 显示
        ImGui::Text("基地血量: %d / 5", game_stats.home_hp_);
        ImGui::SameLine();
        ImGui::Text("COST: %d", static_cast<int>(game_stats.cost_));
        ImGui::SameLine();
        ImGui::Text("剩余波次: %ld", waves.waves_.size());
        ImGui::SameLine();
        if (waves.waves_.size() > 0)
        {
            ImGui::Text("下一波时间: %d", static_cast<int>(waves.next_wave_count_down_));
        }
        ImGui::SameLine();
        ImGui::Text("击杀数量: %d / %d", game_stats.enemy_killed_count_, game_stats.enemy_count_);
        ImGui::SameLine();
        ImGui::Text("当前关卡: %d", session_data->getLevelNumber());
        ImGui::End();
    }

    void DebugUISystem::renderSettingUI()
    {
        if (!ImGui::Begin("设置工具", nullptr, ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::End();
            spdlog::error("设置工具窗口打开失败");
            return;
        }
        // 场景控制
        auto &game_state = context_.getGameState();
        ImGui::SetNextItemShortcut(ImGuiKey_P, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
        if (game_state.isPaused())
        { // 如果游戏暂停，则显示“继续游戏”按钮，快捷键 P
            if (ImGui::Button("继续游戏"))
            {
                game_state.setState(engine::core::State::Playing);
            }
        }
        else
        { // 如果游戏运行中，则显示“暂停游戏”按钮，快捷键也是 P
            if (ImGui::Button("暂停游戏"))
            {
                game_state.setState(engine::core::State::Paused);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("重新开始"))
        {
            context_.getDispatcher().enqueue<game::defs::RestartEvent>();
        }
        if (ImGui::Button("返回标题"))
        {
            context_.getDispatcher().enqueue<game::defs::BackToTitleEvent>();
        }
        ImGui::SameLine();
        if (ImGui::Button("保存"))
        {
            context_.getDispatcher().enqueue<game::defs::SaveEvent>();
        }
        ImGui::Separator();

        // 游戏速度调节
        auto &time = context_.getTime();
        float time_scale = time.getTimeScale();
        if (ImGui::Button("0.5倍速"))
        {
            time_scale = 0.5f;
            time.setTimeScale(time_scale);
        }
        ImGui::SameLine();
        if (ImGui::Button("1倍速"))
        {
            time_scale = 1.0f;
            time.setTimeScale(time_scale);
        }
        ImGui::SameLine();
        if (ImGui::Button("2倍速"))
        {
            time_scale = 2.0f;
            time.setTimeScale(time_scale);
        }
        ImGui::SliderFloat("游戏速度", &time_scale, 0.5f, 2.0f);
        time.setTimeScale(time_scale);

        // 音乐音量调节
        float music_volume = context_.getAudioPlayer().getMusicVolume();
        ImGui::SliderFloat("音乐音量", &music_volume, 0.0f, 1.0f);
        context_.getAudioPlayer().setMusicVolume(music_volume);
        float sound_volume = context_.getAudioPlayer().getSoundVolume();
        ImGui::SliderFloat("音效音量", &sound_volume, 0.0f, 1.0f);
        context_.getAudioPlayer().setSoundVolume(sound_volume);

        // 切换调试工具显示 （勾选结果保存在show_debug_ui_中）
        ImGui::Checkbox("显示调试工具", &show_debug_ui_);
        ImGui::End();
    }

    void DebugUISystem::renderDebugUI()
    {
        if (!show_debug_ui_)
            return;
        if (!ImGui::Begin("调试工具", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::End();
            spdlog::error("调试工具窗口打开失败");
            return;
        }
        auto &game_stats = registry_.ctx().get<game::data::GameStats &>();
        if (ImGui::Button("COST + 10"))
        {
            game_stats.cost_ += 10;
        }
        if (ImGui::Button("COST + 100"))
        {
            game_stats.cost_ += 100;
        }
        if (ImGui::Button("通关"))
        {
            context_.getDispatcher().enqueue<game::defs::LevelClearEvent>();
        }
        // TODO: 未来可按需添加其他调试工具
        ImGui::End();
    }

    void DebugUISystem::onUIPortraitHoverEnterEvent(const game::defs::UIPortraitHoverEnterEvent &event)
    {
        hovered_portrait_ = event.name_id_;
    }

    void DebugUISystem::onUIPortraitHoverExitEvent()
    {
        hovered_portrait_ = entt::null;
    }
} // namespace game::system