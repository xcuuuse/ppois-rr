#include <sc-memory/sc_memory_headers.hpp>

#include <keynodes/graph_keynodes.hpp>

#include "create_graph_agent.hpp"
#include <string>

CreateGraphAgent::CreateGraphAgent()
{
    m_logger = utils::ScLogger(utils::ScLogger::ScLogType::File, "logs/create_graph_agent.log", utils::ScLogLevel::Debug, true);
}

ScAddr CreateGraphAgent::GetActionClass() const {
    //return GraphKeynodes::action_
}
