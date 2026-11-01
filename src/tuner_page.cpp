#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/NavBar.h"
#include "../include/Tuning.h"
#include "../include/Balance.h"
#include "../include/Archetype.h"
#include <algorithm>
#include <array>
#include <optional>
#include <cmath>
#include <cstdio>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

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

// what each group's formula looks like, shown under the factor list
std::vector<std::string> formulaFor(const std::string& g) {
    if (g == "Player start") return {"stats at series 0 (before any level-up)"};
    if (g == "Level gain") return {"gain(series s) = base + (s-1)*per", "applied each time you reach series s", "stat(S) = start + sum of gains 1..S"};
    if (g == "Funding") return {"cost(s) = base * s^exponent / slowdown(t)", "slowdown = 1/(1+e^(K*(t/20y - C))), floor min", "funding per kill = money * per_money"};
    if (g == "Mobs") return {"stat = (base + series*per) * difficulty", "then random +/- variance", "HP, ATK, DEF, SPD, $"};
    if (g == "Bosses") return {"HP = hp_base*(1+series)^hp_exp*diff*hp_mult", "others = (base+series*per)*diff*mult"};
    if (g == "Difficulty") return {"multiplies every enemy stat", "spawn mix: % easy/medium/hard, rest legendary"};
    if (g == "Encounters") return {"roll: single | small group | large group | boss+minions", "group mobs get stat * scale"};
    if (g == "Nemesis") return {"stat = (base+series*per) * (1+sigmoid(t)*time_max)", "          * typeMult * power^defeats * mult", "type = years 1-5/6-10/11-15/16-20"};
    if (g == "Special mobs") return {"base = base + series*per", "Safety = base*mult (ignores difficulty)", "Conference/Paper = base*mult*difficulty"};
    if (g == "Review bosses") return {"stat = base*(1+year*yg)*(1+month/12*mg)*diff"};
    if (g == "Shadow") return {"mirrors YOUR stats: hp=runway*hp_mult,", "atk=hustle*atk_mult, def=NDAs*def_mult"};
    if (g == "Named monsters") return {"stat = configBase*(1+series*growth)*power_mult", "minions use minion_* bases"};
    if (g == "Combat") return {"hit = max(1,(attacker - defender)*mult) +/- variance"};
    if (g == "Equipment") return {"stat = (base + series*per) * slotMult * (1 +/- var)"};
    if (g == "Equip value") return {"price = sum(stat*weight) * rarityMult, min price"};
    if (g == "Quests") return {"budget = remaining clock * weight/sum(weights)", "sprints = budget / avg sprint seconds"};
    if (g == "Quest goals") return {"target in [lo, base + tier/div], tier = quest index"};
    if (g == "Patent effects") return {"attack = power + hustle/div - NDAs/div"};
    if (g == "Stat effects") return {"crit = base + luck*per (cap); dmg x (base + syn*per + passive)", "dodge = (defAgi-atkSpd)/(sum) * edge (cap)", "fumble = max(0,-techDebt)*per (cap); break/battle same", "potency = 1+syn*per; hype cost x (1-syn*per)", "extra action = (agi/foeSpd - 1)*per (cap)"};
    if (g == "Strategy") return {"danger = foeDmg*rounds*0.5/runway", "buff/defense only if danger > ratio, not active", "heal only below frac; debuff not on dying foe", "score x classWeight / (1+cost/maxHype*w) x noise"};
    if (g == "Class") return {"mods flat at creation, grow per series (config/classes.txt)", "passives scale by passive.* factors", "c key in tuner: cycle projection class"};
    return {};
}

struct Line { std::string name; std::vector<double> y; Color color; };

// ---- selectable plot variables ----
enum Metric {
    M_SERIES, M_RUNWAY, M_HYPE, M_HUSTLE, M_NDAS, M_FUNDCOST,
    M_MOB_HP, M_MOB_ATK, M_MOB_DEF, M_BOSS_HP, M_BOSS_ATK, M_BOSS_DEF, M_NEM_HP, M_NEM_ATK, M_NEM_DEF,
    M_ADV_MOB, M_ADV_BOSS, M_ADV_NEM, M_HITS_MOB, M_HITS_BOSS, M_HITS_NEM,
    M_LIFE, M_DAYS, M_SECENC, M_FUNDENC, M_DMGENC, M_QBUDGET, M_QSPRINTS, M_COUNT
};
const char* kMetricName[M_COUNT] = {
    "series", "runway", "hype", "hustle", "NDAs", "next-series cost",
    "mob HP", "mob ATK", "mob DEF", "boss HP", "boss ATK", "boss DEF", "nem HP", "nem ATK", "nem DEF",
    "adv mob", "adv boss", "adv nem", "hits mob", "hits boss", "hits nem",
    "enc/life", "days/series", "sec/encounter", "funding/enc", "dmg/encounter", "quest days", "quest sprints"};

