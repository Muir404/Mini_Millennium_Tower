[调试模式]
编译命令：cmake --preset clang-debug
构建命令：cmake --build --preset build-debug

运行命令：SDL_AUDIO_DRIVER=dummy ./build-clang-debug/TDSDL-Linux
说明：在调试模式下运行游戏需指定使用虚拟音频驱动，原因在于本游戏基于 SDL3 音频子系统实现，而当前 SDL3 版本的音频驱动存在缺陷。

[发布模式]
编译命令：cmake --preset clang-release
构建命令：cmake --build --preset build-release

运行命令：./build-clang-release/TDSDL-Linux
说明：无

[工作流]
工作流命令：cmake --workflow --preset full-ci
说明：无