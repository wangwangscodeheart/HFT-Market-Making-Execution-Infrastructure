#pragma once
#include "p03/local_quote_runner.hpp"
#include <span>
namespace p03 {
enum class LocalReplayMismatch { None,EmptyTrace,InvalidConfiguration,TraceCapacity,FrameShape,StepError,
    CoreState,StrategyDecision,CoreCalls,OrderViews,PeerSnapshot,DeliveryState,ReplayException,RecordedInput,AttemptShape,AttemptResult,RunnerState };
struct LocalReplayReview {Error error{Error::None};LocalReplayMismatch mismatch{LocalReplayMismatch::None};std::size_t frame{},matched_frames{};};
// Verifies only the recorded accepted-frame prefix in a fresh local runner.
// Not a durable restore, rejected-attempt audit, or recovery authorization.
[[nodiscard]] LocalReplayReview verify_local_quote_trace(const LocalQuoteRunnerConfig&,std::span<const LocalQuoteFrame>);
struct LocalQuoteAttempt {
    std::size_t ordinal{};LogicalTime at{};LocalQuoteInput input;Error error{Error::None};
    std::size_t frames_before{},frames_after{};bool stopped{},cleanup_exhausted{};std::string canonical;
};
struct LocalExperimentReview {
    Error error{Error::None};LocalReplayMismatch mismatch{LocalReplayMismatch::None};
    std::size_t attempt{},matched_attempts{},matched_frames{};
};
// Bounded offline experiment replay, including supplied rejected attempts.
// Caller owns collection; this is not a runtime audit writer or completeness proof.
[[nodiscard]] LocalExperimentReview verify_local_quote_experiment(const LocalQuoteRunnerConfig&,
    std::span<const LocalQuoteAttempt>,std::span<const LocalQuoteFrame>);
}
