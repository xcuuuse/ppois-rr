#include <sc-memory/test/sc_test.hpp>
#include <sc-memory/sc_memory.hpp>

#include "../agents/CalculatingAndAnalysDegreeOfVerticesAgent.hpp"
#include "../keynodes/GraphKeynodes.hpp"

using CalculatingAndAnalysDegreeOfVerticesAgentTest = ScMemoryTest;

TEST_F(CalculatingAndAnalysDegreeOfVerticesAgentTest, EulerianGraphWithCycle)
{
  m_ctx->SubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_calculating_degree_of_vertices);

  // Создаем дорожную сеть
  ScAddr const graphAddr = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street_graph, graphAddr);

  // Создаем перекрестки
  ScAddr const intersection1 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection2 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection3 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection3);

  // Связываем перекрестки с сетью
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection3);

  // Создаем улицы (ребра графа)
  ScAddr const street12 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street23 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street31 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street12);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street23);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street31);

  // Создаем связи между улицами и перекрестками
  ScAddr const arc12_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection1);
  ScAddr const arc12_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_2);

  ScAddr const arc23_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection2);
  ScAddr const arc23_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_3);

  ScAddr const arc31_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection3);
  ScAddr const arc31_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_1);

  action.SetArguments(graphAddr);

  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedSuccessfully());

  ScStructure const resultStructure = action.GetResult();
  EXPECT_FALSE(resultStructure.IsEmpty());

  // Проверяем, что сеть помечена как имеющая Эйлеров цикл
  ScIterator3Ptr eulerianIt = m_ctx->CreateIterator3(
      GraphKeynodes::concept_euler_cycle, ScType::ConstPermPosArc, graphAddr);
  EXPECT_TRUE(eulerianIt->Next());

  // Проверяем степени вершин (должны быть все степени 2)
  ScIterator5Ptr degreeIt1 = m_ctx->CreateIterator5(
      intersection1,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt1->Next());

  ScAddr link1Addr = degreeIt1->Get(2);
  std::string degree1Str;
  m_ctx->GetLinkContent(link1Addr, degree1Str);
  int degree1 = std::stoi(degree1Str);
  EXPECT_EQ(degree1, 2);

  ScIterator5Ptr degreeIt2 = m_ctx->CreateIterator5(
      intersection2,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt2->Next());

  ScAddr link2Addr = degreeIt2->Get(2);
  std::string degree2Str;
  m_ctx->GetLinkContent(link2Addr, degree2Str);
  int degree2 = std::stoi(degree2Str);
  EXPECT_EQ(degree2, 2);

  ScIterator5Ptr degreeIt3 = m_ctx->CreateIterator5(
      intersection3,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt3->Next());

  ScAddr link3Addr = degreeIt3->Get(2);
  std::string degree3Str;
  m_ctx->GetLinkContent(link3Addr, degree3Str);
  int degree3 = std::stoi(degree3Str);
  EXPECT_EQ(degree3, 2);

  m_ctx->UnsubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();
}

TEST_F(CalculatingAndAnalysDegreeOfVerticesAgentTest, NonEulerianGraph)
{
  m_ctx->SubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_calculating_degree_of_vertices);

  ScAddr const graphAddr = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street_graph, graphAddr);

  // Создаем 4 перекрестка (цепочка)
  ScAddr const intersection1 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection2 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection3 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection4 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection4);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection4);

  // Создаем только 3 улицы (создаем цепочку)
  ScAddr const street12 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street23 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street34 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street12);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street23);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street34);

  ScAddr const arc12_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection1);
  ScAddr const arc12_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_2);

  ScAddr const arc23_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection2);
  ScAddr const arc23_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_3);

  ScAddr const arc34_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street34, intersection3);
  ScAddr const arc34_4 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street34, intersection4);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc34_3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc34_4);

  action.SetArguments(graphAddr);

  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedSuccessfully());

  ScStructure const resultStructure = action.GetResult();
  EXPECT_FALSE(resultStructure.IsEmpty());

  // Проверяем, что сеть НЕ помечена как эйлерова
  ScIterator3Ptr eulerianIt = m_ctx->CreateIterator3(
      GraphKeynodes::concept_euler_cycle, ScType::ConstPermPosArc, graphAddr);
  EXPECT_FALSE(eulerianIt->Next());

  // Проверяем, что сеть помечена как не эйлерова
  ScIterator3Ptr nonEulerianIt = m_ctx->CreateIterator3(
      GraphKeynodes::concept_no_euler_cycle, ScType::ConstPermPosArc, graphAddr);
  EXPECT_TRUE(nonEulerianIt->Next());

  // Проверяем степени вершин (1, 2, 2, 1)
  ScIterator5Ptr degreeIt1 = m_ctx->CreateIterator5(
      intersection1,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt1->Next());

  std::string degreeStr1;
  m_ctx->GetLinkContent(degreeIt1->Get(2), degreeStr1);
  EXPECT_EQ(std::stoi(degreeStr1), 1);

  ScIterator5Ptr degreeIt2 = m_ctx->CreateIterator5(
      intersection2,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt2->Next());

  std::string degreeStr2;
  m_ctx->GetLinkContent(degreeIt2->Get(2), degreeStr2);
  EXPECT_EQ(std::stoi(degreeStr2), 2);

  ScIterator5Ptr degreeIt3 = m_ctx->CreateIterator5(
      intersection3,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt3->Next());

  std::string degreeStr3;
  m_ctx->GetLinkContent(degreeIt3->Get(2), degreeStr3);
  EXPECT_EQ(std::stoi(degreeStr3), 2);

  ScIterator5Ptr degreeIt4 = m_ctx->CreateIterator5(
      intersection4,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIt4->Next());

  std::string degreeStr4;
  m_ctx->GetLinkContent(degreeIt4->Get(2), degreeStr4);
  EXPECT_EQ(std::stoi(degreeStr4), 1);

  m_ctx->UnsubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();
}

