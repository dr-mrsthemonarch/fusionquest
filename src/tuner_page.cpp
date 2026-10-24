#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/NavBar.h"
#include "../include/Tuning.h"
#include "../include/Balance.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace {
std::string num(double v) {
    char b[48];
    double a = std::fabs(v);
    if (a >= 1e9) std::snprintf(b, sizeof(b), "%.1e", v);
    else std::snprintf(b, sizeof(b), a >= 1000 ? "%.0f" : a >= 100 ? "%.1f" : "%.3g", v);
    return b;
}
std::string val(const TuneParam& p) {
    char b[48];
    if (std::fabs(p.value) >= 1e7) std::snprintf(b, sizeof(b), "%.3g", p.value);
    else std::snprintf(b, sizeof(b), p.step < 1.0 ? "%.3f" : "%.1f", p.value);
    return b;
}
Element row(const std::string& l, const std::string& v) { return hbox({text(l) | size(WIDTH, EQUAL, 18), text(v)}); }
Element cell(const std::string& s, int w) { return text(s) | size(WIDTH, EQUAL, w); }
double turns(double hp, double dmg) { return dmg <= 0 ? 1e6 : std::ceil(hp / dmg); }

// ftxui braille graph of ys (index 0..n-1 stretched to the width). log10 scale optional.
Element plot(const std::string& title, const std::vector<double>& ys, Color c, bool logy) {
    std::vector<double> v = ys;
    for (auto& y : v) y = logy ? std::log10(std::max(y, 1e-2)) : y;
    double lo = *std::min_element(v.begin(), v.end()), hi = *std::max_element(v.begin(), v.end());
    if (hi - lo < 1e-9) hi = lo + 1;
    auto fn = [v, lo, hi](int w, int h) {
        std::vector<int> out(std::max(w, 1), 0);
        for (int i = 0; i < w; ++i) {
            double x = w > 1 ? static_cast<double>(i) * (v.size() - 1) / (w - 1) : 0;
            size_t i0 = static_cast<size_t>(x), i1 = std::min(i0 + 1, v.size() - 1);
            double y = v[i0] + (v[i1] - v[i0]) * (x - i0);
            out[i] = std::clamp(static_cast<int>((y - lo) / (hi - lo) * (h - 1)), 0, h - 1);
        }
        return out;
    };
    auto raw = [&](double t) { return logy ? std::pow(10.0, t) : t; };
    return vbox({text(title + "  " + num(raw(lo)) + " .. " + num(raw(hi)) + (logy ? " (log)" : "")) | color(c),
                 graph(fn) | size(HEIGHT, EQUAL, 3) | color(c)});
}

struct State {
    int view = 0;  // 0 factors+results, 1 projection
    int group = 0, selected = 0, series = 1, quest = 1;
    double year = 0;
    bool init = false;
    std::vector<std::string> entries;
    std::vector<size_t> map;
    std::string msg;
};
}  // namespace

