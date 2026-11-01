#ifndef PLAYERSTRATEGY_H
#define PLAYERSTRATEGY_H

#include <random>
#include <string>

class Character;
class BattleEncounter;
class Monster;

// result of player action choice. empty patent = basic hit on lowest-HP enemy.
struct PlayerChoice {
    std::string patent;
    bool aoe = false;
    Monster* target = nullptr;   // enemy target (attack/debuff). self-targeted patents ignore it
    double score = 0.0;
};

// scored patent selection: hard filters (no recast while active, no heal at high HP, no
// buff/defense on trivial fights), class weights, hype reserve for heals, small noise.
namespace PlayerStrategy {
PlayerChoice choose(Character& pc, BattleEncounter& enc, std::mt19937& rng);
}

#endif
