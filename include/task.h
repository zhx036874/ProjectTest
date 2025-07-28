#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>
#include <functional>
#include <chrono>

class Task : public QObject {
    Q_OBJECT

public:
    enum class Priority {
        Low,
        Normal,
        High
    };

    enum class Status {
        Created,
        Queued,
        Running,
        Completed,
        Failed
    };

    explicit Task(QObject *parent = nullptr);
    explicit Task(std::function<void()> function, Priority priority = Priority::Normal, QObject *parent = nullptr);
    ~Task();

    // 设置任务函数
    void setFunction(std::function<void()> function);

    // 设置和获取任务优先级
    void setPriority(Priority priority);
    Priority priority() const;

    // 获取任务状态
    Status status() const;

    // 获取错误信息
    QString errorMessage() const;

    // 获取任务执行时间
    qint64 executionTime() const;

    // 执行任务
    void execute();

private:
    std::function<void()> m_function;
    Priority m_priority;
    Status m_status;
    QString m_errorMessage;
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_endTime;

    void setStatus(Status status);
    void setErrorMessage(const QString& message);

signals:
    void started();
    void finished();
    void failed(const QString& error);

friend class ThreadPool;
};

#endif // TASK_H