// debug only (--debug-tune). view 0: factors (left, editable live) + results. view 1: year-by-year projection.
Component CreateTunerPage(int* selected_page, GameEngine* engine, Closure exit_closure) {
    auto st = std::make_shared<State>();
    auto groups = std::make_shared<std::vector<std::string>>();
    for (const auto& p : Tuning::I().params())
        if (std::find(groups->begin(), groups->end(), p.group) == groups->end()) groups->push_back(p.group);

    auto menu = Menu(&st->entries, &st->selected);
    auto save_btn = Button("Save Tuning", [st] {
        st->msg = Tuning::I().save("tuning.txt") ? "Wrote ./tuning.txt  (copy to config/tuning.txt, rebuild to ship)" : "Save failed";
    });
    auto reset_btn = Button("Reset All", [st] { Tuning::I().resetAll(); st->msg = "All factors reset to defaults"; });
    auto nav = CreateNavBar(selected_page, engine, std::move(exit_closure), TUNER_PAGE);
    auto container = Container::Vertical({menu, Container::Horizontal({save_btn, reset_btn}), nav});

    auto factorsView = [=]() -> Element {
        auto& tn = Tuning::I();
        const auto& params = tn.params();
        const std::string& grp = (*groups)[st->group];

        st->entries.clear();
        st->map.clear();
        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i].group != grp) continue;
            std::string k = params[i].key;
            k.resize(26, ' ');
            st->entries.push_back(k + val(params[i]) + (params[i].modified() ? " *" : ""));
            st->map.push_back(i);
        }
        if (st->entries.empty()) st->entries.push_back("(none)");
        st->selected = std::clamp(st->selected, 0, static_cast<int>(st->entries.size()) - 1);

        Element detail = text("");
        if (st->selected < static_cast<int>(st->map.size())) {
            const auto& p = params[st->map[st->selected]];
            detail = vbox({text(p.desc) | bold,
                           text("value " + val(p) + "   default " + num(p.def) + "   step " + num(p.step)) | dim});
        }

        // ---- results (same formulas the game uses) ----
        const int S = st->series;
        const double elapsed = st->year * 365.0 * 86400.0;
        Character* ch = engine->getCharacter();
        const bool real = engine->isInitialized() && ch && ch->getSeries() == S;
        Balance::PlayerStats P = Balance::player(S);
        double hustle, ndas, runway;
        std::string gearNote;
        if (real) {  // actual equipped gear
            const auto& pr = ch->getProfile();
            hustle = pr.hustle; ndas = pr.ndas; runway = ch->getMaxRunway();
            gearNote = "REAL equipped gear";
        } else {
            double gh, gn;
            Balance::modelGear(S, gh, gn);
            hustle = P.hustle + gh; ndas = P.ndas + gn; runway = P.runway;
            gearNote = "modeled gear +" + num(gh) + " hustle +" + num(gn) + " NDAs";
        }
        const auto mob = Balance::mob(S, Balance::difficultyMult(1));
        const auto bos = Balance::boss(S, Balance::difficultyMult(1));
        const int yr = static_cast<int>(st->year) + 1;
        const int ntype = yr <= 5 ? 0 : yr <= 10 ? 1 : yr <= 15 ? 2 : 3;
        const auto nem = Balance::nemesis(S, elapsed, ntype, 0);
        const double pm = T("combat.player_dmg_mult"), em = T("combat.enemy_dmg_mult");

        auto fight = [&](const char* name, const Balance::EnemyStats& e) {
            double tk = turns(e.hp, std::max(1.0, (hustle - e.def) * pm));
            double tl = turns(runway, std::max(1.0, (e.atk - ndas) * em));
            std::string s = std::string(name) + ": you " + num(tk) + " hits / it " + num(tl) + " hits -> " + (tk <= tl ? "WIN" : "LOSE");
            return text(s) | color(tk <= tl ? Color::Green : Color::Red);
        };
        auto enemy = [&](const char* n, const Balance::EnemyStats& e) {
            return row(n, "HP " + num(e.hp) + " ATK " + num(e.atk) + " DEF " + num(e.def) + " SPD " + num(e.spd) + " $" + num(e.money));
        };
        auto qp = engine->getQuestSystem()->preview(st->quest, elapsed);

        Element results = vbox({
            text("series " + std::to_string(S) + "  year " + num(st->year) + "  quest #" + std::to_string(st->quest)) | bold,
            separator(),
            row("Runway/Hype", num(P.runway) + " / " + num(P.hype)),
            row("Hustle/Agility", num(P.hustle) + " / " + num(P.agility)),
            row("NDAs/Luck", num(P.ndas) + " / " + num(P.luck)),
            row("Synergy/TD/Bus", num(P.synergy) + " / " + num(P.techDebt) + " / " + num(P.bus)),
            row("Next series cost", "$" + num(Balance::fundingRequired(S + 1, elapsed))),
            text("Fight uses " + gearNote) | dim,
            row("Hustle/NDAs used", num(hustle) + " / " + num(ndas)),
            separator(),
            enemy("Mob", mob), enemy("Boss", bos), enemy("Nemesis", nem),
            separator(),
            fight("vs Mob", mob), fight("vs Boss", bos), fight("vs Nemesis", nem),
            separator(),
            row("Quest budget", num(qp.budget / 86400.0) + " days (hustle " + num(qp.hustleBudget / 86400.0) + ")"),
            row("Hustles/devs", std::to_string(qp.hustlesRequired) + " / " + std::to_string(qp.devs)),
            row("Avg sprint", num(qp.avgSprintSeconds) + " s -> " + std::to_string(qp.sprints) + " sprints"),
        }) | border;

        return hbox({
            vbox({menu->Render() | vscroll_indicator | frame | flex, separator(), detail}) | border | size(WIDTH, EQUAL, 46),
            results | flex,
        }) | flex;
    };

    auto projectionView = [=]() -> Element {
        const auto pj = Balance::project();
        Elements rows;
        rows.push_back(hbox({cell("yr", 3), cell("ser", 4), cell("runway", 7), cell("hustle", 7), cell("ndas", 6),
                             cell("mob y/it", 11), cell("boss y/it", 11), cell("nem y/it", 13), cell("life", 7), cell("d/next", 7)}) | bold);
        int firstMob = -1, firstBoss = -1, firstNem = -1;
        std::vector<double> ser, advMob, advBoss, advNem, life, days;
        for (size_t y = 0; y < pj.atYear.size(); ++y) {
            const auto& m = pj.atYear[y];
            const bool lm = m.hitsMob > m.survMob, lb = m.hitsBoss > m.survBoss, ln = m.hitsNemesis > m.survNemesis;
            if (lm && firstMob < 0) firstMob = static_cast<int>(y);
            if (lb && firstBoss < 0) firstBoss = static_cast<int>(y);
            if (ln && firstNem < 0) firstNem = static_cast<int>(y);
            ser.push_back(m.series);
            advMob.push_back(m.survMob / std::max(1.0, m.hitsMob));
            advBoss.push_back(m.survBoss / std::max(1.0, m.hitsBoss));
            advNem.push_back(m.survNemesis / std::max(1.0, m.hitsNemesis));
            life.push_back(m.encountersPerLife);
            days.push_back(m.secondsToNextSeries / 86400.0);
            Element r = hbox({cell(std::to_string(y), 3), cell(std::to_string(m.series), 4), cell(num(m.runway), 7),
                              cell(num(m.hustle), 7), cell(num(m.ndas), 6),
                              cell(num(m.hitsMob) + "/" + num(m.survMob), 11), cell(num(m.hitsBoss) + "/" + num(m.survBoss), 11),
                              cell(num(m.hitsNemesis) + "/" + num(m.survNemesis), 13), cell(num(m.encountersPerLife), 7),
                              cell(num(m.secondsToNextSeries / 86400.0), 7)});
            Color c = lm ? Color::Red : lb ? Color::Yellow : ln ? Color::Magenta : Color::White;
            rows.push_back(r | color(c));
        }
        auto yr = [](int y) { return y < 0 ? std::string("never") : "year " + std::to_string(y); };
        Element summary = vbox({
            text("Series at year 20: " + std::to_string(pj.seriesAtYear.back())) | bold,
            text("Player loses to a mob from:    " + yr(firstMob)) | color(firstMob < 0 ? Color::Green : Color::Red),
            text("Player loses to a boss from:   " + yr(firstBoss)) | color(firstBoss < 0 ? Color::Green : Color::Yellow),
            text("Player loses to nemesis from:  " + yr(firstNem)) | color(firstNem < 0 ? Color::Green : Color::Magenta),
            text("columns: y/it = hits you need / hits it needs to kill you. life = encounters per life.") | dim,
            text("model: modeled gear, no patents/healing, medium+mix difficulty, always wins.") | dim,
        });
        Element plots = vbox({
            plot("Series reached", ser, Color::Cyan, false),
            plot("Mob advantage (it needs / you need)", advMob, Color::Green, true),
            plot("Boss advantage", advBoss, Color::Yellow, true),
            plot("Nemesis advantage", advNem, Color::Magenta, true),
            plot("Encounters per life", life, Color::Red, true),
            plot("Days to next series", days, Color::Blue, false),
        }) | border | flex;
        return hbox({vbox({vbox(rows) | border, summary | border}) | size(WIDTH, EQUAL, 70), plots}) | flex;
    };

    auto renderer = Renderer(container, [=] {
        if (!st->init) {  // first open: start at the player's current series
            st->init = true;
            if (engine->isInitialized() && engine->getCharacter()) st->series = std::max(1, engine->getCharacter()->getSeries());
        }
        return window(text(std::string("Tuner (debug) - ") + (st->view == 0 ? "factors: " + (*groups)[st->group] : "year-by-year projection")) | bold | center,
                      vbox({
                          st->view == 0 ? factorsView() : projectionView(),
                          text("Up/Down select  +/- step  [ ] x10  r reset  g/G group  s/S series  y/Y year  q/Q quest  p = projection view") | dim | center,
                          text(st->msg) | color(Color::Green) | center,
                          hbox({save_btn->Render(), reset_btn->Render()}) | center,
                          nav->Render(),
                      }));
    });

    return CatchEvent(renderer, [=](Event e) {
        auto& tn = Tuning::I();
        auto adjust = [&](double mult) {
            if (st->selected < static_cast<int>(st->map.size())) {
                const auto& p = tn.params()[st->map[st->selected]];
                tn.set(p.key, p.value + mult * p.step);
            }
            return true;
        };
        if (e == Event::Character('p')) { st->view = 1 - st->view; return true; }
        if (e == Event::Character('+') || e == Event::Character('=')) return adjust(1);
        if (e == Event::Character('-')) return adjust(-1);
        if (e == Event::Character(']')) return adjust(10);
        if (e == Event::Character('[')) return adjust(-10);
        if (e == Event::Character('r')) {
            if (st->selected < static_cast<int>(st->map.size())) tn.reset(st->map[st->selected]);
            return true;
        }
        const int ng = static_cast<int>(groups->size());
        if (e == Event::Character('g')) { st->group = (st->group + 1) % ng; st->selected = 0; return true; }
        if (e == Event::Character('G')) { st->group = (st->group + ng - 1) % ng; st->selected = 0; return true; }
        if (e == Event::Character('s')) { st->series = std::min(400, st->series + 1); return true; }
        if (e == Event::Character('S')) { st->series = std::max(1, st->series - 1); return true; }
        if (e == Event::Character('y')) { st->year = std::min(20.0, st->year + 1); return true; }
        if (e == Event::Character('Y')) { st->year = std::max(0.0, st->year - 1); return true; }
        if (e == Event::Character('q')) { st->quest = std::min(21, st->quest + 1); return true; }
        if (e == Event::Character('Q')) { st->quest = std::max(1, st->quest - 1); return true; }
        return false;
    });
}
