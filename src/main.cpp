#include "threadpool.h"
#include "task.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <chrono>
#include <random>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 创建线程池，使用系统建议的线程数
    ThreadPool pool;
    qDebug() << "Thread pool created with" << pool.threadCount() << "threads";

    // 连接信号以处理任务完成和错误
    QObject::connect(&pool, &ThreadPool::taskCompleted, [](std::shared_ptr<Task> task) {
        qDebug() << "Task completed in" << task->executionTime() << "ms";
    });

    QObject::connect(&pool, &ThreadPool::taskFailed, [](std::shared_ptr<Task> task, const QString& error) {
        qDebug() << "Task failed:" << error;
    });

    QObject::connect(&pool, &ThreadPool::errorOccurred, [](const QString& error) {
        qDebug() << "Thread pool error:" << error;
    });

    // 创建一些测试任务
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sleepDist(100, 2000);

    // 添加一些高优先级任务
    for (int i = 0; i < 3; ++i) {
        auto task = std::make_shared<Task>([i, &sleepDist, &gen]() {
            qDebug() << "Executing high priority task" << i;
            QThread::msleep(sleepDist(gen));
            if (i == 1) throw std::runtime_error("Simulated error in high priority task");
        }, Task::Priority::High);
        pool.addTask(task);
    }

    // 添加一些普通优先级任务
    for (int i = 0; i < 5; ++i) {
        auto task = std::make_shared<Task>([i, &sleepDist, &gen]() {
            qDebug() << "Executing normal priority task" << i;
            QThread::msleep(sleepDist(gen));
        });
        pool.addTask(task);
    }

    // 添加一些低优先级任务
    for (int i = 0; i < 3; ++i) {
        auto task = std::make_shared<Task>([i, &sleepDist, &gen]() {
            qDebug() << "Executing low priority task" << i;
            QThread::msleep(sleepDist(gen));
        }, Task::Priority::Low);
        pool.addTask(task);
    }

    // 添加一个无效任务测试错误处理
    auto invalidTask = std::make_shared<Task>();
    pool.addTask(invalidTask);

    // 等待所有任务完成
    QThread::sleep(10);

    // 停止线程池
    pool.stop();
    qDebug() << "Thread pool stopped";

    return app.exec();
}