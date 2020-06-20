/*****************************************************************************/
/**
 * @file    geAsyncOp.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/18
 * @brief   Thread synchronization primitives used by AsyncOps.
 *
 * Thread synchronization primitives used by AsyncOps and their callers.
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
#include "geException.h"
#include "geAny.h"

namespace geEngineSDK {
  using std::atomic;

  /**
   * @brief Thread synchronization primitives used by AsyncOps and their callers.
   */
  class GE_UTILITY_EXPORT AsyncOpSyncData
  {
   public:
    Mutex m_mutex;
    Signal m_condition;
  };

  /**
   * @brief Flag used for creating async operations signaling that we want to
   *        create an empty AsyncOp with no internal memory storage.
   */
  struct GE_UTILITY_EXPORT AsyncOpEmpty {};

  /**
   * @brief Object you may use to check on the results of an asynchronous operation.
   *        Contains uninitialized data until hasCompleted() returns true.
   * @note  You are allowed (and meant) to copy this by value.
   * @note  You'll notice m_isCompleted isn't synchronized. This is because we're okay if
   *        m_isCompleted reports true a few cycles too late, which is not relevant for
   *        practical use. And in cases where you need to ensure operation has completed
   *        you will usually use some kind of synchronization primitive that includes a
   *        memory barrier anyway.
   */
  class GE_UTILITY_EXPORT AsyncOp
  {
   private:
    struct AsyncOpData
    {
      AsyncOpData() = default;
      Any m_returnValue;
      volatile atomic<bool> m_isCompleted{false};
    };

   public:
    AsyncOp() : m_data(ge_shared_ptr_new<AsyncOpData>()) {}
    explicit AsyncOp(AsyncOpEmpty) {}
    explicit AsyncOp(const SPtr<AsyncOpSyncData>& syncData)
      : m_data(ge_shared_ptr_new<AsyncOpData>()),
        m_syncData(syncData) {}

    AsyncOp(AsyncOpEmpty, const SPtr<AsyncOpSyncData>& syncData)
      : m_syncData(syncData) {}

    /**
     * @brief Returns true if the async operation has completed.
     */
    bool
    hasCompleted() const;

    /**
     * @brief Blocks the caller thread until the AsyncOp completes.
     * @note  Do not call this on the thread that is completing the async op,
     *        as it will cause a deadlock. Make sure the command you are waiting
     *        for is actually queued for execution because a deadlock will occur otherwise.
     */
    void
    blockUntilComplete() const;

    /**
     * @brief Retrieves the value returned by the async operation. Only valid
     *        if hasCompleted() returns true.
     */
    template<typename T>
    T
    getReturnValue() const {
#if GE_DEBUG_MODE
      if (!hasCompleted()) {
        GE_EXCEPT(InternalErrorException,
                  "Trying to get AsyncOp return value but the operation hasn't completed.");
      }
#endif
      //Be careful if cast throws an exception. It doesn't support casting of
      //polymorphic types. Provided and returned types must be EXACT.
      //(You'll have to cast the data yourself when completing the operation)
      return any_cast<T>(m_data->m_returnValue);
    }

    /**
     * @brief Retrieves the value returned by the async operation as a generic
     *        type. Only valid if hasCompleted() returns true.
     */
    Any
    getGenericReturnValue() const {
      return m_data->m_returnValue;
    }

   public:
    /**
     * @brief Mark the async operation as completed.
     */
    void
    _completeOperation(Any returnValue);

    /**
     * @brief Mark the async operation as completed, without setting a return value.
     */
    void
    _completeOperation();

   private:
    SPtr<AsyncOpData> m_data;
    SPtr<AsyncOpSyncData> m_syncData;
  };
}
