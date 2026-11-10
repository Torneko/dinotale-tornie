#include "purchase_audio0284.hpp"
#include <cstdio>
#include <functional>
#include <limits>
#include <stdexcept>
#include <thread>

namespace a=workshop::purchase_audio0284;
static unsigned checks=0;
static void need(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
static float peak(const a::Pcm& pcm){float result=0;for(float x:pcm)result=std::max(result,std::abs(x));return result;}
struct State {
 unsigned opens=0,queues=0,starts=0,closes=0,rate=0,channels=0;
 bool available=true,queue_ok=true,throw_open=false,throw_queue=false;
 std::size_t queued=0;a::Pcm received{};std::function<void()> during_queue;
};
struct Backend {
 State* s;
 bool open(unsigned rate,unsigned channels){++s->opens;s->rate=rate;s->channels=channels;if(s->throw_open)throw std::runtime_error("open");return s->available;}
 std::size_t queued_bytes(){return s->queued;}
 bool queue(std::span<const float> pcm){++s->queues;if(s->during_queue)s->during_queue();if(s->throw_queue)throw std::runtime_error("queue");if(!s->queue_ok)return false;need(pcm.size()==a::samples,"Exact PCM sample count");std::copy(pcm.begin(),pcm.end(),s->received.begin());s->queued=pcm.size_bytes();return true;}
 void start(){++s->starts;}
 void close(){++s->closes;}
};
int main(){try {
 need(a::frames==5292&&a::samples==10584&&a::samples*sizeof(float)==42336,"120ms stereo44100 format");
 a::Pcm full{},quiet{},muted{},loud{};a::render(full,100);a::render(quiet,3);a::render(muted,0);a::render(loud,200);
 need(full.front()==0&&full.back()==0,"Silent endpoints");need(peak(full)>.14f&&peak(full)<=a::peak_limit,"Audible bounded peak");need(peak(quiet)<=a::peak_limit*.03f,"Three percent global volume bound");
 double energy=0;float step=0;
 for(std::size_t i=0;i<a::samples;++i){need(std::isfinite(full[i])&&std::abs(full[i])<=a::peak_limit,"Finite unclipped PCM");need(quiet[i]==full[i]*.03f,"Global gain applied exactly once");need(muted[i]==0,"Mute renders silence");need(loud[i]==full[i],"Over100 percent is clamped");energy+=double(full[i])*full[i];if(i>=2)step=std::max(step,std::abs(full[i]-full[i-2]));if(i%2==0)need(full[i]==full[i+1],"Stereo channels agree");}
 need(energy/a::samples>.005&&step<.025f,"Bounded energy and smooth sample changes");
 for(double gain:{-1.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()}){a::render(muted,gain);need(peak(muted)==0,"Invalid or negative volume is silent");}
 State state;{a::Player<Backend> player(Backend{&state});need(!player.play(0)&&!player.play(-3)&&!player.play(std::numeric_limits<double>::quiet_NaN()),"Muted requests rejected");need(!state.opens&&!state.queues,"Mute never opens or queues");need(player.play(3),"First committed sound queues");need(state.opens==1&&state.rate==44100&&state.channels==2&&state.queues==1&&state.starts==1,"One lazy host device at fixed format");need(state.received==quiet,"Queued content respects3percent");need(!player.play(100)&&state.queues==1,"Busy device does not append a second voice");state.queued=0;need(player.play(100)&&state.opens==1&&state.queues==2,"Drained device reuses original handle");player.shutdown();player.shutdown();need(state.closes==1&&!player.play(100),"Shutdown is idempotent and prevents reentry");}need(state.closes==1,"Destructor does not close twice");
 State failed;failed.available=false;{a::Player<Backend> player(Backend{&failed});need(!player.play(3)&&!player.play(100),"Unavailable device is a no-op");need(failed.opens==1&&!failed.queues&&!failed.starts&&!failed.closes,"Device failure is attempted only once");}
 State queue_fail;queue_fail.queue_ok=false;{a::Player<Backend> player(Backend{&queue_fail});need(!player.play(3)&&queue_fail.opens==1&&queue_fail.queues==1&&!queue_fail.starts,"Queue failure is a no-op");queue_fail.queue_ok=true;need(player.play(3)&&queue_fail.opens==1&&queue_fail.queues==2&&queue_fail.starts==1,"Queue retry does not reopen device");}
 State throwing;throwing.throw_open=true;{a::Player<Backend> player(Backend{&throwing});need(!player.play(3)&&!player.play(3)&&throwing.opens==1&&!throwing.queues,"Open exception never escapes and is not retried");}
 State queue_throw;queue_throw.throw_queue=true;{a::Player<Backend> player(Backend{&queue_throw});need(!player.play(3)&&!queue_throw.starts,"Queue exception never escapes");}
 State busy;{a::Player<Backend> player(Backend{&busy});busy.during_queue=[&]{bool queued=true;std::thread contender([&]{queued=player.play(3);});contender.join();need(!queued,"Contending request returns without waiting");};need(player.play(3)&&busy.queues==1,"Busy guard preserves one voice");}
#ifndef WORKSHOP_CONTENT_BUILD
 need(!workshop_purchase_sound0284(),"Non-content builds have a link-free no-op");
#endif
 std::printf("PASS %u purchase audio checks; frames=%zu samples=%zu bytes=%zu peak3=%.8f\n",checks,a::frames,a::samples,a::samples*sizeof(float),double(peak(quiet)));return 0;
 }catch(const std::exception& error){std::fprintf(stderr,"FAIL: %s\n",error.what());return 1;}}
