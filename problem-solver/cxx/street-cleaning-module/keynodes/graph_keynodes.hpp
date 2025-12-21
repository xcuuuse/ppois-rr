#include <sc-memory/sc_keynodes.hpp>

class GraphKeynodes: public ScKeynodes{
public:
    //TODO: написать формализацию действия подсчёта степени вершин (done)
    static inline ScKeynode const action_calculating_and_analys_degree_of_vertices{
            "action_calculating_and_analys_degree_of_vertices", ScType::ConstNodeClass};
    
    static inline ScKeynode const action_find_optimal_route{
    "action_find_optimal_route", ScType::ConstNodeClass};
    static inline ScKeynode const action_graph_analysing{
    "action_graph_analysing", ScType::ConstNodeClass};
    static inline ScKeynode const action_graph_calculate_degree_of_vertices{
    "action_calculate_degree_of_vertices", ScType::ConstNodeClass};
    //TODO: написать формализацию отношениям "иметь степень", "иметь/не иметь эйлеров цикл (done)"
    
    static inline ScKeynode const nrel_degree{"nrel_degree", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_has_euler_cycle{"nrel_has_euler_cycle", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_has_no_euler_cycle{"nrel_has_no_euler_cycle", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_cleaning_route{"nrel_cleaning_route", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_square{"nrel_square", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_street_length{"nrel_street_length", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_street{"nrel_street", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_optimal_route{"nrel_optimal_route", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_connects{"nrel_connects", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_route_length{"nrel_route_length", ScType::ConstNodeNonRole};
    static inline ScKeynode const rrel_start_point{"rrel_start_point", ScType::ConstNodeNonRole};
    static inline ScKeynode const rrel_end_point{"rrel_end_point", ScType::ConstNodeNonRole};
    //TODO: формализация классов "содержит/не содержит эйлеров цикл (done)"
    static inline ScKeynode const concept_cleaning_route{"concept_cleaning_route", ScType::ConstNodeClass};
    static inline ScKeynode const concept_district{"concept_district", ScType::ConstNodeClass};
    static inline ScKeynode const concept_euler_cycle{"concept_euler_cycle", ScType::ConstNodeClass};
    static inline ScKeynode const concept_euler_cycle_graph{"concept_euler_cycle_graph", ScType::ConstNodeClass};
    static inline ScKeynode const concept_graph{"concept_graph", ScType::ConstNodeClass};
    static inline ScKeynode const concept_intersection{"concept_intersection", ScType::ConstNodeClass};
    static inline ScKeynode const concept_no_euler_cycle{"concept_no_euler_cycle", ScType::ConstNodeClass};
    static inline ScKeynode const concept_square{"concept_square", ScType::ConstNodeClass};
    static inline ScKeynode const concept_street_graph{"concept_street_graph", ScType::ConstNodeClass};
    static inline ScKeynode const concept_street{"concept_street", ScType::ConstNodeClass};
    static inline ScKeynode const concept_vertex{"concept_vertex", ScType::ConstNodeClass};
};