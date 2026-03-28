# DESIGN

## Summary

本设计系统用于统一 `device-app-qml` 页面风格，基线为当前桌面工业上位机风格（截图对标），允许小幅现代化优化（圆角/留白微调），但不改变核心布局骨架。

## Visual Tokens

### Color

- `bg.app`: `#e8edf3`
- `bg.workspace`: `#f1f3f6`
- `bg.toolbar`: `#e6eaef`
- `bg.panel`: `#ffffff`
- `bg.panelSubtle`: `#f5f7fa`
- `bg.chart`: `#f4f5f7`
- `bg.primary`: `#1ea2ff`
- `bg.primarySoft`: `#d7e6ff`
- `text.primary`: `#2f3a4a`
- `text.secondary`: `#4b5563`
- `text.inverse`: `#ffffff`
- `border.default`: `#ccd2dc`
- `border.subtle`: `#d7dbe3`
- `border.strong`: `#c8cfd8`
- `chart.linePrimary`: `#10a4ea`

### Typography

- `font.menu`: `14`
- `font.body`: `12`
- `font.nav`: `14`
- `font.title`: `16`

### Size / Spacing

- `radius.sm`: `3`
- `radius.md`: `4`
- `button.h`: `22`
- `input.h`: `21`
- `checkbox.indicator`: `13`
- `tab.h`: `22`
- `gap.xs`: `3`
- `gap.sm`: `4`
- `gap.md`: `6`
- `gap.lg`: `8`

## Layout Spec

### Main Shell

- 顶部菜单栏（固定高度）
- 左侧窄导航（固定宽度）
- 右侧工作区（工具条 + 主内容）

### Tune Page

- 顶部操作按钮条（单行紧凑按钮）
- 头部三栏摘要区：扫描方式、检测器/稳定区、`m/z` 摘要表
- 中部主谱图 + 下部 TIC（主图约 72%，下图约 28%）
- 右侧标签区：参数配置 / 仪器状态 / 设备连接

## Component Rules

核心控件必须使用公共组件和 Token：

- 按钮：`AppButton`
- 复选框：`AppCheckBox`
- 面板：`AppPanel`
- 标签头：`AppTabHeader`

允许页面内覆盖：

- 图表曲线颜色
- 局部留白
- 低优先级文案颜色

禁止页面内硬编码（除临时实验代码）：

- 主色/边框色
- 核心字号档位
- 按钮、输入、复选框尺寸

## Mapping

- Theme 入口：`web-qml/qml/theme/Theme.qml`
- Token JS：`web-qml/qml/theme/Tokens.js`
- 基础组件：`web-qml/qml/components/*.qml`
- 样板页：`web-qml/qml/Main.qml`

