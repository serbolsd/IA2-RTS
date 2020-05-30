/*****************************************************************************/
/**
 * @file    geServiceLocator.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/18
 * @brief   A locator system that allows you to quickly find a service of a
 *          specific type.
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

namespace geEngineSDK {
  using std::atomic;

  /**
  * @brief A locator system that allows you to quickly find a service of a specific type.
  * @note This is similar to a singleton pattern but unlike singleton the active instance
  *       is not required to be available and can be replaced with another system during
  *       runtime, or completely removed.
  */
  template<class T>
  class ServiceLocator
  {
   public:
    /**
     * @brief Returns an instance of the service we are looking for, if one is available.
     * @note  Can return null.
     */
    static T*
    instance() {
      return m_service;
    }

    /**
     * @brief Starts providing a new service when "instance()" is called.
     *        Replaces the previous service.
     */
    static void
    _provide(T* service) {
      m_service = service;
    }

    /**
     * @brief Stops providing a service when "instance()" is called.
     *        Ignored if the current service doesn't match the provided service.
     */
    static void
    _remove(T* service) {
      m_service.compare_exchange_strong(service, nullptr);
    }

   private:
    static atomic<T*> m_service;
  };

  template <class T>
  atomic<T*> ServiceLocator<T>::m_service(nullptr);
}
