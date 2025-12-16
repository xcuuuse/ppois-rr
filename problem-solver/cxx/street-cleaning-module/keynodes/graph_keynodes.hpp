#include <sc-memory/sc_keynodes.hpp>

class GraphKeynodes: public ScKeynodes{
    //TODO: написать формализацию действия подсчёта степени вершин
    static inline ScKeynode const action_calculating_degree_of_vertices{
            "action_calculating_degree_of_vertices", ScType::ConstNodeClass};
    static inline ScKeynode const action_find_optimal_route{
    "action_find_optimal_route", ScType::ConstNodeClass};
    //TODO: написать формализацию отношениям "иметь степень", "иметь/не иметь эйлеров цикл"
    static inline ScKeynode const nrel_degree{"nrel_degree", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_has_euler_cycle{"nrel_has_euler_cycle", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_cleaning_route{"nrel_cleaning_route", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_square{"nrel_square", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_street_length{"nrel_street_length", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_street{"nrel_street", ScType::ConstNodeNonRole};
    //TODO: формализация классов "содержит/не содержит эйлеров цикл"
    static inline ScKeynode const concept_euler_cycle{"concept_euler_cycle", ScType::ConstNodeClass};
    static inline ScKeynode const concept_no_euler_cycle{"concept_no_euler_cycle", ScType::ConstNodeClass};
    static inline ScKeynode const concept_cleaning_route{"concept_cleaning_route", ScType::ConstNodeClass};
    static inline ScKeynode const concept_district{"concept_district", ScType::ConstNodeClass};
    static inline ScKeynode const concept_square{"concept_square", ScType::ConstNodeClass};
    static inline ScKeynode const concept_street{"concept_street", ScType::ConstNodeClass};
};