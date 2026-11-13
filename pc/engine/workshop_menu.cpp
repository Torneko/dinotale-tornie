#ifdef WORKSHOP_NATIVE_COOP
extern "C" int workshop_boss_solo_done();
extern "C" int workshop_battle_done();
extern "C" int workshop_battle_prepare();
extern "C" unsigned workshop_battle_challenges();
extern "C" int workshop_battle_awarded();
extern "C" int workshop_battle_challenge_readable();
extern "C" int workshop_battle_diagnostic();
extern "C" int workshop_battle_failed();
#endif
#include "workshop_menu.hpp"

#include "custom_session.hpp"

#include "workshop_menu_game.hpp"

#include "workshop_loader.hpp"

#include "achievements.hpp"

#include "collections.hpp"

#include "hybrid_mode.hpp"
#include "game_modes0320.hpp"
#include "bonus_game0352.hpp"
#include "story_hybrid0320.hpp"
#include "story_rights0320.hpp"
#include "known_yoshis_runtime0355.hpp"
#include "book_unlock_art0355.hpp"
#include "stb/stb_image.h"
#include "hybrid_finds.hpp"
#include "story_hard_bridge0330.hpp"
#include "story_hard0330.hpp"
#include "hybrid_completion0290.hpp"
#include "metal_yoshis0300.hpp"
#include "elemental_yoshis0355.hpp"
#include "heart_shop0283.hpp"
#include "purchase_audio0284.hpp"
#include "menu_music_gate0310.hpp"
#include "menu_music_session0310.hpp"
#include <chrono>
#include <random>

#include "linux_process.hpp"

#include "recompui/recompui.h"

#include "recompui/config.h"
#ifdef WORKSHOP_NATIVE_COOP
#define WORKSHOP_COOP_HEALTH_RECOMP_UI
#include "coop_health_hud0250.hpp"
#endif


#include "elements/ui_image.h"

#include "librecomp/game.hpp"

#include "ultramodern/ultramodern.hpp"

#include "json/json.hpp"

#include <array>

#include <atomic>

#include <bit>

#include <filesystem>

#include <fstream>

#include <cstdio>

#include <thread>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#endif



extern SDL_Window* window;

