# API_GUIDE

## Summary

这份文档整理当前项目的核心类型、设备接口、服务接口和仓储接口，作为 UI、服务层和后续真实设备接入的共同契约参考。

当前约束：

- UI 不直接操作具体设备协议
- 设备层统一通过 `IDeviceAdapter`
- 业务核心类型统一放在 `src/core/types.h`
- 本地持久化统一通过 `Repository + Service`

## 核心枚举

定义位置：

- `src/core/types.h`

### ScanMode

- `FullScan`
- `SelectedIon`

用途：

- 标识全扫描或选择离子扫描

### DetectorType

- `ElectronMultiplier`
- `FaradayCup`

用途：

- 标识检测器类型

### InstrumentSwitch

- `ForePump`
- `ForeValve`
- `MolecularPump`
- `InletValve`
- `Filament`
- `Multiplier`

用途：

- 对应工具栏和设备开关控制

## 核心数据类型

### DeviceConnectionConfig

字段：

- `host`
- `port`

用途：

- 最近连接记录
- 设备连接配置

### ScanSettings

用途：

- 扫描参数统一模型

主要字段：

- `mode`
- `massStart`
- `massEnd`
- `scanSpeed`
- `scanTimeMs`
- `flybackTimeMs`
- `sampleRateHz`
- `massAxisSlope`
- `massAxisOffset`
- `targetDwellTimeMs`
- `targetPeakWidth`
- `rampVoltage`
- `targetIons`
- `targetVoltages`

补充能力：

- `isValid(QString *errorMessage = nullptr) const`

### TuneParameters

用途：

- 调谐页参数配置模型

主要字段：

- `detector`
- `repellerVoltage`
- `lens1Voltage`
- `lens2Voltage`
- `highMassCompensation`
- `lowMassCompensation`
- `multiplierVoltage`
- `rodVoltage`
- `eVoltage`
- `electronEnergy`
- `filamentCurrent`
- `outerDeflectionLensVoltage`
- `innerDeflectionLensVoltage`
- `preQuadrupoleFrontVoltage`
- `preQuadrupoleRearVoltage`

### MonitorMethod

用途：

- 监测页方法文件模型

主要字段：

- `name`
- `scanSettings`
- `detector`
- `dwellTimeMs`

### InstrumentStatus

用途：

- 设备状态快照

主要字段：

- `connected`
- `scanning`
- `vacuum`
- `temperatures`
- `switchStates`
- `lastError`

### SpectrumFrame

用途：

- 单帧谱图数据模型

主要字段：

- `timestamp`
- `scanMode`
- `detector`
- `masses`
- `intensities`
- `peaks`
- `parameterSnapshot`

### PeakInfo

用途：

- 谱峰信息

���段：

- `mass`
- `intensity`

## 设备接口

定义位置：

- `src/device/device_adapter.h`

### IDeviceAdapter

职责：

- 屏蔽 Mock / Real 设备差异
- 统一连接、扫描、状态读取与持久化触发接口

方法：

- `connectToDevice(const DeviceConnectionConfig &config)`
- `disconnectFromDevice()`
- `readStatusSnapshot()`
- `applyScanSettings(const ScanSettings &settings)`
- `applyTuneParameters(const TuneParameters &parameters)`
- `applyDataProcessingSettings(const DataProcessingSettings &settings)`
- `setSwitchState(InstrumentSwitch instrumentSwitch, bool on)`
- `startScan(ScanMode mode)`
- `stopScan()`
- `calibrateMassAxis()`
- `saveCurrentFrame()`

信号：

- `connectionChanged(bool connected)`
- `statusUpdated(const InstrumentStatus &status)`
- `frameUpdated(const SpectrumFrame &frame)`
- `calibrationFinished(bool success, const QString &message)`
- `frameReadyToPersist(const SpectrumFrame &frame)`
- `errorOccurred(const QString &message)`

## 服务接口

定义位置：

- `src/app/services.h`

### ConnectionService

职责：

- 管理设备连接和最近连接配置

## Gateway API（新增）

定义位置：

- `src/server/gateway_server.h`
- `src/server/gateway_server.cpp`

### REST

- `GET /health`
- `GET /api/status`
- `POST /api/connection/connect`
- `POST /api/connection/disconnect`
- `POST /api/scan/start`
- `POST /api/scan/stop`
- `POST /api/tune/apply`
- `GET /api/monitor/method/list`
- `POST /api/monitor/method/save`
- `POST /api/monitor/method/load`
- `POST /api/frame/save`

响应格式：

- 成功：`{"ok": true, "data": ...}`
- 失败：`{"ok": false, "error": "message"}`

### 实时事件流

- `GET /api/stream`（SSE）
- 事件类型：
  - `status`：仪器状态更新
  - `frame`：谱图帧更新
  - `error`：设备错误事件

### ScanControlService

职责：

- 管理扫描启动/停止/校正
- 做扫描前条件检查

关键方法：

- `canStartScan(...)`
- `startScan(...)`
- `stopScan()`
- `calibrateMassAxis()`

### TuneService

职责：

- 管理调谐参数的获取和下发

### MonitorService

职责：

- 管理监测方法保存、加载和应用

### PersistenceService

职责：

- 管理单帧数据保存、加载与 CSV 导出

### SettingsService

职责：

- 管理系统设置、谱图设置、数据处理设置、仪控设置

## 仓储接口

定义位置：

- `src/storage/repositories.h`

### SettingsRepository

负责：

- 最近连接
- 调谐参数
- 数据处理设置
- 系统设置
- 谱图设置
- 仪器控制设置

### MethodRepository

负责：

- 监测方法保存/读取
- 方法列表枚举

### FrameRepository

负责：

- 单帧数据保存/读取
- CSV 导出

## JSON 转换

定义位置：

- `src/core/types.h`
- `src/core/types.cpp`

当前支持：

- 多个核心类型的 `toJson(...)`
- 多个核心类型的 `fromJson(...)`

作用：

- 本地配置持久化
- 方法文件保存
- 单帧数据序列化

## 当前接口设计原则

- 新业务类型优先进入 `src/core/types.h`
- UI 只依赖服务层和核心类型
- 设备协议字段不要直接渗透到 UI
- 存储格式默认围绕 JSON/CSV 展开
