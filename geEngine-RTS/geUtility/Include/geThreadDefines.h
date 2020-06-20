/*****************************************************************************/
/**
 * @file    geThreadDefines.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/09
 * @brief   Defines for mutex and thread utilities
 *
 * Here are the defines of the mutex and threads for the engine
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/
#pragma once

/**
 * @brief Define the auto mutex name
 */
#define GE_AUTO_MUTEX_NAME mutex

/****************************************************************************/
/**
 * Includes
 */
/****************************************************************************/
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "geSpinLock.h"

/**
 * @brief Returns the number of logical CPU cores.
 */
#define GE_THREAD_HARDWARE_CONCURRENCY std::thread::hardware_concurrency()

/**
 * @brief Returns the ThreadId of the current thread.
 */
#define GE_THREAD_CURRENT_ID std::this_thread::get_id()

/**
 * @brief Causes the current thread to sleep for the provided amount of
 *        milliseconds.
 */
#define GE_THREAD_SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));

/**
 * @brief Wrapper for the C++ std::mutex.
 */
using Mutex = std::mutex;

/**
 * @brief Wrapper for the C++ std::recursive_mutex.
 */
using RecursiveMutex = std::recursive_mutex;

/**
 * @brief Wrapper for the C++ std::condition_variable.
 */
using Signal = std::condition_variable;

/**
 * @brief Wrapper for the C++ std::thread.
 */
using Thread = std::thread;

/**
 * @brief Wrapper for the C++ std::thread::id.
 */
using ThreadId = std::thread::id;

/**
 * @brief Wrapper for the C++ std::unique_lock<std::mutex>.
 */
using Lock = std::unique_lock<Mutex>;

/**
 * @brief Wrapper for the C++ std::unique_lock<std::recursive_mutex>.
 */
using RecursiveLock = std::unique_lock<RecursiveMutex>;
