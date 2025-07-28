#include "threadpool.h"
#include "task.h"
#include <QDebug>

// 构造函数
// @param numThreads 线程池中的线程数量，默认使用系统建议的线程数
// @param parent 父对象指针，用于Qt对象树管理
ThreadPool::ThreadPool(int numThreads, QObject *parent)
    : QObject(parent)
    , m_running(true)      // 初始化运行状态为true
    , m_activeThreads(0)   // 初始化活动线程数为0
{
    initialize(numThreads);
}

ThreadPool::~ThreadPool() {
    cleanup();
}

// 初始化线程池
// @param numThreads 要创建的线程数量
void ThreadPool::initialize(int numThreads) {
    // 检查线程数量是否有效
    if (numThreads <= 0) {
        setError("Invalid number of threads");
        return;
    }

    // 创建并初始化工作线程
    for (int i = 0; i < numThreads; ++i) {
        // 创建新线程
        QThread* thread = new QThread(this);
        // 连接线程完成信号
        connect(thread, &QThread::finished, this, &ThreadPool::handleThreadFinished);
        
        thread->start();
        m_threads.append(thread);

        // 创建工作对象并移动到新线程
        QObject* worker = new QObject;
        worker->moveToThread(thread);

        // 连接工作对象销毁信号到线程退出槽
        connect(worker, &QObject::destroyed, thread, &QThread::quit);
        // 设置线程的工作循环
        connect(thread, &QThread::started, [this, worker]() {
            while (m_running) {
                std::shared_ptr<Task> task;
                {
                    // 获取任务时加锁保护
                    QMutexLocker locker(&m_mutex);
                    // 等待任务或线程池停止
                    while (m_running && m_taskQueue.isEmpty()) {
                        m_condition.wait(&m_mutex);
                    }
                    if (!m_running) break;
                    
                    // 获取下一个要执行的任务
                    task = getNextTask();
                }

                // 如果获取到任务则执行
                if (task) {
                    ++m_activeThreads;  // 增加活动线程计数
                    try {
                        task->execute();  // 执行任务
                        emit taskCompleted(task);  // 发送任务完成信号
                    }
                    catch (const std::exception& e) {
                        // 处理标准异常
                        emit taskFailed(task, QString::fromStdString(e.what()));
                    }
                    catch (...) {
                        // 处理未知异常
                        emit taskFailed(task, "Unknown error occurred");
                    }
                    --m_activeThreads;  // 减少活动线程计数
                }
            }
            worker->deleteLater();
        });
    }
}

// 清理线程池资源
void ThreadPool::cleanup() {
    m_running = false;  // 停止所有工作循环
    {
        QMutexLocker locker(&m_mutex);
        m_condition.wakeAll();  // 唤醒所有等待的线程
    }

    // 等待所有线程完成并释放资源
    for (QThread* thread : m_threads) {
        thread->wait();   // 等待线程结束
        delete thread;    // 删除线程对象
    }
    m_threads.clear();    // 清空线程列表
    m_taskQueue.clear();  // 清空任务队列
}

// 添加任务到线程池
// @param task 要添加的任务
// @return 是否成功添加任务
bool ThreadPool::addTask(std::shared_ptr<Task> task) {
    // 检查任务是否有效
    if (!task) {
        setError("Cannot add null task");
        return false;
    }

    // 检查线程池是否在运行
    if (!m_running) {
        setError("ThreadPool is not running");
        return false;
    }

    // 加锁保护任务队列
    QMutexLocker locker(&m_mutex);
    task->setStatus(Task::Status::Queued);  // 更新任务状态
    m_taskQueue.enqueue(task);             // 将任务加入队列
    m_condition.wakeOne();                 // 唤醒一个等待的线程
    return true;
}

int ThreadPool::pendingTasks() const {
    QMutexLocker locker(&m_mutex);
    return m_taskQueue.size();
}

void ThreadPool::stop() {
    cleanup();
}

bool ThreadPool::isRunning() const {
    return m_running;
}

int ThreadPool::threadCount() const {
    return m_threads.size();
}

// 获取下一个要执行的任务
// @return 优先级最高的任务，如果队列为空则返回nullptr
std::shared_ptr<Task> ThreadPool::getNextTask() {
    if (m_taskQueue.isEmpty()) return nullptr;

    // 查找优先级最高的任务
    auto it = std::max_element(m_taskQueue.begin(), m_taskQueue.end(),
        [](const std::shared_ptr<Task>& a, const std::shared_ptr<Task>& b) {
            return static_cast<int>(a->priority()) < static_cast<int>(b->priority());
        });

    if (it != m_taskQueue.end()) {
        auto task = *it;
        m_taskQueue.removeOne(task);
        return task;
    }

    return nullptr;
}

// 设置错误信息并发送错误信号
// @param error 错误信息
void ThreadPool::setError(const QString& error) {
    m_lastError = error;
    emit errorOccurred(error);
}

// 处理线程完成的槽函数
void ThreadPool::handleThreadFinished() {
    // 获取发送信号的线程对象
    QThread* thread = qobject_cast<QThread*>(sender());
    if (thread) {
        m_threads.removeOne(thread);  // 从线程列表中移除已完成的线程
    }
}