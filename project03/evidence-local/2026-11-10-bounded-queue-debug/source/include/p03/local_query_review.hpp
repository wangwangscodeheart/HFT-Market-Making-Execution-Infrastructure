#pragma once
#include "p03/local_sim.hpp"
namespace p03 {
enum class LocalQueryStatus {Complete,Incomplete,Invalid};
struct LocalQueryReview {LocalQueryStatus status{LocalQueryStatus::Invalid};Error error{Error::InvalidInput};};
struct LocalQueryDelivery {LocalQueryReview review;std::vector<BusinessEvent> events;};
// Validated synchronous LocalSimulator facts only; trades precede missing exact
// canceled-order terminals. Never fabricates a CancelReport/action success.
[[nodiscard]] LocalQueryDelivery plan_local_query_delivery(const SimQueryResult&,QueryId,const SimConfig&,const CoreLoop&);
// Checks one detached full LocalSim snapshot against its explicit model inputs.
// Does not apply trades, overwrite positions, clear stops or authorize recovery.
[[nodiscard]] LocalQueryReview review_local_query(const SimQuerySnapshot&,const SimConfig&);
[[nodiscard]] LocalQueryReview review_local_query(const SimQueryResult&,QueryId expected_query,const SimConfig&);
[[nodiscard]] LocalQueryReview review_local_query_for_core(const SimQueryResult&,QueryId,const SimConfig&,const CoreLoop&);
// Post-delivery view comparison, not a recovery certificate or permission reset.
[[nodiscard]] LocalQueryReview compare_local_query_with_core(const SimQueryResult&,QueryId,const SimConfig&,const CoreLoop&);
} // namespace p03
