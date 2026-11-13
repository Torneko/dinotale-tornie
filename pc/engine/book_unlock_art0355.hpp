#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>
namespace workshop::book_unlock_art0355 {
struct Point {float x,y;};
} // namespace workshop::book_unlock_art0355
#include "book_mask_refinement0355.hpp"
namespace workshop::book_unlock_art0355 {
struct Layer {unsigned color=0,x=0,y=0,width=0,height=0;std::vector<char> rgba;};
// Presentation masks traced over the approved 1448x1086 illustration. The PNG
// itself is never rewritten. Fruit, scenery and the left page are excluded.
inline constexpr Point orange[]{
{892,208},{898,204},{908,203},{916,205},{925,210},{932,217},{937,227},{939,238},{948,229},{960,223},{972,220},{986,221},{1001,227},{1014,236},{1025,250},{1033,266},{1036,283},{1034,300},{1028,314},{1017,326},{1004,334},{988,340},{971,341},{962,338},{961,350},{956,360},{953,365},{961,365},{971,372},{977,383},{978,394},{971,403},{962,405},{957,400},{955,411},{952,420},{960,409},{966,399},{975,392},{986,392},{995,398},{1001,407},{1004,418},{1001,428},{992,441},{980,453},{968,464},{955,472},{943,472},{937,468},{932,459},{932,452},{925,452},{919,447},{916,438},{909,447},{900,452},{887,453},{877,450},{871,459},{876,466},{881,478},{880,492},{873,505},{862,514},{847,517},{832,513},{820,507},{811,494},{807,477},{810,460},{818,447},{830,436},{839,431},{847,431},{851,423},{837,424},{828,418},{822,415},{821,404},{822,392},{830,396},{839,397},{847,393},{853,384},{860,379},{866,368},{863,359},{849,355},{836,352},{825,349},{817,343},{813,335},{814,326},{818,323},{819,315},{825,311},{830,311},{829,305},{836,304},{845,312},{851,321},{862,326},{875,329},{877,325},{867,317},{859,307},{850,304},{843,299},{840,288},{842,278},{847,270},{859,265},{854,262},{849,253},{849,244},{854,239},{861,239},{867,245},{867,230},{872,218},{883,210}};
inline constexpr Point violet[]{
{1213,239},{1222,237},{1237,239},{1249,248},{1256,266},{1256,287},{1264,292},{1269,290},{1275,283},{1276,275},{1272,267},{1264,264},{1275,265},{1282,272},{1282,281},{1276,290},{1281,296},{1285,306},{1284,317},{1279,328},{1270,332},{1275,339},{1276,352},{1270,359},{1287,351},{1301,340},{1310,334},{1310,326},{1316,325},{1322,331},{1322,339},{1328,332},{1336,333},{1338,340},{1331,349},{1340,344},{1348,347},{1348,354},{1341,362},{1327,371},{1316,376},{1302,383},{1295,394},{1300,403},{1303,400},{1308,395},{1314,398},{1317,409},{1317,420},{1312,430},{1303,437},{1306,445},{1321,444},{1334,451},{1341,465},{1344,482},{1342,501},{1334,516},{1323,525},{1309,529},{1296,525},{1286,518},{1280,508},{1278,495},{1276,484},{1269,475},{1255,470},{1245,468},{1238,461},{1237,467},{1223,468},{1213,465},{1224,458},{1228,471},{1224,489},{1216,499},{1206,506},{1193,509},{1161,507},{1147,503},{1134,497},{1124,490},{1116,481},{1111,469},{1113,457},{1121,447},{1133,441},{1150,438},{1167,442},{1183,450},{1192,448},{1196,439},{1195,425},{1194,413},{1199,400},{1177,402},{1162,403},{1151,399},{1144,393},{1143,385},{1146,379},{1142,374},{1145,367},{1152,364},{1162,368},{1167,373},{1178,375},{1188,372},{1185,366},{1177,362},{1168,359},{1152,358},{1136,354},{1121,346},{1111,335},{1105,320},{1104,306},{1108,290},{1116,277},{1127,269},{1143,263},{1159,263},{1174,265},{1187,272},{1193,256},{1201,245}};
inline constexpr Point brown[]{
{876,766},{889,761},{902,763},{913,768},{923,778},{928,789},{936,780},{950,773},{964,769},{982,771},{998,778},{1012,789},{1020,803},{1024,820},{1022,836},{1015,850},{1005,859},{989,866},{974,870},{960,868},{953,883},{942,890},{931,895},{947,895},{961,893},{977,885},{988,883},{997,886},{1006,891},{1006,899},{1012,904},{1010,916},{1001,919},{989,919},{1004,918},{1021,919},{1035,928},{1043,943},{1043,960},{1036,977},{1024,990},{1008,995},{993,992},{980,984},{975,972},{971,960},{969,950},{960,959},{946,964},{950,974},{947,990},{941,1003},{932,1009},{916,1013},{900,1009},{889,1001},{884,992},{880,978},{878,963},{868,964},{858,962},{852,953},{850,941},{852,930},{858,922},{863,914},{864,901},{858,908},{852,909},{848,904},{846,894},{848,882},{854,869},{860,860},{869,855},{859,862},{851,861},{843,855},{839,846},{840,836},{846,825},{853,822},{847,818},{840,813},{838,805},{840,795},{846,789},{855,787},{862,790},{860,783},{864,775},{872,769}};
inline constexpr std::array<std::span<const Point>,6> shapes{orange,violet,turquoise_refined,lime_refined,brown,green_refined};
inline bool inside(std::span<const Point> shape,float x,float y){
 bool value=false;for(size_t i=0,j=shape.size()-1;i<shape.size();j=i++){
  const auto a=shape[i],b=shape[j];
  if(((a.y>y)!=(b.y>y))&&x<(b.x-a.x)*(y-a.y)/(b.y-a.y)+a.x)value=!value;
 }return value;
}
inline float edge_distance2(std::span<const Point> shape,float x,float y){
 float best=1.e20f;
 for(size_t i=0,j=shape.size()-1;i<shape.size();j=i++){
  const auto a=shape[j],b=shape[i];const float dx=b.x-a.x,dy=b.y-a.y,len=dx*dx+dy*dy;
  const float t=len?std::clamp(((x-a.x)*dx+(y-a.y)*dy)/len,0.f,1.f):0.f;
  const float ex=x-a.x-t*dx,ey=y-a.y-t*dy;best=std::min(best,ex*ex+ey*ey);
 }return best;
}
inline bool warm_character_pixel(unsigned color,unsigned r,unsigned g,unsigned b,unsigned x,unsigned y){
 const auto hi=std::max({r,g,b}),lo=std::min({r,g,b});
 const bool neutral=(hi-lo<=20&&hi<100)||(lo>80&&r*100>=g*98&&g*100>=b*98&&hi-lo<90);
 if(color==8)return neutral||(r*100>g*112&&r*100>b*115)||(y>=385&&b*100>r*112&&b*100>g*105);
 if(color==9){if(x>1338)return false;return neutral||(r*100>b*50&&b*100>r*85&&b*100>g*125)||(r*100>g*125&&r*100>b*125)||(r*100>=g*98&&g*100>b*125&&r*100>b*115);}
 if(color==12){const float dx=float(x)-905.f,dy=float(y)-825.f,bx=float(x)-934.f,by=float(y)-916.f;const bool face_or_belly=dx*dx/(52.f*52.f)+dy*dy/(60.f*60.f)<=1.f||bx*bx/(40.f*40.f)+by*by/(50.f*50.f)<=1.f;const bool cream=(hi-lo<=20&&r*100>=g*98&&g*100>=b*98)||(lo>80&&r*100>=g*104&&g*100>=b*108&&hi-lo<100);return (face_or_belly&&neutral)||cream||(r*100>=g*112&&g*100>b*105)||(r*100>g*120&&r*100>b*120);}
 return true;
}
inline bool warm_pigment(unsigned color,unsigned r,unsigned g,unsigned b,unsigned y){
 if(color==8)return (r*100>g*112&&r*100>b*115)||(y>=385&&b*100>r*112&&b*100>g*105);
 if(color==9)return (r*100>b*50&&b*100>r*85&&b*100>g*125)||(r*100>g*125&&r*100>b*125);
 if(color==12)return (r*100>=g*112&&g*100>b*105)||(r*100>g*120&&r*100>b*120);
 return false;
}
inline bool cool_edge_pixel(unsigned color,unsigned r,unsigned g,unsigned b,unsigned y){
 const auto hi=std::max({r,g,b}),lo=std::min({r,g,b});
 const bool neutral=(hi-lo<=20&&hi<100)||(lo>80&&r*100>=g*98&&g*100>=b*98&&hi-lo<90);
 const bool red=r*100>g*125&&r*100>b*125,pink=r*100>g*125&&b*100>g*120;
 const bool brown=r*100>g*110&&g*100>b*110;
 if(color==10)return neutral||red||pink||(g*100>r*125&&b*100>r*125&&(y>550||g*100>b*85));
 if(color==11)return neutral||red||brown;
 if(color==13)return neutral||red||pink||(g*100>r*160&&g*100>b*110);
 return false;
}
inline bool fruit_area(unsigned x,unsigned y){
 // All six approved still-life fruits and their leaves/stems are excluded
 // from character desaturation, including the peach beside the violet boot.
 return (x>=1014&&x<=1118&&y>=408&&y<=523)
  ||(x>=1052&&x<=1135&&y>=548&&y<=637)
  ||(x>=1049&&x<=1139&&y>=906&&y<=1008)
  ||(x>=773&&x<=852&&y>=889&&y<=995)
  ||(x>=1340&&x<=1440&&y>=460&&y<=590)
  ||(x>=1347&&x<=1447&&y>=628&&y<=788);
}
inline std::vector<Layer> layers(std::span<const unsigned char> source,unsigned width,unsigned height){
 std::vector<Layer> result;
 if(width!=1448||height!=1086||source.size()!=size_t(width)*height*4)return result;
 std::vector<unsigned char> owner(size_t(width)*height,255);
 std::vector<float> priority(size_t(width)*height,1.e20f);
 for(unsigned i=0;i<shapes.size();++i){
  unsigned left=width,top=height,right=0,bottom=0;
  for(auto p:shapes[i]){left=std::min(left,unsigned(p.x));top=std::min(top,unsigned(p.y));right=std::max(right,unsigned(p.x)+2);bottom=std::max(bottom,unsigned(p.y)+2);}
  const bool warm=i==0||i==1||i==4;const unsigned margin=warm?19:5;left-=margin;top-=margin;right+=margin-1;bottom+=margin-1;Layer out{8+i,left,top,right-left,bottom-top,{}};
  out.rgba.resize(size_t(out.width)*out.height*4,0);
  for(unsigned y=top;y<bottom;++y)for(unsigned x=left;x<right;++x){
   if(fruit_area(x,y))continue;
   const auto a=(size_t(y)*width+x)*4,b=(size_t(y-top)*out.width+x-left)*4;
   if(warm&&!warm_character_pixel(8+i,source[a],source[a+1],source[a+2],x,y))continue;
   const bool allow_edge=warm||cool_edge_pixel(8+i,source[a],source[a+1],source[a+2],y);
   const float pigment_radius2=i==1?324.f:i==0&&y<390?324.f:i==4?256.f:144.f;
   const float radius2=warm?(warm_pigment(8+i,source[a],source[a+1],source[a+2],y)?pigment_radius2:36.f):16.f;
   unsigned covered=0;for(float dy:{.25f,.75f})for(float dx:{.25f,.75f})covered+=inside(shapes[i],x+dx,y+dy)||(allow_edge&&edge_distance2(shapes[i],x+dx,y+dy)<=radius2);
   if(!covered)continue;
   const auto pixel=size_t(y)*width+x;
   const float rank=inside(shapes[i],x+.5f,y+.5f)?-1.f:edge_distance2(shapes[i],x+.5f,y+.5f);
   if(owner[pixel]!=255){
    if(rank>=priority[pixel])continue;
    auto& previous=result[owner[pixel]];const auto prior=(size_t(y-previous.y)*previous.width+x-previous.x)*4;
    previous.rgba[prior+3]=0;
   }
   owner[pixel]=uint8_t(i);priority[pixel]=rank;
   const auto grey=unsigned((54u*source[a]+183u*source[a+1]+19u*source[a+2]+128u)>>8);
   out.rgba[b]=out.rgba[b+1]=out.rgba[b+2]=char(grey);out.rgba[b+3]=char(source[a+3]*covered/4);
  }result.push_back(std::move(out));
 }return result;
}
} // namespace workshop::book_unlock_art0355
