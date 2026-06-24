# MyFps

[English](README.md) | [简体中文](README_zh-CN.md)

`MyFps` 是基于 UE 5.5 开发的多人 FPS Deathmatch Demo，用于展示游戏客户端方向的工程能力。项目重点是服务器权威战斗、数据驱动武器、第一/第三人称表现分离，以及完整可连续进行的比赛闭环。

## 已实现功能

- 服务器使用独立 `BulletTrace` 通道执行 Hitscan 命中、伤害、死亡与得分。
- 生命、死亡、分数、准备状态、击杀信息、比赛状态均通过复制同步。
- 基于 `UMyFpsWeaponDefinition` Primary Data Asset 的数据驱动武器系统。
- 武器拆分为 Inventory、Combat、View、Pickup 四个职责模块。
- 1P/3P 武器挂载、换弹蒙太奇分层、基础 Turn In Place。
- 武器拾取、替换、丢弃物理、白边高亮和世界空间拾取提示。
- HitMarker、击杀反馈、命中音效、方向受击圆环和 Niagara 曳光。
- 自动/手动重生、结算、Ready、自动下一局。
- 主菜单单人/多人/设置、LAN Host/Join、房主 Start/End Game 控制、玩家显示名同步。

## 架构职责

| 模块 | 职责 |
| --- | --- |
| `GameInstance` | 本机跨地图设置、显示名、上次 IP、Host Settings、菜单和旅行标记；不复制。 |
| `GameMode` | 仅服务器存在，负责开始/结束比赛、重生、得分、胜负、发枪和移除客户端。 |
| `GameState` | 复制比赛全局状态：比赛开始/结束、目标分数、胜者、Ready 数量、最近击杀信息。 |
| `PlayerState` | 复制每名玩家的名字、分数、击杀、死亡和准备状态。 |
| `PlayerController` | 本地输入、鼠标状态、Client/Server RPC 边界。 |

## 武器模块

| 模块 | 职责 |
| --- | --- |
| `UMyFpsWeaponDefinition` | Mesh、弹药、射速、Montage、Niagara、声音与时序等静态数据。 |
| `UMyFpsWeaponInventoryComponent` | 当前武器与弹药等可复制运行时状态。 |
| `UMyFpsWeaponCombatComponent` | 服务端射击、Hitscan、伤害、换弹和曳光触发。 |
| `UMyFpsWeaponViewComponent` | 1P/3P Mesh、Socket、动画和视觉表现。 |
| `AMyFpsWeaponPickupActor` | 世界拾取物、提示、高亮、掉落物理和交互。 |

## 主菜单与 LAN 流程

主菜单使用一个 Root Widget 与 `WidgetSwitcher` 管理页面：

```text
主页面
  -> 单人游戏
  -> 多人游戏
       -> Host Settings -> OpenLevel(GameplayMap?listen)
       -> Join Page -> ClientTravel(IP:7777)
  -> 设置 -> 本机显示名
  -> 退出游戏
```

只有监听服务器房主能按 `F6` 打开游戏内控制面板：

- `Start Game`：服务器开始比赛，并隐藏面板。
- `End Game`：服务器停止比赛，远程客户端回主菜单，房主回到未开局状态。

## 构建与运行

环境要求：UE 5.5、Visual Studio 2022 C++ 桌面开发工具、Windows 10/11。

使用 UE 5.5 打开 `MyFps.uproject`，或执行：

```powershell
& 'G:\project\MyFps_UE5.4\Source\MyFps\BuildMyFps.bat'
```

本机双进程测试：启动两个独立 `-game` 进程。实例 A 在 MainMenu 创建 Host，实例 B 输入 `127.0.0.1` Join；房主按 `F6` 后点击 `Start Game`。

## 当前验证

已在 PIE 与两个独立 `-game` 进程中验证：Host/Join、显示名同步、记分板、Kill Feed、房主 Start/End、双向伤害、拾取/丢弃、换弹、死亡重生、结算、Ready 和连续下一局。

## 后续计划

- `GameMode::PreLogin` 房间密码校验。
- 服务端最大人数限制。
- 游戏内 Lobby/玩家列表和 Join 失败提示。
- 第二把差异化武器。
- Surface Type 命中反馈与 Unreal Insights 性能证据。