struct Preset { const char* name; bool logy, baseline; std::vector<std::pair<Metric, Color>> lines; };
const std::vector<Preset>& presets() {
    static const std::vector<Preset> p = {
        {"Progress (series)", false, false, {{M_SERIES, Color::Cyan}}},
        {"Hit advantage (it needs/you need)", true, true, {{M_ADV_MOB, Color::Green}, {M_ADV_BOSS, Color::Yellow}, {M_ADV_NEM, Color::Magenta}}},
        {"Your hustle vs enemy DEF", true, false, {{M_HUSTLE, Color::Cyan}, {M_MOB_DEF, Color::Green}, {M_BOSS_DEF, Color::Yellow}, {M_NEM_DEF, Color::Magenta}}},
        {"Your runway vs enemy ATK", true, false, {{M_RUNWAY, Color::Cyan}, {M_MOB_ATK, Color::Green}, {M_BOSS_ATK, Color::Yellow}, {M_NEM_ATK, Color::Magenta}}},
        {"Your NDAs vs enemy ATK", true, false, {{M_NDAS, Color::Cyan}, {M_MOB_ATK, Color::Green}, {M_BOSS_ATK, Color::Yellow}, {M_NEM_ATK, Color::Magenta}}},
        {"Encounters per life", true, true, {{M_LIFE, Color::Red}}},
        {"Pacing: days per series", true, false, {{M_DAYS, Color::Blue}}},
        {"Pacing: seconds per encounter", true, false, {{M_SECENC, Color::Blue}}},
        {"Funding: cost vs income/enc", true, false, {{M_FUNDCOST, Color::Red}, {M_FUNDENC, Color::Green}}},
        {"Enemy HP", true, false, {{M_MOB_HP, Color::Green}, {M_BOSS_HP, Color::Yellow}, {M_NEM_HP, Color::Magenta}}},
        {"Hits YOU need to kill", true, false, {{M_HITS_MOB, Color::Green}, {M_HITS_BOSS, Color::Yellow}, {M_HITS_NEM, Color::Magenta}}},
        {"Damage taken per encounter", true, false, {{M_DMGENC, Color::Red}, {M_RUNWAY, Color::Cyan}}},
        {"Player growth", true, false, {{M_RUNWAY, Color::Cyan}, {M_HYPE, Color::Blue}, {M_HUSTLE, Color::Green}, {M_NDAS, Color::Yellow}}},
        {"Quest pacing (days / sprints)", true, false, {{M_QBUDGET, Color::Cyan}, {M_QSPRINTS, Color::Yellow}}},
    };
    return p;
}
const char* kXModes[4] = {"Year (0-20)", "Series (1-60)", "Quest index (1-21)", "Sweep selected factor (x0.5..x1.5)"};

struct Data {
    std::vector<double> xs;
    std::vector<std::array<double, M_COUNT>> pts;
    double xlo = 0, xhi = 1, marker = 0;
    std::string xlabel;
};

std::array<double, M_COUNT> makePoint(const Balance::SeriesMetrics& m, double reached, const Balance::SeriesMetrics* /*unused*/,
                                      double qDays, double qSprints, double nextCost) {
    std::array<double, M_COUNT> a{};
    a[M_SERIES] = reached; a[M_RUNWAY] = m.runway; a[M_HYPE] = m.player.hype; a[M_HUSTLE] = m.hustle; a[M_NDAS] = m.ndas;
    a[M_FUNDCOST] = nextCost;
    a[M_MOB_HP] = m.mob.hp; a[M_MOB_ATK] = m.mob.atk; a[M_MOB_DEF] = m.mob.def;
    a[M_BOSS_HP] = m.boss.hp; a[M_BOSS_ATK] = m.boss.atk; a[M_BOSS_DEF] = m.boss.def;
    a[M_NEM_HP] = m.nemesis.hp; a[M_NEM_ATK] = m.nemesis.atk; a[M_NEM_DEF] = m.nemesis.def;
    a[M_ADV_MOB] = m.survMob / std::max(1.0, m.hitsMob);
    a[M_ADV_BOSS] = m.survBoss / std::max(1.0, m.hitsBoss);
    a[M_ADV_NEM] = m.survNemesis / std::max(1.0, m.hitsNemesis);
    a[M_HITS_MOB] = m.hitsMob; a[M_HITS_BOSS] = m.hitsBoss; a[M_HITS_NEM] = m.hitsNemesis;
    a[M_LIFE] = m.encountersPerLife; a[M_DAYS] = m.secondsToNextSeries / 86400.0;
    a[M_SECENC] = m.secondsPerEncounter; a[M_FUNDENC] = m.fundingPerEncounter; a[M_DMGENC] = m.dmgPerEncounter;
    a[M_QBUDGET] = qDays; a[M_QSPRINTS] = qSprints;
    return a;
}

