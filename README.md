# Qt线程池实现

这是一个基于Qt的线程池实现项目，提供了高效的任务调度和执行机制。

## 项目架构

项目主要包含以下核心组件：

### Task类
- 任务的基本单元，继承自QObject
- 支持任务优先级（高、中、低）
- 提供任务状态跟踪（创建、排队、运行、完成、失败）
- 支持任务执行时间统计
- 提供错误处理机制

### ThreadPool类
- 线程池管理器，继承自QObject
- 支持动态创建和管理工作线程
- 实现基于优先级的任务调度
- 提供任务队列管理
- 支持异常处理和错误报告

## 核心功能

1. 任务管理
   - 支持添加、执行和取消任务
   - 任务优先级调度
   - 任务状态监控
   - 执行时间统计

2. 线程管理
   - 自动创建最优线程数
   - 线程生命周期管理
   - 线程安全的任务分配

3. 错误处理
   - 异常捕获和处理
   - 错误信息传递
   - 任务失败恢复

## 使用方法

1. 创建线程池
```cpp
ThreadPool pool(QThread::idealThreadCount());
```

2. 创建任务
```cpp
auto task = std::make_shared<Task>([](){ 
    // 任务逻辑
}, Task::Priority::Normal);
```

3. 添加任务到线程池
```cpp
pool.addTask(task);
```

4. 处理任务结果
```cpp
connect(&pool, &ThreadPool::taskCompleted, [](std::shared_ptr<Task> task) {
    // 处理任务完成
});

connect(&pool, &ThreadPool::taskFailed, [](std::shared_ptr<Task> task, const QString& error) {
    // 处理任务失败
});
```

## 注意事项

1. 线程安全
   - 所有任务操作都是线程安全的
   - 使用互斥锁保护共享资源

2. 资源管理
   - 线程池会自动管理线程资源
   - 停止线程池时会等待所有任务完成

3. 错误处理
   - 建议为每个任务添加错误处理逻辑
   - 监听线程池的错误信号
   - 
4.  该项目全由AIIDE生成，代码经测试，请谨慎使用。
   -该项目分别由Cusor 与Trace AIIDE生成，请谨慎使用。
