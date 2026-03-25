# CHANGELOG

## 0.1.0

初始可运行版本，完成四极质谱上位机桌面原型的基础搭建。

### Added

- 基于 `Qt 6 Widgets + CMake` 的桌面应用骨架
- 主窗口、顶部菜单、左侧导航、页面栈结构
- 调谐页、监测页、数据页、设置页基础界面
- 调谐页谱图、头部按钮区、右侧参数/状态/连接区
- 监测页 `RIC / TIC`、方法按钮区、参数配置区
- 设置页连接、存储、管道服务测试布局
- 扫描设置、系统设置、谱图设置、数据处理等弹窗
- `IDeviceAdapter` 统一设备接口
- `MockDeviceAdapter` 模拟连接、状态、扫描和谱图数据
- `RealDeviceAdapter` 预留实现壳层
- `ConnectionService / ScanControlService / TuneService / MonitorService / PersistenceService / SettingsService`
- `SettingsRepository / MethodRepository / FrameRepository`
- JSON / CSV 本地持久化基础能力
- GitHub Releases 自动发布基础设施
- macOS / Windows 本地打包脚本
- 项目基础文档体系

### Notes

- 当前仍以 Mock 设备联调和界面对标为主
- 真实设备协议、签名、公证、安装器尚未纳入本版本
