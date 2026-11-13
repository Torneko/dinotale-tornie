#include "bonus_authoring0353.hpp"
#include "workshop_data.hpp"
#include <limits>
namespace workshop::bonus_authoring0353 {
namespace {
constexpr unsigned air_width=3,air_height=4;
// Native flat solid(0701) and flat one-way appui(0601), approached from above.
bool flat_floor(uint16_t value){return value==0x0701||value==0x0601;}
uint32_t crc(std::span<const uint8_t> bytes){uint32_t value=0xFFFFFFFF;for(auto b:bytes){value^=b;for(unsigned n=0;n<8;++n)value=(value>>1)^((value&1)?0xEDB88320:0);}return ~value;}
void word(std::vector<uint8_t>& b,size_t at,uint32_t value){terrain::word(b,at,value);}
void mix(std::vector<uint8_t>& b,uint32_t value){for(int n=24;n>=0;n-=8)b.push_back(uint8_t(value>>n));}
float f(uint32_t bits){return std::bit_cast<float>(bits);}uint32_t bits(float v){return std::bit_cast<uint32_t>(v);}
struct Grid {
    uint16_t resource=0;float x=0,y=0;unsigned width=0,height=0,loaded=0;
    std::vector<uint8_t> pages,collision;
    std::optional<CellEvidence> cell(int tx,int ty,bool floor=false)const{
        if(tx<0||ty<0||tx>=int(width/16)||ty>=int(height/16))return {};
        size_t pi=size_t(ty/16)*(width/256)+unsigned(tx/16);if(pi>=pages.size()||pages[pi]==255||pages[pi]>=loaded)return {};
        const size_t at=size_t(ty%16)*loaded*16+size_t(pages[pi])*16+unsigned(tx%16);if(at*2+2>collision.size())return {};
        return CellEvidence{resource,terrain::be16(collision,at*2),unsigned(at),tx,ty,floor};
    }
    std::optional<Clearance> prove(int tx,int ty)const{
        auto centre=cell(tx,ty,true);if(!centre||!flat_floor(centre->mask))return {};
        Clearance proof;proof.resource=resource;proof.floor_mask=centre->mask;proof.width_tiles=air_width;proof.height_tiles=air_height;proof.floor_x=tx;proof.floor_y=ty;proof.contact={bits(x+(float(tx)+0.5f)*16),bits(y+float(ty)*16)};
        for(int dx=-1;dx<=1;++dx){auto c=cell(tx+dx,ty,true);if(!c||c->mask!=proof.floor_mask)return {};if(!dx)proof.floor_cell=c->cell;proof.cells.push_back(*c);}
        for(int dy=-int(air_height);dy<0;++dy)for(int dx=-1;dx<=1;++dx){auto c=cell(tx+dx,ty+dy);if(!c||c->mask)return {};proof.cells.push_back(*c);}
        std::vector<uint8_t> evidence;mix(evidence,resource);mix(evidence,width);mix(evidence,height);mix(evidence,proof.contact[0]);mix(evidence,proof.contact[1]);
        for(const auto& c:proof.cells){mix(evidence,c.cell);mix(evidence,c.mask);mix(evidence,uint32_t(c.x));mix(evidence,uint32_t(c.y));mix(evidence,c.floor);}
        proof.crc32=crc(evidence);return proof;
    }
};
using Resources=std::map<uint16_t,std::pair<size_t,size_t>>;
Resources resources(std::span<const uint8_t> rom){Resources out;for(unsigned n=0;n<1582;++n){size_t at=0xA1C90+size_t(n)*12;out.emplace(u16(rom,at),std::pair{terrain::bank(rom,u32(rom,at+4)),terrain::bank(rom,u32(rom,at+8))});}return out;}
std::vector<Grid> grids(std::span<const uint8_t> rom,const bonus_rooms0351::Room& room,const Resources& resources){
    std::vector<Grid> out;for(const auto& item:room.placements){const auto kind=uint16_t(item.packed>>16);if(kind<0x8000||uint16_t(item.packed)!=1)continue;const size_t a=resources.at(kind).first;
        if(a+0x64>rom.size()||u16(rom,a)||u32(rom,a+8)!=0x8006E548||!u32(rom,a+0x4C))continue;
        const size_t d=terrain::bank(rom,u32(rom,a+0x20));require(u16(rom,d+2)==256&&u16(rom,d+4)==16&&u16(rom,d+6)==16,"Bonus portal terrain geometry unsupported");
        Grid g;g.resource=kind;g.x=f(item.point[0])-160;g.y=f(item.point[1])-120;g.width=u16(rom,d+8);g.height=u16(rom,d+10);require(g.width&&g.height&&g.width%256==0&&g.height%256==0,"Bonus portal terrain dimensions invalid");
        g.pages=terrain::block(rom,u32(rom,a+0x40));g.collision=terrain::block(rom,u32(rom,a+0x50));require(g.collision.size()%512==0&&g.collision.size()<=131072,"Bonus portal collision storage invalid");g.loaded=unsigned(g.collision.size()/512);require(g.loaded>0,"Bonus portal collision pages absent");out.push_back(std::move(g));
    }return out;
}
bool loaded(const std::vector<Grid>& maps,Point point){for(const auto& g:maps)if(g.cell(int(std::floor((f(point[0])-g.x)/16)),int(std::floor((f(point[1])-g.y)/16))))return true;return false;}
std::optional<Clearance> at(const std::vector<Grid>& maps,Point point,float foot){for(const auto& g:maps){float px=(f(point[0])-g.x-8)/16,py=(f(point[1])+foot-g.y)/16;if(px!=std::trunc(px)||py!=std::trunc(py))continue;if(auto proof=g.prove(int(px),int(py)))return proof;}return {};}
bool distinct(Point a,Point b,float foot){const float ax=f(a[0]),ay=f(a[1])+foot,bx=f(b[0]),by=f(b[1])+foot;return std::abs(ax-bx)>=air_width*16||std::abs(ay-by)>=air_height*16;}
std::vector<uint16_t> preloads(std::span<const uint8_t> rom,size_t header){std::vector<uint16_t> out;for(size_t at=0x60;at<0x100;at+=2){auto kind=u16(rom,header+at);if(kind==0xFFFF)return out;out.push_back(kind);}throw std::runtime_error("Bonus preload terminator absent inside native list");}
ContactCalibration calibration(const bonus_rooms0351::Catalog& catalog,const std::map<unsigned,std::vector<Grid>>& maps){
    for(const auto& room:catalog.rooms)for(const auto& edge:room.passages)if(edge.native.editable_pot)for(const auto& g:maps.at(room.id)){
        const auto point=edge.source.point;const int tx=int(std::floor((f(point[0])-g.x)/16)),first=int(std::floor((f(point[1])-g.y)/16));
        for(int ty=first;ty<=first+1;++ty){const float foot=g.y+float(ty)*16-f(point[1]);if(foot<=0||foot>16)continue;auto proof=g.prove(tx,ty);if(proof&&proof->contact[0]==point[0])return {room.id,edge.source.index,edge.native.kind,g.resource,point,bits(foot),*proof};}
    }
    throw std::runtime_error("Bonus pot contact calibration unavailable");
}
uint16_t template_kind(const bonus_rooms0351::Room&){return 0x4208;}
Site native_site(const bonus_rooms0351::Room& room,unsigned ordinal,const bonus_rooms0351::Placement& item,Provenance provenance,const std::vector<Grid>& maps,float foot,bool reuse){
    Site site;site.room=room.id;site.ordinal=ordinal;site.point=item.point;site.provenance=provenance;site.source_placement_index=item.index;site.source_packed=item.packed;site.template_kind=reuse?uint16_t(item.packed>>16):template_kind(room);site.collision=at(maps,site.point,foot);site.reuse_native_pot=reuse;site.needs_factory=!reuse;site.needs_dynamic_admission=!room.entry;return site;
}
std::vector<Site> sites(const bonus_rooms0351::Room& room,unsigned ordinal,const bonus_rooms0351::Catalog& catalog,const std::vector<Grid>& maps,float foot){
    std::vector<Site> out;std::set<Point> points;
    auto add=[&](const bonus_rooms0351::Placement& placement,Provenance provenance,bool reuse){if((f(placement.point[0])!=0||f(placement.point[1])!=0)&&loaded(maps,placement.point)&&points.insert(placement.point).second)out.push_back(native_site(room,ordinal,placement,provenance,maps,foot,reuse));};
    for(const auto& edge:room.passages)if(edge.native.editable_pot)add(edge.source,Provenance::NativePot,true);
    if(room.entry)add(room.entry->placement,Provenance::NativeArrival,false);
    for(const auto& other:catalog.rooms)for(const auto& edge:other.passages)if(edge.native.destination==room.id&&edge.native_target)add(*edge.native_target,Provenance::NativeArrival,false);
    for(const auto& candidate:room.candidates)add(candidate.target.placement,Provenance::NativeArrival,false);
    for(const auto& edge:room.passages)add(edge.source,Provenance::NativeExit,false);
    return out;
}
Plan compose(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const bonus_rooms0351::Catalog& catalog){
    const auto res=resources(original);std::map<unsigned,std::vector<Grid>> maps;for(const auto& room:catalog.rooms)maps.emplace(room.id,grids(original,room,res));
    Plan plan;plan.bonus=route.bonus;plan.seed=route.seed;plan.original_sha256=catalog.source_sha256;plan.calibration=calibration(catalog,maps);const float foot=f(plan.calibration.foot_offset_bits);plan.all_sites_present=true;
    for(unsigned ordinal=0;ordinal<route.steps.size();++ordinal){const auto& step=route.steps[ordinal];const auto* source=catalog.find(step.key.room);if(step.role!=bonus_routes0351::StepRole::Ordinary&&(!source||source->world!=route.bonus))continue;require(source&&source->course==step.key.course&&source->root==step.key.root&&source->world==route.bonus,"Bonus portal source provenance invalid");
        AuthoredRoom room;room.ordinal=ordinal;room.key=step.key;room.role=step.role;room.native_arrival=source->entry;room.dynamic_arrivals=source->candidates;room.needs_dynamic_admission=!source->entry;room.original_count=room.authored_static_count=unsigned(source->placements.size());room.preload_types.push_back(template_kind(*source));
        const auto& map=maps.at(source->id);auto candidates=sites(*source,ordinal,catalog,map,foot);
        if(!candidates.empty()){room.forward=candidates.front();room.forward->direction=Direction::Forward;}
        for(const auto& site:candidates)if(room.forward&&distinct(site.point,room.forward->point,foot)){room.backward=site;room.backward->direction=Direction::Backward;break;}
        // Only exact native floor/air cells may supply the missing site. The
        // point is a tile-centre contact calibrated from the native pot family.
        if(!room.forward||!room.backward){std::optional<Site> best;double distance=std::numeric_limits<double>::infinity();Point origin=source->entry?source->entry->placement.point:(room.forward?room.forward->point:Point{});
            for(const auto& g:map)for(unsigned y=air_height;y<g.height/16;++y)for(unsigned x=1;x+1<g.width/16;++x){auto proof=g.prove(int(x),int(y));if(!proof)continue;Point point{proof->contact[0],bits(f(proof->contact[1])-foot)};if(room.forward&&!distinct(point,room.forward->point,foot))continue;
                const double dx=double(f(point[0]))-f(origin[0]),dy=double(f(point[1]))-f(origin[1]),d=dx*dx+dy*dy;if(d>=distance)continue;
                Site site;site.room=source->id;site.ordinal=ordinal;site.point=point;site.provenance=Provenance::CollisionDerived;site.template_kind=template_kind(*source);site.collision=*proof;site.needs_dynamic_admission=room.needs_dynamic_admission;best=std::move(site);distance=d;
            }
            if(best){if(room.forward){room.backward=*best;room.backward->direction=Direction::Backward;}else{room.forward=*best;room.forward->direction=Direction::Forward;}}
        }
        room.needs_second_site=!room.forward||!room.backward;plan.all_sites_present=plan.all_sites_present&&!room.needs_second_site;
        for(const auto& edge:source->passages){bool selected=(room.forward&&room.forward->reuse_native_pot&&room.forward->source_placement_index==edge.source.index)||(room.backward&&room.backward->reuse_native_pot&&room.backward->source_placement_index==edge.source.index);if(!selected)room.unselected_native_exits.push_back(edge);}
        room.needs_native_exit_policy=!room.unselected_native_exits.empty();plan.rooms.push_back(std::move(room));
    }return plan;
}
}
Plan build(std::span<const uint8_t> original,const bonus_routes0351::Route& route){require(bonus_routes0351::validate(route),"Bonus authoring requires a canonical route");const auto catalog=bonus_rooms0351::read(original);return compose(original,route,catalog);}
Prepared clone_preloads(std::span<const uint8_t> original,std::span<const uint8_t> content,const bonus_routes0351::Route& route,std::span<const uint16_t> extras){
    require(extras.size()<=2,"Bonus preload extras exceed two types");const auto plan=build(original,route);const auto res=resources(original);require(content.size()>=0x1000000&&content.size()<=0x1400000&&u32(content,0)==0x80371240,"Bonus prepared content size/magic invalid");
    const uint32_t bank15=u32(content,0xA79D4+60);require(bank15==0||bank15==0x1000000,"Bonus bank15 belongs to another allocation");require(bank15||content.size()==0x1000000,"Bonus appended content requires established bank15");
    Prepared out;out.rom.assign(content.begin(),content.end());out.allocation_begin=content.size();
    for(const auto& room:plan.rooms){const auto id=room.key.room;const auto source_header=terrain::bank(original,u32(original,0xA84A0+id*8+4));const auto header=terrain::bank(content,u32(content,0xA84A0+id*8+4));require(header+0x118<=content.size(),"Bonus prepared header truncated");
        for(size_t at=0;at<0x118;++at)if(at<0x60||at>=0x100)require(content[header+at]==original[source_header+at],"Bonus prepared descriptor changed outside preload list");
        const auto inherited=preloads(original,source_header);auto list=preloads(content,header);require(list.size()>=inherited.size()&&std::equal(inherited.begin(),inherited.end(),list.begin()),"Bonus native preload prefix changed");const unsigned before=unsigned(list.size());
        for(uint16_t kind:extras){require(res.contains(kind),"Bonus preload type absent from original resources");if(std::find(list.begin(),list.end(),kind)==list.end())list.push_back(kind);}
        for(uint16_t kind:room.preload_types){require(res.contains(kind),"Bonus pot template resource missing");if(std::find(list.begin(),list.end(),kind)==list.end())list.push_back(kind);}
        require(list.size()-before<=2&&0x60+(list.size()+1)*2<=0x100,"Bonus native preload list capacity exceeded");if(list.size()==before)continue;
        for(size_t n=before+1;n<=list.size();++n)require(u16(content,header+0x60+n*2)==0,"Bonus preload padding occupied");
        const size_t cloned=(out.rom.size()+15)&~size_t(15),stamp=(cloned+0x118+15)&~size_t(15),end=stamp+32;require(end<=0x1400000,"Bonus bank15 append exhausted");out.rom.resize(end,0);std::copy_n(content.begin()+header,0x118,out.rom.begin()+cloned);
        for(size_t n=0;n<list.size();++n)terrain::half(out.rom,cloned+0x60+n*2,list[n]);terrain::half(out.rom,cloned+0x60+list.size()*2,0xFFFF);
        constexpr std::array<uint8_t,8> magic{'B','N','S','0','3','5','3',0};std::copy(magic.begin(),magic.end(),out.rom.begin()+stamp);word(out.rom,stamp+8,1);word(out.rom,stamp+12,id);word(out.rom,stamp+16,crc(original.subspan(source_header,0x118)));const uint32_t checksum=crc(std::span(out.rom).subspan(cloned,0x118));word(out.rom,stamp+20,checksum);word(out.rom,stamp+24,unsigned(list.size()));word(out.rom,stamp+28,0x118);
        word(out.rom,0xA79D4+60,0x1000000);word(out.rom,0xA84A0+id*8+4,0x0F000000u|uint32_t(cloned-0x1000000));out.clones.push_back({id,header,cloned,before,unsigned(list.size()),checksum});
    }out.allocation_end=out.rom.size();return out;
}
}
