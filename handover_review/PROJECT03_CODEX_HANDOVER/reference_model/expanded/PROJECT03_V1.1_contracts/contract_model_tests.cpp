// PROJECT 03 V1.1 executable architecture contract model.
// No SDK, trading core, actual queue, fsync or real-thread correctness claims.
// Each model operation below represents one specified atomic transition.
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

void check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

struct Journal {
    int produced=0, written=0, durable=0, acknowledged=0, writerEpoch=1;
    bool poisoned=false;
    void produce() { ++produced; }
    bool write(int seq) {
        if (poisoned || seq!=written+1 || seq>produced) return false;
        written=seq; return true;
    }
    void sync(bool success) {
        if (poisoned) return;
        if (success) durable=written; else poisoned=true;
    }
    bool ack(int seq, int epoch) {
        if (epoch!=writerEpoch) return false;
        if (seq<0 || seq>produced || seq>durable) { poisoned=true; return false; }
        acknowledged=std::max(acknowledged,seq); return true;
    }
    bool eligible(int required) const {
        return !poisoned && required>0 && required<=acknowledged;
    }
};

struct Batch { int first, last; bool valid; };
struct Scan { int prefix=0; bool failed=false; };
Scan scan(const std::vector<Batch>& batches) {
    Scan out;
    for (std::size_t i=0;i<batches.size();++i) {
        const auto& b=batches[i];
        if (!b.valid || b.first!=out.prefix+1 || b.last<b.first) {
            // A damaged final uncommitted tail can be isolated; a sequence
            // hole or any damage followed by later records cannot be skipped.
            out.failed=b.valid || i+1!=batches.size(); return out;
        }
        out.prefix=b.last;
    }
    return out;
}

enum class Phase { Queued, Claimed, Committed, Calling, Returned, Aborted };
struct Gate {
    bool open=true;
    std::uint64_t epoch=1, permitEpoch=0, latchVersion=0;
    int active=0, calls=0;
    Phase phase=Phase::Queued;
    bool claim(std::uint64_t expected, bool durable=true) {
        if (!open || epoch!=expected || !durable || phase!=Phase::Queued) return false;
        permitEpoch=epoch; phase=Phase::Claimed; ++active; return true;
    }
    bool commitCall() {
        if (phase!=Phase::Claimed) return false;
        if (!open || permitEpoch!=epoch) {
            phase=Phase::Aborted; --active; return false;
        }
        phase=Phase::Committed; return true;
    }
    void invoke() {
        if (phase==Phase::Committed) { phase=Phase::Calling; ++calls; }
    }
    void returned() {
        if (phase==Phase::Calling) { phase=Phase::Returned; --active; }
    }
    void kill() {
        check(epoch<std::numeric_limits<std::uint64_t>::max(),"epoch exhausted");
        open=false; ++epoch; ++latchVersion;
    }
    bool quiesced() const { return !open && active==0; }
    bool reset(bool allEvidence) {
        if (!allEvidence || !quiesced()) return false;
        open=true; return true;
    }
};

struct Parts { int e,h,p,l,v; };
struct Order {
    int q, b=0, r=0, x=0;
    bool noFuture=false, conflict=false;
    std::optional<int> finalQty;
    std::map<std::string,std::pair<int,std::string>> trades;
    explicit Order(int quantity):q(quantity){}
    void lower(int qty, bool verified, bool comparableNew=false) {
        if (!verified) return;
        if (qty<0 || qty>q) { conflict=true; return; }
        if (comparableNew && qty<r) conflict=true;
        r=std::max(r,qty);
        if (finalQty && r>*finalQty) conflict=true;
    }
    void terminal(std::optional<int> exact) {
        noFuture=true;
        if (!exact) return;
        if (*exact<0 || *exact>q) { conflict=true; return; }
        if (finalQty && *finalQty!=*exact) conflict=true;
        finalQty=exact;
        if (b>*exact || r>*exact) conflict=true;
        r=std::max(r,*exact);
    }
    void trade(const std::string& id, int qty, const std::string& economic,
               bool valid=true) {
        if (!valid || qty<=0 || id.empty()) return;
        const auto it=trades.find(id);
        if (it!=trades.end()) {
            if (it->second!=std::make_pair(qty,economic)) conflict=true;
            return;
        }
        trades.emplace(id,std::make_pair(qty,economic));
        if (qty>q-b) { x+=qty; conflict=true; }
        else b+=qty;
        if (finalQty && b>*finalQty) conflict=true;
    }
    Parts parts() const {
        const int e=std::max(b,r);
        if (conflict) throw std::runtime_error("normal partition is invalid in Conflict");
        if (!noFuture) return {e,e-b,q-e,0,0};
        if (finalQty) return {e,e-b,0,q-*finalQty,0};
        return {e,e-b,0,0,q-e};
    }
    int unresolvedHold() const {
        if (conflict) return q-b+x; // Full remaining order risk plus exceptions.
        const auto a=parts(); return a.h+a.p+a.v;
    }
};

