#include <sc-memory/sc_keynodes.hpp>

class GraphKeynodes: public ScKeynodes{
    static inline ScKeynode const action_find_optimal_route{
    "action_find_optimal_route", ScType::ConstNodeClass};
    static inline ScKeynode const nrel_cleaning_route{
    "nrel_cleaning_route", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_square{"nrel_square", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_street_length{"nrel_street_length", ScType::ConstNodeNonRole};
    static inline ScKeynode const nrel_street{"nrel_street", ScType::ConstNodeNonRole};
    static inline ScKeynode const concept_cleaning_route{"concept_cleaning_route", ScType::ConstNodeClass};
    static inline ScKeynode const concept_district{"concept_district", ScType::ConstNodeClass};
    static inline ScKeynode const concept_square{"concept_square", ScType::ConstNodeClass};
    static inline ScKeynode const concept_street{"concept_street", ScType::ConstNodeClass};
};