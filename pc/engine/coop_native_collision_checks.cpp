#include "coop_native_collision.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
using namespace workshop::coop_collision;
constexpr uint32_t p0=0x80100000,p1=0x80101000,b0=0x80102000,c0=0x80103000,
    b1=0x80104000,c1=0x80105000,e0=0x80106000,e1=0x80107000,
    fruit=0x80108000,enemy=0x80109000,shadow=0x8010A000,find_egg=0x8010B000,
    unknown=0x8010C000,work=0x80300000;
unsigned assertions=0;
void need(bool value,const char* message){++assertions;if(!value)throw std::runtime_error(message);}
void put(uint8_t* r,uint32_t a,uint32_t v){auto* rdram=r;const gpr at=gpr(int32_t(a));MEM_W(0,at)=int32_t(v);}
void put_half(uint8_t* r,uint32_t a,uint16_t v){auto* rdram=r;const gpr at=gpr(int32_t(a));MEM_H(0,at)=v;}
void put_byte(uint8_t* r,uint32_t a,uint8_t v){auto* rdram=r;const gpr at=gpr(int32_t(a));MEM_B(0,at)=v;}
void make(uint8_t* r,uint32_t a,uint16_t kind,uint32_t parent=0,uint32_t data=0){
    put(r,a,0x19971110);put_half(r,a+12,kind);put_byte(r,a+6,1);
    put(r,a+0x180,parent);put(r,a+0x1F4,data);
}
void pair(uint8_t* r,uint32_t a,uint32_t b){put(r,work+4,a);put(r,work+0x3C,b);}
}