// Indices: LongToday, LongYesterday, ShortToday, ShortYesterday.
struct Positions {
    std::array<int,4> booked{}, frozen{}, observed{};
    int day=1;
    bool needsReview=false;
    bool reserve(int bucket,int qty) {
        if (qty<=0 || qty>booked.at(bucket)-frozen.at(bucket)) return false;
        frozen.at(bucket)+=qty; return true;
    }
    bool close(int bucket,int qty,int sourceDay) {
        if (sourceDay!=day || qty<=0 || qty>frozen.at(bucket)) {
            needsReview=true; return false;
        }
        frozen.at(bucket)-=qty; booked.at(bucket)-=qty; return true;
    }
    void observe(const std::array<int,4>& value) { observed=value; }
    bool roll(bool unresolved, bool evidence) {
        if (unresolved || !evidence || std::any_of(frozen.begin(),frozen.end(),
                                                  [](int x){return x!=0;})) {
            needsReview=true; return false;
        }
        booked[1]+=booked[0]; booked[0]=0;
        booked[3]+=booked[2]; booked[2]=0;
        ++day; needsReview=false; return true;
    }
};
struct Resource { int need, covered; bool verified; };
struct Money {
    int available=1000, unanchoredCharges=0, buffer=0;
    std::vector<Resource> resources;
    int headroom() const {
        int out=available-unanchoredCharges-buffer;
        for(const auto& r:resources)
            out-=std::max(0,r.need-(r.verified?r.covered:0));
        return out;
    }
};

struct Coverage {
    bool orders=false,trades=false,positions=false,funds=false;
    bool oldDays=false,complete=false,stableCut=false,identities=false;
    bool all() const {
        return orders && trades && positions && funds && oldDays && complete
               && stableCut && identities;
    }
};
Coverage fullCoverage() { return {true,true,true,true,true,true,true,true}; }
struct Certificate {
    int loss=0,recovery=0,stateVersion=0;
    std::uint64_t latch=0;
    bool durable=false;
};
struct Recovery {
    Gate gate;
    int loss=0, recovery=0, cancelAttempts=0, stateVersion=0;
    bool callbackLoss=false, readyPaused=false, auditIncomplete=false;
    bool auditGap=false, forceExternal=false;
    void overflow() {
        ++loss; callbackLoss=true; readyPaused=false; gate.kill();
    }
    void begin() { ++recovery; readyPaused=false; }
    void economicFact() { ++stateVersion; readyPaused=false; }
    bool queryMatches(int queryId,int expectedQuery,int rec,int session,int expectedSession) const {
        return queryId==expectedQuery && rec==recovery && session==expectedSession;
    }
    std::optional<Certificate> certify(const Coverage& c, bool externallyReconciled,
                                      int capturedLoss, bool durable) {
        if (!c.all() || capturedLoss!=loss || !gate.quiesced()
            || (forceExternal && !externallyReconciled)) return std::nullopt;
        return Certificate{loss,recovery,stateVersion,gate.latchVersion,durable};
    }
    bool publish(const Certificate& c) {
        if (!c.durable || c.loss!=loss || c.recovery!=recovery || c.stateVersion!=stateVersion
            || c.latch!=gate.latchVersion) return false;
        readyPaused=true; return true;
    }
    bool reset(const Certificate& c) {
        // Models one shared-gate critical section, not an unlocked precheck.
        if (!readyPaused || !c.durable || c.loss!=loss || c.recovery!=recovery || c.stateVersion!=stateVersion
            || c.latch!=gate.latchVersion || !gate.quiesced()) return false;
        if (!gate.reset(true)) return false;
        callbackLoss=false; auditGap=false; return true;
    }
    bool emergencyCancel(bool reliableExistingIdentity) {
        gate.kill();
        if (!reliableExistingIdentity) return false;
        auditIncomplete=true; auditGap=true; forceExternal=true;
        ++cancelAttempts; return true; // Attempt only: no success state is set.
    }
};

