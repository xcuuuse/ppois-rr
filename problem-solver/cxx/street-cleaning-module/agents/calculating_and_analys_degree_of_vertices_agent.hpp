#pragma once

#include <sc-memory/sc_agent.hpp>
#include "utils/euler.hpp"

class CalculatingAndAnalysDegreeOfVerticesAgent : public ScActionInitiatedAgent
{
public:
  CalculatingAndAnalysDegreeOfVerticesAgent();
  ScAddr GetActionClass() const override;
  ScResult DoProgram(ScAction & action) override;

private:
  ScAddrToValueUnorderedMap<int> CalculateVertexesDegrees(ScAddr const & graphAddr);
  Euler GetGraphEulerianStatus(ScAddr const & graphAddr, ScAddrToValueUnorderedMap<int> vertices);
  ScStructure CreateAnalysisResult(ScAddr const & graphAddr, ScAddrToValueUnorderedMap<int> vertices, Euler);
};
