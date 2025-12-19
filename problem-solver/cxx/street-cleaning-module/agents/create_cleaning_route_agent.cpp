#include "create_cleaning_route_agent.hpp"

#include <sc-memory/sc_memory.hpp>
#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_iterator.hpp>
#include <sc-memory/sc_link.hpp>

#include "keynodes/graph_keynodes.hpp"
//#include "errors/AgentsErrors.hpp"

#include <stack>
#include <map>
#include <algorithm>
#include <string>

#define EPS 1e-9
#define INF 1e9

CreateCleaningRouteAgent::CreateCleaningRouteAgent()
{
  m_logger =
      utils::ScLogger(utils::ScLogger::ScLogType::Console, "logs/find_route_agent.log", utils::ScLogLevel::Debug, true);
}

ScAddr CreateCleaningRouteAgent::GetActionClass() const
{
  return GraphKeynodes::action_find_optimal_route;
}

ScResult CreateCleaningRouteAgent::DoProgram(ScAction & action)
{
  m_logger.Info("Начало поиска маршрута");
  auto const & [networkAddr] = action.GetArguments<1>();

  if (!m_context.IsElement(networkAddr))
  {
    m_logger.Error("Дорожная сеть не найдена");
    return action.FinishWithError();
  }

  ScAddrVector tempElements;
  TempToMapping.clear();

  try
  {
    UpgradeToEuler(networkAddr, tempElements);
    m_logger.Info("Граф подготовлен. Временных элементов: " + std::to_string(tempElements.size()));

    ScAddr startVertex = GetStartVertex(networkAddr);

    ScAddrVector route = FindEulerCycle(networkAddr);
    m_logger.Info("Маршрут найден: " + std::to_string(route.size()) + " улиц");

    ScStructure resultStructure = CreateCleaningRouteStructure(networkAddr, route);
    action.SetResult(resultStructure);

    Cleanup(tempElements);
  }
  catch (std::exception & e)
  {
    m_logger.Error("Ошибка: " + std::string(e.what()));
    Cleanup(tempElements);
    return action.FinishWithError();
  }

  m_logger.Info("Успешное завершение работы");
  return action.FinishSuccessfully();
}

ScAddrVector CreateCleaningRouteAgent::FindEulerCycle(ScAddr const & networkAddr)
{
  ScAddr startVertex = GetStartVertex(networkAddr);
  AdjList adj;

  ScAddrUnorderedSet processedStreets;

  ScIterator3Ptr intersectionIt = m_context.CreateIterator3(networkAddr, ScType::ConstPermPosArc, ScType::Unknown);
  while (intersectionIt->Next())
  {
    ScAddr const vertexAddr = intersectionIt->Get(2);
    ScIterator3Ptr const isIntersectionIt =
        m_context.CreateIterator3(GraphKeynodes::concept_intersection, ScType::ConstPermPosArc, vertexAddr);

    if (isIntersectionIt->Next())
    {
      ScAddr intersection = intersectionIt->Get(2);

      ScIterator5Ptr streetIt = m_context.CreateIterator5(
          ScType::Unknown,
          ScType::ConstCommonArc,
          intersection,
          ScType::ConstPermPosArc,
          GraphKeynodes::nrel_connects);

      while (streetIt->Next())
      {
        ScAddr streetNode = streetIt->Get(0);

        if (processedStreets.find(streetNode) != processedStreets.end())
          continue;

        processedStreets.insert(streetNode);

        ScAddr intersection1, intersection2;
        ScIterator5Ptr neighborIt = m_context.CreateIterator5(
            streetNode,
            ScType::ConstCommonArc,
            ScType::Unknown,
            ScType::ConstPermPosArc,
            GraphKeynodes::nrel_connects);

        while (neighborIt->Next())
        {
          ScAddr neighbor = neighborIt->Get(2);
          if (neighbor != intersection)
          {
            adj[intersection].push_back({streetNode, neighbor, false});
          }
        }
      }
    }
  }

  std::stack<std::pair<ScAddr, ScAddr>> pathStack;
  ScAddrVector path;

  pathStack.push({startVertex, ScAddr::Empty});

  while (!pathStack.empty())
  {
    ScAddr currentV = pathStack.top().first;
    bool hasUnusedEdge = false;

    if (adj.find(currentV) != adj.end())
    {
      for (auto & edge : adj[currentV])
      {
        if (!edge.isUsed)
        {
          edge.isUsed = true;

          for (auto & backEdge : adj[edge.intersection])
          {
            if (backEdge.street == edge.street && backEdge.intersection == currentV && !backEdge.isUsed)
            {
              backEdge.isUsed = true;
              break;
            }
          }

          pathStack.push({edge.intersection, edge.street});
          hasUnusedEdge = true;
          break;
        }
      }
    }
    if (!hasUnusedEdge)
    {
      ScAddr street = pathStack.top().second;
      if (street.IsValid())
      {
        if (TempToMapping.find(street) != TempToMapping.end())
        {
          path.push_back(TempToMapping[street]);
        }
        else
        {
          path.push_back(street);
        }
      }
      pathStack.pop();
    }
  }

  return path;
}

