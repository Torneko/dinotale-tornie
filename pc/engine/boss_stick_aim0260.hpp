#pragma once
// Diagnostic controller arithmetic only. Native game state is never written.
// Source model: ultramodern/input.cpp convert_to_n64_range; native22B20 and
// 22A58. Only an ordinary host controller vector is produced. No RDRAM API.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
namespace workshop::boss_stick_aim0260 {
struct Axes {int x=0,y=0;};
struct Host {float x=0,y=0;};
inline constexpr float pi=3.14159265358979323846f,alpha=1.39414574f;
inline bool valid(Axes old){return old.x>=-54&&old.x<=54&&old.y>=-54&&old.y<=54;}
inline int filter(int raw,int old){const int sum=raw+old,half=sum>=0?sum/2:-((-sum+1)/2);return half>14?std::min(half-14,54):half< -14?std::max(half+14,-54):0;}
inline Axes convert(Host input){
 float magnitude=std::min(std::sqrt(input.x*input.x+input.y*input.y),1.f),angle=std::atan2(input.y,input.x),mod=std::fmod(angle+2*pi,pi/2);
 if(mod>pi/4)mod=pi/2-mod;
 const float radius=82.f*std::sin(alpha)/std::sin(pi-mod-alpha),scaled=magnitude*radius;
 return {int(int8_t(std::cos(angle)*scaled)),int(int8_t(std::sin(angle)*scaled))};
}
// Native SDK trigonometry is quantized, not continuous std::sin/cos.
// Immutable quarter-wave SINS values at800AA3B0; source funcs60/61.
inline constexpr std::array<int16_t,1024> native_sines{
 0,50,100,150,201,251,301,352,402,452,503,553,603,654,704,754,
 804,855,905,955,1006,1056,1106,1156,1207,1257,1307,1358,1408,1458,1508,1559,
 1609,1659,1709,1760,1810,1860,1910,1961,2011,2061,2111,2161,2212,2262,2312,2362,
 2412,2463,2513,2563,2613,2663,2713,2763,2814,2864,2914,2964,3014,3064,3114,3164,
 3214,3264,3314,3365,3415,3465,3515,3565,3615,3665,3715,3765,3815,3865,3915,3964,
 4014,4064,4114,4164,4214,4264,4314,4364,4414,4463,4513,4563,4613,4663,4713,4762,
 4812,4862,4912,4961,5011,5061,5110,5160,5210,5260,5309,5359,5408,5458,5508,5557,
 5607,5656,5706,5755,5805,5855,5904,5953,6003,6052,6102,6151,6201,6250,6299,6349,
 6398,6448,6497,6546,6595,6645,6694,6743,6792,6842,6891,6940,6989,7038,7087,7137,
 7186,7235,7284,7333,7382,7431,7480,7529,7578,7627,7676,7725,7773,7822,7871,7920,
 7969,8018,8066,8115,8164,8213,8261,8310,8359,8407,8456,8505,8553,8602,8650,8699,
 8747,8796,8844,8893,8941,8989,9038,9086,9134,9183,9231,9279,9328,9376,9424,9472,
 9520,9568,9617,9665,9713,9761,9809,9857,9905,9953,10001,10048,10096,10144,10192,10240,
 10288,10335,10383,10431,10478,10526,10574,10621,10669,10717,10764,10812,10859,10907,10954,11001,
 11049,11096,11143,11191,11238,11285,11332,11380,11427,11474,11521,11568,11615,11662,11709,11756,
 11803,11850,11897,11944,11991,12038,12084,12131,12178,12224,12271,12318,12364,12411,12457,12504,
 12551,12597,12643,12690,12736,12783,12829,12875,12921,12968,13014,13060,13106,13152,13198,13244,
 13290,13336,13382,13428,13474,13520,13566,13611,13657,13703,13749,13794,13840,13885,13931,13976,
 14022,14067,14113,14158,14204,14249,14294,14339,14385,14430,14475,14520,14565,14610,14655,14700,
 14745,14790,14835,14880,14925,14969,15014,15059,15104,15148,15193,15237,15282,15326,15371,15415,
 15460,15504,15548,15593,15637,15681,15725,15769,15813,15857,15901,15945,15989,16033,16077,16121,
 16165,16208,16252,16296,16339,16383,16427,16470,16514,16557,16600,16644,16687,16730,16774,16817,
 16860,16903,16946,16989,17032,17075,17118,17161,17204,17247,17289,17332,17375,17417,17460,17503,
 17545,17588,17630,17672,17715,17757,17799,17841,17884,17926,17968,18010,18052,18094,18136,18178,
 18220,18261,18303,18345,18386,18428,18470,18511,18553,18594,18636,18677,18718,18760,18801,18842,
 18883,18924,18965,19006,19047,19088,19129,19170,19211,19251,19292,19333,19373,19414,19454,19495,
 19535,19576,19616,19656,19696,19737,19777,19817,19857,19897,19937,19977,20017,20056,20096,20136,
 20176,20215,20255,20294,20334,20373,20413,20452,20491,20530,20570,20609,20648,20687,20726,20765,
 20804,20843,20881,20920,20959,20997,21036,21075,21113,21152,21190,21228,21267,21305,21343,21381,
 21419,21457,21495,21533,21571,21609,21647,21685,21722,21760,21797,21835,21873,21910,21947,21985,
 22022,22059,22096,22133,22171,22208,22245,22281,22318,22355,22392,22429,22465,22502,22538,22575,
 22611,22648,22684,22720,22757,22793,22829,22865,22901,22937,22973,23009,23044,23080,23116,23151,
 23187,23223,23258,23293,23329,23364,23399,23435,23470,23505,23540,23575,23610,23645,23679,23714,
 23749,23783,23818,23853,23887,23921,23956,23990,24024,24058,24093,24127,24161,24195,24229,24262,
 24296,24330,24364,24397,24431,24464,24498,24531,24564,24598,24631,24664,24697,24730,24763,24796,
 24829,24862,24894,24927,24960,24992,25025,25057,25090,25122,25154,25187,25219,25251,25283,25315,
 25347,25379,25410,25442,25474,25505,25537,25568,25600,25631,25663,25694,25725,25756,25787,25818,
 25849,25880,25911,25942,25972,26003,26034,26064,26095,26125,26155,26186,26216,26246,26276,26306,
 26336,26366,26396,26426,26455,26485,26514,26544,26573,26603,26632,26661,26691,26720,26749,26778,
 26807,26836,26865,26893,26922,26951,26979,27008,27036,27065,27093,27121,27150,27178,27206,27234,
 27262,27290,27317,27345,27373,27400,27428,27456,27483,27510,27538,27565,27592,27619,27646,27673,
 27700,27727,27754,27780,27807,27834,27860,27887,27913,27939,27966,27992,28018,28044,28070,28096,
 28122,28147,28173,28199,28224,28250,28275,28301,28326,28351,28377,28402,28427,28452,28477,28501,
 28526,28551,28576,28600,28625,28649,28674,28698,28722,28746,28770,28794,28818,28842,28866,28890,
 28914,28937,28961,28984,29008,29031,29054,29078,29101,29124,29147,29170,29193,29216,29238,29261,
 29284,29306,29329,29351,29373,29396,29418,29440,29462,29484,29506,29528,29550,29571,29593,29614,
 29636,29657,29679,29700,29721,29742,29763,29784,29805,29826,29847,29868,29888,29909,29930,29950,
 29970,29991,30011,30031,30051,30071,30091,30111,30131,30151,30170,30190,30209,30229,30248,30267,
 30287,30306,30325,30344,30363,30382,30401,30419,30438,30457,30475,30494,30512,30530,30548,30567,
 30585,30603,30621,30639,30656,30674,30692,30709,30727,30744,30762,30779,30796,30813,30830,30847,
 30864,30881,30898,30915,30931,30948,30964,30981,30997,31013,31030,31046,31062,31078,31094,31109,
 31125,31141,31157,31172,31188,31203,31218,31234,31249,31264,31279,31294,31309,31323,31338,31353,
 31367,31382,31396,31411,31425,31439,31453,31467,31481,31495,31509,31523,31537,31550,31564,31577,
 31591,31604,31617,31630,31643,31656,31669,31682,31695,31708,31720,31733,31746,31758,31770,31783,
 31795,31807,31819,31831,31843,31855,31866,31878,31890,31901,31913,31924,31935,31947,31958,31969,
 31980,31991,32002,32012,32023,32034,32044,32055,32065,32075,32086,32096,32106,32116,32126,32136,
 32145,32155,32165,32174,32184,32193,32203,32212,32221,32230,32239,32248,32257,32266,32275,32283,
 32292,32300,32309,32317,32325,32333,32342,32350,32358,32365,32373,32381,32389,32396,32404,32411,
 32419,32426,32433,32440,32447,32454,32461,32468,32475,32481,32488,32494,32501,32507,32513,32520,
 32526,32532,32538,32544,32549,32555,32561,32566,32572,32577,32583,32588,32593,32598,32603,32608,
 32613,32618,32623,32628,32632,32637,32641,32645,32650,32654,32658,32662,32666,32670,32674,32678,
 32681,32685,32688,32692,32695,32698,32702,32705,32708,32711,32714,32716,32719,32722,32724,32727,
 32729,32732,32734,32736,32738,32740,32742,32744,32746,32748,32749,32751,32753,32754,32755,32757,
 32758,32759,32760,32761,32762,32763,32763,32764,32765,32765,32766,32766,32766,32766,32766,32767,
};
inline float native_atan(float x){
 int reciprocal=0;if(x>1){x=1.f/x;reciprocal=1;}else if(x< -1){x=1.f/x;reciprocal=-1;}
 const float square=x*x;float tail=0;
 for(int k=24;k>0;--k){const float term=float(k),term_square=term*term,numerator=term_square*square;float denominator=term+term;denominator+=1.f;denominator+=tail;tail=numerator/denominator;}
 const float denominator=1.f+tail,value=x/denominator;
 return reciprocal>0?1.5707963705062866f-value:reciprocal<0?-1.5707963705062866f-value:value;
}
inline float native_atan2(float y,float x){
 if(y==0&&x==0)return 0;if(x==0)return y<0?-1.5707963705062866f:1.5707963705062866f;
 if(x<0){const float value=native_atan(std::abs(y/x));return y<0?-(3.1415927410125732f-value):3.1415927410125732f-value;}return native_atan(y/x);
}
inline float native_trig(float degrees,bool cosine){
 while(degrees<=0)degrees+=360.f;while(degrees>360)degrees-=360.f;
 uint16_t phase=uint16_t(int(degrees*182.0388946533203f));if(cosine)phase=uint16_t(phase+0x4000);
 const unsigned v=phase>>4,index=v&0x3FF;const int value=(v&0x400)?native_sines[1023-index]:native_sines[index];
 return float((v&0x800)?-value:value)*3.0518509447574615e-5f;
}
inline bool force_x(Axes raw){
 const int px=filter(raw.x,raw.x),py=filter(raw.y,raw.y);
 const float radians=native_atan2(float(py),float(px)),degrees=(radians*180.f)/3.141592025756836f,angle=std::abs(degrees);
 const float co=std::abs(native_trig(angle,true)),si=std::abs(native_trig(angle,false));if(co<1e-6f)return false;
 const float radius=std::sqrt(float(px*px+py*py));return radius>=53.f/co-20.f*si&&(angle<=50.f||angle>=130.f);
}
inline Axes native_axes(Axes raw,Axes old){return {force_x(raw)?(raw.x>=0?54:-54):filter(raw.x,old.x),filter(raw.y,old.y)};}
inline std::optional<Host> inverse_raw(Axes raw){
 if(raw.x< -82||raw.x>82||raw.y< -82||raw.y>82)return {};
 if(!raw.x&&!raw.y)return Host{};
 for(float bias:{.25f,.125f,0.f}){
  const float x=float(raw.x)+(raw.x>0?bias:raw.x<0?-bias:0),y=float(raw.y)+(raw.y>0?bias:raw.y<0?-bias:0),angle=std::atan2(y,x);
  float mod=std::fmod(angle+2*pi,pi/2);if(mod>pi/4)mod=pi/2-mod;
  const float radius=82.f*std::sin(alpha)/std::sin(pi-mod-alpha);
  Host host{x/radius,y/radius};if(std::hypot(host.x,host.y)>1.000001f)continue;
  const auto actual=convert(host);if(actual.x==raw.x&&actual.y==raw.y)return host;
 }
 return {};
}
struct Candidate {Host host;Axes raw;bool force=false;};
inline const std::vector<Candidate>& legal_candidates(){
 static const auto candidates=[](){std::vector<Candidate> out;for(int x=-82;x<=82;++x)for(int y=-82;y<=82;++y){const Axes raw{x,y};if(auto host=inverse_raw(raw))out.push_back({*host,raw,force_x(raw)});}return out;}();return candidates;
}
struct Result {Host host;Axes raw,filtered;float cosine=-1;};
inline std::optional<Result> toward(float x,float y,Axes old,float preferred_magnitude=32){
 const float length=std::hypot(x,y);if(!valid(old)||!std::isfinite(length)||length<.0001f||length>100000||!std::isfinite(preferred_magnitude)||preferred_magnitude<1||preferred_magnitude>54)return {};
 std::optional<Result> best;float best_magnitude=1e30f;
 for(const auto& c:legal_candidates()){
  const Axes filtered{c.force?(c.raw.x>=0?54:-54):filter(c.raw.x,old.x),filter(c.raw.y,old.y)};
  const float size=std::hypot(float(filtered.x),float(filtered.y));if(size<1)continue;
  const float cosine=(filtered.x*x+filtered.y*y)/(size*length),magnitude_error=std::abs(size-preferred_magnitude);
  if(!best||cosine>best->cosine+1e-6f||(std::abs(cosine-best->cosine)<=1e-6f&&magnitude_error<best_magnitude)){best=Result{c.host,c.raw,filtered,cosine};best_magnitude=magnitude_error;}
 }
 return best;
}
struct Mouth {
 bool observed=false,right=true;unsigned posture=0;float scale_x=1,scale_y=1;
 std::span<const float> offsets_x,offsets_y;
 bool valid()const{
  if(!observed||(posture!=0&&posture!=10&&posture!=20&&posture!=30)||offsets_x.size()!=80||offsets_y.size()!=80||!std::isfinite(scale_x)||!std::isfinite(scale_y)||scale_x<=0||scale_y<=0||scale_x>2||scale_y>2)return false;
  for(unsigned i=0;i<80;++i)if(!std::isfinite(offsets_x[i])||!std::isfinite(offsets_y[i])||std::abs(offsets_x[i])>64||std::abs(offsets_y[i])>64)return false;
  return true;
 }
};
struct MouthRay {float forward=0,up=0,distance=0;};
inline std::optional<MouthRay> from_mouth(const Mouth& mouth,float px,float py,float tx,float ty){
 if(!mouth.valid()||!std::isfinite(px)||!std::isfinite(py)||!std::isfinite(tx)||!std::isfinite(ty))return {};
 const float facing=mouth.right?1.f:-1.f;if((tx-px)*facing< -4)return {};
 float angle=std::atan2(py-ty,std::abs(tx-px))*180/pi;MouthRay out;
 for(unsigned step=0;step<4;++step){
  const unsigned bin=angle>=0?unsigned((angle+.5f)*.1f):40+unsigned((angle-.5f)*-.1f),index=mouth.posture+bin;if(index>=80)return {};
  const float x=px+facing*mouth.scale_x*mouth.offsets_x[index],y=py+mouth.scale_y*mouth.offsets_y[index];
  out={(tx-x)*facing,y-ty,std::hypot(tx-x,ty-y)};angle=std::clamp(std::atan2(out.up,out.forward)*180/pi,-90.f,90.f);
 }
 if(out.forward< -4)return {};return out;
}

}
