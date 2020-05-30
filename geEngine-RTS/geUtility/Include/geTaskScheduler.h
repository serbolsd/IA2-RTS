/*****************************************************************************/
/**
 * @file    geTaskScheduler.h
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
#pragma once

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesUtil.h"
#include "geModule.h"
#include "geThreadPool.h"

namespace geEngineSDK {
  using std::function;
  using std::atomic;

  class TaskScheduler;

  /**
   * @brief Task priority. Tasks with higher priority will get executed sooner.
   */
  namespace TASKPRIORITY {
    enum E {
      kVeryLow  = 98,
      kLow      = 99,
      kNormal   = 100,
      kHigh     = 101,
      kVeryHigh = 102
    };
  }

  /**
   * @brief Represents a single task that may be queued in the TaskScheduler.
   * @note	Thread safe.
   */
  class GE_UTILITY_EXPORT Task
  {
    struct PrivatelyConstruct {};
  
   public:
    Task(const PrivatelyConstruct& dummy,
         const String& name,
         function<void()> taskWorker,
         TASKPRIORITY::E priority,
         SPtr<Task> dependency);

    /**
     * @brief Creates a new task. Task should be provided to TaskScheduler in
     *        order for it to start.
     * @param[in] name  Name you can use to more easily identify the task.
     * @param[in] taskWorker  Worker method that does all of the work in the task.
     * @param[in] priority (optional) Higher priority means the tasks will be executed sooner.
     * @param[in] dependency (optional) Task dependency if one exists. If provided the task
     *            will not be executed until its dependency is complete.
     */
    static SPtr<Task>
    create(const String& name,
      function<void()> taskWorker,
      TASKPRIORITY::E priority = TASKPRIORITY::kNormal,
      SPtr<Task> dependency = nullptr);

    /**
     * @brief Returns true if the task has completed.
     */
    bool
    isComplete() const;

    /**
     * @brief Returns true if the task has been canceled.
     */
    bool
    isCanceled() const;

    /**
     * @brief Blocks the current thread until the task has completed.
     * @note  While waiting adds a new worker thread, so that the blocking
     *        threads core can be utilized.
     */
    void
    wait();

    /**
     * @brief Cancels the task and removes it from the TaskSchedulers queue.
     */
    void
    cancel();

   private:
    friend class TaskScheduler;

    String m_name;
    TASKPRIORITY::E m_priority;
    uint32 m_taskId;
    function<void()> m_taskWorker;
    SPtr<Task> m_taskDependency;
    
    /**
     * 0 - Inactive
     * 1 - In progress
     * 2 - Completed
     * 3 - Canceled
     */
    atomic<uint32> m_state;

    TaskScheduler* m_parent;
  };

  /**
   * @brief Represents a task scheduler running on multiple threads. You may
   *        queue tasks on it from any thread and they will be executed in user
   *        specified order on any available thread.
   * @note  Thread safe.
   * @note  This type of task scheduler uses a global queue and is best used for
   *        coarse granularity of tasks. (Number of tasks in the order of hundreds.
   *        Higher number of tasks might require different queuing and locking mechanism,
   *        potentially at the cost of flexibility.)
   * @note  By default the task scheduler will create as many threads as there are physical
   *        CPU cores. You may add or remove threads using addWorker()/removeWorker() methods.
   */
  class GE_UTILITY_EXPORT TaskScheduler : public Module<TaskScheduler>
  {
   public:
    TaskScheduler();
    ~TaskScheduler();

    /**
     * @brief Queues a new task.
     */
    void
    addTask(SPtr<Task> task);

    /**
     * @brief Adds a new worker thread which will be used for executing queued tasks.
     */
    void
    addWorker();

    /**
     * @brief Removes a worker thread (as soon as its current task is finished).
     */
    void
    removeWorker();

    /**
     * @brief Returns the maximum available worker threads (maximum number of
     *        tasks that can be executed simultaneously).
     */
    uint32
    getNumWorkers() const {
      return m_maxActiveTasks;
    }

   protected:
    friend class Task;

    /**
     * @brief Main task scheduler method that dispatches tasks to other threads.
     */
    void
    runMain();

    /**
     * @brief Worker method that runs a single task.
     */
    void
    runTask(SPtr<Task> task);

    /**
     * @brief Blocks the calling thread until the specified task has completed.
     */
    void
    waitUntilComplete(const Task* task);

    /**
     * @brief Method used for sorting tasks.
     */
    static bool
    taskCompare(const SPtr<Task>& lhs, const SPtr<Task>& rhs);

    HThread m_taskSchedulerThread;
    Set<SPtr<Task>, function<bool(const SPtr<Task>&, const SPtr<Task>&)>> m_taskQueue;
    Vector<SPtr<Task>> m_activeTasks;
    uint32 m_maxActiveTasks;
    uint32 m_nextTaskId;
    bool m_shutdown;
    bool m_checkTasks;

    Mutex m_readyMutex;
    Mutex m_completeMutex;
    Signal m_taskReadyCond;
    Signal m_taskCompleteCond;
  };
}
