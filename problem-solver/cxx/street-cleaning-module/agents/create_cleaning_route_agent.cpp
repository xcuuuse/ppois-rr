#include "create_cleaning_route_agent.hpp"

#include <sc-memory/sc_memory.hpp>
#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_iterator.hpp>
#include <sc-memory/sc_link.hpp>

#include "keynodes/graph_keynodes.hpp"
#include "errors/agent_errors.hpp"

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
  auto const & [graphAddr] = action.GetArguments<1>();

  if (!m_context.IsElement(graphAddr))
  {
    m_logger.Error("Дорожная сеть не найдена");
    return action.FinishWithError();
  }

  ScAddrVector tempElements;
  TempToMapping.clear();

  try
  {
    // Добавляем фиктивные улицы, чтобы сделать все вершины чётной степени (условие Эйлера)
    UpgradeToEuler(graphAddr, tempElements);
    m_logger.Info("Граф подготовлен. Временных элементов: " + std::to_string(tempElements.size()));

    ScAddr startVertex = GetStartVertex(graphAddr);

    // Строим эйлеров цикл в модифицированном графе
    ScAddrVector route = FindEulerCycle(graphAddr);
    m_logger.Info("Маршрут найден: " + std::to_string(route.size()) + " улиц");

    // Формируем SC-структуру результата
    ScStructure resultStructure = CreateCleaningRouteStructure(graphAddr, route);
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

ScAddrVector CreateCleaningRouteAgent::FindEulerCycle(ScAddr const & graphAddr)
{
  ScAddr startVertex = GetStartVertex(graphAddr);
  AdjList adj;  // Список смежности: вершина -> список рёбер

  ScAddrUnorderedSet processedStreets;

  // Проходим по всем перекрёсткам в сети
  ScIterator3Ptr intersectionIt = m_context.CreateIterator3(graphAddr, ScType::ConstPermPosArc, ScType::Unknown);
  while (intersectionIt->Next())
  {
    ScAddr const vertexAddr = intersectionIt->Get(2);
    // Проверяем, является ли элемент перекрёстком
    ScIterator3Ptr const isIntersectionIt =
        m_context.CreateIterator3(GraphKeynodes::concept_intersection, ScType::ConstPermPosArc, vertexAddr);

    if (isIntersectionIt->Next())
    {
      ScAddr intersection = intersectionIt->Get(2);

      // Находим все улицы, инцидентные этому перекрёстку
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

        // Находим второй конец улицы (другой перекрёсток)
        ScIterator5Ptr neighbourIt = m_context.CreateIterator5(
            streetNode,
            ScType::ConstCommonArc,
            ScType::Unknown,
            ScType::ConstPermPosArc,
            GraphKeynodes::nrel_connects);

        while (neighbourIt->Next())
        {
          ScAddr neighbour = neighbourIt->Get(2);
          if (neighbour != intersection)
          {
            // Добавляем ориентированное ребро (улицу) в список смежности
            // Важно: порядок полей в Edge — street, isUsed, intersection
            adj[intersection].push_back({streetNode, neighbour, false});
          }
        }
      }
    }
  }

  // Алгоритм поиска эйлерова цикла (Hierholzer)
  std::stack<std::pair<ScAddr, ScAddr>> pathStack; // (текущая вершина, улица, по которой пришли)
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

          // Помечаем обратное ребро как использованное (граф неориентированный)
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
      // Завершаем обход текущей ветви
      ScAddr street = pathStack.top().second;
      if (street.IsValid())
      {
        // Если улица временная — подставляем оригинал из TempToMapping
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

// Возвращает степень вершины (из связанного узла-ссылки)
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

// Получает стартовую вершину маршрута из rrel_start_point
ScAddr CreateCleaningRouteAgent::GetStartVertex(ScAddr const & graphAddr)
{
  ScIterator5Ptr const it = m_context.CreateIterator5(
      graphAddr,
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

// Удаляет временно созданные элементы (дубликаты улиц)
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

// Формирует SC-структуру маршрута: кортеж улиц + длина
ScStructure CreateCleaningRouteAgent::CreateCleaningRouteStructure(ScAddr const & graphAddr, ScAddrVector route)
{
  ScStructure result = m_context.GenerateStructure();

  int pos = 1;
  double length = 0.0;

  ScAddr routeTuple = m_context.GenerateNode(ScType::ConstNodeTuple);

  ScAddr const & arcCommonAddr = m_context.GenerateConnector(ScType::ConstCommonArc, graphAddr, routeTuple);
  ScAddr const & rrelOptimalRoute =
      m_context.GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_optimal_route, arcCommonAddr);

  if (route.empty())
  {
    throw std::runtime_error("Маршрут пустой");
  }

  result << routeTuple << graphAddr << arcCommonAddr << rrelOptimalRoute;

  for (auto const & node : route)
  {
    // Добавляем улицу в кортеж с позиционным отношением
    ScAddr const & positionNumber = m_context.ResolveElementSystemIdentifier("rrel_" + std::to_string(pos));

    ScAddr const & arcPermPosAddr = m_context.GenerateConnector(ScType::ConstPermPosArc, routeTuple, node);
    ScAddr const & rrelPosition = m_context.GenerateConnector(ScType::ConstPermPosArc, positionNumber, arcPermPosAddr);
    result << node << positionNumber << arcPermPosAddr << rrelPosition;

    double streetLength = GetStreetLength(node);
    length += streetLength;
    pos++;
  }

  // Добавляем общую длину маршрута
  ScAddr const & routeLength = m_context.GenerateLink(ScType::ConstNodeLink);
  m_context.SetLinkContent(routeLength, std::to_string(length));

  ScAddr const & arcCommonAddr2 = m_context.GenerateConnector(ScType::ConstCommonArc, routeTuple, routeLength);
  ScAddr const & nrelRouteLength =
      m_context.GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_route_length, arcCommonAddr2);

  result << routeLength << arcCommonAddr2 << nrelRouteLength;

  return result;
}

// Приводит граф к эйлерову виду, добавляя кратчайшие дублирующие пути между нечётными вершинами
void CreateCleaningRouteAgent::UpgradeToEuler(ScAddr const & graphAddr, ScAddrVector & tempElements)
{
  // Если граф уже помечен как эйлеров — ничего не делаем
  ScIterator3Ptr networkIt = m_context.CreateIterator3(
      GraphKeynodes::concept_euler_cycle_graph, ScType::ConstPermPosArc, graphAddr);
  if (networkIt->Next())
  {
    return;
  }

  // Собираем все вершины нечётной степени
  ScAddrVector oddVertices;
  ScIterator3Ptr intersectionIt = m_context.CreateIterator3(graphAddr, ScType::ConstPermPosArc, ScType::Unknown);

  while (intersectionIt->Next())
  {
    ScAddr const vertexAddr = intersectionIt->Get(2);

    ScIterator3Ptr const isIntersectionIt =
        m_context.CreateIterator3(GraphKeynodes::concept_intersection, ScType::ConstPermPosArc, vertexAddr);

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
    return; // Граф уже эйлеров
  }

  // Строим матрицу кратчайших расстояний между нечётными вершинами
  std::vector<std::vector<double>> dists(n, std::vector<double>(n));
  std::map<std::pair<int, int>, std::list<ScAddr>> pathsCache;

  for (int i = 0; i < n; i++)
  {
    for (int j = i + 1; j < n; j++)
    {
      Route result = FindShortestPath(oddVertices[i], oddVertices[j]);

      if (result.edges.empty() && oddVertices[i] != oddVertices[j])
        result.distance = INF;

      dists[i][j] = dists[j][i] = result.distance;
      pathsCache[{i, j}] = result.edges;
      pathsCache[{j, i}] = ScAddrList(result.edges.rbegin(), result.edges.rend());
    }
  }

  // Решаем задачу минимального паросочетания на нечётных вершинах
  int fullMask = (1 << n) - 1;
  std::vector<double> memo(1 << n, -1.0);
  double minWeight = SolveMatching(fullMask, n, dists, memo);
  m_logger.Info("Минимальный вес паросочетания: " + std::to_string(minWeight));

  // Восстанавливаем пары вершин для дублирования путей
  std::vector<std::pair<int, int>> bestPairs;
  GetPairs(fullMask, n, dists, memo, bestPairs);

  // Дублируем улицы по найденным путям (создаём временные элементы)
  for (auto const & pair : bestPairs)
  {
    ScAddrList edgesToDuplicate = pathsCache[pair];

    for (ScAddr const & street : edgesToDuplicate)
    {
      // Находим два конца улицы
      ScIterator5Ptr endsIt = m_context.CreateIterator5(
          street,
          ScType::ConstCommonArc,
          ScType::Unknown,
          ScType::ConstPermPosArc,
          GraphKeynodes::nrel_connects);

      ScAddr intersection1, intersection2;
      bool foundEnds = false;
      if (endsIt->Next()) intersection1 = endsIt->Get(2);
      if (endsIt->Next()) { intersection2 = endsIt->Get(2); foundEnds = true; }
      if (!foundEnds) continue;

      // Создаём копию улицы
      ScAddr tempStreet = m_context.GenerateNode(ScType::ConstNode);
      TempToMapping[tempStreet] = street;

      // Присоединяем к тем же перекрёсткам
      ScAddr arcCommonAddr1 = m_context.GenerateConnector(ScType::ConstCommonArc, tempStreet, intersection1);
      ScAddr arcCommonAddr2 = m_context.GenerateConnector(ScType::ConstCommonArc, tempStreet, intersection2);
      ScAddr rel1 = m_context.GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arcCommonAddr1);
      ScAddr rel2 = m_context.GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arcCommonAddr2);

      tempElements.push_back(tempStreet);
      tempElements.push_back(arcCommonAddr1);
      tempElements.push_back(arcCommonAddr2);
      tempElements.push_back(rel1);
      tempElements.push_back(rel2);
    }
  }
}

