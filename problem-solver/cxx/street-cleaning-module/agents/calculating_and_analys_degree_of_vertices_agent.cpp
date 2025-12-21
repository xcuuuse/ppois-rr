#include "calculating_and_analys_degree_of_vertices_agent.hpp"

#include <sc-memory/sc_memory.hpp>
#include <sc-memory/utils/sc_logger.hpp>

#include "keynodes/graph_keynodes.hpp"

#include <exception>
#include <string>

// Конструктор агента
CalculatingAndAnalysDegreeOfVerticesAgent::CalculatingAndAnalysDegreeOfVerticesAgent()
{
  m_logger = utils::ScLogger(
      utils::ScLogger::ScLogType::File, 
      "logs/calculating_and_analys_degree_of_vertices_agent.log", 
      utils::ScLogLevel::Debug, 
      true);
}

// Метод возвращает идентификатор действия, которое обрабатывает этот агент
ScAddr CalculatingAndAnalysDegreeOfVerticesAgent::GetActionClass() const
{
  return GraphKeynodes::action_calculating_and_analys_degree_of_vertices;
}

// ОСНОВНОЙ МЕТОД: Выполняется при запуске агента
ScResult CalculatingAndAnalysDegreeOfVerticesAgent::DoProgram(ScAction & action)
{
  // ШАГ 1: Получаем входной параметр - адрес дорожной сети
  auto const & [graphAddr] = action.GetArguments<1>();

  // ШАГ 2: Проверяем существование дорожной сети
  if (!m_context.IsElement(graphAddr))
  {
    m_logger.Error("Дорожная сеть не найдена");
    return action.FinishWithError();
  }

  // ШАГ 3: Логируем начало анализа
  m_logger.Info("Начало анализа сети: " + m_context.GetElementSystemIdentifier(graphAddr));
  
  // ШАГ 4: Рассчитываем степени всех вершин
  ScAddrToValueUnorderedMap<int> vertices = CalculateVertexesDegrees(graphAddr);

  // ШАГ 5: Определяем, есть ли Эйлеров цикл в графе
  Euler graphStatus = GetGraphEulerianStatus(graphAddr, vertices);

  // ШАГ 6: Создаем структуру с результатами анализа
  ScStructure analysisResult = CreateAnalysisResult(graphAddr, vertices, graphStatus);
  action.SetResult(analysisResult);

  // ШАГ 7: Запускаем агент для поиска маршрута
  ScAction routeAction = m_context.GenerateAction(GraphKeynodes::action_find_optimal_route);
  routeAction.SetArguments(graphAddr);
  routeAction.Initiate();

  // ШАГ 8: Логируем успешное завершение
  m_logger.Info("Анализ завершен успешно");

  return action.FinishSuccessfully();
}


// МЕТОД: Расчет степеней всех вершин (перекрестков и площадей) в графе
ScAddrToValueUnorderedMap<int> CalculatingAndAnalysDegreeOfVerticesAgent::CalculateVertexesDegrees(ScAddr const & graphAddr)
{
  // Создаем словарь для хранения результатов: вершина -> степень
  ScAddrToValueUnorderedMap<int> vertices;

  // Ищем все элементы, связанные с дорожной сетью
  // Итератор ищет все дуги типа ConstPermPosArc от graphAddr к любым элементам
  ScIterator3Ptr const networkIt3 = m_context.CreateIterator3(graphAddr, ScType::ConstPermPosArc, ScType::Unknown);

  while (networkIt3->Next()) 
  {
    // Получаем адрес элемента
    ScAddr const vertexAddr = networkIt3->Get(2);

    // Проверяем, является ли этот элемент перекрестком
    ScIterator3Ptr const isIntersectionIt =
        m_context.CreateIterator3(GraphKeynodes::concept_intersection, ScType::ConstPermPosArc, vertexAddr);
    
    // Проверяем, является ли этот элемент площадью
    ScIterator3Ptr const isSquareIt =
        m_context.CreateIterator3(GraphKeynodes::concept_square, ScType::ConstPermPosArc, vertexAddr);
    
    // Если элемент является перекрестком ИЛИ площадью
    bool isIntersection = isIntersectionIt->Next();
    bool isSquare = isSquareIt->Next();
    
    if (isIntersection || isSquare)
    {
      int degree = 0;  // Счетчик степени вершины
      
      // Считаем количество улиц, связанных с этой вершиной
      // Ищем все дуги, которые через отношение nrel_connects связывают что-то с вершиной
      ScIterator5Ptr const vertexIt5 = m_context.CreateIterator5(
          ScType::Unknown,           // Что связано с вершиной (улица)
          ScType::ConstCommonArc,    // Дуга к вершине
          vertexAddr,                // Вершина (перекресток/площадь)
          ScType::ConstPermPosArc,   // Дуга к отношению
          GraphKeynodes::nrel_connects); // Отношение "соединен с"

      while (vertexIt5->Next())  // Пока находим связанных улиц
      {
        degree++;  // Увеличиваем счетчик на 1
      }

      // Сохраняем результат в словаре: вершина -> ее степень
      vertices[vertexAddr] = degree;
      
      // Логируем тип вершины и ее степень
      if (isIntersection)
        m_logger.Debug("Найден перекресток: " + m_context.GetElementSystemIdentifier(vertexAddr) + 
                       ", степень: " + std::to_string(degree));
      if (isSquare)
        m_logger.Debug("Найдена площадь: " + m_context.GetElementSystemIdentifier(vertexAddr) + 
                       ", степень: " + std::to_string(degree));
    }
  }

  // Логируем общее количество найденных вершин
  m_logger.Info("Всего вершин найдено: " + std::to_string(vertices.size()));
  
  // Возвращаем словарь со всеми вершинами и их степенями
  return vertices;
}


