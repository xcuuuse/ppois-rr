#pragma once

#include <sc-memory/sc_agent.hpp>
#include <vector>

#include "utils/edge.hpp"
#include "utils/route.hpp"

class CreateCleaningRouteAgent : public ScActionInitiatedAgent
{
public:
    CreateCleaningRouteAgent();
    ScAddr GetActionClass() const override;
    ScResult DoProgram(ScAction & action) override;

private:
    using AdjList = std::map<ScAddr, std::vector<Edge>, ScAddrLessFunc>;
    using PQPair = std::pair<double, ScAddr>;

    ScAddrToValueUnorderedMap<ScAddr> TempToMapping;

    void UpgradeToEuler(ScAddr const & networkAddr, ScAddrVector & tempElements);
    ScStructure CreateCleaningRouteStructure(ScAddr const & networkAddr, ScAddrVector route);
    ScAddrVector FindEulerCycle(ScAddr const & networkAddr);
    void Cleanup(ScAddrVector const & tempElements);
    int GetVertexDegree(ScAddr const & vertexAddr);
    ScAddr GetStartVertex(ScAddr const & networkAddr);

    void GetPairs(
        int mask,
        int n,
        std::vector<std::vector<double>> const & dists,
        std::vector<double> const & memo,
        std::vector<std::pair<int, int>> & resultPairs);
    double SolveMatching(int mask, int n, std::vector<std::vector<double>> const & dists, std::vector<double> & memo);
    Route FindShortestPath(ScAddr const & start, ScAddr const & end);
    double GetStreetLength(ScAddr const & streetAddr);
};