namespace {

using namespace recompui;

bool enabled=false,french=true;
bool purchase_sound=true,preferences_saved=true;

std::atomic<int> boot_mode{-1};

std::atomic<int> session_mode{-1};

std::atomic<int> startup_mode{-1};

std::filesystem::path pc_root;

ContextId menu_context=ContextId::null();

ContextId game_link_context=ContextId::null();

std::array<Image*,6> unknown_book_yoshis0355{};
unsigned illustrated_known0355=~0u;
void refresh_book_yoshis0355(){
 const auto known=workshop::known_yoshis_runtime0355::snapshot().knownMask18;
 if(known==illustrated_known0355)return;illustrated_known0355=known;
 for(unsigned i=0;i<6;++i)if(auto layer=unknown_book_yoshis0355[i]){
  if(known&(1u<<(8+i)))layer->display_hide();else layer->display_show();
 }
}
Element* resume_button=nullptr;

Element* session_menu_button=nullptr;

Label* resume_label=nullptr;

bool loading_mode=false;

bool resume_pending=false;

// Explicit ownership survives Options overlays and native paused snapshots
// that lag one frame behind Resume. Workers never change this ownership.
bool book_music_owned0310=false;
bool music_configured0310=false;
void claim_book_music0310(){
    if(book_music_owned0310)return;
    workshop_menu_native_audio_gate0310(true);
    book_music_owned0310=true;
}
void release_book_music0310(){
    workshop::menu_music_session0310::suspend(); // Stop/clear before native wake.
    book_music_owned0310=false;
    workshop_menu_native_audio_gate0310(false);
}
void shutdown_book_music0310(){
    workshop::menu_music_session0310::shutdown();
    book_music_owned0310=false;
    workshop_menu_native_audio_gate0310(true); // Exiting: never wake native music.
}
void frame_book_music0310(){
    if(!music_configured0310 && recomp::is_rom_valid(u8"yoshisstory.n64.us")){
        char* base=SDL_GetBasePath();
        if(base){
            auto directory=std::filesystem::u8path(base);SDL_free(base);
            recomp::GameEntry game{};game.game_id=u8"yoshisstory.n64.us";
            auto config=recomp::get_config_path();
            workshop::menu_music_session0310::configure(directory,config/"menu-music0310",config/game.stored_filename());
            music_configured0310=true;
        }
    }
    // No IO, rendering or joins here. PCM publication and playback only.
    workshop::menu_music_session0310::frame(book_music_owned0310,
        ultramodern::is_game_started()&&!book_music_owned0310,
        recompui::config::sound::get_main_volume());
}


Element* focus_pending=nullptr;

bool focus_waited=false;

unsigned snapshot_frame=~0u;

Element *book=nullptr,*main_page=nullptr,*detail_page=nullptr;

Label *detail_title=nullptr,*detail_body=nullptr,*footer=nullptr;

std::array<Element*,4> actions{};

std::array<Label*,4> action_labels{};

std::array<std::function<void()>,4> action_callbacks{};

std::array<Element*,8> choices{};

std::array<Label*,8> choice_labels{};

nlohmann::json progress;

bool detail_open=false;

uint64_t last_button_activation=0;

constexpr Color ink{73,43,25,255},paper{247,229,198,255},ribbon{245,219,176,255},edge{201,161,104,255};

const char* tr(const char* fr,const char* en){return french?fr:en;}

bool save_menu_preferences(bool new_french,bool new_sound){
    const std::filesystem::path file="book-menu.json";
    auto temporary=file;temporary+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    try{
        auto value=nlohmann::json::object();
        if(std::filesystem::exists(file)){
            if(std::filesystem::file_size(file)>65536)throw std::runtime_error("preferences-size");
            std::ifstream source(file);source>>value;
            if(!value.is_object())throw std::runtime_error("preferences-shape");
        }
        value["version"]=1;value["language"]=new_french?"fr":"en";value["purchaseSound"]=new_sound;
        {std::ofstream stream(temporary,std::ios::binary|std::ios::trunc);stream<<value.dump(2)<<'\n';stream.flush();if(!stream)throw std::runtime_error("preferences-write");}
#ifdef _WIN32
        if(!MoveFileExW(temporary.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("preferences-replace");
#else
        std::filesystem::rename(temporary,file);
#endif
        preferences_saved=true;return true;
    }catch(const std::exception& error){
        std::error_code ignored;std::filesystem::remove(temporary,ignored);
        preferences_saved=false;fprintf(stderr,"[BookPreferences] save failed: %s\n",error.what());return false;
    }
}
void purchase_feedback(){if(purchase_sound)(void)workshop_purchase_sound0284();}


nlohmann::json story_progress0320=nlohmann::json::object();
nlohmann::json hard_progress0330=nlohmann::json::object();
void refresh_canonical_progress0320(){
    const auto rights=workshop::story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin");
    workshop::known_yoshis_runtime0355::canonical(rights);
    if(!rights.present||!rights.readable){progress={{"valid",false}};return;}
    const auto finds=workshop::finds::view();
    const unsigned owned=finds.initialized&&finds.readable&&finds.migrated?finds.found:rights.legacy_owned;
    progress={{"valid",true},{"storyMask",rights.story},{"trialMask",rights.trial},{"extraYoshiMask",owned}};
}
void write_progress_summary0320(){
    const int selected=session_mode.load();
    if(selected==workshop::game_modes0320::Bonus){
        const auto state=workshop::bonus_game0352::view();
        std::ofstream summary("menu-progress-bonus0352.json");
        summary<<nlohmann::json{{"schema",1},{"bonusUnlockedMask",state.access.unlocked_mask},{"bonusCompletedMask",state.session.completed_mask},{"fullRouteAvailable",state.full_route_available}}.dump(2)<<'\n';return;
    }
    const bool hybrid=selected==workshop::game_modes0320::StoryHybrid,hard=selected==workshop::game_modes0320::StoryHard;
    const char* name=hard?(workshop::story_hard_bridge0330::preview()?"menu-progress-story-hard-preview.json":"menu-progress-story-hard.json"):hybrid?"menu-progress-story-hybrid.json":"menu-progress.json";
    std::ofstream summary(name);summary<<(hard?hard_progress0330:hybrid?story_progress0320:progress).dump(2)<<'\n';
}
void home();
void story_hard_page0330();
void trial_modes0352();
void bonus_levels0352(unsigned page=0);
bool bonus_outcome_page0353(const workshop::bonus_game0352::View&);
bool bonus_resume_allowed0353(const workshop::bonus_game0352::View&);

void open_book();

void resume_game();

// Config tabs can be rebuilt and destroy their buttons while the per-frame

// callback continues. Never leave its non-owning pointer referring to one.

class SessionMenuButton:public Button {

public:

    using Button::Button;

    ~SessionMenuButton() override {

        if(session_menu_button==this)session_menu_button=nullptr;

    }

};

class BookButton:public Element {

    Style hover,focus_style;

    std::function<void()> callback;

    void activate(){

        auto now=SDL_GetTicks64();

#ifdef WORKSHOP_CONTENT_BUILD

        if(workshop_test_book_shop())fprintf(stderr,"[BookShopTest] activate id=%s now=%llu last=%llu callback=%d\n",get_id().c_str(),(unsigned long long)now,(unsigned long long)last_button_activation,int(bool(callback)));

#endif

        if(last_button_activation && now-last_button_activation<200)return;

        last_button_activation=now;if(callback)callback();

    }

protected:

    void process_event(const Event& event) override {

        if(event.type==EventType::Click)activate();

        if(event.type==EventType::MenuAction && std::get<EventMenuAction>(event.variant).action==MenuAction::Accept)activate();

        if(event.type==EventType::Focus)set_style_enabled(focus_state,std::get<EventFocus>(event.variant).active);

        if(event.type==EventType::Hover)set_style_enabled(hover_state,std::get<EventHover>(event.variant).active);

    }

public:

    void diagnostic_click(){

        auto* element=get_current_context().get_document()->GetElementById(get_id());

#ifdef WORKSHOP_CONTENT_BUILD

        if(workshop_test_book_shop())fprintf(stderr,"[BookShopTest] click id=%s target=%s tag=%s\n",get_id().c_str(),element->GetId().c_str(),element->GetTagName().c_str());

#endif

        element->DispatchEvent(Rml::EventId::Click,Rml::Dictionary{});

    }

    BookButton(ResourceId id,Element* parent,Label** label,std::function<void()> cb):Element(id,parent,Events(EventType::Click,EventType::Focus,EventType::Hover,EventType::MenuAction),"button"),callback(std::move(cb)){

        set_display(Display::Flex);set_align_items(AlignItems::Center);set_justify_content(JustifyContent::Center);

        set_background_color(ribbon);set_border_color(edge);set_border_width(2);set_border_radius(15);

        set_color(ink);set_font_weight(750);set_cursor(Cursor::Pointer);enable_focus();

        hover.set_background_color(Color{236,239,174,255});focus_style.set_background_color(Color{209,235,95,255});

        focus_style.set_border_color(Color{119,146,38,255});focus_style.set_border_width(4);

        add_style(&hover,hover_state);add_style(&focus_style,focus_state);

        *label=get_current_context().create_element<Label>(this,"",LabelStyle::Normal);

        (*label)->set_color(ink);(*label)->set_font_size(32);(*label)->set_font_weight(750);

        (*label)->set_text_align(TextAlign::Center);

    }

};

class BookRoot:public Element {

protected:

    void process_event(const Event& event) override {

        if(event.type!=EventType::MenuAction)return;

        auto action=std::get<EventMenuAction>(event.variant).action;

        if(loading_mode)return;

        if(action==MenuAction::Back || action==MenuAction::Toggle){

            if(detail_open)home();

            else if(ultramodern::is_game_started())resume_game();

            else recompui::open_quit_game_prompt();

        }

    }

public:

    BookRoot(ResourceId id,Element* parent):Element(id,parent,Events(EventType::MenuAction)){}

};

void rect(Element* element,float left,float top,float width,float height){

    element->set_position(Position::Absolute);element->set_left(left,Unit::Percent);element->set_top(top,Unit::Percent);

    element->set_width(width,Unit::Percent);element->set_height(height,Unit::Percent);

}

void update_titles(){

    const char* fr[]={"Story","Trial","Création","Statistiques et succès","Mode hybride","Options","Practice","Boss Battle"};

    const char* en[]={"Story","Trial","Creation","Statistics & achievements","Hybrid mode","Options","Practice","Boss Battle"};

    for(unsigned i=0;i<choices.size();i++)choice_labels[i]->set_text(french?fr[i]:en[i]);

    if(progress.is_object() && progress.value("valid",false) && progress.value("trialMask",0u)!=0xFFFFFF){

        choice_labels[4]->set_text(std::string(tr("Mode hybride · ","Hybrid mode · "))+std::to_string(std::popcount(progress.value("trialMask",0u)))+"/24 Trial");

        choice_labels[4]->set_font_size(27);

    }

    footer->set_text(tr("Flèches : choisir    Entrée / A : valider    Échap : retour","Arrows: choose    Enter / A: confirm    Esc: back"));

    if(resume_label)resume_label->set_text(tr("Reprendre la partie","Resume game"));

}

void home(){

    if(auto focused=menu_context.get_focused_element())focused->blur();

    detail_open=false;detail_page->display_hide();main_page->display_show();

    refresh_book_yoshis0355();
    update_titles();focus_pending=choices[0];focus_waited=false;

    menu_context.set_autofocus_element(choices[0]);

    fprintf(stderr,"[BookMenu] page=home entries=8 language=%s\n",french?"fr":"en");

}

void panel(const std::string& title,const std::string& body){

    if(auto focused=menu_context.get_focused_element())focused->blur();

    focus_pending=nullptr;focus_waited=false;menu_context.set_autofocus_element(nullptr);

    detail_open=true;main_page->display_hide();detail_page->display_show();

    detail_title->set_text(title);detail_body->set_text(body);

    for(auto button:actions)button->display_hide();

    fprintf(stderr,"[BookMenu] panel=%s bodyBytes=%zu\n",title.c_str(),body.size());

}

void action(unsigned slot,const std::string& title,std::function<void()> callback){

    actions[slot]->display_show();action_labels[slot]->set_text(title);action_callbacks[slot]=std::move(callback);

    if(!focus_pending){focus_pending=actions[slot];focus_waited=false;menu_context.set_autofocus_element(actions[slot]);}

}

void back(unsigned slot=0){action(slot,tr("Retour","Back"),home);}

void resume_game(){

    if(loading_mode)return;
    if(workshop::bonus_game0352::active()&&bonus_outcome_page0353(workshop::bonus_game0352::view()))return;
    if(workshop::bonus_game0352::active()&&workshop::bonus_game0352::view().interrupted){home();footer->set_text(tr("La tentative a été interrompue. Choisis un autre mode pour quitter cet essai.","The attempt was interrupted. Choose another mode to leave this preview."));return;}

    // Finish dispatching the button/key event before hiding its document or

    // waking the native producer of display lists.

    resume_pending=true;

}

void open_book(){
    refresh_canonical_progress0320();

    if(loading_mode)return;

    if(recompui::config::get_config_modal()->is_open_now() && !recompui::config::close())return;

    if(ultramodern::is_game_started()){

        recompui::set_idle_ui_rendering(true);

        workshop::book::pause();

    }

    claim_book_music0310();

    auto previous=recompui::try_close_current_context();

    menu_context.open();home();
    if(workshop::bonus_game0352::active())bonus_outcome_page0353(workshop::bonus_game0352::view());

    if(!recompui::is_context_shown(menu_context))recompui::show_context(menu_context,"");

    menu_context.close();if(previous!=ContextId::null())previous.open();

    fprintf(stderr,"[BookLink] ui=open-book\n");fflush(stderr);

}

// A selected Bonus survives the existing runtime handoff as numeric argv.
// No extra file is written and the destination revalidates the selection.
std::vector<std::string> bonus_launch_arguments0353(int mode){
    if(mode<0&&workshop::bonus_game0352::active()&&workshop::bonus_game0352::preview_requested()){
        const auto state=workshop::bonus_game0352::view();
        if(state.full_route_available&&state.selected_bonus>=1&&state.selected_bonus<=6){
            const auto& attempt=state.session.attempts[state.selected_bonus-1];
            return {"--workshop-bonus-finales0353",std::to_string(state.selected_bonus),std::to_string(attempt?attempt->seed:24)};
        }
    }
    if(mode!=workshop::game_modes0320::Bonus||!workshop::bonus_game0352::launch_requested())return {};
    const auto selected=workshop::bonus_game0352::pending_launch();
    if(!selected||selected->bonus<1||selected->bonus>6)return {};
    return {"--workshop-bonus-selection0353",std::to_string(selected->bonus),std::to_string(selected->seed),selected->new_attempt?"1":"0"};
}
bool bonus_resume_allowed0353(const workshop::bonus_game0352::View& state){
    return !state.interrupted&&!state.completed&&!state.game_over;
}
bool bonus_launch_authorized0353(){
    return workshop::bonus_game0352::preview_requested()||workshop::bonus_game0352::launch_requested();
}

void route_native(int mode){

    loading_mode=true;resume_button->display_hide();

    panel(tr("Changement de mode","Changing mode"),tr("Ouverture du mode choisi…","Opening the selected mode…"));

    footer->set_text(tr("Chargement…","Loading…"));

#ifdef _WIN32

    // Parked game memory stays untouched. A mode change starts one fresh

    // runtime after the previous runtime has flushed and closed its profile.

    auto deadline=SDL_GetTicks64()+2000;

    while(!workshop::book::snapshot().paused && SDL_GetTicks64()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(5));

    if(workshop::book::snapshot().paused){

        // This API's worker barrier drains pending EEPROM writes before it

        // reopens the same file. No native game thread writes while parked.

        ultramodern::change_save_file({},ultramodern::get_save_file_path().stem().u8string());

        if(window){

            int x,y,w,h;SDL_GetWindowPosition(window,&x,&y);SDL_GetWindowSize(window,&w,&h);

            std::ofstream file("book-window.json");file<<nlohmann::json{{"x",x},{"y",y},{"width",w},{"height",h},{"maximized",bool(SDL_GetWindowFlags(window)&SDL_WINDOW_MAXIMIZED)}}.dump(2);

        }

        std::array<wchar_t,32768> executable{};

        if(GetModuleFileNameW(nullptr,executable.data(),DWORD(executable.size()))){

            write_progress_summary0320();

            std::wstring command=L"\""+std::wstring(executable.data())+L"\" --workshop-wait-parent "+std::to_wstring(GetCurrentProcessId())+L" --workshop-main-menu"+(mode>=0?L" --workshop-start-mode "+std::to_wstring(mode):L"");
            for(const auto& argument:bonus_launch_arguments0353(mode))command+=L" "+std::wstring(argument.begin(),argument.end());

            STARTUPINFOW startup{};startup.cb=sizeof(startup);

            PROCESS_INFORMATION process{};

            if(CreateProcessW(executable.data(),command.data(),nullptr,nullptr,FALSE,0,nullptr,std::filesystem::current_path().c_str(),&startup,&process)){

                fprintf(stderr,"[BookLink] handoff mode=%d parent=%lu child=%lu save-barrier=complete\n",mode,GetCurrentProcessId(),process.dwProcessId);fflush(stderr);

                CloseHandle(process.hThread);CloseHandle(process.hProcess);

                shutdown_book_music0310();
                ultramodern::quit();return;

            }

        }

    }

#elif defined(__linux__)

    auto deadline=SDL_GetTicks64()+2000;

    while(!workshop::book::snapshot().paused && SDL_GetTicks64()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(5));

    if(workshop::book::snapshot().paused){

        ultramodern::change_save_file({},ultramodern::get_save_file_path().stem().u8string());

        if(window){

            int x,y,w,h;SDL_GetWindowPosition(window,&x,&y);SDL_GetWindowSize(window,&w,&h);

            std::ofstream file("book-window.json");file<<nlohmann::json{{"x",x},{"y",y},{"width",w},{"height",h},{"maximized",bool(SDL_GetWindowFlags(window)&SDL_WINDOW_MAXIMIZED)}}.dump(2);

        }

        write_progress_summary0320();

        std::vector<std::string> next_args{"--workshop-wait-parent",std::to_string(workshop::linux_process::id()),"--workshop-main-menu"};
        if(mode>=0){next_args.push_back("--workshop-start-mode");next_args.push_back(std::to_string(mode));}
        const auto bonus_args=bonus_launch_arguments0353(mode);next_args.insert(next_args.end(),bonus_args.begin(),bonus_args.end());
        if(workshop::linux_process::launch(workshop::linux_process::executable(),next_args,std::filesystem::current_path())){

            fprintf(stderr,"[BookLink] Linux handoff mode=%d parent=%u save-barrier=complete\n",mode,workshop::linux_process::id());fflush(stderr);

            shutdown_book_music0310();
            ultramodern::quit();return;

        }

    }

#endif

    loading_mode=false;

    panel(tr("Mode indisponible","Mode unavailable"),tr("Le mode n’a pas pu être ouvert. Ta partie est toujours en pause.","The mode could not be opened. Your game is still paused."));

    action(0,tr("Reprendre la partie","Resume game"),resume_game);back(1);

}

// Custom traversals own one isolated runtime per level. Complete the file
// checkpoint while the native thread is parked, then end that process. Joining
// legacy runtime threads here can stall while a victory actor waits for RDP.
void close_custom_runtime(bool advance){
    workshop::book::pause();auto deadline=SDL_GetTicks64()+2000;
    while(!workshop::book::snapshot().paused && SDL_GetTicks64()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(5));
    if(!workshop::book::snapshot().paused){footer->set_text(tr("Fermeture en attente… réessaie dans un instant.","Waiting to close… try again shortly."));return;}
    ultramodern::change_save_file({},ultramodern::get_save_file_path().stem().u8string());
    if(workshop::custom_session::ready() && !workshop::custom_session::close(advance))return;
    if(window){int x,y,w,h;SDL_GetWindowPosition(window,&x,&y);SDL_GetWindowSize(window,&w,&h);std::ofstream file("book-window.json");file<<nlohmann::json{{"x",x},{"y",y},{"width",w},{"height",h},{"maximized",bool(SDL_GetWindowFlags(window)&SDL_WINDOW_MAXIMIZED)}}.dump(2);file.flush();}
    fprintf(stderr,"[CustomSession] checkpoint-close save-barrier=complete result=%d advance=%d\n",int(workshop::custom_session::ready()),int(advance));std::fflush(nullptr);
    shutdown_book_music0310();
    std::_Exit(0);
}
void launch_native(int mode){

    if(workshop::custom_session::active() && ultramodern::is_game_started()){

        panel(tr("Partie personnalisée","Custom game"),tr("Ferme cette session pour revenir à l’Atelier. Tu pourras y reprendre la partie ou lancer un autre mode depuis DinoTale.","Close this session to return to the Workshop. Resume there, or launch another mode from DinoTale."));

        action(0,tr("Retour à l’Atelier","Return to Workshop"),[]{close_custom_runtime(false);});action(1,tr("Reprendre","Resume"),resume_game);return;

    }

    if(!workshop::game_modes0320::valid(mode))return;
    if(mode==workshop::game_modes0320::Bonus&&!bonus_launch_authorized0353()){bonus_levels0352();return;}
    if(mode==workshop::game_modes0320::StoryHard){
        const auto rights=workshop::story_hard_bridge0330::access();
        const auto local=workshop::story_hard0330::view();
        if((!workshop::story_hard_bridge0330::preview()&&!workshop::story_hard_access0330::entitled(rights))||!local.readable||!local.saved){story_hard_page0330();return;}
    }
    if(mode==workshop::game_modes0320::StoryHybrid){
        // The base EEPROM is the authority for Trial completion. A fresh
        // alternate Story save is deliberately not a source of shared rights.
        refresh_canonical_progress0320();
        const auto local=workshop::story_hybrid0320::view();
        if(!progress.value("valid",false)||progress.value("trialMask",0u)!=0xFFFFFF||!workshop::collections::view().readable||!workshop::collections::view().state.owns(workshop::collections::Item::Hybrid)||!local.readable||!local.saved||!workshop::finds::view().readable||!workshop::finds::view().saved||!workshop::metal_yoshis0300::view().readable||!workshop::metal_yoshis0300::view().saved){
            panel(tr("Story hybride verrouillé","Hybrid Story locked"),tr("Le mode hybride doit être acheté et les 24 Trial classiques terminés. Sa progression doit aussi être lisible.","Purchase Hybrid mode and clear all 24 classic Trial levels. Its progress must also be readable."));back();return;
        }
    }
    if(mode==3 && (!workshop::hybrid_mode::readable() || !workshop::hybrid_mode::entitled(progress.value("trialMask",0u),workshop::collections::view().state.owns(workshop::collections::Item::Hybrid)))){

        panel(tr("Mode hybride verrouillé","Hybrid mode locked"),tr("Il faut acheter ce mode et terminer les 24 Trial. Vérifie aussi que son fichier de records est lisible.","Purchase this mode and clear all 24 Trial levels. Its records file must also be readable."));back();return;

    }

    if(ultramodern::is_game_started()){

        panel(tr("Changer de mode ?","Change mode?"),tr("La partie en cours sera fermée. Les niveaux déjà terminés et tes déblocages sont conservés.","The current session will end. Completed levels and unlocks are preserved."));

        action(0,tr("Changer de mode","Change mode"),[mode]{route_native(mode);});

        action(1,tr("Reprendre la partie","Resume game"),resume_game);back(2);return;

    }

    if(!recomp::is_rom_valid(u8"yoshisstory.n64.us")){

        panel(tr("Jeu introuvable","Game not found"),tr("La ROM originale USA est requise dans ce profil.","The original USA ROM is required in this profile."));back();return;

    }

#ifdef WORKSHOP_NATIVE_COOP
    if(mode==4&&!workshop_battle_prepare()){
        panel(tr("Bataille indisponible","Battle unavailable"),tr("Ce profil utilise déjà une autre session spéciale.","This profile already uses another special session."));back();return;
    }
#endif
    boot_mode.store(mode);

    session_mode.store(mode);

    workshop::hybrid_mode::activate(workshop::game_modes0320::hybrid_content(mode),workshop::game_modes0320::hybrid_trial_records(mode));
    if(mode!=5)workshop::story_hybrid0320::deactivate();

    workshop::achievements::start_session(unsigned(mode));
    workshop::hybrid_completion0290::start_session(unsigned(mode));
    workshop::metal_yoshis0300::start_session(unsigned(mode),mode!=4);
    workshop::elemental_yoshis0355::start_session(unsigned(mode),true);

    workshop::collections::start_session(unsigned(mode));

    fprintf(stderr,"[BookMenu] launch=%s\n",mode==0?"story":mode==5?"story-hybrid":mode==6?"story-hard":mode==7?"bonus":mode==1?"trial":mode==3?"hybrid":mode==4?"boss-battle":"practice");fflush(stderr);

    release_book_music0310();
    recomp::start_game(u8"yoshisstory.n64.us",{});recompui::hide_all_contexts();

}

bool launch_workshop(bool games){

#ifdef __linux__

    auto executable=pc_root/"build/workshop-linux/YoshiStory-AtelierLinux";

    return std::filesystem::exists(executable) && workshop::linux_process::launch(executable,

        {pc_root.string(),games?"--custom-games":"--editor"},pc_root);

#else

    auto executable=pc_root/"build/workshop-0.21.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.20.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.19.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.18.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.17.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.16.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.15.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.14.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.13.0/YoshiStory-AtelierPC.exe";
    if(!std::filesystem::exists(executable))executable=pc_root/"build/workshop-0.8.0/YoshiStory-AtelierPC.exe";

    if(!std::filesystem::exists(executable))return false;

#ifdef _WIN32

    std::wstring command=L"\""+executable.wstring()+L"\" \""+pc_root.wstring()+L"\" "+(games?L"--custom-games":L"--editor");

    STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_SHOWNORMAL;

    PROCESS_INFORMATION process{};

    if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,pc_root.c_str(),&startup,&process))return false;

    CloseHandle(process.hThread);CloseHandle(process.hProcess);return true;

#else

    return false;

#endif

#endif

}

void collection_page();

void buy_upgrade(workshop::collections::Item item);

void creation(){

    panel(tr("Création","Creation"),tr("Duplique un niveau, modifie ses objets et choisis son icône.\n\nCompose aussi une liste de niveaux pour tes parties personnalisées. L’éditeur s’ouvre dans sa fenêtre Atelier.","Duplicate a level, edit its objects and choose its icon.\n\nArrange levels into custom games. The editor opens in its Workshop window."));

    action(0,tr("Éditeur de niveaux","Level editor"),[]{

        if(!workshop::collections::editor_unlocked()){

            buy_upgrade(workshop::collections::Item::Editor);return;

        }

        if(!launch_workshop(false)){panel(tr("Atelier indisponible","Workshop unavailable"),tr("L’Atelier n’a pas pu être ouvert.","The Workshop could not be opened."));back();}

    });

    action(1,tr("Parties personnalisées","Custom games"),[]{if(!launch_workshop(true)){panel(tr("Atelier indisponible","Workshop unavailable"),tr("L’Atelier n’a pas pu être ouvert.","The Workshop could not be opened."));back();}});back(2);

}

void boss_battle(){
    std::string challenge_text;
#ifdef WORKSHOP_NATIVE_COOP
    const auto mask=workshop_battle_challenges();challenge_text=tr("Défis remportés : ","Challenges won: ")+std::to_string(std::popcount(mask))+" / 18\n\n";
    if(!workshop_battle_challenge_readable())challenge_text+=tr("Le fichier des défis est illisible. Il est conservé ; les nouveaux résultats ne peuvent pas être enregistrés.\n\n","The challenge file cannot be read. It has been preserved; new results cannot be saved.\n\n");
    for(unsigned color=0;color<18;++color){challenge_text+=(mask&(1u<<color))?"[x] ":"[ ] ";if(!workshop::known_yoshis_runtime0355::snapshot().known(color))challenge_text+="???";else if(color==6)challenge_text+=tr("Blanc","White");else if(color==7)challenge_text+=tr("Noir","Black");else if(color<14){const auto& id=workshop::coop_health0250::identities[color];challenge_text+=french?id.french:id.english;}else challenge_text+=color==14?tr("Or","Gold"):color==15?tr("Argent","Silver"):color==16?tr("Feu/lave","Fire/Lava"):tr("Eau/glace","Water/Ice");challenge_text+=color%2?"\n":" | ";}
#endif
    panel("Boss Battle",tr("Dix combats, un Yoshi. Les prochains boss restent cachés.\n\nAu repos : Lucky Tree, bloc d’œufs et pot vers le combat suivant. L’arbre garde les fruits consommés et dispose de deux régénérations par session.\n\nAprès les boss 3, 6 et 9, réveille le personnage numéroté : après une défaite, le même Yoshi reprend avec la vie et les œufs enregistrés. Un grand cœur à usage unique apparaît après le cinquième boss.","Ten battles, one Yoshi. Upcoming bosses stay hidden.\n\nAt each rest stop: Lucky Tree, egg block and pot to the next fight. The tree remembers consumed fruit and has two regenerations per session.\n\nAfter bosses 3, 6 and 9, wake the numbered character: after a defeat, the same Yoshi returns with the saved health and eggs. A one-use big heart appears after the fifth boss."));
#ifdef WORKSHOP_NATIVE_COOP
    action(0,tr("Commencer la bataille","Start battle"),[]{launch_native(4);});
#else
    action(0,tr("Mode indisponible dans cette version","Mode unavailable in this build"),[]{panel("Boss Battle",tr("Cette version ne contient pas les combats natifs.","This build does not include native boss encounters."));back();});
#endif
    action(1,tr("Défis par Yoshi","Yoshi challenges"),[challenge_text]{panel(tr("Les 14 défis","The 14 challenges"),challenge_text);action(0,tr("Retour","Back"),boss_battle);});back(2);
}

void statistics();
void heart_shop(const std::string& notice={});

void buy_heart_bonus(workshop::heart_wallet0283::Bonus bonus){
    namespace h=workshop::heart_wallet0283;
    uint64_t seed=uint64_t(std::chrono::steady_clock::now().time_since_epoch().count());
    try{std::random_device random;seed^=(uint64_t(random())<<32)^random();}catch(...){}
    const auto result=workshop::heart_shop0283::wallet().purchase(bonus,seed);
    std::string message;
    if(result.committed()){purchase_feedback();message=tr("Achat enregistré. Le bonus attend la prochaine tentative.","Purchase saved. The bonus is queued for the next attempt.");}
    else if(result.status==h::Status::AlreadyPending)message=tr("Ce bonus est déjà en attente.","This bonus is already queued.");
    else if(result.status==h::Status::InsufficientFunds)message=tr("Solde insuffisant. Aucun cœur dépensé.","Insufficient balance. No hearts spent.");
    else message=tr("Achat non enregistré. Aucun cœur dépensé.","Purchase not saved. No hearts spent.");
    heart_shop(message);
}

void heart_shop(const std::string& notice){
    namespace h=workshop::heart_wallet0283;
    const auto view=workshop::heart_shop0283::wallet().snapshot();
    std::string text=tr("Cœurs disponibles : ","Available hearts: ")+std::to_string(view.value.balance())+" / 72\n\n";
    text+=tr("Les trois cœurs rouges de chaque niveau Trial rapportent chacun un cœur de monnaie. Ils restent présents et soignent normalement en rejouant. Un cœur déjà compté ne rapporte pas de monnaie supplémentaire.","Each of the three red hearts per Trial level awards one currency heart. They remain present and heal normally on replay. An already counted heart awards no additional currency.");
    text+=tr("\n\nChaque achat rend le même nombre de cœurs déjà comptés, choisis au hasard, de nouveau disponibles à la collecte.","\n\nEach purchase makes the same number of randomly chosen, already counted hearts available to earn again.");
    text+=tr("\n\nBonus pour la prochaine tentative en Story, Trial, Hybride ou Boss Battle :\n5 cœurs : commencer avec 8 PV, sans augmenter le maximum.\n3 cœurs : commencer avec 6 œufs.","\n\nBonuses for the next attempt in Story, Trial, Hybrid or Boss Battle:\n5 hearts: start with 8 HP, without raising the maximum.\n3 hearts: start with 6 eggs.");
    if(view.value.pending.life)text+=tr("\n\nVie de départ : bonus en attente.","\n\nStarting health: bonus queued.");
    if(view.value.pending.eggs)text+=tr("\nŒufs de départ : bonus en attente.","\nStarting eggs: bonus queued.");
    if(!view.readable)text+=tr("\n\nFichier de cœurs indisponible : les achats et la collecte de monnaie sont désactivés. La sauvegarde est conservée.","\n\nHeart file unavailable: purchases and currency collection are disabled. The save is preserved.");
    else if(!view.last_write_succeeded)text+=tr("\n\nLe dernier enregistrement n’a pas abouti.","\n\nThe latest save did not succeed.");
    if(!workshop::heart_shop0283::runtime_available)text+=tr("\n\nLes bonus ne sont pas disponibles dans cette compilation.","\n\nBonuses are unavailable in this build.");
    if(!notice.empty())text+="\n\n"+notice;
    panel(tr("Boutique des cœurs","Heart shop"),text);
    if(workshop::heart_shop0283::runtime_available && view.readable && !view.value.pending.life && view.value.balance()>=h::life_cost)
        action(0,tr("Vie de départ · 5 cœurs","Starting health · 5 hearts"),[]{buy_heart_bonus(h::Bonus::Life);});
    if(workshop::heart_shop0283::runtime_available && view.readable && !view.value.pending.eggs && view.value.balance()>=h::eggs_cost)
        action(1,tr("6 œufs · 3 cœurs","6 eggs · 3 hearts"),[]{buy_heart_bonus(h::Bonus::Eggs);});
    action(3,tr("Retour aux statistiques","Back to statistics"),statistics);
}

void buy_upgrade(workshop::collections::Item item){

    using namespace workshop::collections;

    auto v=view();const bool hybrid=item==Item::Hybrid;const unsigned price=prices[unsigned(item)];

    const std::string name=hybrid?tr("Mode hybride","Hybrid mode"):tr("Éditeur de niveaux","Level editor");

    std::string text=tr("Prix : ","Price: ")+std::to_string(price)+tr(" pièces turquoises\nSolde : "," turquoise coins\nBalance: ")+std::to_string(v.state.balance());

    text+=hybrid?tr("\n\n75 % de fruits hybrides compatibles, Lucky Fruit hybride et six couleurs de Maskass. Les 24 Trial terminés restent nécessaires. Les records sont séparés ; les succès sont partagés.","\n\n75% compatible hybrid fruit, hybrid Lucky Fruit and six Shy Guy colors. All 24 Trial clears are still required. Records are separate; achievements are shared."):

        tr("\n\nRécolte les 480 pièces uniques et achète les deux œufs ainsi que le mode hybride. Les 40 dernières pièces permettent d’acheter l’éditeur.","\n\nCollect all 480 unique coins and purchase both eggs and Hybrid mode. The final 40 coins buy the editor.");

    if(v.state.owns(item))text+=tr("\n\nAchat déjà effectué.","\n\nAlready purchased.");

    auto preview=v.state;const bool allowed=v.readable && preview.buy(item,true);

    if(!v.state.owns(item) && !allowed)text+=tr("\n\nSolde ou conditions d’achat insuffisants.","\n\nInsufficient balance or purchase requirements.");

    panel(name,text);

    if(allowed)action(0,tr("Acheter · ","Buy · ")+std::to_string(price)+tr(" pièces"," coins"),[item]{

        if(purchase(item)){purchase_feedback();buy_upgrade(item);}

        else{panel(tr("Achat non enregistré","Purchase not saved"),tr("Aucune pièce n’a été dépensée.","No coins were spent."));action(0,tr("Retour à la boutique","Back to shop"),collection_page);}

    });

    action(1,tr("Retour à la boutique","Back to shop"),collection_page);

}

void upgrades(){

    panel(tr("Mode hybride et éditeur","Hybrid mode & editor"),tr("Les achats sont conservés dans ton profil. Le mode hybride exige aussi les 24 Trial terminés. L’éditeur utilise les 40 pièces restantes après les trois premiers achats.","Purchases are saved in your profile. Hybrid mode also requires all 24 Trial clears. The editor uses the remaining 40 coins after the first three purchases."));

    action(0,tr("Mode hybride · 200 pièces","Hybrid mode · 200 coins"),[]{buy_upgrade(workshop::collections::Item::Hybrid);});

    action(1,tr("Éditeur · 40 pièces","Editor · 40 coins"),[]{buy_upgrade(workshop::collections::Item::Editor);});

    action(2,tr("Retour à la boutique","Back to shop"),collection_page);

}

void buy_egg(workshop::collections::Item item){

    auto v=workshop::collections::view();const bool white=item==workshop::collections::Item::White;

    const std::string name=white?tr("Œuf du Yoshi blanc","White Yoshi egg"):tr("Œuf du Yoshi noir","Black Yoshi egg");

    if(v.state.owns(item)){

        panel(name,tr("Achat déjà effectué. Termine les 24 Trial puis trouve l’œuf dans les niveaux pour débloquer le Yoshi.","Already purchased. Complete all 24 Trial levels, then find the egg in the courses to unlock the Yoshi."));

        action(0,tr("Retour à la boutique","Back to shop"),collection_page);return;

    }

    std::string text=tr("Prix : 120 pièces turquoises\nSolde : ","Price: 120 turquoise coins\nBalance: ")+std::to_string(v.state.balance());

    text+=tr("\n\nL’achat autorise la recherche de l’œuf, après les 24 Trial. Il faut encore le trouver pour débloquer le Yoshi.","\n\nBuying authorizes the egg search after all 24 Trial levels. You must still find it to unlock the Yoshi.");

    if(!v.readable || v.state.balance()<120)text+=tr("\n\nSolde insuffisant ou fichier de collection indisponible.","\n\nInsufficient balance or collection file unavailable.");

    panel(name,text);

    if(v.readable && v.state.balance()>=120)action(0,tr("Acheter · 120 pièces","Buy · 120 coins"),[item]{

        if(!workshop::collections::purchase(item)){

            panel(tr("Achat non enregistré","Purchase not saved"),tr("Aucune pièce n’a été dépensée. Vérifie le fichier de collection.","No coins were spent. Check the collection file."));

            action(0,tr("Retour à la boutique","Back to shop"),collection_page);

        }else{purchase_feedback();buy_egg(item);}

    });

    action(1,tr("Retour à la boutique","Back to shop"),collection_page);

}

void collection_page(){

    auto v=workshop::collections::view();

    std::string text=tr("Pièces uniques récoltées : ","Unique coins collected: ")+std::to_string(v.state.earned())+" / 480\n";

    text+=tr("Solde : ","Balance: ")+std::to_string(v.state.balance())+tr(" pièces\n\n"," coins\n\n");

    for(unsigned page=0;page<6;page++){

        for(unsigned col=0;col<4;col++)text+=std::to_string(page+1)+"-"+std::to_string(col+1)+": "+std::to_string(std::popcount(v.state.levels[page*4+col].claimed))+"/20  ";

        text+="\n";

    }

    text+=tr("\nChaque pièce donne 3 points à chaque tentative, mais de la monnaie seulement à sa première collecte. La monnaie est conservée dès la collecte.","\nEach coin gives 3 points each run, but currency only on its first collection. Currency is saved immediately.");

    text+=tr("\n\nMode hybride : 200 pièces. Éditeur : les 40 restantes après les trois achats.","\n\nHybrid mode: 200 coins. Editor: the remaining 40 after the first three purchases.");

    if(!v.readable)text+=tr("\n\nFichier illisible conservé : collecte monétaire et achats désactivés.","\n\nUnreadable file preserved: currency collection and purchases disabled.");

    else if(!v.saved)text+=tr("\n\nLe dernier résultat n’a pas pu être enregistré.","\n\nThe latest result could not be saved.");

    panel(tr("Collection et boutique","Collection & shop"),text);

    action(0,tr("Œuf blanc · 120 pièces","White egg · 120 coins"),[]{buy_egg(workshop::collections::Item::White);});

    action(1,tr("Œuf noir · 120 pièces","Black egg · 120 coins"),[]{buy_egg(workshop::collections::Item::Black);});

    action(2,tr("Mode hybride et éditeur","Hybrid mode & editor"),upgrades);

    action(3,tr("Retour aux statistiques","Back to statistics"),statistics);

}

void achievement_details(unsigned level){

    auto ledger=workshop::achievements::view();const auto& e=ledger.levels[level];

    std::string text=tr("Fins de niveau observées : ","Observed clears: ")+std::to_string(e.completions)+"\n";

    text+=tr("Meilleure santé à l’arrivée : ","Best finishing health: ")+std::to_string(e.bestHealth)+" / 8\n";

    text+=std::string(e.fullHealth?"[✓] ":"[ ] ")+tr("Terminer avec toute sa vie","Finish at full health")+"\n";

    text+=std::string(e.allHearts?"[✓] ":"[ ] ")+tr("Trois cœurs rouges puis terminer : ","Three red hearts and finish: ")+std::to_string(e.bestHearts)+" / 3\n\n";

    text+=tr("Terminer avec 30 fruits identiques :\n","Finish with 30 identical fruits:\n");

    const char* fr[]={"Pomme","Banane","Pastèque","Raisin","Melon","","Pêche","Mûre","Bleuet","Poire verte","Prune","Kiwi"};

    const char* en[]={"Apple","Banana","Watermelon","Grapes","Melon","","Peach","Blackberry","Blueberry","Green pear","Plum","Kiwi"};

    for(unsigned fruit=0;fruit<12;fruit++)if(workshop::achievements::fruit_mask&(1u<<fruit))

        text+=std::string(e.sameFruit&(1u<<fruit)?"[✓] ":"[ ] ")+(french?fr[fruit]:en[fruit])+"\n";

    text+=tr("\nStory, Trial et mode hybride comptent pour les mêmes succès. Practice et les niveaux de l’éditeur sont exclus.","\nStory, Trial and Hybrid mode count toward the same achievements. Practice and editor levels are excluded.");

    if(!ledger.readable)text+=tr("\n\nLe fichier de succès est illisible : il est conservé et aucun nouveau résultat ne l’écrase.","\n\nThe achievement file cannot be read: it is preserved and no new result overwrites it.");

    else if(!ledger.saved)text+=tr("\n\nLe dernier résultat n’a pas pu être enregistré.","\n\nThe latest result could not be saved.");

    panel(tr("Succès · niveau ","Achievements · level ")+std::to_string(level/4+1)+"-"+std::to_string(level%4+1),text);

    action(0,tr("Niveau précédent","Previous level"),[level]{achievement_details((level+23)%24);});

    action(1,tr("Niveau suivant","Next level"),[level]{achievement_details((level+1)%24);});

    action(2,tr("Retour aux statistiques","Back to statistics"),statistics);

}

void statistics(){

    if(progress.is_null() || !progress.value("valid",false)){

        panel(tr("Statistiques et succès","Statistics & achievements"),tr("La progression n’a pas pu être lue. Ta sauvegarde est conservée.","Progress could not be read. Your save is preserved."));back();return;

    }

    const unsigned story=progress.value("storyMask",0u),trial=progress.value("trialMask",0u),extras=progress.value("extraYoshiMask",0u);
    const auto hybrid_progress=workshop::hybrid_completion0290::view();
    const auto story_hybrid=workshop::story_hybrid0320::view();
    const auto hard=workshop::story_hard0330::view();

    std::string text=tr("Niveaux terminés en Story : ","Story levels cleared: ")+std::to_string(std::popcount(story))+" / 24\n";

    text+=tr("Niveaux terminés en Trial : ","Trial levels cleared: ")+std::to_string(std::popcount(trial))+" / 24\n";
    if(hybrid_progress.readable)text+=tr("Niveaux terminés en Trial hybride : ","Hybrid Trial levels cleared: ")+std::to_string(hybrid_progress.count)+" / 24\n";
    else text+=tr("Suivi du Trial hybride illisible, conservé.\n","Unreadable Hybrid Trial progress preserved.\n");

    if(story_hybrid.present&&story_hybrid.readable)text+=tr("Mondes terminés en Story hybride : ","Hybrid Story worlds cleared: ")+std::to_string(story_hybrid.count)+" / 6\n";
    if(hard.present&&hard.readable&&hard.scope==workshop::story_hard0330::Scope::Production)text+=tr("Réussites Story difficile : ","Hard Story runs cleared: ")+std::to_string(hard.runs_won)+tr(" · Yoshis : "," · Yoshis: ")+std::to_string(std::popcount(hard.completed))+" / 18\n";
    text+=tr("Yoshis hybrides débloqués : ","Hybrid Yoshis unlocked: ")+std::to_string(std::popcount(extras))+" / 6\n\n";

    const auto elements=workshop::elemental_yoshis0355::view();
    if(elements.initialized&&elements.readable){
        text+=tr("Yoshis élémentaires trouvés : ","Elemental Yoshis found: ")+std::to_string(std::popcount(elements.found))+" / 2\n";
        text+=std::string(workshop::known_yoshis_runtime0355::snapshot().label(16,tr("Feu/lave","Fire/Lava")))+" : ";text+=elements.found&1?tr("trouvé","found"):tr("à trouver","not found");
        text+=" · "+std::string(workshop::known_yoshis_runtime0355::snapshot().label(17,tr("Eau/glace","Water/Ice")))+" : ";text+=elements.found&2?tr("trouvé","found"):tr("à trouver","not found");text+="\n\n";
    }
    const auto metals=workshop::metal_yoshis0300::view();
    if(metals.readable){
        text+=tr("Yoshis or et argent trouvés : ","Gold and Silver Yoshis found: ")+std::to_string(std::popcount(metals.found))+" / 2\n";
        text+=std::string(workshop::known_yoshis_runtime0355::snapshot().label(14,tr("Or","Gold")))+" : ";text+=(metals.found&1?tr("trouvé","found"):tr("à trouver","not found"));
        text+=" · "+std::string(workshop::known_yoshis_runtime0355::snapshot().label(15,tr("Argent","Silver")))+" : ";text+=(metals.found&2?tr("trouvé","found"):tr("à trouver","not found"));
        text+=tr("\nLes 24 Trial hybrides terminés permettent de chercher leurs œufs aux emplacements du Maskass blanc, un à la fois.\n\n","\nClear all 24 Hybrid Trial levels to search for their eggs at White Shy Guy locations, one at a time.\n\n");
        if(!metals.saved)text+=tr("Dernier résultat or/argent non enregistré.\n","Latest Gold/Silver result not saved.\n");
    }else text+=tr("Suivi or/argent illisible, conservé.\n\n","Unreadable Gold/Silver progress preserved.\n\n");

    for(unsigned page=0;page<6;page++){

        text+=tr("Page ","Page ")+std::to_string(page+1)+" : ";

        for(unsigned level=0;level<4;level++){

            unsigned bit=1u<<(page*4+level);text+=std::to_string(level+1)+"[";

            text+=(story&bit)?"S":"-";text+=(trial&bit)?"T":"-";text+=hybrid_progress.readable?((hybrid_progress.mask&bit)?"H":"-"):"?";text+="]  ";

        }

        text+="\n";

    }

    auto ledger=workshop::achievements::view();unsigned full=0,same=0,hearts=0;

    for(const auto& level:ledger.levels){full+=level.fullHealth;same+=std::popcount(level.sameFruit);hearts+=level.allHearts;}

    text+=tr("\nS = Story, T = Trial, H = Hybride\n\nPleine santé : ","\nS = Story, T = Trial, H = Hybrid\n\nFull health: ")+std::to_string(full)+" / 24\n";

    text+=tr("Défis 30 fruits identiques : ","30 identical fruit challenges: ")+std::to_string(same)+" / 264\n";

    text+=tr("Trois cœurs rouges et niveau terminé : ","Three red hearts and level cleared: ")+std::to_string(hearts)+" / 24\n";

    text+=tr("\nLes succès comptent les tentatives réellement observées, sans attribution rétroactive.","\nAchievements count observed runs, without retroactive awards.");

    if(!ledger.readable)text+=tr("\nFichier de succès illisible, conservé.","\nUnreadable achievement file preserved.");

    else if(!ledger.saved)text+=tr("\nDernier résultat non enregistré.","\nLatest result not saved.");

    if(hybrid_progress.readable && !hybrid_progress.saved)text+=tr("\nProgression hybride non enregistrée.","\nHybrid progress not saved.");
    if(!hybrid_progress.legacy_readable)text+=tr("\nAnciens records hybrides illisibles : import différé, fichier conservé.","\nPrevious hybrid records unreadable: import deferred, file preserved.");

    panel(tr("Statistiques et succès","Statistics & achievements"),text);

    action(0,tr("Succès par niveau","Achievements by level"),[]{achievement_details(0);});

    action(1,tr("Collection et boutique","Collection & shop"),collection_page);back(2);
    action(3,tr("Boutique des cœurs","Heart shop"),[]{heart_shop();});

}

void hybrid(){

    unsigned trial=progress.value("trialMask",0u);bool complete=progress.value("valid",false) && trial==0xFFFFFF;

    std::string body=complete?tr("Condition remplie : les 24 niveaux Trial sont terminés.\n\n","Requirement met: all 24 Trial levels are cleared.\n\n"):

        tr("Termine les 24 niveaux Trial pour débloquer ce mode.\nProgression : ","Clear all 24 Trial levels to unlock this mode.\nProgress: ")+std::to_string(std::popcount(trial))+" / 24\n\n";

    const bool bought=workshop::collections::view().state.owns(workshop::collections::Item::Hybrid);

    body+=tr("75 % des fruits ordinaires deviennent des hybrides compatibles. Les melons sont conservés. Lucky Fruit est choisi parmi les six hybrides ; les Maskass utilisent leurs six couleurs.\n\nLes records de ce mode sont séparés. Les succès, dont les 30 fruits identiques, sont communs au Trial classique et au mode hybride.","75% of ordinary fruit become compatible hybrids. Melons stay unchanged. Lucky Fruit is one of the six hybrids; Shy Guys use their six colors.\n\nMode records are separate. Achievements, including 30 identical fruit, are shared with classic Trial.");

    body+=bought?tr("\n\nMode acheté.","\n\nMode purchased."):tr("\n\nPrix : 200 pièces turquoises.","\n\nPrice: 200 turquoise coins.");

    if(!workshop::hybrid_mode::readable())body+=tr("\nFichier de records illisible conservé ; mode indisponible.","\nUnreadable records file preserved; mode unavailable.");

    const auto hybrid_progress=workshop::hybrid_completion0290::view();
    if(hybrid_progress.readable)body+=tr("\n\nNiveaux terminés en Trial hybride : ","\n\nHybrid Trial levels cleared: ")+std::to_string(hybrid_progress.count)+" / 24";
    else body+=tr("\n\nSuivi des réussites hybrides illisible, conservé.","\n\nUnreadable hybrid completion progress preserved.");
    if(hybrid_progress.readable && !hybrid_progress.saved)body+=tr("\nDernière progression hybride non enregistrée.","\nLatest hybrid progress not saved.");
    if(!hybrid_progress.legacy_readable)body+=tr("\nImport des anciens records hybrides différé ; fichier conservé.","\nPrevious hybrid record import deferred; file preserved.");

    body+=tr("\n\nAprès les 24 réussites hybrides, deux nouveaux œufs permettent de trouver les Yoshis or et argent. Leurs fruits hybrides donnent 3 points et 3 de vie ; les fruits classiques donnent 2 et 2.","\n\nAfter all 24 Hybrid clears, find two new eggs to recruit Gold and Silver Yoshis. Hybrid fruits give them 3 points and 3 health; classic fruits give 2 of each.");

    panel(tr("Mode hybride","Hybrid mode"),body);

    if(complete && bought && workshop::hybrid_mode::readable())action(0,tr("Jouer au mode hybride","Play Hybrid mode"),[]{launch_native(3);});

    else if(!bought)action(0,tr("Acheter le mode","Purchase mode"),[]{buy_upgrade(workshop::collections::Item::Hybrid);});

    back(1);

}

void story_modes0320(){
    const auto local=workshop::story_hybrid0320::view();
    std::string text=tr("Le Story classique conserve sa partie et ses records. Le Story hybride possède sa propre partie et ses records.\n\nDans le Story hybride, un nouvel œuf hybride devient trouvable après chaque monde terminé. Les Yoshis déjà obtenus et les achats sont conservés. Les fruits et les Maskass suivent les règles du mode hybride.","Classic Story keeps its own run and records. Hybrid Story has a separate run and records.\n\nIn Hybrid Story, one new hybrid egg becomes searchable after each world cleared. Previously acquired Yoshis and purchases are shared. Fruit and Shy Guys use Hybrid mode rules.");
    if(local.present&&local.readable)text+=tr("\n\nMondes terminés en Story hybride : ","\n\nHybrid Story worlds cleared: ")+std::to_string(local.count)+" / 6";
    if(!local.readable||!local.saved)text+=tr("\nProgression hybride illisible ou non enregistrée, conservée.","\nUnreadable or unsaved Hybrid Story progress preserved.");
    text+=tr("\n\nAccès hybride : mode acheté et 24 Trial classiques terminés.","\n\nHybrid access: purchased mode and 24 classic Trial clears.");
    panel(tr("Story","Story"),text);
    action(0,tr("Story classique","Classic Story"),[]{launch_native(0);});
    action(1,tr("Story hybride","Hybrid Story"),[]{launch_native(5);});
    action(2,tr("Story difficile","Hard Story"),story_hard_page0330);back(3);
}
void story_hard_page0330(){
    const auto rights=workshop::story_hard_bridge0330::access();const auto local=workshop::story_hard0330::view();
    const bool preview=workshop::story_hard_bridge0330::preview();
    std::string text=tr("Commence avec le Yoshi vert seul. Un œuf aléatoire apparaît près du point 4 de chaque niveau ; dans le 5-3, il se trouve près de l’entrée de la dernière salle. Les Yoshis recrutés puis gardés en vie jusqu’à Bowser valident leur réussite au terme des six niveaux.\n\nLes réussites de chaque Yoshi se cumulent entre les parties. Un Yoshi perdu puis secouru ne valide pas cette fois-ci. Cette partie et ses records sont séparés des autres Story.\n\n","Start with Green Yoshi alone. Each level has one random egg near checkpoint 4; in 5-3 it is near the final room entrance. Yoshis recruited and kept alive through Bowser earn their clear after all six levels.\n\nEach Yoshi’s clears accumulate across runs. A lost and rescued Yoshi does not earn this run’s clear. The run and its records are separate from other Story modes.\n\n");
    if(preview)text+=tr("Aperçu de test séparé : aucun succès ni déblocage permanent n’est attribué.\n\n","Separate test preview: no permanent achievement or unlock is awarded.\n\n");
    else{
        text+=tr("Condition : trouver les 16 Yoshis et remporter le Boss Battle avec chacun.\nYoshis trouvés : ","Requirement: find all 16 Yoshis and win Boss Battle with each one.\nYoshis found: ")+std::to_string(std::popcount(rights.owned16))+" / 16\n";
        text+=tr("Défis Boss Battle validés : ","Boss Battle challenges cleared: ")+std::to_string(std::popcount(rights.bossWins16))+" / 16\n\n";
    }
    if(local.present&&local.readable){
        text+=tr("Parties réussies : ","Successful runs: ")+std::to_string(local.runs_won)+"\n";
        text+=tr("Yoshis ayant réussi : ","Yoshis with a clear: ")+std::to_string(std::popcount(local.completed))+" / 18\n";
    }
    const bool play=(preview||workshop::story_hard_access0330::entitled(rights))&&local.readable&&local.saved;
    if(!play)text+=tr("\nMode verrouillé jusqu’à validation de toutes les conditions. Les sauvegardes sont conservées.","\nLocked until all requirements are met. Existing saves are preserved.");
    panel(tr("Story difficile","Hard Story"),text);
    if(play)action(0,preview?tr("Jouer à l’aperçu","Play preview"):tr("Jouer au Story difficile","Play Hard Story"),[]{launch_native(6);});
    action(1,tr("Retour aux modes Story","Back to Story modes"),story_modes0320);back(2);
}
void options(){

    std::string description=tr("Langue de ce menu : Français\n\nLes textes du jeu d’origine restent en anglais pour le moment. Le volume, les manettes et l’affichage se règlent dans les paramètres du jeu.","Menu language: English\n\nThe original game remains in English for now. Volume, controllers and display can be adjusted in game settings.");
    if(!preferences_saved)description+=tr("\n\nLe réglage n’a pas pu être enregistré. Le fichier existant est conservé.","\n\nThe setting could not be saved. The existing file is preserved.");
    panel(tr("Options","Options"),description);

    action(0,french?"Menu language: English":"Langue du menu : Français",[]{

        if(save_menu_preferences(!french,purchase_sound))french=!french;update_titles();options();

    });

    action(1,tr("Réglages du jeu","Game settings"),[]{recompui::config::open();});back(2);
    action(3,purchase_sound?tr("Son des achats : activé","Purchase sounds: on"):tr("Son des achats : désactivé","Purchase sounds: off"),[]{
        if(save_menu_preferences(french,!purchase_sound))purchase_sound=!purchase_sound;
        options();
    });

}


void trial_modes0352(){
    panel(tr("Trial","Trial"),tr("Rejoue les niveaux classiques ou découvre les six niveaux bonus associés aux pages du livre.","Replay classic courses or discover the six bonus courses associated with the book pages."));
    action(0,tr("Trial classique","Classic Trial"),[]{launch_native(1);});
    action(1,tr("Niveaux bonus","Bonus courses"),[]{bonus_levels0352(0);});back(2);
}
void launch_bonus0353(unsigned page,bool new_attempt){
    if(!workshop::bonus_game0352::request_bonus(page+1,new_attempt)){
        bonus_levels0352(page);
        footer->set_text(tr("Ce bonus ne peut pas être ouvert pour le moment. La progression est conservée.","This bonus cannot be opened right now. Progress has been preserved."));return;
    }
    launch_native(workshop::game_modes0320::Bonus);
}
bool bonus_outcome_page0353(const workshop::bonus_game0352::View& state){
    if(!state.completed&&!state.game_over)return false;
    const unsigned selected=state.selected_bonus;
    std::string text;
    if(selected>=1&&selected<=6){
        text=tr("Niveau bonus 7-","Bonus course 7-")+std::to_string(selected)+"\n\n";
        if(state.completed){
            text+=tr("Le parcours est terminé !", "The course is complete!");
            const bool saved=state.session.readable&&state.session.last_write_succeeded&&(state.session.completed_mask&(1u<<(selected-1)));
            text+=saved?tr("\nLa réussite est enregistrée.","\nThe clear has been saved."):tr("\nL’enregistrement de la réussite n’a pas pu être confirmé. Les fichiers existants sont conservés.","\nThe clear could not be confirmed as saved. Existing files have been preserved.");
        }else text+=tr("Tous les Yoshis de cette tentative ont été perdus. Les réussites déjà enregistrées sont conservées.","All Yoshis in this attempt have been lost. Previously saved clears are preserved.");
    }else text=tr("Consulte les niveaux bonus pour choisir la suite. La progression enregistrée est conservée.","Open the bonus courses to choose what comes next. Saved progress is preserved.");
    panel(state.completed?tr("Bonus terminé","Bonus complete"):tr("Équipe épuisée","Team exhausted"),text);
    if(selected>=1&&selected<=6){
        const unsigned page=selected-1;
        const bool playable=state.full_route_available&&(state.access.unlocked_mask&(1u<<page))&&state.session.readable&&state.session.last_write_succeeded;
        if(playable)action(0,state.completed?tr("Rejouer","Play again"):tr("Nouvelle tentative","New attempt"),[page]{launch_bonus0353(page,true);});
        action(1,tr("Voir ce bonus","View this bonus"),[page]{bonus_levels0352(page);});
        action(2,tr("Autres bonus","Other bonuses"),[page]{bonus_levels0352((page+1)%6);});
    }else action(1,tr("Niveaux bonus","Bonus courses"),[]{bonus_levels0352(0);});
    action(3,tr("Menu principal","Main menu"),home);return true;
}
void bonus_levels0352(unsigned page){
    page%=6;
    workshop::bonus_game0352::refresh(recomp::get_config_path());
    const auto state=workshop::bonus_game0352::view();
    const auto combined=state.access.classic_mask|state.access.hybrid_mask;
    const unsigned cleared=std::popcount((combined>>(page*4))&15u);
    const bool unlocked=(state.access.unlocked_mask&(1u<<page))!=0;
    std::string text=tr("Termine les quatre niveaux de cette page en Trial classique ou hybride. Les réussites des deux modes comptent ensemble.","Clear this page's four courses in classic or Hybrid Trial. Clears from both modes count together.");
    text+=tr("\n\nProgression : ","\n\nProgress: ")+std::to_string(cleared)+" / 4";
    text+=unlocked?tr("\nCondition remplie.","\nRequirement met."):tr("\nCondition encore à remplir.","\nRequirement not yet met.");
    text+=tr("\n\nLe parcours réunira toutes les salles du monde, dans un ordre conservé pendant la tentative. Chaque bonus possède sa propre finale.","\n\nThe route will include every room in the world, in an order kept throughout the attempt. Each bonus has its own finale.");
    if(!state.full_route_available)
        text+=tr("\n\nCe niveau bonus est en préparation et n'est pas encore jouable.","\n\nThis bonus course is being prepared and is not playable yet.");
    else
        text+=tr("\n\nDans les salles à un seul pot, maintiens L en entrant pour revenir en arrière. Entre normalement pour avancer.","\n\nIn rooms with one pot, hold L while entering to go back. Enter normally to move forward.");
    if(state.full_route_available)
        text+=tr("\n\nAprès fermeture du jeu, Reprendre revient à la première salle, avec le même ordre et les salles déjà validées. Pendant la partie, les checkpoints conservent la reprise native.","\n\nAfter closing the game, Resume returns to the first room, keeping the same order and cleared rooms. During play, checkpoints retain native continuation.");
    if(!state.access.classic_readable||!state.access.hybrid_readable)
        text+=tr("\nUne progression n'a pas pu être lue ; elle est conservée.","\nSome progress could not be read; it has been preserved.");
    panel(tr("Niveau bonus 7-","Bonus course 7-")+std::to_string(page+1),text);
    const bool playable=state.full_route_available&&unlocked&&state.session.readable&&state.session.last_write_succeeded;
    if(playable){
        const auto& attempt=state.session.attempts[page];
        const bool ended_here=state.selected_bonus==page+1&&(state.completed||state.game_over);
        const bool resume=attempt&&attempt->phase!=workshop::bonus_session0351::Phase::Completed&&!ended_here;
        action(0,resume?tr("Reprendre","Resume"):tr("Jouer","Play"),[page,resume]{launch_bonus0353(page,!resume);});
        if(resume)action(1,tr("Recommencer","Restart"),[page]{launch_bonus0353(page,true);});
        action(2,tr("Bonus suivant","Next bonus"),[page]{bonus_levels0352((page+1)%6);});
        action(3,tr("Retour au Trial","Back to Trial"),trial_modes0352);
    }else{
        action(0,tr("Bonus précédent","Previous bonus"),[page]{bonus_levels0352((page+5)%6);});
        action(1,tr("Bonus suivant","Next bonus"),[page]{bonus_levels0352((page+1)%6);});
        action(2,tr("Retour au Trial","Back to Trial"),trial_modes0352);back(3);
    }
}

void choose(unsigned item){

    switch(item){case 0:story_modes0320();break;case 1:trial_modes0352();break;case 2:creation();break;case 3:statistics();break;case 4:hybrid();break;case 5:options();break;case 6:launch_native(2);break;case 7:boss_battle();break;}

}

void create(LauncherMenu* menu){

    menu_context=get_current_context();

    menu->remove_default_title();

    auto unused=menu->init_game_options_menu(u8"yoshisstory.n64.us","yoshisstory","DinoTale",{},GameOptionsMenuLayout::Left);unused->display_hide();

    auto container=menu->get_menu_container();container->set_inset(0);

    auto root=menu_context.create_element<BookRoot>(container);rect(root,0,0,100,100);root->set_background_color(Color{72,155,176,255});

    book=menu_context.create_element<Element>(root);book->set_position(Position::Absolute);book->set_left(0);book->set_top(0);

    std::ifstream art_file("assets/workshop-menu/book-approved.png",std::ios::binary);

    std::vector<char> art_bytes((std::istreambuf_iterator<char>(art_file)),{});

    if(art_bytes.empty())throw std::runtime_error("Missing approved book menu artwork");

    recompui::queue_image_from_bytes_file("?/workshop/book-approved",art_bytes);

    auto art=menu_context.create_element<Image>(book,"?/workshop/book-approved");rect(art,0,0,100,100);art->set_pointer_events(PointerEvents::None);
    // The corrected reference approves only the right-hand illustration.
    // Keep the original full backdrop and clip the unmodified new PNG to its
    // right half; the live left-hand menu and original title stay intact.
    std::ifstream right_file("assets/workshop-menu/book-right-approved.png",std::ios::binary);
    std::vector<char> right_bytes((std::istreambuf_iterator<char>(right_file)),{});
    if(!right_bytes.empty()){
        recompui::queue_image_from_bytes_file("?/workshop/book-right-approved",right_bytes);
        auto right_clip=menu_context.create_element<Element>(book);rect(right_clip,50,0,50,100);
        right_clip->set_overflow(Overflow::Hidden);right_clip->set_pointer_events(PointerEvents::None);
        auto right_art=menu_context.create_element<Image>(right_clip,"?/workshop/book-right-approved");
        rect(right_art,-100,0,200,100);right_art->set_pointer_events(PointerEvents::None);
        int width=0,height=0,channels=0;
        auto pixels=stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(right_bytes.data()),int(right_bytes.size()),&width,&height,&channels,4);
        if(pixels){
            const auto layers=workshop::book_unlock_art0355::layers({pixels,size_t(width)*height*4},unsigned(width),unsigned(height));
            stbi_image_free(pixels);
            for(unsigned i=0;i<layers.size();++i){
                const auto& data=layers[i];const auto key="?/workshop/locked-yoshi-0355-"+std::to_string(data.color);
                recompui::queue_image_from_bytes_rgba32(key,data.rgba,data.width,data.height);
                auto layer=menu_context.create_element<Image>(right_clip,key);
                rect(layer,200.f*data.x/1448.f-100.f,100.f*data.y/1086.f,200.f*data.width/1448.f,100.f*data.height/1086.f);
                layer->set_pointer_events(PointerEvents::None);unknown_book_yoshis0355[i]=layer;
            }
            if(layers.size()!=6)fprintf(stderr,"[BookArtwork0355] Unsupported illustration dimensions; approved image retained.\n");
        }else fprintf(stderr,"[BookArtwork0355] Could not decode illustration for lock effect.\n");
    }else fprintf(stderr,"[BookMenu] corrected right illustration missing; original artwork retained.\n");


