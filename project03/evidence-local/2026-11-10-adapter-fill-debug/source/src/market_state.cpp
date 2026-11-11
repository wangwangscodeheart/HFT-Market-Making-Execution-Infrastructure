#include "p03/market_state.hpp"
#include <type_traits>
namespace p03 {
MarketState::MarketState(MarketStateConfig c):config_(std::move(c)),generation_(config_.initial_generation){
    config_.contract.check_configuration();
    if(config_.scope.account.empty() || config_.scope.account.size()>128 || config_.scope.position_epoch==0 ||
       config_.scope.contract!=config_.contract.key() || config_.scope.day!=config_.contract.day ||
       config_.source.empty() || config_.source.size()>128 || config_.source_epoch==0 || generation_==0 ||
       config_.required_depth==0 || config_.required_depth>max_market_depth || config_.max_age==0 ||
       (config_.origin!=EvidenceOrigin::LocalFixture && config_.origin!=EvidenceOrigin::LocalSimulator))throw std::invalid_argument("explicit local market profile required");
}
Error MarketState::touch(LogicalTime now){
    if(now<last_time_)return Error::OutOfOrderTime;last_time_=now;
    if(status_==FeedStatus::Valid && last_received_ && now-*last_received_>config_.max_age){status_=FeedStatus::Stale;reason_=FeedReason::Expired;}
    return Error::None;
}
Error MarketState::invalid(Error error,FeedReason reason){status_=FeedStatus::Invalid;reason_=reason;return error;}
Error MarketState::tick(LogicalTime now){return touch(now);}
Error MarketState::disconnect(LogicalTime now){
    const auto error=touch(now);if(error!=Error::None)return error;
    status_=FeedStatus::Recovering;reason_=FeedReason::Disconnected;requires_new_generation_=true;return Error::None;
}
Error MarketState::local_loss(LogicalTime now){
    const auto error=disconnect(now);if(error==Error::None)reason_=FeedReason::LocalLoss;return error;
}
Error MarketState::begin_subscription(std::uint64_t generation,LogicalTime now){
    if(day_boundary_halted_)return Error::WrongTradingDay;
    if(generation<=generation_)return Error::WrongIdentity;
    const auto error=touch(now);if(error!=Error::None)return error;
    generation_=generation;last_sequence_=0;last_received_.reset();snapshot_.reset();requires_new_generation_=false;
    status_=FeedStatus::Recovering;reason_=FeedReason::AwaitingFreshSnapshot;return Error::None;
}
Error MarketState::update(const MarketSnapshot& book,LogicalTime now){
    if(book.scope.account!=config_.scope.account || book.scope.contract!=config_.scope.contract || book.scope.position_epoch!=config_.scope.position_epoch)return Error::WrongIdentity;
    if(book.origin!=config_.origin || book.source!=config_.source || book.source_epoch!=config_.source_epoch)return Error::UntrustedEvidence;
    if(book.subscription_generation!=generation_)return Error::WrongIdentity;
    const auto error=touch(now);if(error!=Error::None)return error;
    if(book.scope.day!=config_.scope.day || day_boundary_halted_){day_boundary_halted_=true;return invalid(Error::WrongTradingDay,FeedReason::TradingDayMismatch);}
    if(requires_new_generation_)return Error::Blocked;
    if(book.receive_sequence==0)return invalid(Error::InvalidInput,FeedReason::SequenceConflict);
    if(book.receive_sequence<=last_sequence_){
        if(book.receive_sequence<last_sequence_ || (snapshot_ && book==*snapshot_))return Error::Duplicate;
        return invalid(Error::EvidenceConflict,FeedReason::SequenceConflict);
    }
    last_sequence_=book.receive_sequence; // Invalid current-profile input also fences older snapshots.
    if(book.valid_fields!=market_required_fields)return invalid(Error::InvalidInput,FeedReason::MissingFields);
    if(book.valid_depth<config_.required_depth || book.valid_depth>max_market_depth)return invalid(Error::InvalidInput,FeedReason::MissingDepth);
    if(book.received_at>now)return invalid(Error::StaleMark,FeedReason::FutureTimestamp);
    if(last_received_ && book.received_at<*last_received_)return invalid(Error::OutOfOrderTime,FeedReason::ReceiveTimeRegression);
    if(now-book.received_at>config_.max_age)return invalid(Error::StaleMark,FeedReason::Expired);
    const auto& c=config_.contract;
    for(std::size_t i=0;i<book.valid_depth;++i){const auto& bid=book.bids[i];const auto& ask=book.asks[i];
        for(const auto& level:{bid,ask})if(level.price<c.low || level.price>c.high || level.price%c.tick_step!=0 || level.quantity<=0 || level.quantity>max_input_lots)return invalid(Error::InvalidInput,FeedReason::InvalidBook);
        if(bid.price>=ask.price || (i>0 && (book.bids[i-1].price<=bid.price || book.asks[i-1].price>=ask.price)))return invalid(Error::InvalidInput,FeedReason::InvalidBook);
    }
    // Copy potentially allocating strings before publishing a new snapshot.
    status_=FeedStatus::Invalid;reason_=FeedReason::ResourceFailure;
    auto candidate=book;
    static_assert(std::is_nothrow_move_assignable_v<MarketSnapshot> && std::is_nothrow_move_constructible_v<MarketSnapshot>);
    snapshot_=std::move(candidate);last_received_=book.received_at;status_=FeedStatus::Valid;reason_=FeedReason::None;return Error::None;
}
FeedHealth MarketState::health(LogicalTime now) const {
    auto status=status_;auto reason=reason_;
    if(now<last_time_){status=FeedStatus::Invalid;reason=FeedReason::TimeRegression;}
    else if(status==FeedStatus::Valid && last_received_ && now-*last_received_>config_.max_age){status=FeedStatus::Stale;reason=FeedReason::Expired;}
    return {status,reason,generation_,last_sequence_,last_received_};
}
std::optional<MarketTop> MarketState::top(LogicalTime now) const {
    if(health(now).status!=FeedStatus::Valid || !snapshot_)return {};
    const auto& b=*snapshot_;return MarketTop{b.scope,b.origin,b.source,b.source_epoch,b.receive_sequence,b.received_at,
        b.bids[0].price,b.asks[0].price,b.bids[0].quantity,b.asks[0].quantity,true,generation_};
}
}
