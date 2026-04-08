# qt_ros_car_slam_navi

## 简短说明

这是一个基于 ROS (catkin) + Qt5 + PCL/VTK 的演示/开发仓库，包含一个可选 GUI（基于 Qt + VTK/PCL 可视化）与非 GUI 的轻量化节点。仓库里也包含一个半成品的 Qt 上位机界面（实时显示速度、RGB/深度图并支持键盘控制），地图显示部分存在部分问题待修复。

## 目录结构（概要）

- `src/marker/` — ROS 包 `marker` 的源码（包含 GUI、核心节点与示例订阅器）。
- `devel/`, `build/`, `install/` — catkin 构建输出目录（由 `catkin_make` 生成）。

## 快速说明

- 默认构建配置：为了兼容可能存在的系统 VTK 多版本问题，仓库默认不强制启用可视化 GUI（CMake 选项 `USE_VIS`，默认 OFF）。
- 如果你的系统有统一且兼容的 VTK（例如系统安装的 VTK），可以开启 GUI 构建。

## 先决条件

- Ubuntu / Debian 系统（已在该环境测试）。
- ROS（建议 ROS 1，例如 melodic / noetic，根据你的系统）。
- Qt5 开发包（如果启用 GUI）。
- PCL、VTK（如需 GUI/可视化），以及常见 ROS 包：`roscpp`, `std_msgs`, `sensor_msgs`, `cv_bridge`, `image_transport`, `nav_msgs` 等。

## 常用包安装示例（Debian/Ubuntu）

```bash
sudo apt update
# 按你的 ROS 发行版安装 ROS 桌面版本，例如：
sudo apt install ros-<distro>-desktop-full
# 常见依赖
sudo apt install build-essential cmake qt5-default libqt5widgets5 libvtk9-dev libpcl-dev libopencv-dev
# ROS dev 依赖
sudo apt install ros-<distro>-roscpp ros-<distro>-cv-bridge ros-<distro>-image-transport ros-<distro>-tf2-ros
```

替换 `<distro>` 为你的 ROS 发行版名称（如 `noetic`）。

## 构建（catkin_make）

1. 在工作区根目录下（包含 `src`）运行：

```bash
catkin_make
```

2. 如果你想启用 GUI（仅当系统 VTK 与 PCL 版本一致时）：

```bash
catkin_make -DUSE_VIS=ON
```

注意：`USE_VIS=ON` 会让包查找 VTK/PCL 的可视化组件并构建 GUI 可执行（例如 `marker_gui`）。

## 运行

- 非 GUI（默认）：

```bash
source devel/setup.bash
rosrun marker marker_node
```

- GUI（在 `USE_VIS=ON` 并且系统环境正确时）：

```bash
source devel/setup.bash
rosrun marker marker_gui
```

> 注意：若没有运行 `roscore`，节点会报 "Failed to contact master"，但这不是段错误（SIGSEGV）。

## 已知问题与调试（重要）

- VTK 多版本冲突：如果你的系统同时存在两个 VTK 版本（例如 apt 安装的 `/usr/lib` 下的 VTK 9.1 与手动安装到 `/usr/local/lib` 的 VTK 9.4），运行 GUI 可执行可能会导致段错误（SIGSEGV）。

检查系统上注册的 VTK 库：

```bash
ldconfig -p | grep libvtk
```

如果你看到 `/usr/local/lib` 与 `/usr/lib`（或 `/lib`）同时列出不同版本，可能导致混合加载并触发崩溃。

### 临时运行时解决（非破坏性）

- 运行时优先使用系统库（仅用于快速测试）：

```bash
LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH rosrun marker marker_gui
```

- 若要彻底解决，多数情况下需要移除或备份 `/usr/local/lib` 下冲突的 VTK 库，或重新编译依赖项使其统一使用同一版本的 VTK。

如果你曾移动过 `/usr/local/lib` 下的 VTK（例如移动到 `/usr/local/lib.vtk-backup`），建议保留备份 24-72 小时确认无副作用，再彻底删除。

## 项目维护建议

- 保持可视化模块为可选（`USE_VIS`），以提高可移植性。
- 在 CI / 文档中记录你的系统 VTK/PCL 要求与构建选项。

## 贡献

欢迎提交 Issue 与 Pull Request。请在提交时说明你的系统环境（OS、ROS 版本、VTK/PCL 版本）。

## 远程仓库

- upstream: https://github.com/Matr1xjt/qt_ros_car_slam_navi.git

## 联系方式

- 在 GitHub 仓库页面提交 Issue 或 PR。
