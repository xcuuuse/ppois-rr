#include <sc-memory/test/sc_test.hpp>
#include <sc-memory/sc_memory.hpp>

#include "../agents/create_cleaning_route_agent.hpp"
#include "../keynodes/graph_keynodes.hpp"

using CreateCleaningRouteAgent = ScMemoryTest;

TEST_F(CreateCleaningRouteAgent, FindEulerianCycleTest)
{
  m_ctx->SubscribeAgent<CreateCleaningRouteAgent>();

  ScAction action = m_ctx->GenerateAction(GraphKeynodes::action_find_optimal_route);

  ScAddr const networkAddr = m_ctx->GenerateNode(ScType::ConstNode);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street_graph, networkAddr);

  m_ctx->GenerateConnector(
      ScType::ConstPermPosArc, GraphKeynodes::concept_euler_cycle_graph, networkAddr);

  ScAddr const intersection1 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection2 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const intersection3 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_intersection, intersection3);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, networkAddr, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, networkAddr, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, networkAddr, intersection3);

  for (ScAddr const & intersection : {intersection1, intersection2, intersection3})
  {
    ScAddr const degreeLink = m_ctx->GenerateLink(ScType::ConstNodeLink);
    m_ctx->SetLinkContent(degreeLink, "2");

    ScAddr const arcCommon = m_ctx->GenerateConnector(ScType::ConstCommonArc, intersection, degreeLink);
    m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_degree, arcCommon);
  }

  ScAddr const arcStart = m_ctx->GenerateConnector(ScType::ConstPermPosArc, networkAddr, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::rrel_start_point, arcStart);

  ScAddr const street12 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street23 = m_ctx->GenerateNode(ScType::ConstNode);
  ScAddr const street31 = m_ctx->GenerateNode(ScType::ConstNode);

  m_ctx->SetElementSystemIdentifier("street_ab", street12);
  m_ctx->SetElementSystemIdentifier("street_bc", street23);
  m_ctx->SetElementSystemIdentifier("street_ca", street31);

  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street12);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street23);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::concept_street, street31);

  ScAddr const length1 = m_ctx->GenerateLink(ScType::ConstNodeLink);
  m_ctx->SetLinkContent(length1, "100.0");
  ScAddr const arcLen1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, length1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_street_length, arcLen1);

  ScAddr const length2 = m_ctx->GenerateLink(ScType::ConstNodeLink);
  m_ctx->SetLinkContent(length2, "150.0");
  ScAddr const arcLen2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, length2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_street_length, arcLen2);

  ScAddr const length3 = m_ctx->GenerateLink(ScType::ConstNodeLink);
  m_ctx->SetLinkContent(length3, "200.0");
  ScAddr const arcLen3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, length3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_street_length, arcLen3);

  ScAddr const arc12_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection1);
  ScAddr const arc12_2 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street12, intersection2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc12_2);

  ScAddr const arc23_2 =

m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection2);
  ScAddr const arc23_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street23, intersection3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_2);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc23_3);

  ScAddr const arc31_3 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection3);
  ScAddr const arc31_1 = m_ctx->GenerateConnector(ScType::ConstCommonArc, street31, intersection1);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_3);
  m_ctx->GenerateConnector(ScType::ConstPermPosArc, GraphKeynodes::nrel_connects, arc31_1);

  action.SetArguments(networkAddr);

  EXPECT_TRUE(action.InitiateAndWait());
  EXPECT_TRUE(action.IsFinishedSuccessfully());

  ScStructure const resultStructure = action.GetResult();
  EXPECT_FALSE(resultStructure.IsEmpty());

  ScAddr routeTuple;
  ScIterator5Ptr tupleIt = m_ctx->CreateIterator5(
      networkAddr,
      ScType::ConstCommonArc,
      ScType::ConstNodeTuple,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_optimal_route);

  EXPECT_TRUE(tupleIt->Next());
  routeTuple = tupleIt->Get(2);
  EXPECT_TRUE(routeTuple.IsValid());

  ScAddrUnorderedSet routeStreets;
  ScIterator3Ptr routeIt = m_ctx->CreateIterator3(routeTuple, ScType::ConstPermPosArc, ScType::Unknown);

  while (routeIt->Next())
  {
    ScAddr element = routeIt->Get(2);

    ScIterator3Ptr streetCheckIt =
        m_ctx->CreateIterator3(GraphKeynodes::concept_street, ScType::ConstPermPosArc, element);

    if (streetCheckIt->Next())
    {
      routeStreets.insert(element);
    }
  }

  EXPECT_EQ(routeStreets.size(), 3);
  EXPECT_TRUE(routeStreets.count(street12) > 0);
  EXPECT_TRUE(routeStreets.count(street23) > 0);
  EXPECT_TRUE(routeStreets.count(street31) > 0);

  ScAddr routeLengthLink;
  ScIterator5Ptr lengthIt = m_ctx->CreateIterator5(
      routeTuple,
      ScType::ConstCommonArc,
      ScType::ConstNodeLink,
      ScType::ConstPermPosArc,
      GraphKeynodes::nrel_route_length);

  EXPECT_TRUE(lengthIt->Next());
  routeLengthLink = lengthIt->Get(2);
  EXPECT_TRUE(routeLengthLink.IsValid());

  std::string lengthStr;
  m_ctx->GetLinkContent(routeLengthLink, lengthStr);
  double totalLength = std::stod(lengthStr);

  double expectedLength = 450.0;
  EXPECT_DOUBLE_EQ(totalLength, expectedLength);

  int positionCount = 0;
  ScIterator3Ptr posIt = m_ctx->CreateIterator3(routeTuple, ScType::ConstPermPosArc, ScType::Unknown);

  while (posIt->Next())
  {
    ScAddr arc = posIt->Get(1);

    ScIterator3Ptr rrelIt = m_ctx->CreateIterator3(ScType::Unknown, ScType::ConstPermPosArc, arc);

    bool hasPosition = false;
    while (rrelIt->Next())
    {
      ScAddr rrel = rrelIt->Get(0);

      ScIterator3Ptr roleIt = m_ctx->CreateIterator3(rrel, ScType::ConstCommonArc, ScType::ConstNodeLink);

      if (roleIt->Next())
      {
        hasPosition = true;
        break;
      }
    }

    if (hasPosition)
    {
      positionCount++;
    }
  }

  EXPECT_EQ(positionCount, 3);

  m_ctx->UnsubscribeAgent<CreateCleaningRouteAgent>();
}