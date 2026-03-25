# ARCHITECTURE

## Summary

项目采用四层结构：

- `UI`
- `Application Service`
- `Device Adapter`
- `Storage / Data Model`

当前实现已经可以支撑桌面端原型联调和 Mock 设备演示，后续真实设备接入应尽量保持 UI 层无感。

## 架构总览图

```text
MainWindow / Pages / Dialogs
          |
          v
Application Services
  - ConnectionService
  - ScanControlService
  - TuneService
  - MonitorService
  - PersistenceService
  - SettingsService
          |
          v
IDeviceAdapter --------------------> Repositories
  - MockDeviceAdapter                - SettingsRepository
  - RealDeviceAdapter                - MethodRepository
                                     - FrameRepository
          |
          v
Core Types / JSON Models
```

## 应用装配关系

当前装配由 `ApplicationContext` 统一完成，关系固定为：

```text
ApplicationContext
  -> MockDeviceAdapter
  -> SettingsRepository
  -> MethodRepository
  -> FrameRepository
  -> ConnectionService
  -> ScanControlService
  -> TuneService
  -> MonitorService
  -> PersistenceService
  -> SettingsService
```

当前默认设备层直接使用 `MockDeviceAdapter`。后续切换到真实设备时，优先替换 `deviceAdapter` 的具体实现，不改 UI 页面调用方式。

## 分层说明

### 1. UI 层

路径：`src/ui`

职责：

- 主窗口布局与页面切换
- 调谐、监测、数据、设置等工作区界面
- 扫描设置、系统设置、谱图设置等弹窗
- 将用户操作转为 service 调用

核心入口：

- `src/ui/main_window.cpp`
- `src/ui/pages/tune_page.cpp`
- `src/ui/pages/monitor_page.cpp`
- `src/ui/pages/data_page.cpp`
- `src/ui/pages/settings_page.cpp`

页面与职责映射：

- `TunePage`
  调谐参数、扫描参数摘要、谱图展示、右侧状态/连接
- `MonitorPage`
  方法运行、RIC/TIC、参数配置、仪器状态
- `DataPage`
  单帧数据展示入口
- `SettingsPage`
  连接、存储、管道服务测试
- `MainWindow`
  页面装配、顶层菜单、全局工具栏、弹窗入口

### 2. Application Service 层

路径：`src/app`

职责：

- 串联 UI、设备适配器和本地存储
- 管理连接、扫描、调谐、方法保存、设置保存
- 对 UI 提供稳定调用面

主要服务：

- `ConnectionService`
- `ScanControlService`
- `TuneService`
- `MonitorService`
- `PersistenceService`
- `SettingsService`

装配入口：

- `ApplicationContext`

服务与页面关系：

- `MainWindow -> ConnectionService`
- `MainWindow -> ScanControlService`
- `TunePage -> TuneService`
- `MonitorPage -> MonitorService`
- `DataPage / FrameViewer -> PersistenceService`
- `Settings / Dialogs -> SettingsService`

### 3. Device Adapter 层

路径：`src/device`

职责：

- 对外暴露统一设备接口
- 封装 Mock 与 Real 设备差异
- 负责连接、扫描、状态读取、开关控制

核心类型：

- `IDeviceAdapter`
- `MockDeviceAdapter`
- `RealDeviceAdapter`

当前状态：

- `MockDeviceAdapter` 为主实现
- `RealDeviceAdapter` 为协议预留壳层

设备层对上游输出：

- 连接状态变更
- 仪器状态快照
- 谱图帧更新
- 校正结果
- 可持久化的单帧数据
- 错误事件

### 4. Storage / Data Model 层

路径：

- `src/core`
- `src/storage`

职责：

- 定义业务核心类型
- 管理设置、方法、单帧数据的本地存储
- 提供 JSON / CSV 的持久化能力

核心类型示例：

- `DeviceConnectionConfig`
- `ScanSettings`
- `TuneParameters`
- `MonitorMethod`
- `InstrumentStatus`
- `SpectrumFrame`
- `PeakInfo`

仓储类型：

- `SettingsRepository`
- `MethodRepository`
- `FrameRepository`

推荐边界：

- 业务字段演进优先改 `src/core/types.h`
- 文件格式演进优先改 `Repository`
- 页面只依赖类型，不直接依赖文件结构

## 主要运行流

### 应用启动

1. `main.cpp` 创建应用
2. `ApplicationContext` 装配 adapter、repository、service
3. `MainWindow` 创建页面并绑定 signals / slots
4. UI 默认加载本地设置和最近连接配置

应用启动数据流：

```text
main.cpp
  -> ApplicationContext
  -> MainWindow
  -> SettingsService / ConnectionService 读取本地配置
  -> 页面初始化默认状态
```

### 调谐流

1. 用户在调谐页设置扫描参数与调谐参数
2. `TunePage` 发出开始扫描或应用参数请求
3. `ScanControlService` 负责联机与前置条件检查
4. `TuneService` 下发调谐参数到设备层
5. `IDeviceAdapter` 返回状态和扫描帧
6. UI 更新谱图与仪器状态

调谐运行数据流：

```text
TunePage
  -> MainWindow
  -> TuneService / ScanControlService
  -> IDeviceAdapter
  -> statusUpdated / frameUpdated
  -> MainWindow
  -> TunePage / MonitorPage / 状态栏
```

### 监测流

1. 用户在监测页设置检测器、扫描方式、方法参数
2. `MonitorService` 保存/加载方法
3. `ScanControlService` 启动或停止扫描
4. `IDeviceAdapter` 输出扫描帧
5. 监测页刷新 `RIC/TIC` 和仪器状态

监测方法数据流：

```text
MonitorPage
  -> MonitorService 保存/加载方法
  -> MethodRepository

MonitorPage
  -> ScanControlService
  -> IDeviceAdapter
  -> frameUpdated
  -> MonitorPage 更新图表
```

### 数据流

1. 单帧数据由 `PersistenceService` 保存
2. `FrameRepository` 负责 JSON / CSV 持久化
3. 数据页和单帧查看弹窗负责加载并展示

设置数据流：

```text
Dialogs / SettingsPage
  -> SettingsService
  -> SettingsRepository
  -> 本地 JSON / QSettings
```

## 页面到服务映射表

| 页面/模块 | 主要依赖服务 | 主要依赖数据 |
| --- | --- | --- |
| `MainWindow` | `ConnectionService`, `ScanControlService`, `SettingsService` | `InstrumentStatus`, `SpectrumFrame` |
| `TunePage` | `TuneService`, `ScanControlService` | `ScanSettings`, `TuneParameters`, `InstrumentStatus` |
| `MonitorPage` | `MonitorService`, `ScanControlService` | `MonitorMethod`, `SpectrumFrame`, `InstrumentStatus` |
| `DataPage` | `PersistenceService` | `SpectrumFrame` |
| `SettingsPage` | `ConnectionService`, `SettingsService` | `DeviceConnectionConfig` |
| `Dialogs` | `SettingsService`, `PersistenceService` | 各类设置类型与单帧数据 |

## 当前架构特点

- UI 不直接依赖具体设备协议
- Mock 设备已经可以驱动界面演示
- 服务层已经承担大部分调度逻辑
- 仓储层承担本地配置、方法和数据持久化

## 当前已知限制

- 真实设备协议尚未落地
- 文档化的接口契约仍偏轻量
- 打包和发布刚完成基础设施，未做签名/公证
- 页面样式仍处于截图对标阶段，设计系统未完全抽象
