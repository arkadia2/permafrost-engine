让我查看 `MOVE_CMD_SET_DEST` 的处理代码以及寻路计算的位置：
        
            
toolName: view_files
            
status: success
          
            
filePath: e:/code/permafrost-engine/src/game/movement.c
          
让我查看导航工作提交和寻路计算的相关代码：
        
            
toolName: search_by_regex
            
status: success
          
query: nav_tick_submit_work
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:/code/permafrost-engine/src/game/movement.c
          
让我查看 `navigation_tick_task` 函数，这是实际执行寻路计算的地方：
        
            
toolName: search_by_regex
            
status: success
          
query: navigation_tick_task
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:/code/permafrost-engine/src/game/movement.c
          
让我查看 `compute_async_fields` 函数，这是寻路计算的核心：
        
            
toolName: search_by_regex
            
status: success
          
query: compute_async_fields
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:/code/permafrost-engine/src/game/movement.c
          
# 移动和寻路计算完整流程分析

## 1. 移动命令处理

从 `MOVE_CMD_SET_DEST` 开始：

```c
case MOVE_CMD_SET_DEST: {
    uint32_t uid = cmd.args[0].val.as_int;
    vec2_t dest_xz = cmd.args[1].val.as_vec2;
    bool attack = cmd.args[2].val.as_bool;
    do_set_dest(uid, dest_xz, attack);
    break;
}
```

- 用户命令触发 `do_set_dest` 函数
- `do_set_dest` 将实体添加到 flock 或创建新的 flock


## 2. 移动更新流程

### 2.1 主更新触发

```c
static void on_update(void *user, void *event)
{
    stalloc_clear(&s_eventargs);
    handle_queued_tick();
}
```

- 游戏主循环调用 `on_update`
- 触发移动更新处理


### 2.2 队列处理

```c
static void handle_queued_tick(void)
{
    if(!s_move_tick_queued)
        return;

    enum move_work_status status = nav_tick_finish_work();
    if(status == WORK_INCOMPLETE)
        return;

    enum movement_hz hz = s_move_work.hz;
    enum eventtype curr_event = event_for_hz(hz);

    s_move_tick_queued = false;
    move_do_tick(curr_event, hz);
}
```

- 检查导航工作是否完成
- 如果完成，调用 `move_do_tick`


### 2.3 核心移动处理

```c
static void move_do_tick(enum eventtype curr_event, enum movement_hz hz)
{
    ASSERT_IN_MAIN_THREAD();
    PERF_PUSH("movement::tick");

    move_consume_work_results();
    move_handle_hz_update(curr_event);
    move_process_cmds();  // 处理移动命令，包括 MOVE_CMD_SET_DEST
    G_SwapFieldCaches(s_move_work.gamestate.map);
    move_release_gamestate();
    disband_empty_flocks();

    /* Run the navigation updates synchronous to the movement tick */
    G_UpdateMap();

    move_prepare_work(hz);
    move_copy_gamestate();

    // 为每个移动实体准备工作数据
    PERF_PUSH("submit move work");
    uint32_t curr;
    kh_foreach_key(s_entity_state_table, curr, {
        // ... 准备实体移动工作数据
        move_push_work((struct move_work_in){...});
    });
    PERF_POP();

    nav_tick_submit_work();  // 提交导航工作
    PERF_POP();
}
```

- `move_process_cmds()` 处理命令队列中的 `MOVE_CMD_SET_DEST` 命令
- 为每个移动实体准备工作数据
- 调用 `nav_tick_submit_work()` 提交导航工作


## 3. 寻路计算

### 3.1 提交导航工作

```c
static void nav_tick_submit_work(void)
{
    ASSERT_IN_MAIN_THREAD();

    if(s_move_work.type == WORK_TYPE_GPU) {
        size_t nwork = s_move_work.nwork;
        size_t size = nwork * sizeof(vec2_t);
        s_move_work.gpu_velocities = stalloc(&s_move_work.mem, size);
    }

    SDL_AtomicSet(&s_tick_task_future.status, FUTURE_INCOMPLETE);
    s_tick_task_tid = Sched_Create(0, navigation_tick_task, NULL, 
            "navigation_tick_task", &s_tick_task_future, TASK_BIG_STACK);
    assert(s_tick_task_tid != NULL_TID);
    s_last_tick = g_frame_idx;
}
```

- 创建导航任务 `navigation_tick_task`
- 任务在后台线程中执行


### 3.2 导航任务执行

```c
static struct result navigation_tick_task(void *arg)
{
    compute_async_fields();      // 计算异步流场（寻路核心）
    compute_desired_velocity();   // 计算期望速度
    fork_join_velocity_computations();  // 执行速度计算

    if(s_move_work.type == WORK_TYPE_GPU) {
        // GPU 处理...
    }

    fork_join_state_updates();
    return NULL_RESULT;
}
```

- `navigation_tick_task` 是寻路计算的入口
- 调用 `compute_async_fields()` 进行流场计算


### 3.3 流场计算（寻路核心）

```c
static void compute_async_fields(void)
{
    /* The field computations can read various navigation state
     * from different threads. This is okay so long as nothing
     * concurrently mutates it.
     */
    N_PrepareAsyncWork();  // 准备异步工作

    for(int i = 0; i < s_move_work.nwork; i++) {
        struct move_work_in *in = &s_move_work.in[i];
        request_async_field(in->ent_uid);  // 为每个实体请求流场
        Sched_TryYield();
    }
    N_AwaitAsyncFields();  // 等待所有流场计算完成
}
```

**这是寻路计算的核心位置**：
- `N_PrepareAsyncWork()` - 准备导航系统
- `request_async_field()` - 为每个实体请求流场数据
- `N_AwaitAsyncFields()` - 等待流场计算完成


## 完整流程总结

```
用户命令 (MOVE_CMD_SET_DEST)
    ↓
do_set_dest() - 将实体添加到 flock
    ↓
move_do_tick() - 处理移动命令
    ↓
nav_tick_submit_work() - 提交导航工作
    ↓
navigation_tick_task() - 导航任务（后台线程）
    ↓
compute_async_fields() - 计算流场（寻路核心）
    ↓
N_PrepareAsyncWork() - 准备导航
    ↓
request_async_field() - 为每个实体请求流场
    ↓
N_AwaitAsyncFields() - 等待流场计算完成
    ↓
compute_desired_velocity() - 计算期望速度
    ↓
fork_join_velocity_computations() - 执行速度计算
    ↓
move_consume_work_results() - 消费计算结果
```


## 关键点

1. **寻路计算位置**：主要在 `compute_async_fields()` 函数中进行
2. **异步执行**：寻路计算在后台线程中执行，不阻塞主线程
3. **流场技术**：使用流场（Flow Field）进行导航计算
4. **分层设计**：命令处理、工作提交、计算执行分离
5. **性能优化**：通过异步计算和流场共享提高性能


## 总结

移动从 `MOVE_CMD_SET_DEST` 命令开始，最终在 `compute_async_fields()` 函数中进行寻路计算。整个流程采用了异步工作模型，寻路计算在后台线程中执行，使用流场技术为实体提供导航数据。这种设计既保证了性能，又提供了流畅的移动体验。