// canvas chart: x axis (any range, 5 labelled ticks + label), y axis (min/max labels, optional log),
// optional y=1 break-even line, marker at the currently selected x.
Element chart(const std::string& title, int wc, int hc, const std::vector<double>& xs, double xlo, double xhi,
              const std::string& xlabel, const std::vector<Line>& lines, bool logy, bool baseline, double marker, bool selected) {
    auto tf = [logy](double v) { return logy ? std::log10(std::max(v, 1e-2)) : v; };
    double lo = 1e300, hi = -1e300;
    for (const auto& l : lines) for (double y : l.y) { lo = std::min(lo, tf(y)); hi = std::max(hi, tf(y)); }
    if (baseline && logy) { lo = std::min(lo, 0.0); hi = std::max(hi, 0.0); }
    if (lo > hi) { lo = 0; hi = 1; }
    if (hi - lo < 1e-9) hi = lo + 1;
    if (xhi - xlo < 1e-12) xhi = xlo + 1;
    wc = std::max(wc, 8);
    hc = std::max(hc, 2);
    // const int W = wc * 2, H = hc * 4;
    const int W = wc * 2;
    const int H = std::max(4, hc * 4 - 3);   // 3 rows reserved for legend / ticks / xlabel
    auto fn = [=](Canvas& c) {
        const int ox = 1, oy = H - 1, pw = W - 3, ph = H - 2;
        auto X = [&](double x) { return std::clamp(ox + static_cast<int>((x - xlo) / (xhi - xlo) * pw), 0, W - 1); };
        auto Y = [&](double t) { return std::clamp(oy - static_cast<int>((t - lo) / (hi - lo) * ph), 0, H - 1); };
        c.DrawPointLine(ox, 0, ox, oy, Color::GrayLight);            // y axis
        c.DrawPointLine(ox, oy, W - 1, oy, Color::GrayLight);        // x axis
        for (int i = 0; i <= 4; ++i) { int x = X(xlo + i * (xhi - xlo) / 4); c.DrawPointLine(x, oy, x, std::min(H - 1, oy + 2), Color::White); }
        if (baseline && logy) for (int x = ox; x < W; x += 4) c.DrawPoint(x, Y(0.0), true, Color::GrayDark);
        if (marker >= xlo && marker <= xhi) for (int y = 0; y < oy; y += 3) c.DrawPoint(X(marker), y, true, Color::GrayDark);
        for (const auto& l : lines)
            for (size_t i = 0; i + 1 < l.y.size() && i + 1 < xs.size(); ++i)
                c.DrawPointLine(X(xs[i]), Y(tf(l.y[i])), X(xs[i + 1]), Y(tf(l.y[i + 1])), l.color);
    };
    auto raw = [logy](double t) { return logy ? std::pow(10.0, t) : t; };
    Elements legend{text((selected ? "> " : "  ") + title + (logy ? " (log) " : " ")) | bold};
    if (selected) legend[0] = legend[0] | inverted;
    for (const auto& l : lines) if (!l.name.empty()) legend.push_back(text(l.name + " ") | color(l.color));
    Elements xl{text("       ")};
    for (int i = 0; i <= 4; ++i) { xl.push_back(text(num(xlo + i * (xhi - xlo) / 4))); if (i < 4) xl.push_back(filler()); }
    return vbox({
        hbox(legend),
        hbox({vbox({text(num(raw(hi))), filler(), text(num(raw(lo)))}) | size(WIDTH, EQUAL, 7), canvas(W, H, fn)}),
        hbox(xl),
        text("       " + xlabel) | dim,
    });
}

struct PlotCfg {
    int xmode = 0;
    std::array<bool, M_COUNT> y{};
    bool logy = true, baseline = false;
};
void loadPreset(PlotCfg& c, int idx) {
    const Preset& p = presets()[idx];
    c.y.fill(false);
    for (const auto& l : p.lines) c.y[l.first] = true;
    c.logy = p.logy;
    c.baseline = p.baseline;
}

