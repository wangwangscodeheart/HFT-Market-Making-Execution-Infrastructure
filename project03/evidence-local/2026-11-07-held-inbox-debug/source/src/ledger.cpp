#include "p03/ledger.hpp"

namespace p03 {
DedupResult ExecutionLedger::classify(const Trade& t) const {
    const auto it=records_.find(t.key);
    if(it==records_.end()) return DedupResult::New;
    return it->second.trade==t ? DedupResult::Identical : DedupResult::Conflicting;
}
void ExecutionLedger::insert(Trade t,LedgerDisposition d) {
    const auto key=t.key;
    if(!records_.emplace(key,ExecutionRecord{std::move(t),d}).second)
        throw std::logic_error("deduplicate before ledger insertion");
}
Lots ExecutionLedger::normal_quantity(OrderId id) const {
    Lots n=0;
    for(const auto& [key,r]:records_) {
        (void)key;
        if(r.trade.order==id && r.disposition==LedgerDisposition::Normal) n=checked_add(n,r.trade.quantity);
    }
    return n;
}
Lots ExecutionLedger::exception_quantity(OrderId id) const {
    Lots n=0;
    for(const auto& [key,r]:records_) {
        (void)key;
        if(r.trade.order==id && r.disposition==LedgerDisposition::Exception) n=checked_add(n,r.trade.quantity);
    }
    return n;
}
} // namespace p03
