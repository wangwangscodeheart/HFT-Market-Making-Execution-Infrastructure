#include "test.hpp"
#include "p03/journal_facts.hpp"
#include <type_traits>
using namespace p03;
namespace {
JournalIdentity writer(){return {{"fixture.journal.stream"},{1}};}
JournalBatchBoundary batch(){return {writer(),{1},{1},{4},{0}};}
}

TEST_CASE(JOURNAL_TYPES_identity_and_counter_domains_are_distinct) {
    static_assert(!std::is_same_v<WriterEpoch,JournalSequence>);
    static_assert(!std::is_convertible_v<JournalSequence,WriterEpoch>);
    static_assert(!std::is_convertible_v<JournalBatchNumber,JournalSequence>);
    CHECK(writer().valid());CHECK(!JournalIdentity{}.valid());
    auto bad=writer();bad.writer_epoch.value=0;CHECK(!bad.valid());
    bad=writer();bad.journal.value.assign(129,'x');CHECK(!bad.valid());
    CHECK_EQ((RequiredJournalPrefix{writer(),{4}}.validate_shape()),Error::None);
    CHECK_EQ((RequiredJournalPrefix{writer(),{0}}.validate_shape()),Error::InvalidInput);
}

TEST_CASE(JOURNAL_TYPES_batch_boundaries_reject_holes_overlap_and_wrap) {
    CHECK_EQ(batch().validate_shape(),Error::None);
    for(int variant=0;variant<5;++variant){
        auto b=batch();
        if(variant==0)b.first.value=0;
        if(variant==1)b.last.value=0;
        if(variant==2)b.first.value=2;
        if(variant==3)b.previous_last.value=1;
        if(variant==4)b.batch.value=0;
        CHECK_EQ(b.validate_shape(),Error::InvalidInput);
    }
    auto overflow=batch();overflow.previous_last.value=std::numeric_limits<std::uint64_t>::max();
    CHECK_EQ(overflow.validate_shape(),Error::IdExhausted);
    auto last=batch();last.first.value=last.last.value=std::numeric_limits<std::uint64_t>::max();
    last.previous_last.value=last.first.value-1;CHECK_EQ(last.validate_shape(),Error::None);
}

TEST_CASE(JOURNAL_TYPES_adjacent_batches_require_same_writer_and_contiguous_ranges) {
    const auto first=batch();auto next=first;next.batch.value=2;next.first.value=5;next.last.value=9;next.previous_last.value=4;
    CHECK_EQ(validate_adjacent_batches(first,next),Error::None);
    auto gap=next;gap.first.value=6;gap.previous_last.value=5;
    CHECK_EQ(gap.validate_shape(),Error::None);CHECK_EQ(validate_adjacent_batches(first,gap),Error::InvalidInput);
    auto overlap=next;overlap.first.value=4;overlap.previous_last.value=3;
    CHECK_EQ(overlap.validate_shape(),Error::None);CHECK_EQ(validate_adjacent_batches(first,overlap),Error::InvalidInput);
    auto skipped=next;skipped.batch.value=3;CHECK_EQ(validate_adjacent_batches(first,skipped),Error::InvalidInput);
    auto other=next;other.writer.writer_epoch.value=2;CHECK_EQ(validate_adjacent_batches(first,other),Error::WrongIdentity);
    other=next;other.writer.journal.value="fixture.other.journal";CHECK_EQ(validate_adjacent_batches(first,other),Error::WrongIdentity);
}

TEST_CASE(JOURNAL_TYPES_live_prefixes_enforce_all_ordering_relations) {
    CHECK_EQ((LiveJournalPrefixes{writer(),{0},{0},{0},{0}}.validate_shape()),Error::None);
    CHECK_EQ((LiveJournalPrefixes{writer(),{9},{7},{4},{4}}.validate_shape()),Error::None);
    CHECK_EQ((LiveJournalPrefixes{writer(),{4},{5},{4},{4}}.validate_shape()),Error::InvalidInput);
    CHECK_EQ((LiveJournalPrefixes{writer(),{9},{4},{5},{4}}.validate_shape()),Error::InvalidInput);
    CHECK_EQ((LiveJournalPrefixes{writer(),{9},{7},{4},{5}}.validate_shape()),Error::InvalidInput);
}

TEST_CASE(JOURNAL_TYPES_ack_reference_requires_exact_batch_end_and_writer) {
    const auto b=batch();JournalDurabilityAcknowledgement ack{writer(),{1},{4}};
    CHECK_EQ(validate_ack_boundary_reference(ack,b),Error::None);
    ack.durable_seq.value=3;CHECK_EQ(validate_ack_boundary_reference(ack,b),Error::InvalidInput);
    ack.durable_seq.value=5;CHECK_EQ(validate_ack_boundary_reference(ack,b),Error::InvalidInput);
    ack.durable_seq.value=4;ack.batch.value=2;CHECK_EQ(validate_ack_boundary_reference(ack,b),Error::InvalidInput);
    ack.batch.value=1;ack.writer.writer_epoch.value=2;CHECK_EQ(validate_ack_boundary_reference(ack,b),Error::WrongIdentity);
    ack.writer=writer();ack.writer.journal.value="fixture.other";CHECK_EQ(validate_ack_boundary_reference(ack,b),Error::WrongIdentity);
}

TEST_CASE(JOURNAL_TYPES_exhausted_batch_numbers_cannot_wrap_to_a_new_batch) {
    auto previous=batch();previous.batch.value=std::numeric_limits<std::uint64_t>::max();
    auto next=previous;next.batch.value=1;next.first.value=5;next.last.value=7;next.previous_last.value=4;
    CHECK_EQ(validate_adjacent_batches(previous,next),Error::IdExhausted);
}
