#include "scene_manager.h"
#include "scene.h"
#include "../core/context.h"
#include <spdlog/spdlog.h>

namespace engine::scene
{

    SceneManager::SceneManager(engine::core::Context &context)
        : context_(context)
    {
        // 注册事件回调
        context_.getDispatcher().sink<engine::utils::PopSceneEvent>().connect<&SceneManager::onPopScene>(*this);
        context_.getDispatcher().sink<engine::utils::PushSceneEvent>().connect<&SceneManager::onPushScene>(*this);
        context_.getDispatcher().sink<engine::utils::ReplaceSceneEvent>().connect<&SceneManager::onReplaceScene>(*this);
        spdlog::trace("[SceneManager] 场景管理器已创建。");
    }

    SceneManager::~SceneManager()
    {
        spdlog::trace("[SceneManager] 场景管理器已销毁。");
        close(); // 即使不手动调用 close 也能确保清理
    }

    Scene *SceneManager::getCurrentScene() const
    {
        if (scene_stack_.empty())
        {
            spdlog::warn("[SceneManager] 场景栈为空，无法获取当前场景。");
            return nullptr;
        }
        return scene_stack_.back().get(); // 返回栈顶场景的裸指针
    }

    void SceneManager::update(float delta_time)
    {
        // 只更新栈顶（当前）场景
        Scene *current_scene = getCurrentScene();
        if (current_scene)
        {
            current_scene->update(delta_time);
        }
        // 执行可能的切换场景操作
        processPendingActions();
    }

    void SceneManager::render()
    {
        // 渲染时需要叠加渲染所有场景，而不只是栈顶
        for (const auto &scene : scene_stack_)
        {
            if (scene)
            {
                scene->render();
            }
        }
    }

    void SceneManager::close()
    {
        spdlog::trace("[SceneManager] 正在关闭场景管理器并清理场景栈...");
        // 清理栈中所有剩余的场景（从顶到底）
        while (!scene_stack_.empty())
        {
            if (scene_stack_.back())
            {
                spdlog::debug("[SceneManager] 正在清理场景 '{}' 。", scene_stack_.back()->getName());
                scene_stack_.back()->clean();
            }
            scene_stack_.pop_back();
        }
        // 一次性断开所有事件回调
        context_.getDispatcher().disconnect(this);
    }

    void SceneManager::onPopScene()
    {
        pending_action_ = PendingAction::Pop;
    }

    void SceneManager::onPushScene(engine::utils::PushSceneEvent &event)
    {
        pending_action_ = PendingAction::Push;
        pending_scene_ = std::move(event.scene);
    }

    void SceneManager::onReplaceScene(engine::utils::ReplaceSceneEvent &event)
    {
        pending_action_ = PendingAction::Replace;
        pending_scene_ = std::move(event.scene);
    }

    void SceneManager::processPendingActions()
    {
        if (pending_action_ == PendingAction::None)
        {
            return;
        }

        switch (pending_action_)
        {
        case PendingAction::Pop:
            popScene();
            break;
        case PendingAction::Replace:
            replaceScene(std::move(pending_scene_));
            break;
        case PendingAction::Push:
            pushScene(std::move(pending_scene_));
            break;
        default:
            break;
        }

        pending_action_ = PendingAction::None;
    }

    void SceneManager::pushScene(std::unique_ptr<Scene> &&scene)
    {
        if (!scene)
        {
            spdlog::warn("[SceneManager] 尝试将空场景压入栈。");
            return;
        }
        spdlog::debug("[SceneManager] 正在将场景 '{}' 压入栈。", scene->getName());

        // 初始化新场景
        if (!scene->isInitialized())
        {
            scene->init(); // 确保只初始化一次
        }

        // 将新场景移入栈顶
        scene_stack_.push_back(std::move(scene));
    }

    void SceneManager::popScene()
    {
        if (scene_stack_.empty())
        {
            spdlog::warn("[SceneManager] 尝试从空场景栈中弹出。");
            return;
        }
        spdlog::debug("[SceneManager] 正在从栈中弹出场景 '{}' 。", scene_stack_.back()->getName());

        // 清理并移除栈顶场景
        if (scene_stack_.back())
        {
            scene_stack_.back()->clean(); // 显式调用清理
        }
        scene_stack_.pop_back();
        if (scene_stack_.empty())
        {
            spdlog::warn("[SceneManager] 场景栈已空，无法弹出,即将退出游戏");
            context_.getDispatcher().enqueue<engine::utils::QuitEvent>();
        }
    }

    void SceneManager::replaceScene(std::unique_ptr<Scene> &&scene)
    {
        if (!scene)
        {
            spdlog::warn("[SceneManager] 尝试用空场景替换。");
            return;
        }
        spdlog::debug("[SceneManager] 正在用场景 '{}' 替换场景 '{}' 。", scene->getName(), scene_stack_.back()->getName());

        // 清理并移除场景栈中所有场景
        while (!scene_stack_.empty())
        {
            if (scene_stack_.back())
            {
                scene_stack_.back()->clean();
            }
            scene_stack_.pop_back();
        }

        // 初始化新场景
        if (!scene->isInitialized())
        {
            scene->init();
        }

        // 将新场景压入栈顶
        scene_stack_.push_back(std::move(scene));
    }

} // namespace engine::scene