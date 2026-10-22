cmake_minimum_required(VERSION 3.31)
project(fusionquest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

include(FetchContent)

FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v7.0.0
)
FetchContent_MakeAvailable(ftxui)
include_directories(${ftxui_SOURCE_DIR}/include)

# ftxui shell / pages
set(UI_SOURCES
        src/main.cpp
        src/ui.cpp
        src/main_menu.cpp
        src/character_creation.cpp
        src/stats_page.cpp
        src/battle_page.cpp
        src/equipment_page.cpp
        src/special_drops_page.cpp
        src/load_page.cpp
        src/functions.cpp
        src/preamble_page.cpp
        src/final_warning_page.cpp
)

# engine (Qt-free, was SPG_Engine game logic)
set(ENGINE_SOURCES
        src/GameEngine.cpp
        src/SaveGame.cpp
        src/Character.cpp
        src/Monster.cpp
        src/MonsterGenerator.cpp
        src/EquipmentGenerator.cpp
        src/ItemGenerator.cpp
        src/Patent.cpp
        src/PatentLibrary.cpp
        src/PatentStrategy.cpp
        src/NemesisSystem.cpp
        src/ExpandedMonsterTypes.cpp
        src/SpecialDropsLibrary.cpp
        src/TemporaryEffect.cpp
        src/NamedMonster.cpp
        src/NamedMonsterLibrary.cpp
        src/BattleEncounter.cpp
        src/Quest.cpp
        src/QuestGenerator.cpp
        src/QuestSystem.cpp
)

set(HEADERS
        include/ui.h
        include/globals.h
        include/progress_bar.h
        include/GameEngine.h
        include/BattleLog.h
        include/SaveGame.h
        include/Character.h
        include/Monster.h
        include/MonsterGenerator.h
        include/EquipmentGenerator.h
        include/Equipment.h
        include/ItemGenerator.h
        include/Patent.h
        include/PatentLibrary.h
        include/PatentStrategy.h
        include/PatentDefinition.h
        include/NemesisSystem.h
        include/ExpandedMonsterTypes.h
        include/SpecialDropsLibrary.h
        include/StdoutSink.h
        include/TemporaryEffect.h
        include/NamedMonster.h
        include/NamedMonsterLibrary.h
        include/BattleEncounter.h
        include/CombatEntity.h
        include/Quest.h
        include/QuestGenerator.h
        include/QuestSystem.h
)

add_executable(fusionquest ${UI_SOURCES} ${ENGINE_SOURCES} ${HEADERS})

target_link_libraries(fusionquest
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)

if (CMAKE_VERSION VERSION_GREATER 3.24)
    set_property(TARGET fusionquest PROPERTY CXX_STANDARD 20)
endif()

if (NOT TARGET ftxui::screen OR NOT TARGET ftxui::dom)
    message(FATAL_ERROR "FTXUI libraries not found!")
endif()

add_custom_command(TARGET fusionquest PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/bin/config
)

# --debug-instakill CLI flag replaces Qt debugDefeatCheckbox. no runtime UI toggle;
# read once at startup in main.cpp (argv parse), passed to GameEngine ctor.
# strip from release builds by simply not documenting/shipping the flag (no code removal needed
# unless you want the branch gone entirely - see main.cpp comment).