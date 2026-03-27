[桌面端]
[调试模式]
编译命令：cmake --preset dev
构建命令：cmake --build --preset dev

运行命令：SDL_AUDIO_DRIVER=dummy ./build/dev/TDSDL-Linux
说明：在调试模式下运行游戏需指定使用虚拟音频驱动，原因在于本游戏基于 SDL3 音频子系统实现，而当前 SDL3 版本的音频驱动存在缺陷。

[发布模式]
编译命令：cmake --preset release
构建命令：cmake --build --preset release

运行命令：./build/release/TDSDL-Linux
说明：无

[性能分析模式]
编译命令：cmake --preset relwithdbg
构建命令：cmake --build --preset relwithdbg
运行命令：./build/relwithdbg/TDSDL-Linux

[说明]
| Preset           | 编译器优化 | Sanitizers       | LTO         | 适用场景                                             |
| :--------------- | :--------- | :--------------- | :---------- | :--------------------------------------------------- |
| **`dev`**        | `-O0 -g`   | **ASAN + UBSAN** | ❌           | **日常开发**。能立即发现内存越界、空指针等致命错误。 |
| **`release`**    | `-O3`      | ❌                | **ThinLTO** | **最终发布**。性能最强。                             |
| **`relwithdbg`** | `-O2 -g`   | ❌                | ❌           | **性能调优**。配合 Tracy 或 Perf 使用。              |

[Web端]
[发布模式]
激活环境：source ./external/emsdk/emsdk_env.sh
编译命令：emcmake cmake -B build_web
构建命令：emake --build build_web -j$(nproc)
创建服务器：python3 -m http.server 8000
访问游戏：http://localhost:8000/build_web/TDSDL-Emscripten.html


