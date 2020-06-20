/*****************************************************************************/
/**
 * @file    geInterval.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/29
 * @brief   Template for numeric interval.
 *
 * Template for numeric interval.
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
#include "geMath.h"
#include <limits>

namespace geEngineSDK {
  using std::numeric_limits;

  /**
   * @brief Template for numeric interval
   */
  template<typename ElementType>
  struct Interval
  {
   public:
    /**
     * @brief Default constructor.
     * The interval is invalid
     */
    Interval()
      : min(numeric_limits<ElementType>::max()),
        max(numeric_limits<ElementType>::min())
    {}

    /**
     * @brief Creates and initializes a new interval with the specified lower
     *        and upper bounds.
     * @param InMin The lower bound of the constructed interval.
     * @param InMax The upper bound of the constructed interval.
     */
    Interval(ElementType InMin, ElementType InMax)
      : min(InMin),
        max(InMax)
    {}

   public:
    /**
     * @brief Offset the interval by adding X.
     * @param X The offset.
     */
    void
    operator+=(ElementType X) {
      if (IsValid()) {
        min += X;
        max += X;
      }
    }

    /**
     * @brief Offset the interval by subtracting X.
     * @param X The offset.
     */
    void
    operator-=(ElementType X) {
      if (IsValid()) {
        min -= X;
        max -= X;
      }
    }

   public:
    /**
     * @brief Computes the size of this interval.
     * @return Interval size.
     */
    ElementType
    size() const {
      return (max - min);
    }

    /**
     * @brief Whether interval is valid (min <= max).
     * @return false when interval is invalid, true otherwise
     */
    ElementType
    isValid() const {
      return (min <= max);
    }

    /**
     * @brief Checks whether this interval contains the specified element.
     * @param Element The element to check.
     * @return true if the range interval the element, false otherwise.
     */
    bool
    contains(const ElementType& Element) const {
      return isValid() && (Element >= min && Element <= max);
    }

    /**
     * @brief Expands this interval to both sides by the specified amount.
     * @param ExpandAmount The amount to expand by.
     */
    void
    expand(ElementType ExpandAmount) {
      if (isValid()) {
        min -= ExpandAmount;
        max += ExpandAmount;
      }
    }

    /**
     * @brief Expands this interval if necessary to include the specified element.
     * @param X The element to include.
     */
    void
    include(ElementType X) {
      if (!isValid()) {
        min = X;
        max = X;
      }
      else {
        if (X < min) {
          min = X;
        }
        if (X > max) {
          max = X;
        }
      }
    }

    /**
     * @brief Interval interpolation
     * @param Alpha interpolation amount
     * @return interpolation result
     */
    ElementType
    interpolate(float Alpha) const {
      if (IsValid()) {
        return min + ElementType(Alpha * size());
      }

      return ElementType();
    }

   public:
    /**
     * @brief Calculates the intersection of two intervals.
     * @param A The first interval.
     * @param B The second interval.
     * @return The intersection.
     */
    friend Interval
    intersect(const Interval& A, const Interval& B) {
      if (A.isValid() && B.isValid()) {
        return Interval(Math::max(A.min, B.min), Math::min(A.max, B.max));
      }

      return Interval();
    }

   public:
    /**
     * @brief Holds the lower bound of the interval.
     */
    ElementType min;

    /**
     * @brief Holds the upper bound of the interval.
     */
    ElementType max;
  };

  /*
   * Default intervals for built-in types
   */

#define DEFINE_INTERVAL_WRAPPER_STRUCT(Name, ElementType)                     \
  struct Name : Interval<ElementType>                                         \
  {                                                                           \
   private:                                                                   \
    typedef Interval<ElementType> super;                                      \
                                                                              \
   public:                                                                    \
    Name() : super() {}                                                       \
                                                                              \
    Name(const super& Other) : super(Other) {}                                \
                                                                              \
    Name(ElementType InMin, ElementType InMax) : super(InMin, InMax) {}       \
                                                                              \
    friend Name                                                               \
    intersect(const Name& A, const Name& B) {                                 \
      return intersect(static_cast<const super&>(A),                          \
                       static_cast<const super&>(B));                         \
    }                                                                         \
  };

  DEFINE_INTERVAL_WRAPPER_STRUCT(FloatInterval, float);
  DEFINE_INTERVAL_WRAPPER_STRUCT(Int32Interval, int32);
}
