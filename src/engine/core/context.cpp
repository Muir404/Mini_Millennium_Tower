#include "context.h"
#include "game_state.h"
#include "../input/input_manager.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../render/text_renderer.h"
#include "../resource/resource_manager.h"
#include "../audio/audio_player.h"
#include <spdlog/spdlog.h>
#include "entt/entt.hpp"

namespace engine::core
{

    Context::Context(
        entt::dispatcher &dispatcher,
        engine::input::InputManager &input_manager,
        engine::render::Renderer &renderer,
        engine::render::Camera &camera,
        engine::render::TextRenderer &texture_renderer,
        engine::resource::ResourceManager &resource_manager,
        engine::audio::AudioPlayer &audio_player,
        engine::core::GameState &game_state) : dispatcher_(dispatcher),
                                               input_manager_(input_manager),
                                               renderer_(renderer),
                                               camera_(camera),
                                               texture_renderer_(texture_renderer),
                                               resource_manager_(resource_manager),
                                               audio_player_(audio_player),
                                               game_state_(game_state)
    {
        spdlog::trace("[Context] 上下文已创建并初始化。");
    }

} // namespace engine::core