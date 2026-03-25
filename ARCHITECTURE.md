# ARCHITECTURE

## Summary

项目采用四层结构：

- `UI`
- `Application Service`
- `Device Adapter`
- `Storage / Data Model`

当前实现已经可以支撑桌面端原型联调和 Mock 设备演示，后续真实设备接入应尽量保持 UI 层无感。

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

## 主要运行流

### 应用启动

1. `main.cpp` 创建应用
2. `ApplicationContext` 装配 adapter、repository、service
3. `MainWindow` 创建页面并绑定 signals / slots
4. UI 默认加载本地设置和最近连接配置

### 调谐流

1. 用户在调谐页设置扫描参数与调谐参数
2. `TunePage` 发出开始扫描或应用参数请求
3. `ScanControlService` 负责联机与前置条件检查
4. `TuneService` 下发调谐参数到设备层
5. `IDeviceAdapter` 返回状态和扫描帧
6. UI 更新谱图与仪器状态

### 监测流

1. 用户在监测页设置检测器、扫描方式、方法参数
2. `MonitorService` 保存/加载方法
3. `ScanControlService` 启动或停止扫描
4. `IDeviceAdapter` 输出扫描帧
5. 监测页刷新 `RIC/TIC` 和仪器状态

### 数据流

1. 单帧数据由 `PersistenceService` 保存
2. `FrameRepository` 负责 JSON / CSV 持久化
3. 数据页和单帧查看弹窗负责加载并展示

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