struct State {
    int view = 0;                      // 0 = factors+plots, 1 = projection table
    int group = 0, lastGroup = -1, selected = 0, series = 1, quest = 1;
    double year = 0;
    bool init = false;
    int left_w = 78, group_w = 17, plot_h = 30;   // resizable panel sizes (dragged with the mouse)
    std::vector<std::string> entries, groupLabels;
    std::vector<size_t> map;
    std::string msg;
    // plots: any number, each with its own x axis + checked y variables
    std::vector<PlotCfg> plots;
    int slot = 0;                      // selected plot (keys 1-9)
    // plot editor popup (checkmarks)
    bool show_modal = false;
    int ex = 0;
    std::array<bool, M_COUNT> ey{};
    bool elog = true, ebase = false;
    int epreset = 0;
    std::string emsg;
    std::vector<std::string> xnames;
};
}  // namespace

// debug only (--debug-tune).
// layout: [groups | factors+formula] | [plots (top) / results (bottom)]; every divider is draggable.
Component CreateTunerPage(int* selected_page, GameEngine* engine, Closure exit_closure) {
    auto st = std::make_shared<State>();
    auto groups = std::make_shared<std::vector<std::string>>();
    for (const auto& p : Tuning::I().params())
        if (std::find(groups->begin(), groups->end(), p.group) == groups->end()) groups->push_back(p.group);
    st->groupLabels = *groups;

    auto group_menu = Menu(&st->groupLabels, &st->group);
    auto param_menu = Menu(&st->entries, &st->selected);

    auto group_panel = Renderer(group_menu, [=] {
        for (size_t i = 0; i < groups->size(); ++i) {
            bool mod = false;
            for (const auto& p : Tuning::I().params()) if (p.group == (*groups)[i] && p.modified()) mod = true;
            st->groupLabels[i] = (*groups)[i] + (mod ? " *" : "");
        }
        return vbox({text("Groups") | bold | center, separator(), group_menu->Render() | vscroll_indicator | frame | flex}) | border;
    });

    auto param_panel = Renderer(param_menu, [=] {
        const auto& params = Tuning::I().params();
        const std::string& grp = (*groups)[st->group];
        if (st->lastGroup != st->group) { st->selected = 0; st->lastGroup = st->group; }
        st->entries.clear();
        st->map.clear();
        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i].group != grp) continue;
            std::string k = params[i].key;
            k.resize(27, ' ');
            st->entries.push_back(k + val(params[i]) + (params[i].modified() ? " *" : ""));
            st->map.push_back(i);
        }
        if (st->entries.empty()) st->entries.push_back("(none)");
        st->selected = std::clamp(st->selected, 0, static_cast<int>(st->entries.size()) - 1);

        Elements info;
        if (st->selected < static_cast<int>(st->map.size())) {
            const auto& p = params[st->map[st->selected]];
            info.push_back(text(p.desc) | bold);
            info.push_back(text("value " + val(p) + "  default " + num(p.def) + "  step " + num(p.step)) | dim);
        }
        for (const auto& f : formulaFor(grp)) info.push_back(text(f) | color(Color::Cyan));
        return vbox({text("Factors: " + grp) | bold, separator(), param_menu->Render() | vscroll_indicator | frame | flex,
                     separator(), vbox(info)}) | border;
    });
    auto left = ResizableSplitLeft(group_panel, param_panel, &st->group_w);

    // everything below reads the same Balance:: functions the game uses
    auto live = [=]() {  // current gear numbers: real if this is the running game's series, else modeled
        struct R { double hustle, ndas, runway; bool real; double gh, gn; } r{};
        Character* ch = engine->getCharacter();
        r.real = engine->isInitialized() && ch && ch->getSeries() == st->series;
        if (r.real) {
            const auto& pr = ch->getProfile();
            r.hustle = pr.hustle; r.ndas = pr.ndas; r.runway = ch->getMaxRunway();
        } else {
            const auto P = Balance::player(st->series);
            Balance::modelGear(st->series, r.gh, r.gn);
            r.hustle = P.hustle + r.gh; r.ndas = P.ndas + r.gn; r.runway = P.runway;
        }
        return r;
    };

    auto plots_panel = Renderer([=] {
        const double elapsed = st->year * 365.0 * 86400.0;
        auto* qs = engine->getQuestSystem();
        auto pointAt = [&](int S, double reached, int quest) {
            double gh, gn;
            Balance::modelGear(S, gh, gn);
            const auto m = Balance::seriesMetrics(S, elapsed, gh, gn);
            const auto qp = qs->preview(quest, elapsed);
            return makePoint(m, reached, nullptr, qp.budget / 86400.0, static_cast<double>(qp.sprints), Balance::fundingRequired(S + 1, elapsed));
        };
        const int yi = std::clamp(static_cast<int>(st->year), 0, 20);
        auto build = [&](int mode) {
            Data d;
            if (mode == 0) {            // years: real projected progression
                const auto pj = Balance::project();
                d.xlabel = "year"; d.xlo = 0; d.xhi = 20; d.marker = st->year;
                for (int y = 0; y <= 20; ++y) {
                    const auto& m = pj.atYear[y];
                    const auto qp = qs->preview(st->quest, y * 365.0 * 86400.0);
                    d.xs.push_back(y);
                    d.pts.push_back(makePoint(m, m.series, nullptr, qp.budget / 86400.0, static_cast<double>(qp.sprints),
                                              Balance::fundingRequired(m.series + 1, y * 365.0 * 86400.0)));
                }
            } else if (mode == 1) {     // series 1..100 at the selected year
                d.xlabel = "series (at year " + num(st->year) + ")"; d.xlo = 1; d.xhi = 100; d.marker = st->series;
                for (int s = 1; s <= 100; ++s) { d.xs.push_back(s); d.pts.push_back(pointAt(s, s, st->quest)); }
            } else if (mode == 2) {     // quest index 1..21
                const auto pj = Balance::project();
                d.xlabel = "quest index (series " + std::to_string(st->series) + ", year " + num(st->year) + ")"; d.xlo = 1; d.xhi = 21; d.marker = st->quest;
                for (int q = 1; q <= 21; ++q) { d.xs.push_back(q); d.pts.push_back(pointAt(st->series, pj.seriesAtYear[yi], q)); }
            } else {                    // sweep the selected factor
                auto& tn = Tuning::I();
                std::string key = "combat.player_dmg_mult";
                if (st->selected < static_cast<int>(st->map.size())) key = tn.params()[st->map[st->selected]].key;
                const double base = tn.get(key.c_str());
                double step = 1;
                for (const auto& p : tn.params()) if (p.key == key) step = p.step;
                const double lo = base != 0 ? base * 0.5 : -10 * step, hi = base != 0 ? base * 1.5 : 10 * step;
                d.xlabel = key + " (series " + std::to_string(st->series) + ", year " + num(st->year) + ")";
                d.xlo = std::min(lo, hi); d.xhi = std::max(lo, hi); d.marker = base;
                for (int k = 0; k <= 20; ++k) {
                    const double v = lo + (hi - lo) * k / 20.0;
                    tn.set(key, v);
                    const auto pj = Balance::project();
                    d.xs.push_back(v);
                    d.pts.push_back(pointAt(st->series, pj.seriesAtYear[yi], st->quest));
                }
                tn.set(key, base);
            }
            return d;
        };
        std::array<std::optional<Data>, 4> cache;   // each x mode computed at most once per frame
        auto dataFor = [&](int mode) -> const Data& { if (!cache[mode]) cache[mode] = build(mode); return *cache[mode]; };

        const int n = static_cast<int>(st->plots.size());
        if (n == 0)
            return vbox({text("No plots. Press 'a' (or + Plot) to add one, 'e' (or Edit Plot) to choose its variables.") | dim | center}) | border;
        st->slot = std::clamp(st->slot, 0, n - 1);
        const int cols = n == 1 ? 1 : 2, rows = (n + cols - 1) / cols;
        const int termw = Terminal::Size().dimx;
        const int panelw = std::max(40, termw - st->left_w - 6);
        const int colw = (panelw - 2) / cols;
        const int wc = colw - 9;
        const int hc = std::max(3, (st->plot_h - 3) / rows - 4);
        static const Color palette[8] = {Color::Cyan, Color::Green, Color::Yellow, Color::Magenta, Color::Red, Color::Blue, Color::White, Color::GrayLight};

        Elements grid;
        for (int r = 0; r < rows; ++r) {
            Elements cells;
            for (int c = 0; c < cols; ++c) {
                const int i = r * cols + c;
                if (i >= n) break;
                const PlotCfg& cfg = st->plots[i];
                const Data& d = dataFor(cfg.xmode);
                std::vector<Line> lines;
                std::string title;
                int k = 0;
                for (int m = 0; m < M_COUNT; ++m) {
                    if (!cfg.y[m]) continue;
                    Line l{kMetricName[m], {}, palette[k++ % 8]};
                    for (const auto& pt : d.pts) l.y.push_back(pt[m]);
                    lines.push_back(std::move(l));
                }
                title = "[" + std::to_string(i + 1) + "] " + (lines.empty() ? std::string("(no variables: press e)") : std::string(kXModes[cfg.xmode]).substr(0, 6));
                cells.push_back(chart(title, wc, hc, d.xs, d.xlo, d.xhi, d.xlabel, lines, cfg.logy, cfg.baseline, d.marker, st->slot == i) |
                                size(WIDTH, EQUAL, colw));
            }
            grid.push_back(hbox(cells));
        }
        return vbox(grid) | border;
    });

    auto results_panel = Renderer([=] {
        const int S = st->series;
        const double elapsed = st->year * 365.0 * 86400.0;
        const auto P = Balance::player(S);
        const auto g = live();
        const double pm = T("combat.player_dmg_mult"), em = T("combat.enemy_dmg_mult");
        const int yr = static_cast<int>(st->year) + 1;
        const int ntype = yr <= 5 ? 0 : yr <= 10 ? 1 : yr <= 15 ? 2 : 3;
        const double med = Balance::difficultyMult(1);
        struct Foe { const char* n; Balance::EnemyStats e; };
        Balance::EnemyStats shadow{g.runway * T("shadow.hp_mult"), g.hustle * T("shadow.atk_mult"), g.ndas * T("shadow.def_mult"), 0, 0};
        const std::vector<Foe> foes = {
            {"Mob", Balance::mob(S, med)}, {"Boss", Balance::boss(S, med)}, {"Nemesis", Balance::nemesis(S, elapsed, ntype, 0)},
            {"Safety Mgr", Balance::special(0, S, med)}, {"Conference", Balance::special(1, S, med)},
            {"Paper", Balance::special(2, S, med)}, {"Review boss", Balance::review(yr, 6, med)}, {"Shadow", shadow}};
        Elements stats, fights;
        for (const auto& f : foes) {
            stats.push_back(row(f.n, "HP " + num(f.e.hp) + " ATK " + num(f.e.atk) + " DEF " + num(f.e.def) + " $" + num(f.e.money)));
            double tk = turns(f.e.hp, std::max(1.0, (g.hustle - f.e.def) * pm));
            double tl = turns(g.runway, std::max(1.0, (f.e.atk - g.ndas) * em));
            fights.push_back(text(std::string(f.n) + ": you " + num(tk) + " / it " + num(tl) + " -> " + (tk <= tl ? "WIN" : "LOSE")) |
                             color(tk <= tl ? Color::Green : Color::Red));
        }
        auto qp = engine->getQuestSystem()->preview(st->quest, elapsed);
        return vbox({
                   text("series " + std::to_string(S) + "  year " + num(st->year) + "  quest #" + std::to_string(st->quest) +
                        "   (s/S y/Y q/Q)  class(c): " + Balance::projectionClassName()) | bold,
                   text(g.real ? "fights use REAL equipped gear" : "fights use modeled gear +" + num(g.gh) + " hustle +" + num(g.gn) + " NDAs") | dim,
                   row("Runway/Hype", num(P.runway) + " / " + num(P.hype)),
                   row("Hustle/NDAs used", num(g.hustle) + " / " + num(g.ndas)),
                   row("Next series cost", "$" + num(Balance::fundingRequired(S + 1, elapsed))),
                   separator(), vbox(stats), separator(), vbox(fights), separator(),
                   row("Quest budget", num(qp.budget / 86400.0) + " d, hustles " + std::to_string(qp.hustlesRequired) + ", devs " + std::to_string(qp.devs)),
                   row("Avg sprint", num(qp.avgSprintSeconds) + " s -> " + std::to_string(qp.sprints) + " sprints"),
               }) | vscroll_indicator | yframe | border;
    });
    auto right = ResizableSplitTop(plots_panel, results_panel, &st->plot_h);
    auto main_split = ResizableSplitLeft(left, right, &st->left_w);

    auto table_panel = Renderer([=] {
        const auto pj = Balance::project();
        Elements rows;
        rows.push_back(hbox({cell("yr", 3), cell("ser", 4), cell("runway", 8), cell("hustle", 8), cell("ndas", 7),
                             cell("mob y/it", 12), cell("boss y/it", 12), cell("nem y/it", 14), cell("life", 8), cell("d/next", 8)}) | bold);
        int fm = -1, fb = -1, fn = -1;
        for (size_t y = 0; y < pj.atYear.size(); ++y) {
            const auto& m = pj.atYear[y];
            const bool lm = m.hitsMob > m.survMob, lb = m.hitsBoss > m.survBoss, ln = m.hitsNemesis > m.survNemesis;
            if (lm && fm < 0) fm = static_cast<int>(y);
            if (lb && fb < 0) fb = static_cast<int>(y);
            if (ln && fn < 0) fn = static_cast<int>(y);
            Color c = lm ? Color::Red : lb ? Color::Yellow : ln ? Color::Magenta : Color::White;
            rows.push_back(hbox({cell(std::to_string(y), 3), cell(std::to_string(m.series), 4), cell(num(m.runway), 8),
                                 cell(num(m.hustle), 8), cell(num(m.ndas), 7),
                                 cell(num(m.hitsMob) + "/" + num(m.survMob), 12), cell(num(m.hitsBoss) + "/" + num(m.survBoss), 12),
                                 cell(num(m.hitsNemesis) + "/" + num(m.survNemesis), 14), cell(num(m.encountersPerLife), 8),
                                 cell(num(m.secondsToNextSeries / 86400.0), 8)}) | color(c));
        }
        auto yrs = [](int y) { return y < 0 ? std::string("never") : "year " + std::to_string(y); };
        return vbox({
                   vbox(rows) | border,
                   text("Series at year 20: " + std::to_string(pj.seriesAtYear.back())) | bold,
                   text("You lose to a mob from:     " + yrs(fm)) | color(fm < 0 ? Color::Green : Color::Red),
                   text("You lose to a boss from:    " + yrs(fb)) | color(fb < 0 ? Color::Green : Color::Yellow),
                   text("You lose to the nemesis from: " + yrs(fn)) | color(fn < 0 ? Color::Green : Color::Magenta),
                   text("y/it = hits you need / hits it needs to kill you. life = encounters per life.") | dim,
                   text("model: modeled gear, no patents/healing, difficulty+encounter mix, you always win.") | dim,
               }) | flex;
    });

    auto save_btn = Button("Save Tuning", [st] {
        st->msg = Tuning::I().save("tuning.txt") ? "Wrote ./tuning.txt  (copy to config/tuning.txt, rebuild to ship)" : "Save failed";
    });
    auto reset_btn = Button("Reset All", [st] { Tuning::I().resetAll(); st->msg = "All factors reset to defaults"; });

    auto newCfg = [] { PlotCfg c; loadPreset(c, 1); return c; };
    auto openEditor = [st, newCfg] {
        if (st->plots.empty()) { st->plots.push_back(newCfg()); st->slot = 0; }
        const PlotCfg& c = st->plots[st->slot];
        st->ex = c.xmode; st->ey = c.y; st->elog = c.logy; st->ebase = c.baseline;
        st->emsg.clear();
        st->show_modal = true;
    };
    auto add_btn = Button("+ Plot", [st, newCfg, openEditor] { st->plots.push_back(newCfg()); st->slot = static_cast<int>(st->plots.size()) - 1; openEditor(); });
    auto del_btn = Button("- Plot", [st] {
        if (st->plots.empty()) return;
        st->plots.erase(st->plots.begin() + st->slot);
        st->slot = std::clamp(st->slot, 0, std::max(0, static_cast<int>(st->plots.size()) - 1));
    });
    auto edit_btn = Button("Edit Plot", [openEditor] { openEditor(); });

    auto nav = CreateNavBar(selected_page, engine, std::move(exit_closure), TUNER_PAGE);
    auto container = Container::Vertical({main_split, Container::Horizontal({save_btn, reset_btn, add_btn, del_btn, edit_btn}), nav});

    st->plots.push_back(newCfg());   // start with one plot

    // ---- popup: checkmarks for the x axis and the y variables ----
    // auto xnames = std::make_shared<std::vector<std::string>>(kXModes, kXModes + 4);x
    // auto xradio = Radiobox(xnames.get(), &st->ex);
    st->xnames.assign(std::begin(kXModes), std::end(kXModes));
    auto xradio = Radiobox(&st->xnames, &st->ex);
    std::vector<Component> ycols;
    for (int c = 0; c < 3; ++c) {
        Components items;
        for (int i = c * 10; i < std::min<int>(M_COUNT, (c + 1) * 10); ++i) items.push_back(Checkbox(kMetricName[i], &st->ey[i]));
        ycols.push_back(Container::Vertical(items));
    }
    auto logcb = Checkbox("log scale (y)", &st->elog);
    auto basecb = Checkbox("break-even line (y = 1)", &st->ebase);
    auto preset_btn = Button("Load preset", [st] {
        const int np = static_cast<int>(presets().size());
        PlotCfg c;
        loadPreset(c, st->epreset);
        st->ey = c.y; st->elog = c.logy; st->ebase = c.baseline;
        st->emsg = std::string("preset: ") + presets()[st->epreset].name;
        st->epreset = (st->epreset + 1) % np;
    });
    auto apply_btn = Button("Apply", [st] {
        if (st->slot < static_cast<int>(st->plots.size())) {
            PlotCfg& c = st->plots[st->slot];
            c.xmode = st->ex; c.y = st->ey; c.logy = st->elog; c.baseline = st->ebase;
        }
        st->show_modal = false;
    });
    auto cancel_btn = Button("Cancel", [st] { st->show_modal = false; });
    auto modal_container = Container::Vertical({
        Container::Horizontal({xradio, ycols[0], ycols[1], ycols[2]}),
        Container::Horizontal({logcb, basecb}),
        Container::Horizontal({preset_btn, apply_btn, cancel_btn}),
    });
    auto modal = Renderer(modal_container, [=] {
        Elements ys;
        for (auto& c : ycols) ys.push_back(c->Render() | flex);
        return window(text("Plot " + std::to_string(st->slot + 1) + ": choose variables") | bold,
                      vbox({
                          hbox({vbox({text("X axis") | bold, separator(), xradio->Render()}) | border,
                                vbox({text("Y variables") | bold, separator(), hbox(ys)}) | border | flex}),
                          hbox({logcb->Render(), text("   "), basecb->Render()}),
                          text(st->emsg) | dim,
                          text("arrows/Tab move, Space/Enter toggles") | dim,
                          hbox({preset_btn->Render(), apply_btn->Render(), cancel_btn->Render()}) | center,
                      })) | size(WIDTH, GREATER_THAN, 78);
    });

    auto renderer = Renderer(container, [=] {
        if (!st->init) {
            st->init = true;
            if (engine->isInitialized() && engine->getCharacter()) st->series = std::max(1, engine->getCharacter()->getSeries());
        }
        return window(text(std::string("Tuner (debug) - ") + (st->view == 0 ? "factors, plots, results" : "year-by-year table")) | bold | center,
                      vbox({
                          st->view == 0 ? main_split->Render() | flex : table_panel->Render() | flex,
                          text("Left/Right groups<->factors  Up/Down  +/- step  [ ] x10  r reset | a add plot  d delete  e edit vars  1-9 pick plot  x x-axis  p table") | dim | center,
                          text(st->msg) | color(Color::Green) | center,
                          hbox({save_btn->Render(), reset_btn->Render(), add_btn->Render(), del_btn->Render(), edit_btn->Render()}) | center,
                          nav->Render(),
                      }));
    });

    auto with_keys = CatchEvent(renderer, [=](Event e) {
        auto& tn = Tuning::I();
        auto adjust = [&](double mult) {
            if (st->selected < static_cast<int>(st->map.size())) {
                const auto& p = tn.params()[st->map[st->selected]];
                tn.set(p.key, p.value + mult * p.step);
            }
            return true;
        };
        if (e == Event::Character('p')) { st->view = 1 - st->view; return true; }
        if (e == Event::Character('c')) {
            static int cls = -1;
            cls = (cls + 2) % (static_cast<int>(Archetypes::I().count()) + 1) - 1;
            Balance::setProjectionClass(cls);
            return true;
        }
        if (e == Event::Character('+') || e == Event::Character('=')) return adjust(1);
        if (e == Event::Character('-')) return adjust(-1);
        if (e == Event::Character(']')) return adjust(10);
        if (e == Event::Character('[')) return adjust(-10);
        if (e == Event::Character('r')) {
            if (st->selected < static_cast<int>(st->map.size())) tn.reset(st->map[st->selected]);
            return true;
        }
        if (e == Event::Character('a')) { st->plots.push_back(newCfg()); st->slot = static_cast<int>(st->plots.size()) - 1; openEditor(); return true; }
        if (e == Event::Character('d')) {
            if (!st->plots.empty()) {
                st->plots.erase(st->plots.begin() + st->slot);
                st->slot = std::clamp(st->slot, 0, std::max(0, static_cast<int>(st->plots.size()) - 1));
            }
            return true;
        }
        if (e == Event::Character('e')) { openEditor(); return true; }
        if (e == Event::Character('x')) {
            if (!st->plots.empty()) st->plots[st->slot].xmode = (st->plots[st->slot].xmode + 1) % 4;
            return true;
        }
        for (int i = 0; i < 9; ++i)
            if (e == Event::Character(static_cast<char>('1' + i)) && i < static_cast<int>(st->plots.size())) { st->slot = i; return true; }
        if (e == Event::Character('s')) { st->series = std::min(400, st->series + 1); return true; }
        if (e == Event::Character('S')) { st->series = std::max(1, st->series - 1); return true; }
        if (e == Event::Character('y')) { st->year = std::min(20.0, st->year + 1); return true; }
        if (e == Event::Character('Y')) { st->year = std::max(0.0, st->year - 1); return true; }
        if (e == Event::Character('q')) { st->quest = std::min(21, st->quest + 1); return true; }
        if (e == Event::Character('Q')) { st->quest = std::max(1, st->quest - 1); return true; }
        return false;
    });
    return Modal(with_keys, modal, &st->show_modal);
}