// Находит кратчайший путь между двумя перекрёстками (алгоритм Дейкстры)
Route CreateCleaningRouteAgent::FindShortestPath(ScAddr const & start, ScAddr const & end)
{
  if (start == end)
  {
    return {0.0, {}};
  }

  struct PQCompare
  {
    bool operator()(PQPair const & a, PQPair const & b) const
    {
      return a.first > b.first;
    }
  };
  std::priority_queue<PQPair, std::vector<PQPair>, PQCompare> pq;

  ScAddrToValueUnorderedMap<double> dist;
  ScAddrToValueUnorderedMap<ScAddr> parentNode;
  ScAddrToValueUnorderedMap<ScAddr> parentEdge;
  ScAddrToValueUnorderedMap<bool> visited;

  dist[start] = 0.0;
  pq.push({0.0, start});

  while (!pq.empty())
  {
    auto [length, intersection] = pq.top();
    pq.pop();

    if (visited[intersection])
      continue;
    visited[intersection] = true;

    if (intersection == end)
    {
      break;
    }

    // Обходим все соседние улицы
    ScIterator5Ptr streetIt = m_context.CreateIterator5(
        ScType::Unknown,
        ScType::ConstCommonArc,
        intersection,
        ScType::ConstPermPosArc,
        GraphKeynodes::nrel_connects);
    while (streetIt->Next())
    {
      ScAddr street = streetIt->Get(0);
      double weight = GetStreetLength(street);

      // Находим соседний перекрёсток
      ScIterator5Ptr neighbourIt = m_context.CreateIterator5(
          street,
          ScType::ConstCommonArc,
          ScType::Unknown,
          ScType::ConstPermPosArc,
          GraphKeynodes::nrel_connects);

      while (neighbourIt->Next())
      {
        ScAddr neighbour = neighbourIt->Get(2);
        if (neighbour == intersection)
          continue;

        double newDist = length + weight;

        if (dist.find(neighbour) == dist.end() || newDist < dist[neighbour])
        {
          dist[neighbour] = newDist;
          parentNode[neighbour] = intersection;
          parentEdge[neighbour] = street;
          pq.push({newDist, neighbour});
        }
      }
    }
  }
  if (dist.find(end) == dist.end())
  {
    return {INF, {}};
  }

  // Восстанавливаем путь
  ScAddrList path;
  ScAddr curr = end;
  while (curr != start)
  {
    if (parentEdge.find(curr) == parentEdge.end())
    {
      return {INF, {}};
    }
    path.push_front(parentEdge[curr]);
    curr = parentNode[curr];
  }

  return {dist[end], path};
}

