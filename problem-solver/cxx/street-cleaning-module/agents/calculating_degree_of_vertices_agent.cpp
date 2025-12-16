#include "calculating_degree_of_vertices_agent.hpp"

#include <cmath>

// Подключаем все необходимые заголовочные файлы для работы с
// sc-памятью.
#include <sc-memory/sc_memory.hpp>

#include "keynodes/graph_keynodes.hpp"

ScAddr CalculatingDegreeOfVertices::GetActionClass() const
{
    return GraphKeynodes::action_calculating_degree_of_vertices;
}

ScResult CalculatingDegreeOfVertices::DoProgram(ScAction & action) {
    // Получаем аргумент действия — граф, для которого
    // необходимо посчитать степени вершин.
    auto const &[graphAddr] = action.GetArguments<1>();

    // Если аргумент для действия не задан, то завершаем выполнение
    // действия с ошибкой.
    if (!m_context.IsElement(graphAddr)) {
        m_logger.Error("Graph not specified.");
        return action.FinishWithError();
    }

    m_logger.Debug("Starting calculation degree of vertices");

    // Структура результата
    ScStructure resultStructure = m_context.GenerateStructure();
    bool hasEulerCycle = true;
    // Итератор по вершинам графа
    ScIterator3Ptr itVertices = m_context.CreateIterator3(
            graphAddr,
            ScType::ConstPermPosArc,
            ScType::ConstNode
    );
    while (itVertices->Next()) {
        ScAddr const &vertexAddr = itVertices->Get(2);
        size_t degree = 0;

        // Считаем дуги, где вершина — начало
        ScIterator3Ptr itOut = m_context.CreateIterator3(
                vertexAddr,
                ScType::ConstPermPosArc,
                ScType::Unknown
        );

        while (itOut->Next()) {
            degree++;
        }

        // Считаем дуги, где вершина — конец
        ScIterator3Ptr itIn = m_context.CreateIterator3(
                ScType::Unknown,
                ScType::ConstPermPosArc,
                vertexAddr
        );

        while (itIn->Next()) degree++;
        if (degree % 2 != 0)
            hasEulerCycle = false;
        m_logger.Debug("Vertex degree: ", degree);

        // узел со степенью вершины
        ScAddr degreeLink = m_context.GenerateLink();
        m_context.SetLinkContent(degreeLink, ScStream(std::to_string(degree)));

        // Связываем вершину со степенью
        ScAddr arc = m_context.GenerateConnector(
                ScType::ConstCommonArc,
                vertexAddr,
                degreeLink
        );

        m_context.GenerateConnector(
                ScType::ConstPermPosArc,
                GraphKeynodes::nrel_degree,
                arc
        );

        // Добавляем в результат
        resultStructure << vertexAddr
                        << degreeLink
                        << arc
                        << GraphKeynodes::nrel_degree;
    }

    ScAddr eulerResultNode = hasEulerCycle
                             ? GraphKeynodes::concept_euler_cycle
                             : GraphKeynodes::concept_no_euler_cycle;

    m_logger.Info(hasEulerCycle
                  ? "Graph has Euler cycle"
                  : "Graph does NOT have Euler cycle");

    ScAddr eulerArc = m_context.GenerateConnector(
            ScType::ConstCommonArc,
            graphAddr,
            eulerResultNode
    );

    ScAddr eulerRelArc = m_context.GenerateConnector(
            ScType::ConstPermPosArc,
            GraphKeynodes::nrel_has_euler_cycle,
            eulerArc
    );

    resultStructure << graphAddr
                    << eulerResultNode
                    << eulerArc
                    << eulerRelArc
                    << GraphKeynodes::nrel_has_euler_cycle;

    action.SetResult(resultStructure);
    return action.FinishSuccessfully();
}