int main() {
    int passed=0,failed=0,killSchedules=0,omsPermutations=0;
    auto test=[&](const char* name,const std::function<void()>& body) {
        try { body(); ++passed; std::cout<<"PASS "<<name<<'\n'; }
        catch(const std::exception& e) { ++failed; std::cout<<"FAIL "<<name<<": "<<e.what()<<'\n'; }
    };
    auto committed=[]() {
        Journal j; j.produce(); check(j.write(1),"write");j.sync(true);return j;
    };
    test("J01_contiguous_prefix",[&]{
        Journal j;j.produce();j.produce();
        check(!j.write(2) && j.written==0,"cannot cross hole");
        check(j.write(1) && j.write(2),"ordered writes");j.sync(true);
        check(j.ack(2,1) && j.acknowledged==2,"cumulative prefix");
    });
    test("J02_lost_ack_probe",[&]{
        auto j=committed();Gate g;check(!j.eligible(1),"ack not known");
        g.kill(); const int sizeBefore=j.produced;
        check(j.ack(j.durable,1),"status reply");
        check(j.produced==sizeBefore && !g.open,"no rewrite or auto reopen");
    });
    test("J03_old_writer_ack",[&]{auto j=committed();j.writerEpoch=2;
        check(!j.ack(1,1) && j.acknowledged==0,"stale ack ignored");});
    test("J04_sync_failure_poison",[&]{auto j=committed();j.ack(1,1);
        j.produce();j.write(2);j.sync(false);
        check(j.durable==1 && !j.eligible(1) && !j.write(2),"failed writer halted");});
    test("J05_recovered_intent_not_authorized",[&]{
        auto out=scan({{1,1,true},{2,2,true}});
        Journal restored;restored.produced=restored.written=out.prefix;
        check(!out.failed && out.prefix==2,"valid structural prefix");
        check(!restored.eligible(2),"disk presence is no old permit");
        Gate recoveryGate;recoveryGate.kill();
        check(!recoveryGate.claim(1,true),"old run intent cannot send");
    });
    test("J06_tail_and_middle_damage",[&]{
        const auto a=scan({{1,1,true},{2,2,false}});
        const auto b=scan({{1,1,true},{2,2,false},{3,3,true}});
        const auto c=scan({{1,1,true},{3,3,true}});
        check(!a.failed && a.prefix==1,"tail prefix");
        check(b.failed && c.failed,"never skip middle or hole");
    });
    test("J07_future_ack_rejected",[&]{auto j=committed();
        check(!j.ack(2,1) && j.poisoned && j.acknowledged==0,"future ack");});

    test("K01_all_kill_insertions",[&]{
        for(int insert=0;insert<=4;++insert) {
            Gate g;std::vector<char> events={'C','B','I','R'};
            events.insert(events.begin()+insert,'K');bool sawQuiesced=false;
            for(char e:events) {
                const int oldCalls=g.calls;
                if(e=='C')g.claim(1);
                if(e=='B')g.commitCall();
                if(e=='I')g.invoke();
                if(e=='R')g.returned();
                if(e=='K')g.kill();
                check(!sawQuiesced || g.calls==oldCalls,"no call after quiesced");
                sawQuiesced=sawQuiesced||g.quiesced();
            }
            check(g.calls==(insert>=2?1:0),"expected permitted call count");
            check(g.active==0 && g.quiesced(),"all permits accounted");++killSchedules;
        }
    });
    test("K02_blocked_call_not_quiesced",[&]{Gate g;g.claim(1);g.commitCall();
        g.invoke();g.kill();check(!g.quiesced(),"SDK not returned");
        g.returned();check(g.quiesced(),"now ended");});
    test("K03_old_epoch_after_reset",[&]{Gate g;g.kill();check(g.reset(true),"reset");
        check(!g.claim(1),"old epoch rejected");check(g.claim(2),"new epoch claim");});
    test("K04_requires_durable_prefix",[&]{Gate g;check(!g.claim(1,false),"no permit");});
    test("K05_overflow_uses_same_gate",[&]{Gate g;g.claim(1);g.commitCall();g.kill();
        check(!g.open && !g.quiesced(),"closed but committed");g.invoke();g.returned();
        check(g.calls==1 && g.quiesced(),"old committed call remains accounted");});
    test("K06_detect_check_then_claim_mutant",[&]{
        std::array<int,3> steps={0,1,2};bool found=false;
        do {bool open=true,readOpen=false,claimed=false;int readAt=-1,claimAt=-1;
            for(int i=0;i<3;++i){if(steps[i]==0){readOpen=open;readAt=i;}
                if(steps[i]==1)open=false;
                if(steps[i]==2){claimed=readOpen;claimAt=i;}}
            if(readAt>=0 && readAt<claimAt && claimed && steps[1]==1)found=true;
        } while(std::next_permutation(steps.begin(),steps.end()));
        check(found,"must expose read Kill claim race in mutant");
    });
    test("K07_detect_wrong_quiescence_mutant",[&]{Gate g;g.claim(1);g.commitCall();g.kill();
        const bool wrong=!g.open && g.calls==0;
        check(wrong && !g.quiesced(),"mutant falsely quiesced");g.invoke();
        check(g.calls==1,"counterexample enters SDK later");});

    test("O01_cumulative_trust",[&]{Order o(3);o.lower(2,false);check(o.r==0,"unverified");
        o.lower(-1,true);check(o.r==0 && o.conflict,"negative quarantined");
        Order b(3);b.lower(4,true);check(b.r==0 && b.conflict,"over Q quarantined");});
    test("O02_evidence_permutations",[&]{std::array<int,6> events={0,1,2,3,4,5};
        do {Order o(3);for(int e:events){if(e<3)o.trade("T"+std::to_string(e),1,"same-price");
                if(e==3)o.lower(1,true);
                if(e==4)o.lower(2,true);
                if(e==5)o.terminal(3);}
            const auto p=o.parts();check(o.b==3 && o.r==3 && p.e==3 && p.h==0
                && !o.conflict && o.unresolvedHold()==0,"same economic result");++omsPermutations;
        }while(std::next_permutation(events.begin(),events.end()));
    });
    test("O03_cancel_late_trade",[&]{Order o(3);o.trade("T1",1,"a");o.terminal(2);
        auto p=o.parts();check(o.b==1 && p.h==1 && p.l==1,"release only one");
        o.trade("T2",1,"b");o.trade("T2",1,"b");p=o.parts();
        check(o.b==2 && p.h==0 && p.l==1,"late booked once");});
    test("O04_terminal_conflict_books_fact",[&]{Order o(3);o.terminal(1);
        o.trade("late",2,"valid");check(o.b==2 && o.conflict,"valid fact retained");
        check(o.unresolvedHold()==1,"remaining risk restored");});
    test("O05_query_push_same_identity",[&]{Order o(3);o.trade("scope:T",1,"economic");
        o.trade("scope:T",1,"economic");check(o.b==1 && o.trades.size()==1,"once");});
    test("O06_same_identity_different_payload",[&]{Order o(3);o.trade("T",1,"price1");
        o.trade("T",1,"price2");check(o.b==1 && o.conflict,"no overwrite");});
    test("O07_exception_overfill_retained",[&]{Order o(3);o.trade("T1",2,"a");
        o.trade("T2",2,"b");o.trade("T2",2,"b");
        check(o.b==2 && o.x==2 && o.b+o.x>o.q && o.conflict,"exception not truncated");});
    test("O08_terminal_without_final_quantity",[&]{Order o(3);o.trade("T1",1,"a");
        o.terminal(std::nullopt);const auto p=o.parts();
        check(p.p==0 && p.v==2 && p.l==0 && o.unresolvedHold()==2,"unknown terminal held");});
    test("O09_comparable_cumulative_regression",[&]{Order o(3);o.lower(2,true);
        o.lower(1,true,true);check(o.r==2 && o.conflict,"trusted later regression conflicts");});

    test("P01_close_reservation",[&]{Positions p;p.booked[1]=3;
        check(p.reserve(1,2) && !p.reserve(1,2),"cannot freeze same lots");
        check(p.close(1,1,1) && p.booked[1]==2 && p.frozen[1]==1,"consume once");});
    test("P02_external_not_overwrite",[&]{Positions p;p.booked={2,1,0,0};
        p.observe({0,3,0,0});check(p.booked[0]==2 && p.observed[1]==3,"separate views");});
    test("P03_unresolved_cross_day",[&]{Positions p;p.booked[0]=2;p.reserve(0,1);
        check(!p.roll(true,true) && p.day==1 && p.frozen[0]==1,"retain old day");
        check(p.close(0,1,1),"late old-day close");
        check(p.roll(false,true) && p.day==2 && p.booked[1]==1,"verified new anchor");
        check(!p.close(1,1,1),"old command cannot become new close yesterday");});
    test("P04_known_money_coverage",[&]{Money m;m.resources={{200,200,true},{100,0,true}};
        check(m.headroom()==900,"included not charged twice");
        m.resources[0].need=250;check(m.headroom()==850,"incremental haircut");});
    test("P05_unknown_money_coverage",[&]{Money m;m.resources={{200,200,false},{100,0,true}};
        check(m.headroom()==700,"unknown coverage counts zero");});
    test("P06_resource_transfer_and_fees",[&]{Money m;m.resources={{200,0,true}};
        const int before=m.headroom(); // Transfer order -> position keeps same resource.
        m.resources[0]={200,0,true};m.unanchoredCharges=10;
        check(m.headroom()==before-10,"no invented credit; fees retained");});

    test("Q01_empty_queue_is_not_recovery",[&]{Recovery r;r.overflow();
        check(!r.gate.open && !r.readyPaused,"drain cannot reopen");});
    test("Q02_missing_trade_coverage",[&]{Recovery r;r.overflow();r.begin();
        auto c=fullCoverage();c.trades=false;check(!r.certify(c,true,1,true),"need trade ledger");});
    test("Q03_missing_old_day_coverage",[&]{Recovery r;r.overflow();r.begin();
        auto c=fullCoverage();c.oldDays=false;check(!r.certify(c,true,1,true),"need old range");});
    test("Q04_wrong_query_identity",[&]{Recovery r;r.overflow();r.begin();
        check(!r.queryMatches(2,1,r.recovery,1,1),"query id");
        check(!r.queryMatches(1,1,r.recovery-1,1,1),"recovery epoch");
        check(!r.queryMatches(1,1,r.recovery,2,1),"source session");});
    test("Q05_loss_during_recovery",[&]{Recovery r;r.overflow();r.begin();
        const int captured=r.loss;r.overflow();
        check(!r.certify(fullCoverage(),true,captured,true),"old collection invalid");});
    test("Q06_certificate_durable_then_explicit_reset",[&]{Recovery r;r.overflow();r.begin();
        auto a=r.certify(fullCoverage(),true,r.loss,false);check(a && !r.publish(*a),"not durable");
        auto b=r.certify(fullCoverage(),true,r.loss,true);check(b && r.publish(*b),"ready paused");
        check(!r.gate.open,"publication cannot open");
        check(r.reset(*b) && r.gate.open && !r.callbackLoss,"explicit reset");});
    test("Q07_reset_loss_orderings",[&]{
        for(bool lossFirst:{false,true}){Recovery r;r.overflow();r.begin();
            auto c=r.certify(fullCoverage(),true,r.loss,true);check(c.has_value(),"certificate");r.publish(*c);
            if(lossFirst){r.overflow();check(!r.reset(*c),"stale certificate rejected");}
            else {check(r.reset(*c),"first reset valid");r.overflow();}
            check(!r.gate.open && r.callbackLoss && r.loss==2,"new loss remains latched");}
    });
    test("Q08_emergency_cancel_audit",[&]{Recovery r;
        check(!r.emergencyCancel(false) && r.cancelAttempts==0,"no guessed identity");
        check(r.emergencyCancel(true) && r.auditIncomplete && r.auditGap,"audit exception");
        r.begin();check(!r.certify(fullCoverage(),false,r.loss,true),"external reconciliation required");
        auto c=r.certify(fullCoverage(),true,r.loss,true);check(c && r.publish(*c) && r.reset(*c),"verified recovery");
        check(r.auditIncomplete && !r.auditGap && r.cancelAttempts==1,"history remains; no cancel success inferred");
    });
    test("Q09_fact_invalidates_certificate",[&]{Recovery r;r.overflow();r.begin();
        auto c=r.certify(fullCoverage(),true,r.loss,true);check(c && r.publish(*c),"initial cert");
        r.economicFact();check(!r.publish(*c) && !r.reset(*c),"state changed after cut");
        auto next=r.certify(fullCoverage(),true,r.loss,true);
        check(next && r.publish(*next) && r.reset(*next),"new validation allows reset");
    });
    std::cout<<"SUMMARY named_tests="<<passed+failed<<" passed="<<passed<<" failed="<<failed
             <<" kill_schedules="<<killSchedules<<" oms_permutations="<<omsPermutations<<'\n';
    return failed?1:0;
}
