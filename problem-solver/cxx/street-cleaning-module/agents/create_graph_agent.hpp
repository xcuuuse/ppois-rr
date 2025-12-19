#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>

#include "utils/utils.hpp"

class CreateGraphAgent : public ScActionInitiatedAgent
{
public:
    CreateGraphAgent();

    // Класс действия: action_find_optimal_route
    ScAddr GetActionClass() const override;

    // Построение графа (площади + улицы)
    ScResult DoProgram(
        ScActionInitiatedEvent const & event,
        ScAction & action) override;
    
private:
    Euler GetGraphEulerStatus(ScAddr const & networkAddr, ScAddrToValueUnorderedMap<int> intersections);
    ScStructure CreateResult(ScAddr const & networkAddr, ScAddrToValueUnorderedMap<int> vertexes, Euler);

};
//FINISH THIS SHIT