    auto cover=menu_context.create_element<Element>(book);rect(cover,4,31.5,45,60.7);cover->set_background_color(paper);cover->set_border_radius(30);cover->set_border_width(3);cover->set_border_color(Color{230,202,160,255});

    main_page=menu_context.create_element<Element>(book);rect(main_page,0,0,100,100);main_page->set_as_navigation_container(NavigationType::Vertical);main_page->set_nav_wrapping(true);

    for(unsigned i=0;i<choices.size();i++){

        choices[i]=menu_context.create_element<BookButton>(main_page,&choice_labels[i],[i]{choose(i);});

        rect(choices[i],6.8,33.1+i*7.15,39.4,6.0);

    }

    resume_button=menu_context.create_element<BookButton>(book,&resume_label,resume_game);

    rect(resume_button,64,3,31,5);resume_label->set_font_size(23);resume_button->display_hide();

    detail_page=menu_context.create_element<Element>(book);rect(detail_page,4.8,32,43.2,59.3);detail_page->set_background_color(paper);detail_page->set_as_navigation_container(NavigationType::Vertical);

    detail_title=menu_context.create_element<Label>(detail_page,"",LabelStyle::Large);rect(detail_title,4,2,92,10);detail_title->set_font_size(35);detail_title->set_color(ink);detail_title->set_font_weight(800);

