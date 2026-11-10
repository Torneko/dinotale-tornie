#include "../heart_wallet0283.hpp"
#include "json/json.hpp"
#include <array>
#include <atomic>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <thread>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace w=workshop::heart_wallet0283;
namespace fs=std::filesystem;
static unsigned checks=0;
static void need(bool condition,const char* reason){++checks;if(!condition)throw std::runtime_error(reason);}
static std::string bytes(const fs::path& path){std::ifstream input(path,std::ios::binary);return {std::istreambuf_iterator<char>(input),{}};}
static void put(const fs::path& path,const std::string& content){std::ofstream output(path,std::ios::binary);output<<content;if(!output)throw std::runtime_error("QA fixture write failed");}
static void no_temps(const fs::path& profile){for(const auto& entry:fs::directory_iterator(profile))need(entry.path().filename().string().find(".tmp-")==std::string::npos,"Failed transaction leaked a temporary wallet");}
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Explicit NEW isolated QA output directory required");
    const auto out=fs::absolute(argv[1]);if(fs::exists(out))throw std::runtime_error("Refusing to overwrite an existing test output");
    fs::create_directories(out);
    need(w::level_count==24&&w::hearts_per_level==3&&w::slot_count==72,"Wallet domain changed");
    need(w::life_cost==5&&w::eggs_cost==3&&w::start_life==8&&w::start_eggs==6,"Confirmed next-attempt offers changed");
    need(w::index(0,0)==0&&w::index(23,2)==71&&!w::index(24,0)&&!w::index(0,3),"Physical-slot domain is not bounded");
    w::Held full;full.set();
    need(!w::plan_spend(full,0,0)&&!w::plan_spend(full,73,0)&&!w::plan_spend({},1,0),"Empty/invalid/overdraw plan accepted");
    const std::array<uint64_t,5> seeds{0,1,42,0x123456789ABCDEF0ull,~uint64_t(0)};
    for(unsigned cost=1;cost<=72;++cost)for(const auto seed:seeds){
        const auto plan=w::plan_spend(full,cost,seed),again=w::plan_spend(full,cost,seed);
        need(bool(plan)&&bool(again),"Affordable debit did not plan");
        need(plan->removed.count()==cost&&plan->remaining.count()==72-cost,"Debit removed an incorrect number of slots");
        need((plan->removed&plan->remaining).none()&&(plan->removed|plan->remaining)==full,"Debit duplicated or invented a slot");
        need(plan->removed==again->removed&&plan->remaining==again->remaining,"Seeded debit changed across identical calls");
    }
    const auto golden=w::plan_spend(full,5,0);w::Held expected;
    for(unsigned bit:{7u,69u,11u,16u,35u})expected.set(bit);
    need(golden&&golden->removed==expected,"Stable SplitMix64/rejection/partial-shuffle sequence changed");
    w::Held sparse;for(unsigned bit:{0u,17u,46u,71u})sparse.set(bit);
    std::map<std::string,unsigned> distribution;
    for(uint64_t seed=0;seed<60000;++seed){
        const auto plan=w::plan_spend(sparse,2,seed);
        need(plan&&plan->removed.count()==2&&(plan->removed&~sparse).none(),"Sparse debit selected an unheld slot or duplicate");
        ++distribution[plan->removed.to_string()];
    }
    need(distribution.size()==6,"Sampler cannot reach every two-of-four subset");
    for(const auto& [subset,count]:distribution){(void)subset;need(count>9300&&count<10700,"Seeded subset frequency is outside the broad uniformity smoke bound");}
    const auto profile=out/"normal";fs::create_directory(profile);
    put(profile/"menu-progress.json","{\"trialMask\":16777215,\"bestHearts\":[2,3]}");
    put(profile/"pc-achievements.json","{\"earned\":true}");
    const auto old_menu=bytes(profile/"menu-progress.json"),old_achievements=bytes(profile/"pc-achievements.json");
    const auto ledger=profile/w::filename;w::Wallet wallet;
    need(wallet.load(profile)&&wallet.snapshot().purchases_enabled()&&!fs::exists(ledger),"Empty explicit profile failed or load wrote a wallet");
    need(wallet.snapshot().value.balance()==0&&wallet.snapshot().value.pending.empty(),"Existing achievements were credited or converted into new currency");
    for(unsigned level=0;level<24;++level)for(unsigned slot=0;slot<3;++slot){
        need(wallet.claim(level,slot).committed(),"Distinct synthetic physical slot did not credit");
        need(wallet.claim(level,slot).status==w::Status::AlreadyHeld,"Duplicate claim credited again");
    }
    need(wallet.snapshot().value.balance()==72&&wallet.snapshot().value.held==full,"Complete synthetic 24x3 wallet is not exactly 72");
    const auto full_bytes=bytes(ledger);
    need(wallet.claim(24,0).status==w::Status::InvalidSlot&&wallet.claim(0,3).status==w::Status::InvalidSlot&&bytes(ledger)==full_bytes,"Invalid physical slot changed persisted wallet");
    need(wallet.spend(0,0).status==w::Status::InvalidCost&&wallet.spend(73,0).status==w::Status::InvalidCost&&bytes(ledger)==full_bytes,"Invalid cost changed persisted wallet");
    need(wallet.load(profile)&&wallet.claim(23,2).status==w::Status::AlreadyHeld&&bytes(ledger)==full_bytes,"Reload lost once-only physical claims");
    const auto debit=wallet.spend(11,12345);
    need(debit.committed()&&debit.removed.count()==11&&debit.balance==61&&wallet.snapshot().value.pending.empty(),"Generic debit did not free exactly eleven held slots");
    unsigned credited=0;
    for(unsigned bit=0;bit<72;++bit){
        const auto result=wallet.claim(bit/3,bit%3);
        need(result.committed()==debit.removed.test(bit),"A replay recredited an unspent heart or failed to recredit a spent one");
        credited+=result.committed()?1u:0u;
    }
    need(credited==11&&wallet.snapshot().value.held==full,"Only the exact eleven spent slots must become eligible again");
    const auto life=wallet.purchase(w::Bonus::Life,0);
    need(life.committed()&&life.removed.count()==5&&life.bonuses.empty()&&wallet.snapshot().value.pending.life&&life.balance==67,"Life purchase did not atomically persist five-slot debit plus pending life");
    const auto life_bytes=bytes(ledger);
    need(wallet.purchase(w::Bonus::Life,1).status==w::Status::AlreadyPending&&bytes(ledger)==life_bytes,"Repeated pending life purchase charged twice");
    const auto eggs=wallet.purchase(w::Bonus::Eggs,42);
    need(eggs.committed()&&eggs.removed.count()==3&&(eggs.removed&life.removed).none()&&eggs.balance==64,"Egg purchase did not debit exactly three currently held slots");
    need(wallet.load(profile)&&wallet.snapshot().value.pending==w::Pending{true,true},"Reload lost next-attempt bonuses");
    const auto take=wallet.consume_pending();
    need(take.committed()&&take.bonuses==w::Pending{true,true}&&take.removed.none()&&take.balance==64&&wallet.snapshot().value.pending.empty(),"Next-attempt consumption changed currency or failed to clear both bonuses");
    const auto consumed_bytes=bytes(ledger);
    need(wallet.consume_pending().status==w::Status::NoPending&&bytes(ledger)==consumed_bytes,"Next-attempt bonuses consumed twice");
    need(bytes(profile/"menu-progress.json")==old_menu&&bytes(profile/"pc-achievements.json")==old_achievements,"Wallet touched existing progression");
    const auto poor=out/"insufficient";fs::create_directory(poor);w::Wallet empty;need(empty.load(poor),"Insufficient profile failed to load");
    need(empty.purchase(w::Bonus::Life,0).status==w::Status::InsufficientFunds&&!fs::exists(poor/w::filename),"Unaffordable purchase wrote a debit or bonus");
    for(unsigned slot=0;slot<3;++slot)need(empty.claim(0,slot).committed(),"Poor-profile synthetic claim failed");
    const auto poor_before=bytes(poor/w::filename);
    need(empty.purchase(w::Bonus::Life,0).status==w::Status::InsufficientFunds&&bytes(poor/w::filename)==poor_before,"Unaffordable life purchase changed a three-heart wallet");
    need(empty.purchase(w::Bonus::Eggs,0).committed()&&empty.snapshot().value.balance()==0&&empty.snapshot().value.pending.eggs,"Three-heart egg purchase did not spend the exact balance");
    need(empty.claim(0,0).committed()&&empty.claim(0,0).status==w::Status::AlreadyHeld,"A spent physical heart did not become eligible once");
    need(empty.purchase(static_cast<w::Bonus>(99),0).status==w::Status::InvalidBonus,"Unknown bonus accepted");
    const auto valid_json=nlohmann::json::parse(full_bytes);
    std::vector<std::string> bad{"broken","",full_bytes+"trailing",std::string(4097,' ')};
    for(unsigned kind=0;kind<10;++kind){
        auto value=valid_json;
        if(kind==0)value["schema"]=2u;if(kind==1)value["schema"]=-1;if(kind==2)value["held"]=std::string(71,'0');
        if(kind==3)value["held"]=std::string(73,'0');if(kind==4)value["held"]=std::string(71,'0')+'2';if(kind==5)value["held"]=nlohmann::json::array();
        if(kind==6)value["balance"]=72u;if(kind==7)value["pending"]["life"]=1;if(kind==8)value["pending"].erase("eggs");if(kind==9)value["pending"]["extra"]=false;
        bad.push_back(value.dump());
    }
    bad.push_back("{\"schema\":1,\"held\":\""+std::string(72,'0')+"\",\"held\":\""+std::string(72,'1')+"\",\"pending\":{\"life\":false,\"eggs\":false}}");
    bad.push_back("{\"schema\":1,\"held\":\""+std::string(72,'0')+"\",\"pending\":{\"life\":false,\"life\":true,\"eggs\":false}}");
    for(size_t n=0;n<bad.size();++n){
        const auto dir=out/("invalid-"+std::to_string(n));fs::create_directory(dir);const auto file=dir/w::filename;put(file,bad[n]);w::Wallet invalid;
        need(!invalid.load(dir)&&!invalid.snapshot().purchases_enabled(),"Malformed/ambiguous JSON enabled purchases");
        need(invalid.claim(0,0).status==w::Status::Unavailable&&invalid.purchase(w::Bonus::Life,0).status==w::Status::Unavailable&&invalid.spend(1,0).status==w::Status::Unavailable&&invalid.consume_pending().status==w::Status::Unavailable,"Unreadable wallet permitted an update");
        need(bytes(file)==bad[n],"Invalid JSON was replaced or reset");
    }
    need(wallet.load(profile),"Valid normal ledger failed to reload");put(ledger,"external-corruption");
    need(wallet.purchase(w::Bonus::Life,7).status==w::Status::Unavailable&&!wallet.snapshot().purchases_enabled()&&bytes(ledger)=="external-corruption","Commit overwrote newly corrupted JSON");
    put(ledger,consumed_bytes);need(wallet.load(profile),"Isolated valid wallet restoration failed");
    fs::remove(ledger);need(wallet.claim(0,0).status==w::Status::Unavailable&&!fs::exists(ledger),"A disappeared wallet was silently reset");
    put(ledger,full_bytes);need(wallet.load(profile),"Full isolated fixture failed to reload");
