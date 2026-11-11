#pragma once
#include "p03/local_sim.hpp"
namespace p03 {
enum class LocalQueryStatus {Complete,Incomplete,Invalid};
struct LocalQueryReview {LocalQueryStatus status{LocalQueryStatus::Invalid};Error error{Error::InvalidInput};};
// Checks one detached full LocalSim snapshot against its explicit model inputs.
// Does not apply trades, overwrite positions, clear stops or authorize recovery.
[[nodiscard]] LocalQueryReview review_local_query(const SimQuerySnapshot&,const SimConfig&);
[[nodiscard]] LocalQueryReview review_local_query(const SimQueryResult&,QueryId expected_query,const SimConfig&);
[[nodiscard]] LocalQueryReview review_local_query_for_core(const SimQueryResult&,QueryId,const SimConfig&,const CoreLoop&);
} // namespace p03
