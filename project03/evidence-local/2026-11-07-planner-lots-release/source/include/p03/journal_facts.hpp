#pragma once
#include "p03/types.hpp"

// M1A value contracts only. These objects do not write files, attest fsync,
// grant dispatch eligibility, or restore an old send permission.
namespace p03 {
struct JournalId {
    std::string value;
    [[nodiscard]] bool valid() const noexcept { return !value.empty() && value.size()<=128; }
    auto operator<=>(const JournalId&) const = default;
};
template<class Tag> struct JournalCounter {
    std::uint64_t value{};
    auto operator<=>(const JournalCounter&) const = default;
};
struct WriterEpochTag; struct JournalSequenceTag; struct JournalBatchTag;
using WriterEpoch=JournalCounter<WriterEpochTag>;
using JournalSequence=JournalCounter<JournalSequenceTag>;
using JournalBatchNumber=JournalCounter<JournalBatchTag>;

struct JournalIdentity {
    JournalId journal;
    WriterEpoch writer_epoch;
    [[nodiscard]] bool valid() const noexcept { return journal.valid() && writer_epoch.value!=0; }
    bool operator==(const JournalIdentity&) const = default;
};

struct JournalBatchBoundary {
    JournalIdentity writer;
    JournalBatchNumber batch;
    JournalSequence first, last, previous_last;
    [[nodiscard]] Error validate_shape() const noexcept {
        if(!writer.valid() || batch.value==0 || first.value==0 || last.value<first.value)
            return Error::InvalidInput;
        if(previous_last.value==std::numeric_limits<std::uint64_t>::max())return Error::IdExhausted;
        return first.value==previous_last.value+1 ? Error::None : Error::InvalidInput;
    }
};

struct RequiredJournalPrefix {
    JournalIdentity writer;
    JournalSequence required_journal_seq;
    [[nodiscard]] Error validate_shape() const noexcept {
        return writer.valid() && required_journal_seq.value!=0 ? Error::None : Error::InvalidInput;
    }
};

struct LiveJournalPrefixes {
    JournalIdentity writer;
    JournalSequence produced, written, durable, acknowledged;
    [[nodiscard]] Error validate_shape() const noexcept {
        if(!writer.valid())return Error::InvalidInput;
        return acknowledged<=durable && durable<=written && written<=produced ? Error::None : Error::InvalidInput;
    }
};

struct JournalDurabilityAcknowledgement {
    JournalIdentity writer;
    JournalBatchNumber batch;
    JournalSequence durable_seq;
    [[nodiscard]] Error validate_shape() const noexcept {
        return writer.valid() && batch.value!=0 && durable_seq.value!=0 ? Error::None : Error::InvalidInput;
    }
};

// Structural relation between two supplied boundaries, not a journal index.
[[nodiscard]] inline Error validate_adjacent_batches(const JournalBatchBoundary& previous,
                                                     const JournalBatchBoundary& next) noexcept {
    const auto first_error=previous.validate_shape();if(first_error!=Error::None)return first_error;
    const auto next_error=next.validate_shape();if(next_error!=Error::None)return next_error;
    if(previous.writer!=next.writer)return Error::WrongIdentity;
    if(previous.batch.value==std::numeric_limits<std::uint64_t>::max())return Error::IdExhausted;
    return next.batch.value==previous.batch.value+1 && next.previous_last==previous.last ? Error::None : Error::InvalidInput;
}

// Confirms only that an acknowledgement names the supplied complete boundary.
// Callers must independently prove that this is a known, contiguous, current
// Writer boundary backed by successful sync. M1A has no such Writer/runtime.
[[nodiscard]] inline Error validate_ack_boundary_reference(const JournalDurabilityAcknowledgement& ack,
                                                           const JournalBatchBoundary& boundary) noexcept {
    const auto ack_error=ack.validate_shape();if(ack_error!=Error::None)return ack_error;
    const auto boundary_error=boundary.validate_shape();if(boundary_error!=Error::None)return boundary_error;
    if(ack.writer!=boundary.writer)return Error::WrongIdentity;
    return ack.batch==boundary.batch && ack.durable_seq==boundary.last ? Error::None : Error::InvalidInput;
}
} // namespace p03