int CreateCleaningRouteAgent::GetVertexDegree(ScAddr const & vertexAddr)
{
  ScIterator5Ptr it = m_context.CreateIterator5(
      vertexAddr,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);

  if (it->Next())
  {
    ScAddr link = it->Get(2);
    std::string vertexDegreeStr;
    m_context.GetLinkContent(link, vertexDegreeStr);
    int degree = std::stoi(vertexDegreeStr);
    return degree;
  }
  else
    throw VertexError("Степень вершины не посчитана");
}

ScAddr CreateCleaningRouteAgent::GetStartVertex(ScAddr const & networkAddr)
{
  ScIterator5Ptr const it = m_context.CreateIterator5(
      networkAddr,
      ScType::ConstPermPosArc,
      ScType::Unknown,
      ScType::ConstPermPosArc,
      GraphKeynodes::rrel_start_point);

  if (it->Next())
  {
    ScAddr startVertex = it->Get(2);
    return startVertex;
  }
  else
    throw StartNodeNotFoundError("Начальная вершина не задана");
}

void CreateCleaningRouteAgent::Cleanup(ScAddrVector const & tempElements)
{
  for (auto const & item : tempElements)
  {
    if (m_context.IsElement(item))
    {
      m_context.EraseElement(item);
    }
  }
}

ScStructure CreateCleaningRouteAgent::CreateCleaningRouteStructure(ScAddr const & networkAddr, ScAddrVector route)
{
  ScStructure result = m_context.GenerateStructure();

  int pos = 1;
  double length = 0.0;

  ScAddr routeTuple = m_context.GenerateNode(ScType::ConstNodeTuple);

  ScAddr const & arcCommonAddr = m_context.GenerateConnector(ScType::ConstCommonArc, networkAddr, routeTuple);
  ScAddr const & rrelOptimalRoute =
      m_context.GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_optimal_route, arcCommonAddr);

  if (route.empty())
  {
    throw std::runtime_error("Маршрут пустой");
  }

  result << routeTuple << networkAddr << arcCommonAddr << rrelOptimalRoute;

  for (auto const & node : route)
  {
    ScAddr const & positionNumber = m_context.ResolveElementSystemIdentifier("rrel_" + std::to_string(pos));

    ScAddr const & arcPermPosAddr = m_context.GenerateConnector(ScType::ConstPermPosArc, routeTuple, node);
    ScAddr const & rrelPosition = m_context.GenerateConnector(ScType::ConstPermPosArc, positionNumber, arcPermPosAddr);
    result << node << positionNumber << arcPermPosAddr << rrelPosition;

    double streetLength = GetStreetLength(node);

    length += streetLength;

    pos++;
  }

  ScAddr const & routeLength = m_context.GenerateLink(ScType::ConstNodeLink);
  m_context.SetLinkContent(routeLength, std::to_string(length));

  ScAddr const & arcCommonAddr2 = m_context.GenerateConnector(ScType::ConstCommonArc, routeTuple, routeLength);
  ScAddr const & nrelRouteLength =
      m_context.GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_route_length, arcCommonAddr2);

  result << routeLength << arcCommonAddr2 << nrelRouteLength;

  return result;
}

void CreateCleaningRouteAgent::UpgradeToEuler(ScAddr const & networkAddr, ScAddrVector & tempElements)
{
  ScIterator3Ptr networkIt = m_context.CreateIterator3(
      GraphKeynodes::concept_graph_with_euler_cycle, ScType::ConstPermPosArc, networkAddr);
  if (networkIt->Next())
  {
    return;
  }

  ScAddrVector oddVertices;
  ScIterator3Ptr intersectionIt = m_context.CreateIterator3(networkAddr, ScType::ConstPermPosArc, ScType::Unknown);

  while (intersectionIt->Next())
  {
    ScAddr const vertexAddr = intersectionIt->Get(2);

    ScIterator3Ptr const isIntersectionIt =
        m_context.CreateIterator3(StreetCleaningKeynodes::concept_intersection, ScType::ConstPermPosArc, vertexAddr);

    if (isIntersectionIt->Next())
    {
      ScAddr const vertex = vertexAddr;

      int degree = GetVertexDegree(vertex);

      if (degree % 2 != 0)
      {
        oddVertices.push_back(vertex);
      }
    }
  }

  m_logger.Info("Нечетных вершин: " + std::to_string(oddVertices.size()));

  int n = oddVertices.size();
  if (n == 0)
  {
    return;
  }

  std::vector<std::vector<double>> dists(n, std::vector<double>(n));
  std::map<std::pair<int, int>, std::list<ScAddr>> pathsCache;

  for (int i = 0; i < n; i++)
  {
    for (int j = i + 1; j < n; j++)
    {
      PathResult result = FindShortestPath(oddVertices[i], oddVertices[j]);


