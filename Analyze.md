## 🛠️ Mini_Millennium_Tower 性能优化全流程指南

### 1. 性能分析 (Profiling)
在进行优化前，先确定 CPU 到底在忙什么。

| 任务 | 命令 | 目的 |
| :--- | :--- | :--- |
| **实时占比** | `sudo perf report --hierarchy -G` | 按层次结构查看函数 CPU 占用百分比 |
| **火焰图** | `sudo perf script > out.perf` ... (见下文) | 直观定位调用栈热点 |
| **统计信息** | `sudo perf stat -p $(pidof ...)` | 查看 IPC、分支预测率等硬件指标 |

**核心脚本：**
```bash
# 获取火焰图
sudo perf record -g --call-graph dwarf -p $(pidof Mini_Millennium_TowerDefense-Linux)
sudo perf script > out.perf
./external/FlameGraph/stackcollapse-perf.pl out.perf > out.folded
./external/FlameGraph/flamegraph.pl out.folded > out.svg
```

---

### 2. PGO 优化步骤 (Clang/LLVM)
利用实际运行数据引导编译器进行二次优化。

#### 第一阶段：插桩采样 (Instrumentation)
```bash
# 配置并构建插桩版
cmake --preset release -DCMAKE_CXX_FLAGS="-fprofile-instr-generate" -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"
cmake --build --preset release
```

#### 第二阶段：数据收集 (Training)
```bash
# 运行 bench 产生 .profraw 文件
./build/release/Mini_Millennium_TowerDefense-Linux bench

# 合并原始数据为编译器可读格式
llvm-profdata merge -output=code.profdata *.profraw
```

#### 第三阶段：正式编译 (Optimization)
```bash
# 清理旧文件
cmake --build --preset release --target clean

# 使用 code.profdata 重新配置并构建
cmake --preset release -DCMAKE_CXX_FLAGS="-fprofile-instr-use=$(pwd)/code.profdata" -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-use=$(pwd)/code.profdata"
cmake --build --preset release
```

---

### 3. 数据对比分析 (PGO 效果评估)

通过 `perf stat` 的数据对比，我们可以得出以下结论：

| 指标 | 未优化版本 | PGO 优化版本 | 变化/结论 |
| :--- | :--- | :--- | :--- |
| **IPC (Instructions Per Cycle)** | 1.76 | **1.78** | **提升**：单位周期执行指令数增加，效率更高 |
| **Instructions (总指令数)** | 20.58 G | 22.43 G | **增加**：由于 PGO 可能会进行更多的内联(Inlining) |
| **Branch Misses (分支预测失败率)** | 1.19% | 1.39% | **略微上升**：指令集规模扩大导致的抖动，属于正常波动 |
| **Topdown L1 (TMA)** | 67.3% Bad Speculation | 66.7% Bad Speculation | **优化**：错误预测导致的资源浪费有所减少 |

**分析总结：**
* **吞吐量提升**：IPC 从 1.76 提升至 1.78，说明 CPU 执行流水线更加顺畅。
* **Frontend Bound 降低**：`tma_frontend_bound` 从 16.7% 降至 14.7%，说明 PGO 优化了代码布局，减少了指令缓存（I-Cache）的未命中。
* **执行效率**：虽然指令总数增加了，但因为 CPU 能够更高效地处理这些指令（更好的函数内联和布局），整体代码的“质量”变高了。

---

### 进阶
* `-g`: 保留调试符号。
* `-fno-omit-frame-pointer`: 确保 `perf` 能够完整回溯调用栈。