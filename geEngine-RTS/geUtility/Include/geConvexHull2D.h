/*****************************************************************************/
/**
 * @file    geConvexHull2D.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/29
 * @brief   Functions to compute a Convex Hull in 2D.
 *
 * Functions to compute a Convex Hull in 2D using the Jarvis march algorithm.
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
#include "geVector2.h"
#include "geVector3.h"
#include "geNumericLimits.h"

namespace geEngineSDK {
  namespace ConvexHull2D {
    /**
     * @brief Returns < 0 if C is left of A-B
     */
    inline float
    computeDeterminant(const Vector3& A, const Vector3& B, const Vector3& C) {
      const float u1 = B.x - A.x;
      const float v1 = B.y - A.y;
      const float u2 = C.x - A.x;
      const float v2 = C.y - A.y;
      return u1 * v2 - v1 * u2;
    }

    /**
     * @brief Returns true if 'a' is more lower-left than 'b'.
     */
    inline bool
    comparePoints(const Vector3& A, const Vector3& B) {
      if (A.x < B.x) {
        return true;
      }

      if (A.x > B.x) {
        return false;
      }

      if (A.y < B.y) {
        return true;
      }

      if (A.y > B.y) {
        return false;
      }

      return false;
    }

    /**
     * @brief Calculates convex hull on xy-plane of points on 'Points' and
     *        stores the indices of the resulting hull in 'OutIndices'.
     * This code was fixed to work with duplicated vertices and precision issues.
     */
    template<typename A = StdAlloc<Vector3>>
    void
    computeConvexHull(const Vector<Vector3, A>& Points,
                      Vector<int32, A>& OutIndices) {
      if (0 == Points.size()) {
        //Early exit here, otherwise an invalid index will be added to the output.
        return;
      }

      //Find lower-leftmost point.
      int32 HullStart = 0;
      int32 HullEnd = 0;

      for (int32 i = 1; i < Points.size(); ++i) {
        if (comparePoints(Points[i], Points[HullStart])) {
          HullStart = i;
        }
        if (comparePoints(Points[HullEnd], Points[i])) {
          HullEnd = i;
        }
      }

      OutIndices.push_back(HullStart);

      if (HullStart == HullEnd) {
        //Convex hull degenerated to a single point
        return;
      }

      //Gift wrap Hull.
      int32 Hull = HullStart;
      int32 LocalEnd = HullEnd;
      bool bGoRight = true;
      bool bFinished = false;

      //Sometimes it hangs on infinite loop,
      //repeating sequence of indices (e.g. 4,9,8,9,8,...)
      while (OutIndices.size() <= Points.size()) {
        int32 NextPoint = LocalEnd;

        for (int32 j = 0; j < Points.size(); ++j) {
          if (j == NextPoint || j == Hull) {
            continue;
          }

          Vector3 A = Points[Hull];
          Vector3 B = Points[NextPoint];
          Vector3 C = Points[j];
          float Deter = computeDeterminant(A, B, C);

          //0.001 Bias is to stop floating point errors, when comparing points
          //on a straight line; KINDA_SMALL_NUMBER was slightly too small to use.
          if (-0.001 > Deter) {
            //C is left of AB, take it
            NextPoint = j;
          }
          else if (0.001 > Deter) {
            if (bGoRight) {
              if (comparePoints(B, C)) {
                //We go right, take it
                NextPoint = j;
              }
            }
            else {
              if (comparePoints(C, B)) {
                //We go left, take it
                NextPoint = j;
              }
            }
          }
          else {
            //C is right of AB, don't take it
          }
        }

        if (NextPoint == HullEnd) {
          //Turn around
          bGoRight = false;
          LocalEnd = HullStart;
        }

        if (NextPoint == HullStart) {
          //Finish
          bFinished = true;
          break;
        }

        OutIndices.push_back(NextPoint);
        Hull = NextPoint;
      }

      //Clear all indices if main loop was left without finishing shape
      if (!bFinished) {
        OutIndices.clear();
      }
    }

    /**
     * @brief Returns < 0 if C is left of A-B
     */
    inline float
    computeDeterminant2D(const Vector2& A, const Vector2& B, const Vector2& C) {
      const float u1 = B.x - A.x;
      const float v1 = B.y - A.y;
      const float u2 = C.x - A.x;
      const float v2 = C.y - A.y;

      return u1 * v2 - v1 * u2;
    }

    /**
     * @brief Alternate simple implementation that was found to work correctly
     *        for points that are very close together (inside the 0-1 range).
     */
    template<typename A = StdAlloc<Vector2>>
    void
    computeConvexHull2(const Vector<Vector2, A>& Points, Vector<int32, A>& OutIndices) {
      if (0 == Points.size()) {
        return;
      }

      //Jarvis march implementation
      int32 LeftmostIndex = -1;
      Vector2 Leftmost(NumLimit::MAX_FLOAT, NumLimit::MAX_FLOAT);

      for (int32 PointIndex = 0; PointIndex < Points.size(); ++PointIndex) {
        if (Points[PointIndex].X < Leftmost.X ||
           (Points[PointIndex].X == Leftmost.X && Points[PointIndex].Y < Leftmost.Y)) {
          LeftmostIndex = PointIndex;
          Leftmost = Points[PointIndex];
        }
      }

      int32 PointOnHullIndex = LeftmostIndex;
      int32 EndPointIndex;

      do {
        OutIndices.push_back(PointOnHullIndex);
        EndPointIndex = 0;

        //Find the 'leftmost' point to the line from the last hull vertex to a candidate
        for (int32 j = 1; j < Points.size(); ++j) {
          if (EndPointIndex == PointOnHullIndex ||
              computeDeterminant2D(Points[EndPointIndex],
                                   Points[OutIndices[OutIndices.size()-1]],
                                   Points[j]) < 0) {
            EndPointIndex = j;
          }
        }

        PointOnHullIndex = EndPointIndex;
      } while (EndPointIndex != LeftmostIndex);
    }
  }
}
