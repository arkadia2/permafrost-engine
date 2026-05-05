让我查看 `Sched_TryYield()` 函数的实现和调度系统的相关代码：
        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          
让我查看调度系统的核心数据结构和初始化代码：
        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          

        
            
toolName: search_by_regex
            
status: success
          
query: ^uint32_t Sched_Create
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          

        
            
toolName: search_by_regex
            
status: success
          
query: ^static.*sched_create
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          

        
            
toolName: search_by_regex
            
status: success
          
query: ^static void sched_main_loop
          

        
            
toolName: search_by_regex
            
status: success
          
query: ^void.*sched.*main
          

        
            
toolName: search_by_regex
            
status: success
          
query: ^static.*worker
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          

        
            
toolName: search_by_regex
            
status: success
          
query: ^bool Sched_Init
          

        
            
toolName: view_files
            
status: success
          
            
filePath: e:\code\permafrost-engine\src\sched.c
          
# Permafrost Engine 调度系统原理和用法分析

## 1. 系统概述

Permafrost Engine 实现了一个**用户级协程调度系统**，支持多线程任务调度。该系统通过用户级上下文切换实现轻量级任务，避免了内核态切换的开销。

## 2. 核心数据结构

### 2.1 任务结构体

```c
struct task{
    struct context ctx;          // 任务上下文
    enum taskstate state;        // 任务状态
    int            prio;        // 任务优先级
    uint32_t       tid;         // 任务 ID
    uint32_t       parent_tid;   // 父任务 ID
    uint32_t       flags;       // 任务标志
    struct request req;          // 任务请求
    uint64_t       retval;      // 返回值
    void          *stackmem;    // 栈内存
    struct future *future;       // Future 对象
    const char    *name;        // 任务名称
    void          *arg;         // 任务参数
    void         (*destructor)(void*); // 析构函数
    void          *darg;        // 析构参数
    struct task   *prev, *next; // 链表指针
    void          *earg;        // 事件参数
    void         (*erelease)(void*); // 事件释放函数
    char          __pad[8];     // 填充
};
```

### 2.2 上下文结构体

```c
struct context{
    uint64_t rbx;      // 通用寄存器
    uint64_t rsp;      // 栈指针
    uint64_t rbp;      // 基指针
    uint64_t rdi;      // 通用寄存器
    uint64_t rsi;      // 通用寄存器
    uint64_t r12;      // 通用寄存器
    uint64_t r13;      // 通用寄存器
    uint64_t r14;      // 通用寄存器
    uint64_t r15;      // 通用寄存器
    uint32_t mxcsr;    // SSE 控制状态
    uint16_t fpucw;    // FPU 控制字
    uint16_t __pad0;   // 填充
    INT128(xmm6);       // SSE 寄存器
    INT128(xmm7);       // SSE 寄存器
    INT128(xmm8);       // SSE 寄存器
    INT128(xmm9);       // SSE 寄存器
    INT128(xmm10);      // SSE 寄存器
    INT128(xmm11);      // SSE 寄存器
    INT128(xmm12);      // SSE 寄存器
    INT128(xmm13);      // SSE 寄存器
    INT128(xmm14);      // SSE 寄存器
    INT128(xmm15);      // SSE 寄存器
};
```

### 2.3 任务状态

```c
enum taskstate{
    TASK_STATE_ACTIVE,           // 活跃状态
    TASK_STATE_READY,            // 就绪状态
    TASK_STATE_SEND_BLOCKED,      // 发送阻塞
    TASK_STATE_RECV_BLOCKED,      // 接收阻塞
    TASK_STATE_REPLY_BLOCKED,     // 回复阻塞
    TASK_STATE_EVENT_BLOCKED,    // 事件阻塞
    TASK_STATE_ZOMBIE,          // 僵尸状态（已结束但未清理）
};
```

## 3. 核心原理

### 3.1 用户级上下文切换

调度系统的核心是**用户级上下文切换**，通过汇编函数 `sched_switch_ctx` 实现：

```c
uint64_t sched_switch_ctx(struct context *save, struct context *restore, 
                       uint64_t retval, void *arg);
```

**工作原理**：
1. 保存当前任务的寄存器状态到 `save` 上下文
2. 从 `restore` 上下文恢复寄存器状态
3. 返回指定的 `retval` 值

**优势**：
- 避免内核态和用户态切换
- 减少系统调用开销
- 提高切换速度

### 3.2 任务调度

#### 3.2.1 任务创建

```c
uint32_t Sched_Create(int prio, task_func_t code, void *arg, 
                    const char *name, struct future *result, int flags)
```

**流程**：
1. 分配任务结构体
2. 初始化任务上下文
3. 设置任务栈和返回地址
4. 将任务放入就绪队列

#### 3.2.2 任务运行

```c
static void sched_task_run(struct task *task)
{
    assert(sched_curr_thread_tid() == NULL_TID);
    sched_set_thread_tid(SDL_ThreadID(), task->tid);
    task->state = TASK_STATE_ACTIVE;
    
    if(SDL_ThreadID() == g_main_thread_id) {
        sched_switch_ctx(&s_main_ctx, &task->ctx, task->retval, task->arg);
    }else{
        int id = sched_curr_thread_worker_id();
        sched_switch_ctx(&s_worker_contexts[id], &task->ctx, task->retval, task->arg);
    }
    
    sched_set_thread_tid(SDL_ThreadID(), NULL_TID);
}
```

**流程**：
1. 设置当前线程的任务 ID
2. 将任务状态设为活跃
3. 从主线程或工作线程上下文切换到任务上下文
4. 任务执行完成后，清理线程任务 ID

#### 3.2.3 任务让出（Yield）

