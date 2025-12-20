#pragma once

#include <sc-memory/sc_agent.hpp>
#include "utils/EulerianStatus.hpp"

class CalculatingAndAnalysDegreeOfVerticesAgent : public ScActionInitiatedAgent
{
public:
  GraphAnalysisAgent();
  ScAddr GetActionClass() const override;
  ScResult DoProgram(ScAction & action) override;

private:
  ScAddrToValueUnorderedMap<int> CalculateVertexesDegrees(ScAddr const & networkAddr);
  EulerianStatus GetGraphEulerianStatus(ScAddr const & networkAddr, ScAddrToValueUnorderedMap<int> vertices);
  ScStructure CreateAnalysisResult(ScAddr const & networkAddr, ScAddrToValueUnorderedMap<int> vertices, EulerianStatus);
};
