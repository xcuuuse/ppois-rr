#include <street_cleaning_module.hpp>

#include "agents/calculating_and_analys_degree_of_vertices_agent.hpp"
#include "agents/create_cleaning_route_agent.hpp"

SC_MODULE_REGISTER(StreetCleaningModule)->Agent<CalculatingAndAnalysDegreeOfVerticesAgent>()->Agent<CreateCleaningRouteAgent>();