// МЕТОД: Определение типа графа (есть ли Эйлеров цикл)
Euler CalculatingAndAnalysDegreeOfVerticesAgent::GetGraphEulerianStatus(
    ScAddr const & graphAddr,
    ScAddrToValueUnorderedMap<int> vertices)  // Словарь вершин и их степеней
{
  // ПЕРЕМЕННЫЕ ДЛЯ АНАЛИЗА:
  int oddDegreeCount = 0;      // Количество вершин с нечетной степенью
  int verticesCount = 0;  // Общее количество вершин
  ScAddr startBfsNode = ScAddr::Empty;  // Начальная точка для обхода графа

  // ЦИКЛ 1: Анализируем все вершины
  for (auto const & [vertex, degree] : vertices)
  {
    // Проверяем четность степени
    if (degree % 2 != 0)
      oddDegreeCount++;  // Увеличиваем счетчик нечетных вершин

    verticesCount++;  // Увеличиваем общий счетчик
    
    // Выбираем первую ненулевую вершину как начальную для обхода BFS
    if (degree > 0 && !startBfsNode.IsValid())
      startBfsNode = vertex;
  }

  // Логируем количество нечетных вершин
  m_logger.Info("Нечетных вершин: " + std::to_string(oddDegreeCount));

  // Подготовка к обходу графа в ширину (BFS)
  ScAddrSet visitedVertices;  // Множество посещенных вершин
  ScAddrQueue queue;               // Очередь для BFS

  // Если есть хотя бы одна вершина с ненулевой степенью
  if (startBfsNode.IsValid())
  {
    queue.push(startBfsNode);                 // Добавляем в очередь
    visitedVertices.insert(startBfsNode); // Отмечаем как посещенный
  }

  // ЦИКЛ 2: ОБХОД ГРАФА В ШИРИНУ (BFS)
  while (!queue.empty())  // Пока очередь не пуста
  {
    // Берем первый элемент из очереди
    ScAddr const currentIntersection = queue.front();
    queue.pop();  // Удаляем из очереди

    // ЦИКЛ 2.1: Ищем все улицы, подключенные к текущему перекрестку/площади
    ScIterator5Ptr const streetIt = m_context.CreateIterator5(
        ScType::Unknown,           // Улица
        ScType::ConstCommonArc,    // Дуга от улицы к перекрестку/площади
        currentIntersection,       // Текущий перекресток/площадь
        ScType::ConstPermPosArc,   // Дуга к отношению
        GraphKeynodes::nrel_connects); // Отношение "соединен с"

    while (streetIt->Next())  // Для каждой найденной улицы
    {
      ScAddr const streetNode = streetIt->Get(0);  // Получаем адрес улицы

      // ЦИКЛ 2.2: Ищем другой конец этой улицы 
      ScIterator5Ptr const neighborIt = m_context.CreateIterator5(
          streetNode,              // Улица
          ScType::ConstCommonArc,  // Дуга от улицы к перекрёстку/площади
          ScType::Unknown,         // Перекрёсток/площадь
          ScType::ConstPermPosArc, // Дуга к отношению
          GraphKeynodes::nrel_connects); // Отношение "соединен с"

      while (neighborIt->Next())  // Для каждого найденного соседа
      {
        ScAddr const nextIntersection = neighborIt->Get(2);  // Получаем соседний перекрёсток/площадь

        // Условия для добавления соседа в очередь:
        // 1. Это не тот же самый перекресток/площадь
        // 2. Он существует в нашем словаре вершин
        // 3. Он еще не был посещен
        if (nextIntersection != currentIntersection && 
            vertices.find(nextIntersection) != vertices.end() &&
            visitedVertices.find(nextIntersection) == visitedVertices.end())
        {
          visitedVertices.insert(nextIntersection);  // Отмечаем как посещенный
          queue.push(nextIntersection);                   // Добавляем в очередь для дальнейшего обхода
        }
      }
    }
  }

  // АНАЛИЗ РЕЗУЛЬТАТОВ:
  // Условие Эйлерова цикла:
  // 1. Нет вершин с нечетной степенью (oddDegreeCount == 0)
  // 2. Все вершины связаны (visitedVertices.size() == verticesCount)
  if (oddDegreeCount == 0 && visitedVertices.size() == verticesCount)
  {
    return Euler::Cycle;  // Граф имеет Эйлеров цикл
  }
  else
  {
    return Euler::None;   // Эйлерова цикла нет
  }
}