#ifdef _WIN32
    HANDLE held=CreateFileW(ledger.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    need(held!=INVALID_HANDLE_VALUE,"QA replace-failure lock failed");
    const auto prior=wallet.snapshot().value;const auto failed=wallet.purchase(w::Bonus::Life,0);CloseHandle(held);
    need(failed.status==w::Status::PersistenceFailed&&failed.removed.none()&&failed.bonuses.empty()&&wallet.snapshot().value==prior&&bytes(ledger)==full_bytes,"Failed replacement partially debited or queued a bonus");
    no_temps(profile);
    const auto retried=wallet.purchase(w::Bonus::Life,0);need(retried.committed()&&retried.removed==expected&&retried.balance==67&&wallet.snapshot().value.pending.life,"A failed purchase was not retryable deterministically");
    const auto pending_bytes=bytes(ledger);const auto pending_value=wallet.snapshot().value;
    held=CreateFileW(ledger.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"QA consume-failure lock failed");
    const auto failed_take=wallet.consume_pending();CloseHandle(held);
    need(failed_take.status==w::Status::PersistenceFailed&&failed_take.bonuses.empty()&&wallet.snapshot().value==pending_value&&bytes(ledger)==pending_bytes,"Failed consumption granted or discarded a next-attempt bonus");
    no_temps(profile);need(wallet.consume_pending().committed()&&wallet.snapshot().value.pending.empty(),"Pending bonus could not be consumed after replacement recovered");
    const auto lock_path=profile/"pc-heart-wallet.lock";const auto unlocked_bytes=bytes(ledger);
    held=CreateFileW(lock_path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"QA wallet-lock fixture failed");
    const auto lock_failed=wallet.claim(2,1);CloseHandle(held);
    need(lock_failed.status==w::Status::PersistenceFailed&&bytes(ledger)==unlocked_bytes,"Independent OS lock did not block a competing transaction");
#endif
    const auto legacy=out/"legacy-schema1";fs::create_directory(legacy);auto legacy_json=nlohmann::json::parse(full_bytes);legacy_json.erase("lastAttempt");put(legacy/w::filename,legacy_json.dump());w::Wallet legacy_wallet;
    need(legacy_wallet.load(legacy)&&legacy_wallet.snapshot().value.balance()==72&&legacy_wallet.snapshot().value.last_attempt.empty(),"Legacy schema1 wallet was reset or rejected");
    const auto scoped=out/"scoped-consume";fs::create_directory(scoped);put(scoped/w::filename,full_bytes);w::Wallet scoped_wallet;need(scoped_wallet.load(scoped),"Scoped consume fixture failed to load");
    need(scoped_wallet.purchase(w::Bonus::Life,1).committed()&&scoped_wallet.purchase(w::Bonus::Eggs,2).committed(),"Scoped pending purchases failed");
    const auto scoped_before=bytes(scoped/w::filename);need(scoped_wallet.consume_pending("bad",{true,false}).status==w::Status::InvalidAttempt&&bytes(scoped/w::filename)==scoped_before,"Invalid attempt token consumed a voucher");
    const std::string token_a(32,'a'),token_b(32,'b');
    need(scoped_wallet.consume_pending(token_a,{true,false}).committed()&&scoped_wallet.snapshot().value.pending==w::Pending{false,true},"Scoped consumption discarded another queued bonus");
    const auto scoped_once=bytes(scoped/w::filename);need(scoped_wallet.load(scoped)&&scoped_wallet.consume_pending(token_a,{true,false}).status==w::Status::AlreadyConsumed&&bytes(scoped/w::filename)==scoped_once,"Same native attempt consumed twice after reload");
    need(scoped_wallet.consume_pending(token_b,{true,false}).status==w::Status::PendingChanged&&bytes(scoped/w::filename)==scoped_once,"Missing applied flag was invented");
    need(scoped_wallet.consume_pending(token_b,{false,true}).committed()&&scoped_wallet.snapshot().value.pending.empty(),"Second actual attempt could not consume remaining egg voucher");
    const auto concurrent=out/"concurrent";fs::create_directory(concurrent);std::array<w::Wallet,12> wallets;
    for(auto& item:wallets)need(item.load(concurrent),"Concurrent wallet fixture failed to load");
    std::atomic_uint accepted{0};std::vector<std::thread> threads;
    for(unsigned n=0;n<wallets.size();++n)threads.emplace_back([&,n]{if(wallets[n].claim(7,2).committed())++accepted;});
    for(auto& thread:threads)thread.join();w::Wallet readback;
    need(accepted==1&&readback.load(concurrent)&&readback.snapshot().value.balance()==1,"Concurrent independent instances credited the same physical slot more than once");
    need(!wallet.load(out/"missing-profile")&&!wallet.snapshot().purchases_enabled(),"Missing profile was silently created");
    nlohmann::json result{{"passed",true},{"checks",checks},{"physicalSlots",72},{"allClaimInputsSynthetic",true},{"uniformitySmokeSamples",60000},{"reachableTwoOfFourSubsets",distribution.size()},{"stableSeedZeroDebitSlots",std::array<unsigned,5>{7,69,11,16,35}},{"spendReopensExactlySpentSlots",true},{"pendingPurchasesAtomic",true},{"nextAttemptBonusesConsumedOnce",true},{"invalidJsonPreserved",true},{"replacementFailureRollsBack",true},{"existingAchievementsUnchanged",true},{"gameLaunched",false},{"nativeCaptureIntegrated",false},{"personalProfileUsed",false},{"nativePhysicalIdentityClosed",false}};
    put(out/"verification.json",result.dump(2)+'\n');std::cout<<result.dump(2)<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
