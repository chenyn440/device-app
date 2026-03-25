# DELIVERY

## Summary

当前项目已经具备一版可演示、可继续扩展的四极质谱上位机桌面原型，覆盖了：

- UI 主框架
- 调谐 / 监测 / 数据 / 设置主页面
- Mock 设备联调
- 本地 JSON / CSV 持久化
- GitHub Releases 发布基础设施
- 基础项目文档体系

这份文档从交付视角概括当前仓库状态，方便对外说明和阶段性验收。

## 当前已交付内容

### 1. 工程与运行基础

- `Qt 6 Widgets + CMake` 工程
- macOS 本地构建运行链路
- 顶部菜单、左侧导航、中心工作区、状态栏

### 2. 页面能力

- 调谐页
  头部操作区、配置摘要区、主谱图、辅助谱图、右侧参数/状态/连接
- 监测页
  方法按钮区、`RIC / TIC`、右侧参数配置和仪器状态
- 数据页
  单帧展示基础入口
- 设置页
  连接、存储、管道服务测试

### 3. 设备能力

- `IDeviceAdapter` 统一设备接口
- `MockDeviceAdapter` 可驱动联机、状态和扫描模拟
- `RealDeviceAdapter` 预留真实协议接入点

### 4. 服务与存储

- 连接服务
- 扫描控制服务
- 调谐服务
- 监测方法服务
- 持久化服务
- 设置服务
- 最近连接、方法、单帧数据、本地设置持久化

### 5. 发布能力

- `scripts/package-macos.sh`
- `scripts/package-windows.ps1`
- `.github/workflows/release.yml`
- `v*` tag 触发 GitHub Releases

### 6. 文档能力

- `README.md`
- `ARCHITECTURE.md`
- `DEVELOPMENT.md`
- `RELEASE.md`
- `UI_GUIDE.md`
- `API_GUIDE.md`
- `TEST_PLAN.md`
- `CONTRIBUTING.md`
- `CHANGELOG.md`

## 当前交付边界

已完成：

- 可运行桌面应用
- Mock 演示链路
- 多页面 UI 对标迭代
- 基础发布链路
- 基础文档体系

未完成：

- 真实设备协议实现
- 自动化测试体系
- 安装器、签名、公证
- 复杂数据处理算法
- 生产级异常恢复和运维监控

## 当前验收建议

### 功能验收

- 应用可启动
- 调谐页和监测页可进入
- Mock 设备可驱动状态刷新
- 单帧数据可保存

### UI 验收

- 主窗口结构正确
- 调谐页与监测页核心区域已成型
- 设置页与数据页具备基础工作流入口

### 发布验收

- macOS 打包脚本可生成 zip
- GitHub Actions workflow 已具备发布结构

## 下一阶段建议

### A. 工程化补强

- 提交 `.gitignore` 和新增文档
- 配置 GitHub remote
- 触发一次真实 GitHub Release

### B. 业务闭环

- 落地 `RealDeviceAdapter`
- 完善扫描设置、方法管理和数据页交互
- 补充异常处理

### C. 质量提升

- 建立基础自动化测试
- 抽离更统一的 UI 样式常量
- 继续完善页面设计约束