int main(int argc,char** argv){
    using namespace workshop::coop_collision;
    try{
        std::vector<uint8_t> memory(0x800000);auto* r=memory.data();Registry q;
        const auto create=[&](uint32_t a,uint16_t kind,uint32_t parent=0,uint32_t data=0){
            need(q.created(a),"Native creation was not admitted");make(r,a,kind,parent,data);
        };
        make(r,p0,0x4001,0,0x80200000);make(r,p1,0x4001,0,0x80202000);
        pair(r,p0,p1);
        need(!q.enabled() && !q.created(p0) && !q.bind_root(r,0,p0),"Default-off accepted native ownership");
        need(!q.ignore_geometry(r,0x8006009C,work),"Default-off suppressed native collision");
        q.configure(true);
        need(!q.bind_root(r,0,p0),"An unobserved native birth was silently adopted");
        create(p0,0x4001,0,0x80200000);create(p1,0x4001,0,0x80202000);
        need(q.bind_root(r,0,p0),"Primary root failed");
        need(!q.bind_root(r,1,p0),"One native root occupied both slots");
        need(!q.ignore_geometry(r,0x8006009C,work),"Filter ran before both roots were bound");
        need(q.bind_root(r,1,p1),"Secondary root failed");
        put_byte(r,p0+6,0);put_byte(r,p1+6,0);
        need(q.bind_root(r,0,p0) && q.bind_root(r,1,p1) && q.ignore_geometry(r,0x8006009C,work),"Native allocated life0 actors were rejected");
        need(!q.bind_root(r,2,p1),"Invalid root slot accepted");
        create(b0,0x400B,p0,0x80204000);create(c0,0x400C,b0,0x80204100);
        create(b1,0x400B,p1,0x80204200);create(c1,0x400C,b1,0x80204300);
        for(auto a:{b0,c0,b1,c1})need(q.register_body(r,a),"Native body graph failed registration");
        create(e0,0x4002);create(e1,0x4002);
        need(q.egg_created(r,e0,0) && q.egg_created(r,e1,1),"Actual creation-result egg registration failed");
        create(fruit,0x4020,p0);create(enemy,0x4030,p1);create(shadow,0x4007,p1);
        create(find_egg,0x401D,p1);make(r,unknown,0x4002);
        for(auto a:{fruit,enemy,shadow,find_egg}){
            need(!q.register_body(r,a),"A carried non-body actor became a player body");
            need(!q.egg_created(r,a,0),"A non-projectile actor became an ordinary egg");
        }
        need(!q.egg_created(r,0,0) && !q.egg_created(r,unknown,0) && !q.egg_created(r,e0,UINT32_MAX),"Unknown/failed egg result accepted");
        constexpr uint32_t functions[]={0x8006009C,0x80060274,0x800603E8,0x80060798,0x80060960};
        const std::array<uint32_t,3> body0={p0,b0,c0},body1={p1,b1,c1};
        for(auto fn:functions){
            for(auto a:body0)for(auto b:body1){
                pair(r,a,b);need(q.ignore_geometry(r,fn,work),"Body pair was not transparent");
                pair(r,b,a);need(q.ignore_geometry(r,fn,work),"Reversed body pair was not transparent");
            }
            for(auto target:body1){pair(r,e0,target);need(q.ignore_geometry(r,fn,work),"P1 egg hit teammate body");pair(r,target,e0);need(q.ignore_geometry(r,fn,work),"Reversed P1 egg pair failed");}
            for(auto target:body0){pair(r,e1,target);need(q.ignore_geometry(r,fn,work),"P2 egg hit teammate body");pair(r,target,e1);need(q.ignore_geometry(r,fn,work),"Reversed P2 egg pair failed");}
            for(auto other:{p0,b0,c0,e0,fruit,enemy,shadow,find_egg,unknown}){
                pair(r,e0,other);need(!q.ignore_geometry(r,fn,work),"Egg suppressed an own/foreign target");
            }
            pair(r,p0,c0);need(!q.ignore_geometry(r,fn,work),"Same-owner native body pair was suppressed");
            pair(r,enemy,p0);need(!q.ignore_geometry(r,fn,work),"Foreign enemy contact was suppressed");
            pair(r,p0,p1);const auto before=memory;recomp_context c{},expected{};
            std::memset(&c,0xA5,sizeof(c));c.r4=gpr(int32_t(work));expected=c;expected.r2=0;
            need(q.filter_geometry(r,&c,fn),"Filtered wrapper result not reported");
            need(std::memcmp(&c,&expected,sizeof(c))==0,"Filter changed native registers besides V0");
            need(memory==before,"Pair predicate wrote native collision/body/health memory");
        }
        need(q.statistics().body_rejections==5 && !q.statistics().egg_rejections,"Actual gate counters included predicate-only reads");
        for(auto n:q.statistics().geometry_rejections)need(n==1,"A geometry rejection counter was missed");
        need(q.statistics().last_actor_a==p0 && q.statistics().last_actor_b==p1 &&
             q.statistics().last_owner_a==0 && q.statistics().last_owner_b==1,"Last pair diagnostic lost its owners");
        q.interactions(true);need(q.interactions(),"Interactions setting did not activate");
        for(auto fn:functions){
            pair(r,p0,p1);need(!q.ignore_geometry(r,fn,work),"Enabled body interactions remained suppressed");
            pair(r,e0,p1);need(q.ignore_geometry(r,fn,work),"Interactions option enabled friendly egg collision");
            pair(r,p0,e1);need(q.ignore_geometry(r,fn,work),"Interactions option enabled reverse friendly egg collision");
        }
        pair(r,e0,p1);recomp_context egg_context{};egg_context.r4=gpr(int32_t(work));
        need(q.filter_geometry(r,&egg_context,0x8006009C) && q.statistics().egg_rejections==1 &&
             q.statistics().last_reason==Rejection::egg,"Actual egg gate counter failed");
        q.interactions(false);
        // Native attachment proof, persistence through48060, owner transfer.
        create(unknown,0x4002);
        need(!q.attached(r,unknown,p0),"Unperformed native attachment was accepted");
        put(r,unknown+0x180,p0);
        need(q.attached(r,unknown,p0) && q.before_detach(r,unknown),"Successful native egg attachment not captured");
        put(r,unknown+0x180,0);
        need(q.egg_owner(r,unknown)==0 && q.before_detach(r,unknown),"Throw/detach erased projectile owner");
        pair(r,unknown,p1);need(q.ignore_geometry(r,0x8006009C,work),"Detached egg lost teammate transparency");
        put(r,unknown+0x180,p1);need(!q.attached(r,unknown,p0),"Mismatched claimed parent accepted");
        need(q.attached(r,unknown,p1) && q.egg_owner(r,unknown)==1,"Actual native reattachment did not transfer owner");
        put(r,unknown+0x180,0);
        pair(r,unknown,p0);need(q.ignore_geometry(r,0x8006009C,work),"Transferred projectile hit new teammate");
        // Reuse even at identical address/kind/private must erase old ownership.
        need(q.created(unknown),"Recycled birth rejected");
        make(r,unknown,0x4002);
        need(!q.egg_owner(r,unknown),"Recycled pool address inherited a previous egg owner");
        pair(r,unknown,p0);need(!q.ignore_geometry(r,0x8006009C,work),"Unattributed recycled egg suppressed a contact");
        need(q.egg_created(r,unknown,0),"New pool incarnation could not be attributed");
        q.forgotten(unknown);q.forgotten(unknown);
        need(!q.egg_owner(r,unknown) && !q.egg_created(r,unknown,0),"Freed identity was still usable");
        put(r,e0+0x1F4,0x80206000);pair(r,e0,p1);
        need(!q.egg_owner(r,e0) && !q.ignore_geometry(r,0x8006009C,work),"Private-data change retained egg ownership");
        need(!q.egg_created(r,e0,0),"Egg registration legitimized unobserved private-data change");
        put(r,e0+0x1F4,0);
        // Native3AE00 completion is the only allowed private0 -> embedded14
        // transition. No lifecycle check is relaxed for an arbitrary pointer.
        Registry finalized;finalized.configure(true);
        need(finalized.created(p0) && finalized.created(p1) &&
             finalized.bind_root(r,0,p0) && finalized.bind_root(r,1,p1),"Private finalizer roots failed");
        const auto prepare_egg=[&]{
            need(finalized.created(unknown),"Finalizer egg birth failed");
            make(r,unknown,0x4002);put_byte(r,unknown+0x1F2,0);
            need(finalized.egg_created(r,unknown,0),"Finalizer known owner failed");
        };
        const auto native_private=[&]{
            // Actual45A34(size14) chooses embedded actor+24C, flag1F2=1.
            put(r,unknown+0x1F4,unknown+0x24C);put_byte(r,unknown+0x1F2,1);
            for(unsigned i=0;i<0x14;i++)put_byte(r,unknown+0x24C+i,0);
        };
        prepare_egg();
        auto init=finalized.begin_egg_private_init(r,unknown);
        need(init!=0 && finalized.begin_egg_private_init(r,unknown)==0,"Missing/duplicate initializer ticket admitted");
        native_private();
        need(!finalized.egg_owner(r,unknown),"Private change was accepted before the controlled completion");
        const auto before_finalize=memory;
        need(finalized.finish_egg_private_init(r,init) && finalized.egg_owner(r,unknown)==0,"Native embedded14 finalization lost its owner");
        need(memory==before_finalize && finalized.statistics().private_finalizations==1,"Finalization wrote guest memory or missed its counter");
        need(!finalized.finish_egg_private_init(r,init) && !finalized.begin_egg_private_init(r,unknown),"Consumed/already initialized egg ticket was reused");
        pair(r,unknown,p1);need(finalized.ignore_geometry(r,0x8006009C,work),"Finalized egg hit teammate");
        put(r,unknown+0x180,p0);need(finalized.before_detach(r,unknown),"Finalized egg attachment was not captured");
        put(r,unknown+0x180,0);need(finalized.egg_owner(r,unknown)==0,"Finalized egg lost its owner after throw");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);
        need(!finalized.finish_egg_private_init(r,init) && !finalized.finish_egg_private_init(r,init),"Unperformed initializer was completed/replayed");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();
        put(r,unknown+0x1F4,0x80206000);
        need(!finalized.finish_egg_private_init(r,init) && !finalized.egg_owner(r,unknown),"Arbitrary private allocation was legitimized");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();put_byte(r,unknown+0x1F2,0);
        need(!finalized.finish_egg_private_init(r,init),"Missing embedded allocation flag accepted");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();put_byte(r,unknown+0x24C+0x13,1);
        need(!finalized.finish_egg_private_init(r,init),"Incomplete native private zeroing accepted");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();put_half(r,unknown+12,0x401D);
        need(!finalized.finish_egg_private_init(r,init),"Changed native kind inherited ordinary egg owner");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);
        finalized.forgotten(unknown);prepare_egg();native_private();
        need(!finalized.finish_egg_private_init(r,init),"Old ticket finalized a freed/recreated pool incarnation");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();
        put(r,p0+0x1F4,0x80207000);
        need(!finalized.finish_egg_private_init(r,init),"Changed root identity finalized an old ownership ticket");put(r,p0+0x1F4,0x80200000);
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);
        put(r,unknown+0x180,p1);need(finalized.attached(r,unknown,p1),"Owner transfer during initializer fixture failed");native_private();
        need(!finalized.finish_egg_private_init(r,init),"Initializer overwrote an intervening native owner transfer");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();finalized.cancel_egg_private_init(init);
        need(!finalized.finish_egg_private_init(r,init),"Cancelled initializer ticket remained usable");
        prepare_egg();init=finalized.begin_egg_private_init(r,unknown);native_private();finalized.configure(true);
        need(!finalized.finish_egg_private_init(r,init) && !finalized.begin_egg_private_init(r,unknown),"Old session initializer adopted an unattributed actor");
        need(!finalized.finish_egg_private_init(r,0) && !finalized.finish_egg_private_init(nullptr,UINT64_MAX),"Invalid initializer token accepted");
        // Body graph and owner identity must stay valid, not just matching kind.
        put(r,c0+0x180,enemy);pair(r,c0,p1);
        need(!q.body_owner(r,c0) && !q.ignore_geometry(r,0x8006009C,work),"A body reparented through enemy was accepted");
        put(r,c0+0x180,b0);put(r,b0+0x180,c0);
        need(!q.register_body(r,c0) && !q.body_owner(r,c0),"Cyclic parent graph accepted");
        put(r,b0+0x180,p0);put(r,c0+0x1F4,0x80204400);
        need(!q.body_owner(r,c0) && !q.register_body(r,c0),"Private-data change retained body ownership");
        put(r,c0+0x1F4,0x80204100);put_byte(r,b1+6,3);
        pair(r,b1,p0);need(!q.ignore_geometry(r,0x8006009C,work),"Freed body filtered a contact");
        put_byte(r,b1+6,1);
        put_byte(r,b1+6,4);pair(r,b1,p0);
        need(!q.ignore_geometry(r,0x8006009C,work),"Invalid life marker accepted");put_byte(r,b1+6,1);
        pair(r,p0,p1);
        for(auto fn:{0u,0x80060624u,0x800608D4u,0x80060ED0u})need(!q.ignore_geometry(r,fn,work),"Unaudited collision function was filtered");
        for(auto a:{0u,0x1000u,work+1,0x807FFF80u,0xFFFFFFFCu})need(!q.ignore_geometry(r,0x8006009C,a),"Invalid work pointer accepted");
        for(auto a:{0u,0x1000u,p0+1,0x807FFF80u,0xFFFFFFFCu}){pair(r,a,p1);need(!q.ignore_geometry(r,0x8006009C,work),"Invalid actor pointer accepted");}
        need(!q.ignore_geometry(nullptr,0x8006009C,work) && !q.filter_geometry(r,nullptr,0x8006009C),"Null native memory/context accepted");
        pair(r,p0,p1);put(r,p0+0x1F4,0x80207000);
        need(!q.ignore_geometry(r,0x8006009C,work) && !q.bind_root(r,0,p0),"Changed root identity was silently rebound");
        put(r,p0+0x1F4,0x80200000);q.forgotten(p1);
        need(!q.ignore_geometry(r,0x8006009C,work) && !q.egg_owner(r,e1),"Absent owner root retained active filtering");
        create(p1,0x4001,0,0x80202000);need(q.bind_root(r,1,p1),"Recreated root failed binding");
        need(!q.egg_owner(r,e1) && !q.body_owner(r,b1),"New owner incarnation inherited prior children/projectiles");
        q.configure();need(!q.enabled() && !q.ignore_geometry(r,0x8006009C,work),"Configuration reset left collision filter active");
        q.configure(true);need(!q.bind_root(r,0,p0),"New session inherited old actor births");
        // Event registry is bounded; overflow disables this optional prototype.
        Registry bounded;bounded.configure(true);
        for(unsigned i=0;i<1024;i++)need(bounded.created(0x80400000+i*0x200),"Bounded registry rejected a valid slot too early");
        need(!bounded.created(0x80600000) && !bounded.enabled(),"Corrupt unbounded birth stream remained active");
        // Real word-swapped RDRAM from a completed QA game run. The helper is
        // executed against a private copy; no native routine is run here.
        need(argc==2,"An existing QA RDRAM fixture is required");
        std::ifstream stream(argv[1],std::ios::binary);
        std::vector<uint8_t> snapshot(0x800000);
        need(bool(stream.read(reinterpret_cast<char*>(snapshot.data()),snapshot.size())) && stream.peek()==std::char_traits<char>::eof(),"QA snapshot is not exactly8MiB");
        auto* native=snapshot.data();Registry from_native;from_native.configure(true);
        std::vector<uint32_t> native_roots,native_bodies;
        for(uint32_t a=0x80000400;a<=0x807FFE00;a+=4){
            if(!actor(native,a))continue;
            const auto kind=half(native,a+12);
            if(kind!=0x4001 && kind!=0x400B && kind!=0x400C)continue;
            need(from_native.created(a),"QA actor birth seed rejected");
            if(kind==0x4001)native_roots.push_back(a);else native_bodies.push_back(a);
        }
        need(native_roots.size()==2,"Real QA fixture does not contain two live4001 roots");
        if(native_roots[1]==word(native,0x800F8828))std::swap(native_roots[0],native_roots[1]);
        need(native_roots[0]==word(native,0x800F8828),"Real QA singleton is not one of the two roots");
        for(unsigned slot=0;slot<2;slot++){
            need(byte(native,native_roots[slot]+6)==0,"Real QA actor no longer has the native life0 allocation marker");
            need(from_native.bind_root(native,slot,native_roots[slot]),"Real native root/private identity rejected");
        }
        unsigned registered_bodies=0;
        for(auto a:native_bodies)if(from_native.register_body(native,a)){
            registered_bodies++;
            need(byte(native,a+6)==0,"Real body allocation marker differed from audit");
        }
        need(registered_bodies==4,"Real QA fixture did not register exactly two400B/400C body graphs");
        const auto native_before=snapshot;
        // Work storage below is a QA overlay only, not a native call or actual
        // contact. Preserve every byte except these declared pair pointers.
        pair(native,native_roots[0],native_roots[1]);
        auto expected_snapshot=native_before;
        pair(expected_snapshot.data(),native_roots[0],native_roots[1]);
        for(auto fn:functions)need(from_native.ignore_geometry(native,fn,work),"Real native life0 root pair was not filtered");
        need(snapshot==expected_snapshot,"Helper changed real native snapshot bytes");
        std::cout<<"Real QA memory fixture: primary="<<std::hex<<native_roots[0]<<" secondary="<<native_roots[1]<<std::dec<<" bodyComponents="<<registered_bodies<<" allocationMarker=0; pair-work pointers are a declared QA overlay.\n";
        std::cout<<"Native collision helper checks PASS: "<<assertions<<" assertions; five geometry gates, both player/egg directions, persistent ownership, generations and read-only guest memory. Native geometry/gameplay was not executed.\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
