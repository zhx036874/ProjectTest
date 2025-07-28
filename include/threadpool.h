#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <functional>
#include <memory>
#include <atomic>
#include <stdexcept>

class Task;

class ThreadPool : public QObject {
    Q_OBJECT

public:
    explicit ThreadPool(int numThreads = QThread::idealThreadCount(), QObject *parent = nullptr);
    ~ThreadPool();

    // 添加任务到线程池
    bool addTask(std::shared_ptr<Task> task);
    
    // 获取当前任务队列大小
    int pendingTasks() const;

    // 停止线程池
    void stop();

    // 获取线程池状态
    bool isRunning() const;

    // 获取线程池大小
    int threadCount() const;

private:
    void initialize(int numThreads);
    void cleanup();
    std::shared_ptr<Task> getNextTask();

private:
    QVector<QThread*> m_threads;
    QQueue<std::shared_ptr<Task>> m_taskQueue;
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    std::atomic<bool> m_running;
    std::atomic<int> m_activeThreads;

    // 错误处理相关
    QString m_lastError;
    void setError(const QString& error);

signals:
    void errorOccurred(const QString& error);
    void taskCompleted(std::shared_ptr<Task> task);
    void taskFailed(std::shared_ptr<Task> task, const QString& error);

private slots:
    void handleThreadFinished();
};

#endif // THREADPOOL_H