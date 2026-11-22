# LayAndRelax 原生 ASI 移植工程（源码初版）

## 当前目标
- GTA V Windows PC 单机版
- 原生 C++ ASI，不依赖 ScriptHookVDotNet / .NET
- 默认手柄 LT + RT 同时按下开始/结束躺下
- 保留原版回血、护甲恢复、慢动作和移动/攻击/瞄准自动起身逻辑

## 编译前提
需要合法安装的 ScriptHookV SDK（包含 `inc` 头文件和 `lib/ScriptHookV.lib`）。
把 SDK 的 `inc` 文件夹放到仓库根目录的 `sdk/inc`，把 `ScriptHookV.lib` 放到 `sdk/lib/ScriptHookV.lib`。
本工程不会捆绑第三方 SDK。

## GitHub Actions
上传整个工程到 GitHub 仓库后，在仓库中加入 SDK 文件（或按你自己的私有方式提供 SDK），
然后运行 Actions 中的 `Build LayAndRelax ASI` 工作流。
成功后会生成 `LayAndRelax.asi` 构建产物。

## 安装
将 `LayAndRelax.asi` 和 `LayAndRelax.ini` 放在 GTA5.exe 同一目录。
需要 ScriptHookV（ASI Loader）支持。

## 配置
`LayAndRelax.ini`：
- `ToggleCombo=LT+RT`：同时按下左扳机 LT（瞄准）和右扳机 RT（攻击）
- `RegenHealth=1`
- `SlowTime=1`
- `Scenario=WORLD_HUMAN_SUNBATHE_BACK`

## 注意
这是依据用户提供的 C# 源码制作的 C++ 移植初版，尚未使用目标 GTA V / ScriptHookV SDK 实际编译和游戏内测试。
不同版本的 ScriptHookV SDK 可能对头文件/导入库名称有差异；若编译报错，需要根据实际 SDK 调整。
