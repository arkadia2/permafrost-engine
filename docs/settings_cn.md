# pf.conf 配置说明

本文档说明 `pf.conf` 中各配置项的作用与默认值。

---

## 音频 (pf.audio)

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `pf.audio.effect_volume` | float | 5.000000 | 音效音量 |
| `pf.audio.master_volume` | float | 0.500000 | 主音量 |
| `pf.audio.music_playback_mode` | int | 1 | 音乐播放模式 (0=关闭, 1=顺序, 2=随机, 3=单曲循环) |
| `pf.audio.music_volume` | float | 0.500000 | 背景音乐音量 |
| `pf.audio.mute_on_focus_loss` | bool | 0 | 窗口失去焦点时自动静音 |

---

## 调试可视化 (pf.debug)

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `pf.debug.enemy_seek_fields_faction_id` | int | 0 | 显示指定阵营的敌方寻敌场 (Enemy Seek Fields) |
| `pf.debug.formation_cell_index` | int | 0 | 显示指定索引的阵型单元格 |
| `pf.debug.navigation_layer` | int | 0 | 显示指定导航层 |
| `pf.debug.paused_frame_step_enabled` | bool | 0 | 暂停时启用逐帧推进 |
| `pf.debug.render_log_mask` | int | 1 | 渲染日志掩码 (位掩码控制日志级别) |
| `pf.debug.show_automation_state` | bool | 0 | 显示自动化状态调试覆盖层 |
| `pf.debug.show_chunk_boundaries` | bool | 0 | 显示地图区块边界 |
| `pf.debug.show_combat_ranges` | bool | 0 | 显示战斗范围圈 |
| `pf.debug.show_combat_targets` | bool | 0 | 显示战斗目标连线 |
| `pf.debug.show_enemy_seek_fields` | bool | 0 | 显示敌方寻敌场可视化 |
| `pf.debug.show_faction_vision` | int | -1 | 显示指定阵营的视野范围 (-1=不显示) |
| `pf.debug.show_first_sel_combined_hrvo` | bool | 0 | 显示首个选中单位的组合 HRVO 避障信息 |
| `pf.debug.show_first_sel_movestate` | bool | 0 | 显示首个选中单位的移动状态 |
| `pf.debug.show_formations` | bool | 0 | 显示阵型调试覆盖层 |
| `pf.debug.show_formations_assignment` | bool | 0 | 显示阵型分配调试信息 |
| `pf.debug.show_formations_cell_arrival_field` | bool | 0 | 显示阵型单元格到达场 |
| `pf.debug.show_formations_forces` | bool | 0 | 显示阵型受力向量 |
| `pf.debug.show_formations_occupied_field` | bool | 0 | 显示阵型占用场 |
| `pf.debug.show_harvester_state` | bool | 0 | 显示采集者状态调试信息 |
| `pf.debug.show_hearing_range` | bool | 0 | 显示听力范围圈 |
| `pf.debug.show_last_cmd_flow_field` | bool | 0 | 显示最后一条指令的流场 |
| `pf.debug.show_navigation_blockers` | bool | 0 | 显示导航阻挡物 |
| `pf.debug.show_navigation_cost_base` | bool | 0 | 显示导航基础代价场 |
| `pf.debug.show_navigation_island_ids` | bool | 0 | 显示导航岛屿 ID |
| `pf.debug.show_navigation_local_island_ids` | bool | 0 | 显示局部导航岛屿 ID |
| `pf.debug.show_navigation_portals` | bool | 0 | 显示导航传送门 |
| `pf.debug.trace_gpu` | bool | 0 | 启用 GPU 追踪 |
| `pf.debug.trace_python` | bool | 0 | 启用 Python 追踪 |

---

## 游戏 (pf.game)

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `pf.game.camera_zoom` | int | 150 | 相机默认缩放级别 |
| `pf.game.combat_hz` | float | 1.000000 | 战斗逻辑更新频率 (Hz) |
| `pf.game.fog_of_war_enabled` | bool | 1 | 启用战争迷雾 |
| `pf.game.healthbar_mode` | int | 1 | 血条显示模式 |
| `pf.game.movement_hz` | int | 20 | 移动逻辑更新频率 (Hz) |
| `pf.game.movement_use_gpu` | bool | 0 | 使用 GPU 加速移动计算 |
| `pf.game.storage_site_ui_mode` | int | 0 | 仓储站点 UI 模式 |

---

## 视频 (pf.video)

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `pf.video.aspect_ratio` | vec2 | 1920x1080 | 目标宽高比 |
| `pf.video.display_mode` | int | 0 | 显示模式 (0=窗口化, 1=无边框窗口, 2=全屏) |
| `pf.video.resolution` | vec2 | 1280x720 | 窗口分辨率 |
| `pf.video.shadows_enabled` | bool | 1 | 启用阴影 |
| `pf.video.use_batch_rendering` | bool | 1 | 启用批量渲染 |
| `pf.video.vsync` | bool | 0 | 启用垂直同步 |
| `pf.video.water_reflection` | bool | 1 | 启用水面反射 |
| `pf.video.water_refraction` | bool | 1 | 启用水面折射 |
| `pf.video.window_always_on_top` | bool | 0 | 窗口始终置顶 |
