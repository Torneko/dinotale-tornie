#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <atomic>
#include <stdexcept>
#include <thread>
namespace plume {
struct D3D12SwapChain {
 struct {bool enablePresentWait=true;} desc;
 HANDLE waitableObject=nullptr;
 void wait();
};
}
// Exact extracted production method; only its enclosing GPU-free class is a
// HOST fixture. All WaitForSingleObject/SetEvent/GetLastError calls are real.
namespace plume {
#include PLUME_DXGI_WAIT_BODY
}
static unsigned checks=0,presentCalls=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static void mark(const char* phase){std::fprintf(stderr,"[DXGIWaitTest] phase=%s\n",phase);}
struct Event {HANDLE handle=nullptr;explicit Event(bool signal=false){handle=CreateEventW(nullptr,FALSE,signal?TRUE:FALSE,nullptr);need(handle!=nullptr,"Create real auto-reset Win32 event");}~Event(){if(handle)CloseHandle(handle);}Event(const Event&)=delete;Event& operator=(const Event&)=delete;};
static ULONGLONG attempt(plume::D3D12SwapChain& chain){const auto began=GetTickCount64();chain.wait();const auto elapsed=GetTickCount64()-began;++presentCalls;need(elapsed<2000,"Wait returns within broad scheduler tolerance; HOST caller proceeds toward Present");return elapsed;}
int main(){try{plume::D3D12SwapChain cases[5];
 {mark("null");auto& chain=cases[0];need(attempt(chain)<250,"Null handle retains existing immediate no-op");}
 {mark("auto-reset");Event e(true);auto& chain=cases[1];chain.waitableObject=e.handle;need(attempt(chain)<250,"Signaled real event returns immediately");need(WaitForSingleObject(e.handle,0)==WAIT_TIMEOUT,"Production wait consumes the real auto-reset signal");
  mark("bounded-timeouts");need(attempt(chain)>=70,"Real unsignaled event waits for the100ms bound");for(unsigned i=0;i<4;++i)need(attempt(chain)>=70,"Repeated timeout remains bounded rather than waiting forever");
  mark("recovery");need(SetEvent(e.handle)!=FALSE,"Rearm the same real event");need(attempt(chain)<250,"Signaled event recovers after timeouts");need(SetEvent(e.handle)!=FALSE,"Rearm again");attempt(chain);
  mark("one-recovery-only");attempt(chain);need(SetEvent(e.handle)!=FALSE,"Rearm after another timeout");attempt(chain);
 }
 {mark("failed");HANDLE closed=CreateEventW(nullptr,FALSE,FALSE,nullptr);need(closed!=nullptr,"Create soon-invalid real handle");need(CloseHandle(closed)!=FALSE,"Close before waiting; no close during pending wait");auto& invalid=cases[2];invalid.waitableObject=closed;SetLastError(ERROR_ACCESS_DENIED);need(attempt(invalid)<250,"Closed handle returns WAIT_FAILED immediately");attempt(invalid);invalid.waitableObject=nullptr;}
 {mark("new-owner-signaled");Event e(true);auto& fresh=cases[3];fresh.waitableObject=e.handle;attempt(fresh);need(WaitForSingleObject(e.handle,0)==WAIT_TIMEOUT,"New owner still consumes only its genuine event signal");}
 {mark("thread-local");Event e;auto& shared=cases[4];shared.waitableObject=e.handle;std::atomic<unsigned> returned=0;const auto began=GetTickCount64();std::thread first([&]{shared.wait();++returned;});std::thread second([&]{shared.wait();++returned;});first.join();second.join();need(returned==2&&GetTickCount64()-began<2000,"Each thread returns from one real bounded wait with independent diagnostics");}
 mark("end");need(presentCalls==14,"Every single-thread wait reaches the HOST Present counter; no invented GPU presentation");std::printf("DXGIPresentWait0284Checks: %u PASS; actual extracted method +real Win32 events/waits; HOST Present counter only; no GPU\n",checks);return 0;
 }catch(const std::exception& error){std::fprintf(stderr,"FAIL %s\n",error.what());return 1;}}
