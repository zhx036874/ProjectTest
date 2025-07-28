#include "task.h"
#include <QDebug>

// 默认构造函数
// @param parent 父对象指针，用于Qt对象树管理
Task::Task(QObject *parent)
    : QObject(parent)
    , m_priority(Priority::Normal)  // 默认优先级为Normal
    , m_status(Status::Created)     // 初始状态为Created
{
}

// 带参数构造函数
// @param function 任务执行函数
// @param priority 任务优先级
// @param parent 父对象指针
Task::Task(std::function<void()> function, Priority priority, QObject *parent)
    : QObject(parent)
    , m_function(function)          // 设置任务函数
    , m_priority(priority)          // 设置优先级
    , m_status(Status::Created)     // 初始状态为Created
{
}

Task::~Task() = default;

// 设置任务函数
// @param function 要执行的函数对象
void Task::setFunction(std::function<void()> function) {
    // 只有在任务创建状态才能设置函数
    if (m_status != Status::Created) {
        qWarning() << "Cannot set function after task has been queued or started";
        return;
    }
    m_function = function;
}

// 设置任务优先级
// @param priority 任务优先级（Low/Normal/High）
void Task::setPriority(Priority priority) {
    // 只有在任务创建状态才能修改优先级
    if (m_status != Status::Created) {
        qWarning() << "Cannot change priority after task has been queued or started";
        return;
    }
    m_priority = priority;
}

// 获取任务优先级
// @return 当前任务的优先级
Task::Priority Task::priority() const {
    return m_priority;
}

// 获取任务状态
// @return 当前任务的状态
Task::Status Task::status() const {
    return m_status;
}

// 获取错误信息
// @return 任务执行过程中的错误信息
QString Task::errorMessage() const {
    return m_errorMessage;
}

// 获取任务执行时间（毫秒）
// @return 任务的执行时间，如果任务未完成则返回0
qint64 Task::executionTime() const {
    // 只有在任务完成或失败时才能获取执行时间
    if (m_status != Status::Completed && m_status != Status::Failed) {
        return 0;
    }
    // 计算任务执行的时间差
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        m_endTime - m_startTime).count();
}

// 执行任务
// 该方法会执行设置的任务函数，并处理可能发生的异常
void Task::execute() {
    // 检查是否设置了任务函数
    if (!m_function) {
        setErrorMessage("No function set for task");
        setStatus(Status::Failed);
        emit failed(m_errorMessage);
        return;
    }

    try {
        // 更新任务状态并记录开始时间
        setStatus(Status::Running);
        emit started();
        m_startTime = std::chrono::steady_clock::now();

        // 执行任务函数
        m_function();

        // 记录结束时间并更新状态
        m_endTime = std::chrono::steady_clock::now();
        setStatus(Status::Completed);
        emit finished();
    }
    catch (const std::exception& e) {
        // 处理标准异常
        m_endTime = std::chrono::steady_clock::now();
        setErrorMessage(QString::fromStdString(e.what()));
        setStatus(Status::Failed);
        emit failed(m_errorMessage);
    }
    catch (...) {
        // 处理未知异常
        m_endTime = std::chrono::steady_clock::now();
        setErrorMessage("Unknown error occurred");
        setStatus(Status::Failed);
        emit failed(m_errorMessage);
    }
}

// 设置任务状态
// @param status 新的任务状态
void Task::setStatus(Status status) {
    if (m_status != status) {
        m_status = status;
    }
}

// 设置错误信息
// @param message 错误信息
void Task::setErrorMessage(const QString& message) {
    m_errorMessage = message;
}