/*****************************************************************************/
/**
 * @file    geAsyncOp.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/18
 * @brief   Thread synchronization primitives used by AsyncOps.
 *
 * Thread synchronization primitives used by AsyncOps and their callers.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geAsyncOp.h"
#include "geDebug.h"

namespace geEngineSDK {
  using std::memory_order_acquire;
  using std::memory_order_release;

  bool
  AsyncOp::hasCompleted() const {
    return m_data->m_isCompleted.load(memory_order_acquire);
  }

  void
  AsyncOp::_completeOperation(Any returnValue) {
    m_data->m_returnValue = returnValue;
    m_data->m_isCompleted.store(true, memory_order_release);

    if (nullptr != m_syncData) {
      m_syncData->m_condition.notify_all();
    }
  }

  void
  AsyncOp::_completeOperation() {
    m_data->m_isCompleted.store(true, memory_order_release);

    if (nullptr != m_syncData) {
      m_syncData->m_condition.notify_all();
    }
  }

  void
  AsyncOp::blockUntilComplete() const {
    if (nullptr == m_syncData) {
      LOGERR("No sync data is available. Cannot block until AsyncOp is complete.");
      return;
    }

    Lock lock(m_syncData->m_mutex);
    while (!hasCompleted()) {
      m_syncData->m_condition.wait(lock);
    }
  }
}
