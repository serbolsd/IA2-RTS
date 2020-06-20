/*****************************************************************************/
/**
 * @file    geTaskScheduler.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/14
 * @brief   Represents a task scheduler running on multiple threads.
 *
 * Represents a task scheduler running on multiple threads. You may queue tasks
 * on it from any thread and they will be executed in user specified order on
 * any available thread.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geTaskScheduler.h"
#include "geThreadPool.h"

namespace geEngineSDK {
  using std::bind;
  using std::find;

  Task::Task(const PrivatelyConstruct&,
             const String& name,
             function<void()> taskWorker,
             TASKPRIORITY::E priority,
             SPtr<Task> dependency)
    : m_name(name),
      m_priority(priority),
      m_taskId(0),
      m_taskWorker(std::move(taskWorker)),
      m_taskDependency(std::move(dependency)),
      m_state(0),
      m_parent(nullptr) {}

  SPtr<Task>
  Task::create(const String& name,
               function<void()> taskWorker,
               TASKPRIORITY::E priority,
               SPtr<Task> dependency) {
    return ge_shared_ptr_new<Task>(PrivatelyConstruct(),
                                   name,
                                   std::move(taskWorker),
                                   priority,
                                   std::move(dependency));
  }

  bool
  Task::isComplete() const {
    return m_state == 2;
  }

  bool
  Task::isCanceled() const {
    return m_state == 3;
  }

  void
  Task::wait() {
    if (nullptr != m_parent) {
      m_parent->waitUntilComplete(this);
    }
  }

  void
  Task::cancel() {
    m_state = 3;
  }

  TaskScheduler::TaskScheduler()
    : m_taskQueue(&TaskScheduler::taskCompare),
      m_maxActiveTasks(GE_THREAD_HARDWARE_CONCURRENCY),
      m_nextTaskId(0),
      m_shutdown(false),
      m_checkTasks(false) {
    m_taskSchedulerThread = ThreadPool::instance().run("TaskScheduler",
                                                       bind(&TaskScheduler::runMain, this));
  }

  TaskScheduler::~TaskScheduler() {
    //Wait until all tasks complete
    {
      Lock activeTaskLock(m_readyMutex);

      while (!m_activeTasks.empty()) {
        SPtr<Task> task = m_activeTasks[0];
        activeTaskLock.unlock();

        task->wait();
        activeTaskLock.lock();
      }
    }

    //Start shutdown of the main queue worker and wait until it exits
    {
      Lock lock(m_readyMutex);
      m_shutdown = true;
    }

    m_taskReadyCond.notify_one();
    m_taskSchedulerThread.blockUntilComplete();
  }

  void
  TaskScheduler::addTask(SPtr<Task> task) {
    Lock lock(m_readyMutex);

    GE_ASSERT(task->m_state != 1 &&
              "Task is already executing, it cannot be executed again until it finishes.");

    task->m_parent = this;
    task->m_taskId = m_nextTaskId++;
    task->m_state.store(0); //Reset state in case the task is getting re-queued

    m_checkTasks = true;
    m_taskQueue.insert(std::move(task));

    //Wake main scheduler thread
    m_taskReadyCond.notify_one();
  }

  void
  TaskScheduler::addWorker() {
    Lock lock(m_readyMutex);
    ++m_maxActiveTasks;

    //A spot freed up, queue new tasks on main scheduler thread if they exist
    m_taskReadyCond.notify_one();
  }

  void
  TaskScheduler::removeWorker() {
    Lock lock(m_readyMutex);

    if (m_maxActiveTasks > 0) {
      --m_maxActiveTasks;
    }
  }

  void
  TaskScheduler::runMain() {
    while (true) {
      Lock lock(m_readyMutex);

      while ((!m_checkTasks || (uint32)m_activeTasks.size() >= m_maxActiveTasks)
             && !m_shutdown) {
        m_taskReadyCond.wait(lock);
      }

      m_checkTasks = false;

      if (m_shutdown) {
        break;
      }

      for (auto iter = m_taskQueue.begin(); iter != m_taskQueue.end();) {
        if ((uint32)m_activeTasks.size() >= m_maxActiveTasks) {
          break;
        }

        SPtr<Task> curTask = *iter;

        if (curTask->isCanceled()) {
          iter = m_taskQueue.erase(iter);
          continue;
        }

        if (nullptr != curTask->m_taskDependency &&
            !curTask->m_taskDependency->isComplete()) {
          ++iter;
          continue;
        }

        iter = m_taskQueue.erase(iter);

        curTask->m_state.store(1);
        m_activeTasks.push_back(curTask);

        ThreadPool::instance().run(curTask->m_name,
                                   bind(&TaskScheduler::runTask, this, curTask));
      }
    }
  }

  void
  TaskScheduler::runTask(SPtr<Task> task) {
    task->m_taskWorker();

    {
      Lock lock(m_readyMutex);

      auto findIter = find(m_activeTasks.begin(), m_activeTasks.end(), task);
      if (findIter != m_activeTasks.end()) {
        m_activeTasks.erase(findIter);
      }
    }

    {
      Lock lock(m_completeMutex);
      task->m_state.store(2);

      m_taskCompleteCond.notify_all();
    }

    //Wake the main scheduler thread in case there are other tasks waiting or
    //this task was someone's dependency
    {
      Lock lock(m_readyMutex);

      m_checkTasks = true;
      m_taskReadyCond.notify_one();
    }
  }

  void
  TaskScheduler::waitUntilComplete(const Task* task) {
    if (task->isCanceled()) {
      return;
    }

    {
      Lock lock(m_completeMutex);

      while (!task->isComplete()) {
        addWorker();
        m_taskCompleteCond.wait(lock);
        removeWorker();
      }
    }
  }

  bool
  TaskScheduler::taskCompare(const SPtr<Task>& lhs, const SPtr<Task>& rhs) {
    //If one tasks priority is higher, that one goes first
    if (lhs->m_priority > rhs->m_priority) {
      return true;
    }

    //Otherwise we go by smaller id, as that task was queued earlier than the other
    return lhs->m_taskId < rhs->m_taskId;
  }
}