TEST_F(CalculatingAndAnalysDegreeOfVerticesAgentTest, GraphWithSquare)
{
  m_ctx->SubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_calculating_degree_of_vertices);

  // Создаем сеть с площадью
  ScAddr const graphAddr = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street_graph, graphAddr);

  // Создаем перекрестки и площадь
  ScAddr const intersection1 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const square = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection3 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_square, square);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection3);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, square);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection3);

  // Создаем улицы
  ScAddr const street1s = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const streets3 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street31 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street1s);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, streets3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street31);

  ScAddr const arc1s_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street1s, intersection1);
  ScAddr const arc1s_s = m_ctx->GenerateConnector(ScType::ConstCommonArc, street1s, square);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc1s_1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc1s_s);

  ScAddr const arcs3_s = m_ctx->GenerateConnector(ScType::ConstCommonArc, streets3, square);
  ScAddr const arcs3_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, streets3, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arcs3_s);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arcs3_3);

  ScAddr const arc31_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection3);
  ScAddr const arc31_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_1);

  action.SetArguments(graphAddr);

  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedSuccessfully());

  // Проверяем, что у площади тоже вычислена степень
  ScIterator5Ptr degreeSquareIt = m_ctx->CreateIterator5(
      square,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeSquareIt->Next());

  std::string degreeSquareStr;
  m_ctx->GetLinkContent(degreeSquareIt->Get(2), degreeSquareStr);
  EXPECT_EQ(std::stoi(degreeSquareStr), 2);

  m_ctx->UnsubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();
}

TEST_F(CalculatingAndAnalysDegreeOfVerticesAgentTest, EmptyNetwork)
{
  m_ctx->SubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_calculating_degree_of_vertices);

  // Создаем пустую сеть (без перекрестков и улиц)
  ScAddr const emptyGraphAddr = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street_graph, emptyGraphAddr);

  action.SetArguments(emptyGraphAddr);

  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedSuccessfully());

  ScStructure const resultStructure = action.GetResult();
  EXPECT_FALSE(resultStructure.IsEmpty());

  // Пустая сеть считается эйлеровой (нет нечетных вершин и связна)
  ScIterator3Ptr eulerianIt = m_ctx->CreateIterator3(
      GraphKeynodes::concept_euler_cycle, ScType::ConstPermPosArc, emptyGraphAddr);
  EXPECT_TRUE(eulerianIt->Next());

  m_ctx->UnsubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();
}

TEST_F(CalculatingAndAnalysDegreeOfVerticesAgentTest, DisconnectedGraph)
{
  m_ctx->SubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_calculating_degree_of_vertices);

  ScAddr const graphAddr = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street_graph, graphAddr);

  // Создаем связный треугольник
  ScAddr const intersection1 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection2 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection3 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection3);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, intersection3);

  ScAddr const street12 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street23 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street31 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street12);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street23);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street31);

  ScAddr arc12_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection1);
  ScAddr arc12_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_2);

  ScAddr arc23_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection2);
  ScAddr arc23_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_3);

  ScAddr arc31_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection3);
  ScAddr arc31_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_1);

  // Добавляем изолированную вершину
  ScAddr const isolated = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, isolated);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, graphAddr, isolated);

  action.SetArguments(graphAddr);

  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedSuccessfully());

  // Граф несвязный, поэтому не эйлеров
  ScIterator3Ptr eulerianIt = m_ctx->CreateIterator3(
      GraphKeynodes::concept_euler_cycle, ScType::ConstPermPosArc, graphAddr);
  EXPECT_FALSE(eulerianIt->Next());

  // Проверяем степень изолированной вершины (должна быть 0)
  ScIterator5Ptr degreeIsolatedIt = m_ctx->CreateIterator5(
      isolated,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_degree);
  EXPECT_TRUE(degreeIsolatedIt->Next());

  std::string degreeIsolatedStr;
  m_ctx->GetLinkContent(degreeIsolatedIt->Get(2), degreeIsolatedStr);
  EXPECT_EQ(std::stoi(degreeIsolatedStr), 0);

  m_ctx->UnsubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();
}

TEST_F(CalculatingAndAnalysDegreeOfVerticesAgentTest, InvalidArgument)
{
  m_ctx->SubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_calculating_degree_of_vertices);

  // Не устанавливаем аргументы и запускаем
  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedWithError());

  m_ctx->UnsubscribeAgent<CalculatingAndAnalysDegreeOfVerticesAgent>();
}