    auto body_scroll=menu_context.create_element<Element>(detail_page);rect(body_scroll,4,14,92,54);body_scroll->set_overflow_y(Overflow::Auto);

    detail_body=menu_context.create_element<Label>(body_scroll,"",LabelStyle::Small);detail_body->set_width(100,Unit::Percent);detail_body->set_height_auto();detail_body->set_font_size(23);detail_body->set_line_height(29);detail_body->set_font_weight(400);detail_body->set_letter_spacing(0);detail_body->set_color(ink);

    for(unsigned i=0;i<4;i++){

        actions[i]=menu_context.create_element<BookButton>(detail_page,&action_labels[i],[i]{auto callback=action_callbacks[i];if(callback)callback();});

        rect(actions[i],4,69+i*7.2,92,6.1);action_labels[i]->set_font_size(23);

    }

    footer=menu_context.create_element<Label>(root,"",LabelStyle::Small);rect(footer,0,96,100,3);footer->set_color(Color{255,255,235,255});footer->set_font_size(20);footer->set_text_align(TextAlign::Center);

    menu_context.set_autofocus_element(choices[0]);

    refresh_canonical_progress0320();

    home();
    if(!ultramodern::is_game_started())claim_book_music0310();

    // Non-keyboard-capturing access to the same book from Story, mode menus