```c
void Sched_TryYield(void)
{
    uint32_t tid = Sched_ActiveTID();
    if(tid == NULL_TID)
        return;

    const char *name = NULL;
    if(!Perf_IsRoot()) {
        PERF_POP_NAME(&name);
    }

    Sched_Request((struct request) { .type = SCHED_REQ_YIELD });

    if(name) {
        PERF_PUSH(name);
    }
}
```

**流程**：
1. 获取当前任务 ID
2. 如果不在任务中，直接返回
3. 保存性能分析名称
4. 发送 YIELD 请求
5. 恢复性能分析名称

**作用**：
- 让当前任务让出 CPU
- 允许其他任务运行
- 实现协作式多任务

### 3.3 多线程工作池

#### 3.3.1 工作线程初始化

```c
bool Sched_Init(void)
{
    // 初始化各种数据结构...
    
    /* On a single-core system, all the tasks will just be run on the main thread */
    s_nworkers = SDL_GetCPUCount() - 1;

    for(int i = 0; i < s_nworkers; i++) {
        // 创建工作线程同步原语...
        
        char threadname[128];
        pf_snprintf(threadname, sizeof(threadname), "worker-%d", i);
        s_worker_threads[i] = SDL_CreateThread(worker_threadfn, threadname, 
                                           (void*)((uintptr_t)i));
        
        sched_wait_workers_idle(1);
        s_idle_workers = 0;
        Perf_RegisterThread(SDL_GetThreadID(s_worker_threads[i]), threadname);
    }
    
    return true;
}
```

**特点**：
- 工作线程数量 = CPU 核心数 - 1（保留一个核心给主线程）
- 每个工作线程有独立的上下文和同步原语
- 支持任务在主线程和工作线程之间切换

#### 3.3.2 工作线程循环

```c
static int worker_threadfn(void *arg)
{
    int id = (uintptr_t)arg;
    worker_notify_done(id);

    while(true) {
        worker_wait_on_cmd(id);
        if(s_worker_quit[id])
            break;
        worker_do_work(id);
        worker_notify_done(id);
        mi_stats_merge();
    }
    return 0;
}
```

**流程**：
1. 等待启动命令
2. 执行任务
3. 通知完成
4. 合并内存统计
5. 循环直到收到退出命令

### 3.4 优先级调度

系统使用**优先队列**调度任务：

```c
static pq_task_t s_ready_queue;        // 通用就绪队列
static pq_task_t s_ready_queue_main;  // 主线程就绪队列
```

**调度策略**：
- 高优先级任务优先执行
- 主线程和工作线程有独立的队列
- 支持任务在暂停期间运行（`TASK_RUN_DURING_PAUSE` 标志）

## 4. 使用方法

### 4.1 创建任务

```c
// 定义任务函数
struct result my_task(void *arg)
{
    printf("Task running with arg: %s\n", (char*)arg);
    
    // 执行任务逻辑...
    
    // 让出 CPU，允许其他任务运行
    Sched_TryYield();
    
    // 继续执行...
    
    return NULL_RESULT;
}

// 创建任务
uint32_t tid = Sched_Create(0, my_task, "Hello, World!", "MyTask", NULL, 0);
```

### 4.2 等待任务完成

```c
// 创建带 Future 的任务
struct future future;
uint32_t tid = Sched_Create(0, my_task, arg, "MyTask", &future, 0);

// 等待任务完成
while(!Sched_FutureIsReady(&future)) {
    Sched_TryYield();
}

// 获取任务结果
struct result result = future.result;
```

### 4.3 任务让出

```c
struct result my_task(void *arg)
{
    // 执行一些工作...
    
    // 让出 CPU，允许其他任务运行
    Sched_TryYield();
    
    // 继续执行...
    
    return NULL_RESULT;
}
```

### 4.4 同步运行任务

```c
// 在主线程中同步运行任务
bool success = Sched_RunSync(tid);
if(success) {
    printf("Task completed successfully\n");
}
```

### 4.5 事件等待

```c
// 等待特定事件
int source;
Task_AwaitEvent(EVENT_UPDATE_START, &source);

// 任务被唤醒后继续执行
printf("Event received from source %d\n", source);
```

## 5. 在导航系统中的应用

在 `nav.c` 中，`Sched_TryYield()` 用于：

```c
// 在长时间运行的导航计算中让出 CPU
for(int i = 0; i < n_iterations; i++) {
    // 执行导航计算...
    
    // 定期让出 CPU，避免阻塞主线程
    Sched_TryYield();
}
```

**作用**：
- 避免长时间计算阻塞主线程
- 允许其他任务（如渲染、输入处理）运行
- 提高游戏响应性

## 6. 技术特点

### 6.1 优势

1. **高性能**：用户级上下文切换，避免内核态开销
2. **轻量级**：任务栈大小可配置（默认 16KB）
3. **可扩展**：支持多线程工作池
4. **灵活调度**：支持优先级调度和协作式多任务
5. **内存高效**：使用对象池管理任务结构

### 6.2 适用场景

1. **异步计算**：导航、物理模拟等长时间计算
2. **I/O 操作**：文件加载、网络请求等
3. **并行处理**：多任务并行执行
4. **游戏逻辑**：AI、动画等需要分步处理的逻辑

## 7. 总结

Permafrost Engine 的调度系统是一个高效、灵活的用户级协程系统，它：

1. **使用用户级上下文切换**实现轻量级任务
2. **支持多线程工作池**提高并行性能
3. **提供优先级调度**优化任务执行顺序
4. **支持协作式多任务**通过 `Sched_TryYield()` 让出 CPU
5. **在导航系统中广泛应用**避免长时间计算阻塞主线程

这个系统为游戏引擎提供了强大的并发执行能力，使得复杂的游戏逻辑和计算能够高效、流畅地运行。