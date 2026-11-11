#pragma once
#include "p03/local_quote_runner.hpp"
#include <span>
namespace p03 {
enum class LocalReplayMismatch { None,EmptyTrace,InvalidConfiguration,TraceCapacity,FrameShape,StepError,
    CoreState,StrategyDecision,CoreCalls,OrderViews,PeerSnapshot,DeliveryState,ReplayException };
struct LocalReplayReview {Error error{Error::None};LocalReplayMismatch mismatch{LocalReplayMismatch::None};std::size_t frame{},matched_frames{};};
// Verifies only the recorded accepted-frame prefix in a fresh local runner.
// Not a durable restore, rejected-attempt audit, or recovery authorization.
[[nodiscard]] LocalReplayReview verify_local_quote_trace(const LocalQuoteRunnerConfig&,std::span<const LocalQuoteFrame>);
}