// МЕТОД: Создание структуры с результатами анализа
ScStructure CalculatingAndAnalysDegreeOfVerticesAgent::CreateAnalysisResult(
    ScAddr const & graphAddr,             // Адрес дорожной сети
    ScAddrToValueUnorderedMap<int> vertices, // Словарь вершин и их степеней
    Euler status)                  // Статус графа
{
  // Создаем новую структуру в памяти для хранения результатов
  ScStructure resultStructure = m_context.GenerateStructure();

  // ШАГ 1: Определяем тип графа и создаем соответствующую связь
  if (status == Euler::Cycle)
  {
    // Создаем связь: сеть -> является Эйлеровым циклом
    ScAddr const graphType = m_context.GenerateConnector(
        ScType::ConstPermPosArc,           // Тип дуги
        GraphKeynodes::concept_euler_cycle, // Класс "Эйлеров цикл"
        graphAddr);                      // Дорожная сеть
    resultStructure << graphType;          // Добавляем в результаты
  }
  else
  {
    // Создаем связь: сеть -> НЕ является Эйлеровым циклом
    ScAddr const graphType = m_context.GenerateConnector(
        ScType::ConstPermPosArc,              // Тип дуги
        GraphKeynodes::concept_no_euler_cycle, // Класс "не Эйлеров цикл"
        graphAddr);                         // Дорожная сеть
    resultStructure << graphType;             // Добавляем в результаты
  }

  // ШАГ 2: Добавляем саму дорожную сеть в результаты
  resultStructure << graphAddr;

  // ЦИКЛ: Обрабатываем все вершины и их степени
  for (auto const & [vertexAddr, degree] : vertices)
  {
    // ШАГ 3.1: Создаем ссылку для хранения числового значения степени
    ScAddr const & vertexDegreeAddr = m_context.GenerateLink(ScType::ConstNodeLink);
    
    // Записываем значение степени в ссылку (например, "3" для Т-образного перекрестка)
    m_context.SetLinkContent(vertexDegreeAddr, std::to_string(degree));

    // ШАГ 3.2: Создаем связь: вершина -> её степень
    ScAddr const & arcCommonAddr = m_context.GenerateConnector(
        ScType::ConstCommonArc,  // Тип дуги
        vertexAddr,              // Вершина
        vertexDegreeAddr);       // Ссылка со значением степени

    // ШАГ 3.3: Создаем отношение "степень" для этой связи
    ScAddr const & nrelVertexDegreeAddr =
        m_context.GenerateConnector(
            ScType::ConstPermPosArc,          // Тип дуги
            GraphKeynodes::nrel_degree,       // Отношение "степень"
            arcCommonAddr);                   // Связь между верной и его степенью

    // ШАГ 3.4: Находим связь между дорожной сетью и текущим перекрестком
    ScIterator3Ptr edgeIt = m_context.CreateIterator3(
        graphAddr,            // Дорожная сеть
        ScType::ConstPermPosArc, // Тип дуги
        vertexAddr);            // Вершина
    
    // Если такая связь существует, добавляем ее в результаты
    if (edgeIt->Next())
      resultStructure << edgeIt->Get(1) << edgeIt->Get(2);  // Дуга и вершина

    // ШАГ 3.5: Добавляем все созданные элементы в структуру результатов
    resultStructure << vertexDegreeAddr     // Ссылка со значением степени
                    << arcCommonAddr        // Связь вершина->степень
                    << nrelVertexDegreeAddr; // Отношение "степень"
  }

  // Возвращаем готовую структуру с результатами анализа
  return resultStructure;
}
