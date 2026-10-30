#include "workshop_menu.hpp"

#include "custom_session.hpp"

#include "workshop_menu_game.hpp"

#include "workshop_loader.hpp"

#include "achievements.hpp"

#include "collections.hpp"

#include "hybrid_mode.hpp"

#include "linux_process.hpp"

#include "recompui/recompui.h"

#include "recompui/config.h"

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

std::atomic<int> boot_mode{-1};

std::atomic<int> session_mode{-1};

std::atomic<int> startup_mode{-1};

std::filesystem::path pc_root;

ContextId menu_context=ContextId::null();

ContextId game_link_context=ContextId::null();

Element* resume_button=nullptr;

Element* session_menu_button=nullptr;

Label* resume_label=nullptr;

bool loading_mode=false;

bool resume_pending=false;

Element* focus_pending=nullptr;

bool focus_waited=false;

unsigned snapshot_frame=~0u;

Element *book=nullptr,*main_page=nullptr,*detail_page=nullptr;

Label *detail_title=nullptr,*detail_body=nullptr,*footer=nullptr;

std::array<Element*,4> actions{};

std::array<Label*,4> action_labels{};

std::array<std::function<void()>,4> action_callbacks{};

std::array<Element*,7> choices{};

std::array<Label*,7> choice_labels{};

nlohmann::json progress;

bool detail_open=false;

uint64_t last_button_activation=0;

constexpr Color ink{73,43,25,255},paper{247,229,198,255},ribbon{245,219,176,255},edge{201,161,104,255};

const char* tr(const char* fr,const char* en){return french?fr:en;}

void home();

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

    const char* fr[]={"Story","Trial","Création","Statistiques et succès","Mode hybride","Options","Practice"};

    const char* en[]={"Story","Trial","Creation","Statistics & achievements","Hybrid mode","Options","Practice"};

    for(unsigned i=0;i<7;i++)choice_labels[i]->set_text(french?fr[i]:en[i]);

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

    update_titles();focus_pending=choices[0];focus_waited=false;

    menu_context.set_autofocus_element(choices[0]);

    fprintf(stderr,"[BookMenu] page=home entries=7 language=%s\n",french?"fr":"en");

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

    // Finish dispatching the button/key event before hiding its document or

    // waking the native producer of display lists.

    resume_pending=true;

}