    // and credits, including places where the original game has no Exit.

    menu_context.close();game_link_context=recompui::create_context();game_link_context.open();

    game_link_context.set_captures_input(false);game_link_context.set_captures_mouse(true);

    Label* link_label=nullptr;

    auto link=game_link_context.create_element<BookButton>(game_link_context.get_root_element(),&link_label,open_book);

    rect(link,88,94,11,4.5);link_label->set_text(tr("Menu principal","Main menu"));link_label->set_font_size(17);link_label->set_letter_spacing(0);

    game_link_context.close();menu_context.open();

}

#ifdef WORKSHOP_CONTENT_BUILD

void diagnostic_shop(){

    if(!workshop_test_book_shop())return;

    static uint64_t last=0;

    static unsigned step=0;

    const auto now=SDL_GetTicks64();

    if(!last){last=now;return;}

    if(now-last<300)return;

    last=now;

    auto click=[](Element* button){button->focus();static_cast<BookButton*>(button)->diagnostic_click();};

    auto key=[](Rml::Input::KeyIdentifier code){

        auto* context=get_current_context().get_document()->GetContext();

        context->ProcessKeyDown(code,0);context->ProcessKeyUp(code,0);

    };

    if(step==0){

        auto* saved=session_menu_button;

        for(unsigned i=0;i<16;++i){

            auto* probe=menu_context.create_element<SessionMenuButton>(detail_page,"lifetime-test",ButtonStyle::Primary);

            session_menu_button=probe;

            if(!detail_page->remove_child(probe))throw std::runtime_error("Lifetime fixture could not detach its button");

            if(session_menu_button){fprintf(stderr,"[BookShopTest] FAILED destroyed session button retained\n");std::fflush(nullptr);shutdown_book_music0310();std::_Exit(24);}

        }

        session_menu_button=saved;

        fprintf(stderr,"[BookShopTest] session button lifetime: 16 create/destroy cycles passed.\n");fflush(stderr);

    }

    const unsigned phase=step%26;

    if(detail_open){

        Element* first=nullptr;

        for(auto button:actions)if(button->get_display()!=Display::None){first=button;break;}

        // At the first key after a page switch, the first visible action must

        // have received focus even on pages where slot zero is unavailable.

        if(phase==1 || phase==3 || phase==6 || phase==9 || phase==12 || phase==14 || phase==18 || phase==22){

            if(!first || menu_context.get_focused_element()!=first){

                fprintf(stderr,"[BookShopTest] FAILED first-visible focus phase=%u\n",phase);std::fflush(nullptr);shutdown_book_music0310();std::_Exit(21);

            }

        }

    }

    fprintf(stderr,"[BookShopTest] step=%u phase=%u detail=%d focus=%s\n",step,phase,int(detail_open),

        menu_context.get_focused_element()?menu_context.get_focused_element()->get_id().c_str():"none");fflush(stderr);

    switch(phase){

    case 0:click(choices[3]);break;

    case 1:key(Rml::Input::KI_LEFT);break;

    case 2:click(actions[1]);break;

    case 3:key(Rml::Input::KI_DOWN);break;

    case 4:

        fprintf(stderr,"[BookShopTest] stale-home-focus accepted=%d\n",int(choices[3]->focus()));fflush(stderr);

        key(Rml::Input::KI_DOWN);

        if(menu_context.get_focused_element()!=actions[0]){fprintf(stderr,"[BookShopTest] FAILED stale focus recovery\n");std::fflush(nullptr);shutdown_book_music0310();std::_Exit(22);}

        key(Rml::Input::KI_DOWN);

        if(menu_context.get_focused_element()!=actions[1]){fprintf(stderr,"[BookShopTest] FAILED navigation after recovery\n");std::fflush(nullptr);shutdown_book_music0310();std::_Exit(23);}

        fprintf(stderr,"[BookShopTest] stale focus recovered; next Down reached second shop button.\n");break;

    case 5:click(actions[0]);break;

    case 6:key(Rml::Input::KI_LEFT);break;

    case 7:click(actions[workshop::collections::view().state.owns(workshop::collections::Item::White)?0:1]);break;

    case 8:click(actions[1]);break;

    case 9:key(Rml::Input::KI_UP);break;

    case 10:click(actions[workshop::collections::view().state.owns(workshop::collections::Item::Black)?0:1]);break;

    case 11:click(actions[2]);break;

    case 12:key(Rml::Input::KI_RIGHT);break;

    case 13:click(actions[0]);break;

    case 14:key(Rml::Input::KI_DOWN);break;

    case 15:click(actions[1]);break;

    case 16:click(actions[2]);break;

    case 17:click(actions[1]);break;

    case 18:key(Rml::Input::KI_LEFT);break;

    case 19:click(actions[1]);break;

    case 20:click(actions[3]);break;

    case 21:click(actions[0]);break;

    case 22:key(Rml::Input::KI_DOWN);break;

    case 23:click(actions[1]);break;

    case 24:click(actions[2]);break;

    case 25:key(Rml::Input::KI_ESCAPE);break;

    }

    if(++step==78){

        fprintf(stderr,"[BookShopTest] passed 78 native click/key steps; isolated test complete.\n");

        std::fflush(nullptr);shutdown_book_music0310();std::_Exit(0);

    }

}