// Получает длину улицы из SC-памяти
double CreateCleaningRouteAgent::GetStreetLength(ScAddr const & streetAddr)
{
  ScIterator5Ptr it = m_context.CreateIterator5(
      streetAddr,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_street_length);

  if (it->Next())
  {
    ScAddr linkAddr = it->Get(2);
    std::string lengthStr;

    if (m_context.GetLinkContent(linkAddr, lengthStr))
    {
      double length = std::stod(lengthStr);
      return length;
    }
  }

  throw NodeIsNotStreetError("Узел не является улицей: " + m_context.GetElementSystemIdentifier(streetAddr));
}

// Решает задачу минимального паросочетания методом динамического программирования по маскам
double CreateCleaningRouteAgent::SolveMatching(
    int mask,
    int n,
    std::vector<std::vector<double>> const & dists,
    std::vector<double> & memo)
{
  if (mask == 0)
    return 0.0;
  if (memo[mask] >= 0.0)
    return memo[mask];

  double minVal = INF;

  int i = 0;
  while (!((mask >> i) & 1))
    i++;

  int maskWithoutI = mask ^ (1 << i);

  for (int j = i + 1; j < n; j++)
  {
    if ((mask >> j) & 1)
    {
      double res = SolveMatching(maskWithoutI ^ (1 << j), n, dists, memo);
      if (dists[i][j] + res < minVal)
        minVal = dists[i][j] + res;
    }
  }
  return memo[mask] = minVal;
}

// Восстанавливает конкретные пары вершин из решения DP
void CreateCleaningRouteAgent::GetPairs(
    int mask,
    int n,
    std::vector<std::vector<double>> const & dists,
    std::vector<double> const & memo,
    std::vector<std::pair<int, int>> & resultPairs)
{
  if (!mask)
    return;

  int i = 0;
  while (!((mask >> i) & 1))
    i++;

  double minVal = memo[mask];
  int maskWithoutI = mask ^ (1 << i);

  for (int j = i + 1; j < n; j++)
  {
    if ((mask >> j) & 1)
    {
      double subRes = memo[maskWithoutI ^ (1 << j)];
      if (subRes >= 0.0 && std::abs(dists[i][j] + subRes - minVal) < EPS)
      {
        resultPairs.push_back({i, j});
        GetPairs(maskWithoutI ^ (1 << j), n, dists, memo, resultPairs);
        return;
      }
    }
  }
}