void open_book(){

    if(loading_mode)return;

    if(recompui::config::get_config_modal()->is_open_now() && !recompui::config::close())return;

    if(ultramodern::is_game_started()){

        recompui::set_idle_ui_rendering(true);

        workshop::book::pause();

    }

    auto previous=recompui::try_close_current_context();

    menu_context.open();home();

    if(!recompui::is_context_shown(menu_context))recompui::show_context(menu_context,"");

    menu_context.close();if(previous!=ContextId::null())previous.open();

    fprintf(stderr,"[BookLink] ui=open-book\n");fflush(stderr);

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

            {std::ofstream summary("menu-progress.json");summary<<progress.dump(2)<<'\n';}

            std::wstring command=L"\""+std::wstring(executable.data())+L"\" --workshop-wait-parent "+std::to_wstring(GetCurrentProcessId())+L" --workshop-main-menu --workshop-start-mode "+std::to_wstring(mode);

            STARTUPINFOW startup{};startup.cb=sizeof(startup);

            PROCESS_INFORMATION process{};

            if(CreateProcessW(executable.data(),command.data(),nullptr,nullptr,FALSE,0,nullptr,std::filesystem::current_path().c_str(),&startup,&process)){

                fprintf(stderr,"[BookLink] handoff mode=%d parent=%lu child=%lu save-barrier=complete\n",mode,GetCurrentProcessId(),process.dwProcessId);fflush(stderr);

                CloseHandle(process.hThread);CloseHandle(process.hProcess);

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

        {std::ofstream summary("menu-progress.json");summary<<progress.dump(2)<<'\n';}

        if(workshop::linux_process::launch(workshop::linux_process::executable(),

            {"--workshop-wait-parent",std::to_string(workshop::linux_process::id()),"--workshop-main-menu","--workshop-start-mode",std::to_string(mode)},std::filesystem::current_path())){

            fprintf(stderr,"[BookLink] Linux handoff mode=%d parent=%u save-barrier=complete\n",mode,workshop::linux_process::id());fflush(stderr);

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
    std::_Exit(0);
}
void launch_native(int mode){

    if(workshop::custom_session::active() && ultramodern::is_game_started()){

        panel(tr("Partie personnalisée","Custom game"),tr("Ferme cette session pour revenir à l’Atelier. Tu pourras y reprendre la partie ou lancer un autre mode depuis DinoTale.","Close this session to return to the Workshop. Resume there, or launch another mode from DinoTale."));

        action(0,tr("Retour à l’Atelier","Return to Workshop"),[]{close_custom_runtime(false);});action(1,tr("Reprendre","Resume"),resume_game);return;

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

    boot_mode.store(mode);

    session_mode.store(mode);

    workshop::hybrid_mode::activate(mode==3);

    workshop::achievements::start_session(unsigned(mode));

    workshop::collections::start_session(unsigned(mode));

    fprintf(stderr,"[BookMenu] launch=%s\n",mode==0?"story":mode==1?"trial":mode==3?"hybrid":"practice");fflush(stderr);

    recomp::start_game(u8"yoshisstory.n64.us",{});recompui::hide_all_contexts();

}

bool launch_workshop(bool games){

#ifdef __linux__

    auto executable=pc_root/"build/workshop-linux/YoshiStory-AtelierLinux";

    return std::filesystem::exists(executable) && workshop::linux_process::launch(executable,

        {pc_root.string(),games?"--custom-games":"--editor"},pc_root);

#else

    auto executable=pc_root/"build/workshop-0.15.0/YoshiStory-AtelierPC.exe";
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

void statistics();

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

        if(purchase(item))buy_upgrade(item);

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

        }else buy_egg(item);

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

    std::string text=tr("Niveaux terminés en Story : ","Story levels cleared: ")+std::to_string(std::popcount(story))+" / 24\n";

    text+=tr("Niveaux terminés en Trial : ","Trial levels cleared: ")+std::to_string(std::popcount(trial))+" / 24\n";

    text+=tr("Yoshis hybrides débloqués : ","Hybrid Yoshis unlocked: ")+std::to_string(std::popcount(extras))+" / 6\n\n";

    for(unsigned page=0;page<6;page++){

        text+=tr("Page ","Page ")+std::to_string(page+1)+" : ";

        for(unsigned level=0;level<4;level++){

            unsigned bit=1u<<(page*4+level);text+=std::to_string(level+1)+"[";

            text+=(story&bit)?"S":"-";text+=(trial&bit)?"T":"-";text+="]  ";

        }

        text+="\n";

    }

    auto ledger=workshop::achievements::view();unsigned full=0,same=0,hearts=0;

    for(const auto& level:ledger.levels){full+=level.fullHealth;same+=std::popcount(level.sameFruit);hearts+=level.allHearts;}

    text+=tr("\nS = Story, T = Trial\n\nPleine santé : ","\nS = Story, T = Trial\n\nFull health: ")+std::to_string(full)+" / 24\n";

    text+=tr("Défis 30 fruits identiques : ","30 identical fruit challenges: ")+std::to_string(same)+" / 264\n";

    text+=tr("Trois cœurs rouges et niveau terminé : ","Three red hearts and level cleared: ")+std::to_string(hearts)+" / 24\n";

    text+=tr("\nLes succès comptent les tentatives réellement observées, sans attribution rétroactive.","\nAchievements count observed runs, without retroactive awards.");

    if(!ledger.readable)text+=tr("\nFichier de succès illisible, conservé.","\nUnreadable achievement file preserved.");

    else if(!ledger.saved)text+=tr("\nDernier résultat non enregistré.","\nLatest result not saved.");

    panel(tr("Statistiques et succès","Statistics & achievements"),text);

    action(0,tr("Succès par niveau","Achievements by level"),[]{achievement_details(0);});

    action(1,tr("Collection et boutique","Collection & shop"),collection_page);back(2);

}

void hybrid(){

    unsigned trial=progress.value("trialMask",0u);bool complete=progress.value("valid",false) && trial==0xFFFFFF;

    std::string body=complete?tr("Condition remplie : les 24 niveaux Trial sont terminés.\n\n","Requirement met: all 24 Trial levels are cleared.\n\n"):

        tr("Termine les 24 niveaux Trial pour débloquer ce mode.\nProgression : ","Clear all 24 Trial levels to unlock this mode.\nProgress: ")+std::to_string(std::popcount(trial))+" / 24\n\n";

    const bool bought=workshop::collections::view().state.owns(workshop::collections::Item::Hybrid);

    body+=tr("75 % des fruits ordinaires deviennent des hybrides compatibles. Les melons sont conservés. Lucky Fruit est choisi parmi les six hybrides ; les Maskass utilisent leurs six couleurs.\n\nLes records de ce mode sont séparés. Les succès, dont les 30 fruits identiques, sont communs au Trial classique et au mode hybride.","75% of ordinary fruit become compatible hybrids. Melons stay unchanged. Lucky Fruit is one of the six hybrids; Shy Guys use their six colors.\n\nMode records are separate. Achievements, including 30 identical fruit, are shared with classic Trial.");

    body+=bought?tr("\n\nMode acheté.","\n\nMode purchased."):tr("\n\nPrix : 200 pièces turquoises.","\n\nPrice: 200 turquoise coins.");

    if(!workshop::hybrid_mode::readable())body+=tr("\nFichier de records illisible conservé ; mode indisponible.","\nUnreadable records file preserved; mode unavailable.");

    panel(tr("Mode hybride","Hybrid mode"),body);

    if(complete && bought && workshop::hybrid_mode::readable())action(0,tr("Jouer au mode hybride","Play Hybrid mode"),[]{launch_native(3);});

    else if(!bought)action(0,tr("Acheter le mode","Purchase mode"),[]{buy_upgrade(workshop::collections::Item::Hybrid);});

    back(1);

}

void options(){

    panel(tr("Options","Options"),tr("Langue de ce menu : Français\n\nLes textes du jeu d’origine restent en anglais pour le moment. Le volume, les manettes et l’affichage se règlent dans les paramètres du jeu.","Menu language: English\n\nThe original game remains in English for now. Volume, controllers and display can be adjusted in game settings."));

    action(0,french?"Menu language: English":"Langue du menu : Français",[]{

        french=!french;std::ofstream file("book-menu.json");file<<nlohmann::json{{"version",1},{"language",french?"fr":"en"}}.dump(2)<<'\n';update_titles();options();

    });

    action(1,tr("Réglages du jeu","Game settings"),[]{recompui::config::open();});back(2);

}

void choose(unsigned item){

    switch(item){case 0:launch_native(0);break;case 1:launch_native(1);break;case 2:creation();break;case 3:statistics();break;case 4:hybrid();break;case 5:options();break;case 6:launch_native(2);break;}

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

    auto cover=menu_context.create_element<Element>(book);rect(cover,4,31.5,45,60.7);cover->set_background_color(paper);cover->set_border_radius(30);cover->set_border_width(3);cover->set_border_color(Color{230,202,160,255});

    main_page=menu_context.create_element<Element>(book);rect(main_page,0,0,100,100);main_page->set_as_navigation_container(NavigationType::Vertical);main_page->set_nav_wrapping(true);

    for(unsigned i=0;i<7;i++){

        choices[i]=menu_context.create_element<BookButton>(main_page,&choice_labels[i],[i]{choose(i);});

        rect(choices[i],6.8,33.1+i*8.15,39.4,6.7);

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

    std::ifstream summary("menu-progress.json");if(summary)try{summary>>progress;}catch(...){progress=nlohmann::json::object();}else progress=nlohmann::json::object();

    home();

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

            if(session_menu_button){fprintf(stderr,"[BookShopTest] FAILED destroyed session button retained\n");std::fflush(nullptr);std::_Exit(24);}

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

                fprintf(stderr,"[BookShopTest] FAILED first-visible focus phase=%u\n",phase);std::fflush(nullptr);std::_Exit(21);

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

        if(menu_context.get_focused_element()!=actions[0]){fprintf(stderr,"[BookShopTest] FAILED stale focus recovery\n");std::fflush(nullptr);std::_Exit(22);}

        key(Rml::Input::KI_DOWN);

        if(menu_context.get_focused_element()!=actions[1]){fprintf(stderr,"[BookShopTest] FAILED navigation after recovery\n");std::fflush(nullptr);std::_Exit(23);}

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

        std::fflush(nullptr);std::_Exit(0);

    }

}

#endif

void ui_frame(){

    if(menu_context==ContextId::null())return;

    if(resume_pending){

        resume_pending=false;

        if(recompui::is_context_shown(menu_context))recompui::hide_context(menu_context);

        recompui::set_idle_ui_rendering(false);

        workshop::book::resume();

        fprintf(stderr,"[BookLink] ui=resume\n");fflush(stderr);

        return;

    }

    int start=startup_mode.exchange(-1);

    if(start>=0 && !ultramodern::is_game_started()){

        auto previous=recompui::try_close_current_context();menu_context.open();

        launch_native(start);menu_context.close();if(previous!=ContextId::null())previous.open();return;

    }

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

    if(workshop::book::take_open_request())open_book();

    bool book_shown=recompui::is_context_shown(menu_context);

    const auto native=workshop::book::snapshot();

    bool show_link=ultramodern::is_game_started() && native.ready && session_mode.load()==0 && boot_mode.load()<0 && !book_shown && !recompui::config::get_config_modal()->is_open_now();

    if(session_menu_button){if(session_mode.load()==0)session_menu_button->display_show();else session_menu_button->display_hide();}

    bool link_shown=recompui::is_context_shown(game_link_context);

    if(show_link && !link_shown)recompui::show_context(game_link_context,"");

    else if(!show_link && link_shown)recompui::hide_context(game_link_context);

    if(!book_shown)return;

    auto previous=recompui::try_close_current_context();menu_context.open();

    auto s=workshop::book::snapshot();

    if(s.ready && s.valid && s.frame!=snapshot_frame){

        snapshot_frame=s.frame;progress={{"valid",true},{"storyMask",s.story},{"trialMask",s.trial},{"extraYoshiMask",s.extras}};

        if(!loading_mode)update_titles();

    }

    if(ultramodern::is_game_started() && !loading_mode)resume_button->display_show();else resume_button->display_hide();

    // Rml applies display changes at its Update, after this callback. Wait one

    // frame before focusing the newly shown page; the old focus was blurred.

    if(focus_pending){

        if(focus_waited && focus_pending->is_focusable()==Element::CanFocus::Yes && focus_pending->focus())focus_pending=nullptr;

        else focus_waited=true;

    }

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

    workshop::hybrid_mode::initialize();

    // The final credits image can stop submitting native display lists.

    // Keep the menu access responsive there while leaving normal frames alone.

    recompui::set_idle_ui_watchdog(true);

    enabled=true;workshop::book::enable();std::ifstream file("book-menu.json");if(file)try{nlohmann::json j;file>>j;french=j.value("language",std::string("fr"))!="en";}catch(...){}

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

void workshop_menu_register(){if(enabled){

    recompui::register_launcher_init_callback(create);recompui::register_launcher_update_callback(update);recompui::register_frame_callback(ui_frame);

    recompui::config::create_tab(tr("Partie","Game"),"workshop-session",[](ContextId context,Element* parent){

        auto page=context.create_element<Element>(parent);page->set_display(Display::Flex);page->set_flex_direction(FlexDirection::Column);page->set_gap(24);page->set_padding(32);

        context.create_element<Label>(page,tr("Le livre et les modes du jeu partagent maintenant le même menu. Tu peux aussi y revenir pendant Story et les crédits.","The book and game modes now share one menu. You can return during Story and credits too."),LabelStyle::Normal);

        auto button=context.create_element<SessionMenuButton>(page,tr("Menu principal","Main menu"),ButtonStyle::Primary);button->add_pressed_callback(open_book);

        session_menu_button=button;button->display_hide();

    });

}}

int workshop_menu_take_boot_mode(){return enabled?boot_mode.exchange(-1):-1;}