#endif

void resolve_deferred_focus(){
    // Rml applies display changes after this callback. Resolve focus after all
    // ui_frame paths and wait one update before focusing the newly shown page.
    if(menu_context==ContextId::null() || !focus_pending ||
       !recompui::is_context_shown(menu_context) ||
       recompui::config::get_config_modal()->is_open_now())return;
    auto previous=recompui::try_close_current_context();
    menu_context.open();
    if(focus_waited && focus_pending->is_focusable()==Element::CanFocus::Yes && focus_pending->focus())focus_pending=nullptr;
    else focus_waited=true;
    menu_context.close();if(previous!=ContextId::null())previous.open();
}

void ui_frame(){

    if(menu_context==ContextId::null())return;

    if(resume_pending){

        resume_pending=false;
        if(workshop::bonus_game0352::active()&&!bonus_resume_allowed0353(workshop::bonus_game0352::view())){open_book();return;}

        if(recompui::is_context_shown(menu_context))recompui::hide_context(menu_context);

        recompui::set_idle_ui_rendering(false);

        release_book_music0310();
        workshop::book::resume();

        fprintf(stderr,"[BookLink] ui=resume\n");fflush(stderr);

        return;

    }

    int start=startup_mode.exchange(-1);

    if(start>=0 && !ultramodern::is_game_started()){

        auto previous=recompui::try_close_current_context();menu_context.open();

        launch_native(start);menu_context.close();if(previous!=ContextId::null())previous.open();return;

    }

#ifdef WORKSHOP_NATIVE_COOP
    if(workshop_battle_failed()){
        static bool shown=false;if(!shown){shown=true;open_book();auto previous=recompui::try_close_current_context();menu_context.open();loading_mode=true;
            panel(tr("Bataille interrompue","Battle ended"),tr("Le parcours s’est arrêté avant la dixième victoire. Aucun défi de parcours complet n’est attribué.","The run ended before the tenth victory. No full-run challenge is awarded."));
            action(0,tr("Menu principal","Main menu"),[]{route_native(-1);});menu_context.close();if(previous!=ContextId::null())previous.open();}return;
    }
    if(workshop_battle_done()){
        static bool shown=false;if(!shown){shown=true;open_book();auto previous=recompui::try_close_current_context();menu_context.open();loading_mode=true;
            std::string result=tr("Les dix combats ont été remportés !\n\n","All ten battles have been won!\n\n");
            result+=workshop_battle_awarded()?tr("Le défi de ce Yoshi est enregistré.","This Yoshi’s challenge has been saved."):workshop_battle_diagnostic()?tr("Profil de test : aucun défi permanent n’est attribué.","Test profile: no permanent challenge is awarded."):tr("Le résultat n’a pas pu être enregistré. Le fichier des défis existant est conservé.","The result could not be saved. The existing challenge file has been preserved.");
            panel(tr("Bataille terminée !","Battle complete!"),result);
            action(0,tr("Menu principal","Main menu"),[]{route_native(-1);});menu_context.close();if(previous!=ContextId::null())previous.open();}return;
    }
    if(workshop_boss_solo_done()){
        static bool shown=false;
        if(!shown){
            shown=true;open_book();auto previous=recompui::try_close_current_context();menu_context.open();loading_mode=true;
            panel(tr("Premier enchaînement terminé","First sequence complete"),tr("Le premier combat et la salle de repos sont terminés.\n\nLes rencontres suivantes sont en préparation. Cet essai ne valide pas le défi des dix boss.","The first battle and rest room are complete.\n\nFurther encounters are being prepared. This test does not complete the ten-boss challenge."));
            action(0,tr("Fermer cet essai","Close this test"),[]{close_custom_runtime(false);});
            menu_context.close();if(previous!=ContextId::null())previous.open();
        }
        return;
    }
#endif
    if(workshop::custom_session::ready()){

        static bool shown=false;static uint64_t opened=0;

        if(!shown){

            shown=true;opened=SDL_GetTicks64();open_book();

            auto previous=recompui::try_close_current_context();menu_context.open();loading_mode=true;

            panel(tr("Niveau personnalisé terminé","Custom level complete"),tr("30 fruits récoltés !\n\nLa partie passe automatiquement à la suite dans quelques secondes. Ta progression personnalisée est conservée séparément.","30 fruits collected!\n\nYour custom game continues automatically in a few seconds. Its progress is saved separately."));

            action(0,tr("Continuer maintenant","Continue now"),[]{close_custom_runtime(true);});

            action(1,tr("Arrêter ici et garder la progression","Stop here and keep progress"),[]{close_custom_runtime(false);});

            menu_context.close();if(previous!=ContextId::null())previous.open();

        }

        if(workshop::custom_session::saved() && SDL_GetTicks64()-opened>=6000 && workshop::book::snapshot().paused){

            close_custom_runtime(true);

        }

        return;

    }

    if(session_mode.load()==workshop::game_modes0320::StoryHard){
        static std::string shown_error;const auto hard_error=workshop::story_hard_bridge0330::error();
        if(hard_error.empty())shown_error.clear();
        else if(hard_error!=shown_error){
            shown_error=hard_error;open_book();auto previous=recompui::try_close_current_context();menu_context.open();
            panel(tr("Story difficile interrompu","Hard Story paused"),std::string(tr("La reprise ou l’enregistrement n’a pas pu être validé. Les fichiers existants sont conservés.\n\n","The continuation or save could not be validated. Existing files have been preserved.\n\n"))+(workshop_hard_continue_refused()?tr("Le choix Nouvelle partie reste disponible dans le jeu.","New Game is still available in the game."):tr("Reviens au menu principal avant de réessayer.","Return to the main menu before trying again.")));
            action(0,tr("Menu principal","Main menu"),[]{route_native(-1);});
            if(workshop_hard_continue_refused())action(1,tr("Revenir au choix de partie","Return to run selection"),resume_game);
            menu_context.close();if(previous!=ContextId::null())previous.open();return;
        }
    }
    if(workshop::book::take_open_request())open_book();

    bool book_shown=recompui::is_context_shown(menu_context);
    if(book_shown)refresh_book_yoshis0355();

    const auto native=workshop::book::snapshot();

    bool show_link=ultramodern::is_game_started() && native.ready && workshop::game_modes0320::story(session_mode.load()) && boot_mode.load()<0 && !book_shown && !recompui::config::get_config_modal()->is_open_now();

    if(session_menu_button){if(workshop::game_modes0320::story(session_mode.load()))session_menu_button->display_show();else session_menu_button->display_hide();}

    bool link_shown=recompui::is_context_shown(game_link_context);

    if(show_link && !link_shown)recompui::show_context(game_link_context,"");

    else if(!show_link && link_shown)recompui::hide_context(game_link_context);

    if(!book_shown)return;

    auto previous=recompui::try_close_current_context();menu_context.open();

    auto s=workshop::book::snapshot();

    if(s.ready && s.valid && s.frame!=snapshot_frame){

        snapshot_frame=s.frame;
        const nlohmann::json observed={{"valid",true},{"storyMask",s.story},{"trialMask",s.trial},{"extraYoshiMask",s.extras}};
        if(session_mode.load()==workshop::game_modes0320::StoryHybrid){
            story_progress0320=observed;
            // Ownership is shared; only this run's native level data is local.
            progress["extraYoshiMask"]=s.extras;
        }
        else if(session_mode.load()==workshop::game_modes0320::StoryHard)hard_progress0330=observed;
        else if(session_mode.load()!=workshop::game_modes0320::Bonus)progress=observed;

        if(!loading_mode)update_titles();

    }

    if(ultramodern::is_game_started() && !loading_mode && (!workshop::bonus_game0352::active()||bonus_resume_allowed0353(workshop::bonus_game0352::view())))resume_button->display_show();else resume_button->display_hide();

#ifdef WORKSHOP_CONTENT_BUILD

    if(workshop_test_book_shop() && s.paused)diagnostic_shop();

    // Bounded integration diagnostics exercise the real Resume callback, on a

    // disposable profile, after native selection has returned to the book.

    static uint64_t test_open_time=0;

    static unsigned test_resumes=0;

    if(workshop_test_book_resume() && s.paused && test_resumes<2){

        if(!test_open_time)test_open_time=SDL_GetTicks64();

        if(SDL_GetTicks64()-test_open_time>=3000){

            ++test_resumes;test_open_time=0;

            fprintf(stderr,"[BookLinkTest] resume=%u\n",test_resumes);fflush(stderr);

            static_cast<BookButton*>(resume_button)->diagnostic_click();

        }

    }

#endif

    menu_context.close();if(previous!=ContextId::null())previous.open();

}

