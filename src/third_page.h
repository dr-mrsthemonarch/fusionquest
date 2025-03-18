#ifndef THIRD_PAGE_H
#define THIRD_PAGE_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <functional>
#include <utility>
#include <vector>
#include <string>

#include "functions.h"

using namespace ftxui;

// Function to create the Fusion Quest page components and renderer
Component CreateFusionQuestPage() {
    // Renderer for the Fusion Quest page
    return Renderer([]
    {
        return vbox({
            // Title "Fusion Quest" centered in a box that fills the screen
            window(
                text("Fusion Quest") | hcenter | bold,
                vbox({
                    text("Welcome to Fusion Quest!"),
                    text("Embark on an epic journey of discovery and adventure."),
                    text("Unlock the secrets of the universe and harness the power of fusion."),
                })
            ) | border | center,

            // Three columns below the inner box
            hbox({
                // Race column
                window(
                    text("Race") | hcenter | bold,
                    vbox({
                        text("Human"),
                        text("Elf"),
                        text("Dwarf"),
                        text("Orc"),
                    }) | flex
                ) | border | flex,

                // Class column
                window(
                    text("Class") | hcenter | bold,
                    vbox({
                        text("Warrior"),
                        text("Mage"),
                        text("Rogue"),
                        text("Cleric"),
                    }) | flex
                ) | border | flex,

                // Stats column
                window(
                    text("Stats") | hcenter | bold,
                    vbox({
                        text("Strength: 10"),
                        text("Dexterity: 8"),
                        text("Intelligence: 12"),
                        text("Wisdom: 9"),
                    }) | flex
                ) | border | flex,
            }) | flex,
        }) | flex;
    });
}

#endif // THIRD_PAGE_H