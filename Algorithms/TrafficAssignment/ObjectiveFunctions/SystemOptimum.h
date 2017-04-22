#pragma once

#include <vectorclass/vectorclass.h>

// Represents the system-optimum (SO) objective function. The flow pattern that minimizes the SO
// objective function (while satisfying the flow conservation constraint) minimizes the total
// travel cost. The SO flow pattern is obtained by iterative shortest-path computations using
// appropriate edge weights.
template <typename TravelCostFunctionT>
class SystemOptimum {
 public:
  // Constructs a SO objective function.
  SystemOptimum(TravelCostFunctionT function) : travelCostFunction(function) {}

  // Returns the weight of edge e, given the flow x on e.
  float getEdgeWeight(const int e, const float x) const {
    return travelCostFunction(e, x) + x * travelCostFunction.derivative(e, x);
  }

  // Returns the weights of eight consecutive edges starting at e, given the flows x on them.
  Vec8f getEdgeWeights(const int e, const Vec8f& x) const {
    return travelCostFunction(e, x) + x * travelCostFunction.derivative(e, x);
  }

 private:
  TravelCostFunctionT travelCostFunction; // A functor returning the travel cost on an edge.
};