void update(LauncherMenu*){

    int width=1280,height=720;recompui::get_window_size(width,height);

    float available=height>0?1080.f*width/height:1920.f;

    float w=std::min(1440.f,available),h=w*.75f;

    book->set_width(w);book->set_height(h);

    // Explicit centering keeps scissor rectangles aligned with scrollable

    // text; this renderer's compatibility path does not transform scissors.

    book->set_left((available-w)/2);book->set_top((1080-h)/2);

}

}

void workshop_menu_enable(){

    workshop::achievements::initialize();

    workshop::collections::initialize();
    workshop::finds::initialize();
    workshop::story_hybrid0320::initialize();
    workshop::story_hard0330::initialize();

    workshop::hybrid_mode::initialize();
    workshop::hybrid_completion0290::initialize("pc-hybrid-completion.json","pc-hybrid-records.json");
    workshop::metal_yoshis0300::initialize();
    workshop::elemental_yoshis0355::initialize();
    (void)workshop::heart_shop0283::wallet();

    // The final credits image can stop submitting native display lists.

    // Keep the menu access responsive there while leaving normal frames alone.

    recompui::set_idle_ui_watchdog(true);

    enabled=true;workshop::book::enable();std::ifstream file("book-menu.json");if(file)try{nlohmann::json j;file>>j;if(!j.is_object())throw std::runtime_error("preferences-shape");french=j.value("language",std::string("fr"))!="en";purchase_sound=j.value("purchaseSound",true);}catch(...){preferences_saved=false;}

    char* base=SDL_GetBasePath();if(base){pc_root=std::filesystem::u8path(base).parent_path().parent_path().parent_path();SDL_free(base);}

#ifdef _WIN32

    std::array<wchar_t,32768> executable{};GetModuleFileNameW(nullptr,executable.data(),DWORD(executable.size()));

    auto utf8_path=[](const std::filesystem::path& path){const auto bytes=path.u8string();return std::string(bytes.begin(),bytes.end());};

    std::ofstream session("book-session.json");session<<nlohmann::json{{"pid",GetCurrentProcessId()},{"executable",utf8_path(std::filesystem::path(executable.data()))},{"profile",utf8_path(std::filesystem::current_path())}}.dump(2);

#elif defined(__linux__)

    std::ofstream session("book-session.json");session<<nlohmann::json{{"pid",workshop::linux_process::id()},{"executable",workshop::linux_process::executable().string()},{"profile",std::filesystem::current_path().string()}}.dump(2);

#endif

}

void workshop_menu_start_mode(int mode){startup_mode=mode;}

bool workshop_menu_wait_parent(unsigned pid){

#ifdef _WIN32

    if(!pid || pid==GetCurrentProcessId())return false;

    HANDLE parent=OpenProcess(SYNCHRONIZE,FALSE,pid);

    if(!parent)return GetLastError()==ERROR_INVALID_PARAMETER;

    DWORD result=WaitForSingleObject(parent,30000);CloseHandle(parent);return result==WAIT_OBJECT_0;

#elif defined(__linux__)

    return workshop::linux_process::wait_parent(pid);

#else

    return false;

#endif

}

void workshop_menu_restore_window(){

    if(startup_mode.load()<0 || !window)return;

    try{

        std::ifstream file("book-window.json");if(!file)return;nlohmann::json p;file>>p;

        int x=p.at("x"),y=p.at("y"),w=p.at("width"),h=p.at("height");

        if(w<640 || h<360 || w>16384 || h>16384 || std::abs(x)>65535 || std::abs(y)>65535)return;

        SDL_SetWindowSize(window,w,h);SDL_SetWindowPosition(window,x,y);

        if(p.value("maximized",false))SDL_MaximizeWindow(window);

    }catch(...){}

}

bool workshop_menu_enabled(){return enabled;}

bool workshop_menu_french(){return enabled && french;}

void workshop_menu_register(){
    // The frontend owns one callback slot; compose both overlays here.
    recompui::register_frame_callback([]{
        if(enabled){ui_frame();resolve_deferred_focus();frame_book_music0310();}
#ifdef WORKSHOP_NATIVE_COOP
        workshop::coop_health0250::ui::frame(ultramodern::is_game_started()&&!workshop::book::snapshot().paused&&!recompui::config::get_config_modal()->is_open_now(),workshop_menu_french());
#endif
    });
if(enabled){

    recompui::register_launcher_init_callback(create);recompui::register_launcher_update_callback(update);

    recompui::config::create_tab(tr("Partie","Game"),"workshop-session",[](ContextId context,Element* parent){

        auto page=context.create_element<Element>(parent);page->set_display(Display::Flex);page->set_flex_direction(FlexDirection::Column);page->set_gap(24);page->set_padding(32);

        context.create_element<Label>(page,tr("Le livre et les modes du jeu partagent maintenant le même menu. Tu peux aussi y revenir pendant Story et les crédits.","The book and game modes now share one menu. You can return during Story and credits too."),LabelStyle::Normal);

        auto button=context.create_element<SessionMenuButton>(page,tr("Menu principal","Main menu"),ButtonStyle::Primary);button->add_pressed_callback(open_book);

        session_menu_button=button;button->display_hide();

    });

}}

int workshop_menu_take_boot_mode(){return enabled?boot_mode.exchange(-1):-1;}


int workshop_menu_session_mode(){return session_mode.load();}
