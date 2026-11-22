#include "script.h"
#include "natives.h"
#include <windows.h>
#include <string>
#include <algorithm>

static HMODULE g_module = nullptr;
static bool g_relaxing = false;
static bool g_regenHealth = true;
static bool g_slowTime = true;
static bool g_comboWasDown = false;
static std::string g_scenario = "WORLD_HUMAN_SUNBATHE_BACK";
static DWORD g_lastHeal = 0;

static std::string ConfigPath()
{
    char path[MAX_PATH]{};
    GetModuleFileNameA(g_module, path, MAX_PATH);
    std::string s(path);
    const size_t slash = s.find_last_of("\\/");
    s = (slash == std::string::npos) ? ".\\" : s.substr(0, slash + 1);
    return s + "LayAndRelax.ini";
}

static void LoadSettings()
{
    const std::string path = ConfigPath();
    char scenario[128] = "WORLD_HUMAN_SUNBATHE_BACK";
    g_regenHealth = GetPrivateProfileIntA("Options", "RegenHealth", 1, path.c_str()) != 0;
    g_slowTime = GetPrivateProfileIntA("Options", "SlowTime", 1, path.c_str()) != 0;
    GetPrivateProfileStringA("Options", "Scenario", "WORLD_HUMAN_SUNBATHE_BACK",
        scenario, sizeof(scenario), path.c_str());
    g_scenario = scenario;

    // Write defaults on first launch without overwriting existing values.
    WritePrivateProfileStringA("Controls", "ToggleCombo", "LT+RT", path.c_str());
    WritePrivateProfileStringA("Options", "RegenHealth", "1", path.c_str());
    WritePrivateProfileStringA("Options", "SlowTime", "1", path.c_str());
    WritePrivateProfileStringA("Options", "Scenario", "WORLD_HUMAN_SUNBATHE_BACK", path.c_str());
}

static void Notify(const char* message)
{
    BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");
    ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(const_cast<char*>(message));
    END_TEXT_COMMAND_THEFEED_POST_TICKER(false, true);
}

static void ResetTimeScale()
{
    SET_TIME_SCALE(1.0f);
}

static void StopRelaxing()
{
    const Ped ped = PLAYER_PED_ID();
    if (DOES_ENTITY_EXIST(ped))
        CLEAR_PED_TASKS(ped);

    if (g_slowTime)
        ResetTimeScale();

    g_relaxing = false;
    Notify("~g~Back on your feet.");
}

static void StartRelaxing()
{
    const Ped ped = PLAYER_PED_ID();

    if (!DOES_ENTITY_EXIST(ped) ||
        IS_ENTITY_DEAD(ped, false) ||
        IS_PED_IN_ANY_VEHICLE(ped, false) ||
        IS_ENTITY_IN_AIR(ped) ||
        IS_PED_SWIMMING(ped) ||
        IS_PED_RAGDOLL(ped) ||
        IS_PED_FALLING(ped) ||
        IS_PED_CLIMBING(ped) ||
        IS_PED_GETTING_UP(ped))
    {
        Notify("~y~Can't relax right now.");
        return;
    }

    TASK_START_SCENARIO_IN_PLACE(ped, g_scenario.c_str(), 0, true);
    g_lastHeal = GET_GAME_TIMER();
    g_relaxing = true;
    Notify("~g~Relaxing...~s~ press ~b~F2~s~ to get up.");
}

static bool ToggleComboPressed()
{
    // GTA V controller controls: 25 = Aim (LT), 24 = Attack (RT).
    return IS_CONTROL_PRESSED(0, 25) && IS_CONTROL_PRESSED(0, 24);
}

static bool CancelInputPressed()
{
    return IS_CONTROL_PRESSED(0, 30) || // Move left/right
           IS_CONTROL_PRESSED(0, 31) || // Move up/down
           IS_CONTROL_PRESSED(0, 21) || // Sprint
           IS_CONTROL_PRESSED(0, 22) || // Jump
           IS_CONTROL_PRESSED(0, 25) || // Aim
           IS_CONTROL_PRESSED(0, 24);   // Attack
}

static void Tick()
{
    const bool comboDown = ToggleComboPressed();
    const bool comboJustPressed = comboDown && !g_comboWasDown;
    g_comboWasDown = comboDown;

    if (comboJustPressed)
    {
        if (g_relaxing)
            StopRelaxing();
        else
            StartRelaxing();

        return;
    }

    // LT+RT is the toggle chord; do not interpret its two component
    // controls as ordinary aim/attack cancellation while held.
    if (comboDown)
        return;

    if (!g_relaxing)
        return;

    const Ped ped = PLAYER_PED_ID();
    if (!DOES_ENTITY_EXIST(ped) || IS_ENTITY_DEAD(ped, false))
    {
        StopRelaxing();
        return;
    }

    if (CancelInputPressed() ||
        IS_PED_IN_ANY_VEHICLE(ped, false) ||
        IS_PED_RAGDOLL(ped))
    {
        StopRelaxing();
        return;
    }

    if (g_slowTime)
        SET_TIME_SCALE(0.85f);

    const DWORD now = GET_GAME_TIMER();
    if (g_regenHealth && (DWORD)(now - g_lastHeal) >= 1000)
    {
        g_lastHeal = now;

        const int health = GET_ENTITY_HEALTH(ped);
        const int maxHealth = GET_ENTITY_MAX_HEALTH(ped);
        if (health < maxHealth)
            SET_ENTITY_HEALTH(ped, std::min(maxHealth, health + 5), 0);

        const Player player = PLAYER_ID();
        const int armor = GET_PED_ARMOUR(ped);
        const int maxArmor = GET_PLAYER_MAX_ARMOUR(player);
        if (armor < maxArmor)
            SET_PED_ARMOUR(ped, std::min(maxArmor, armor + 2));
    }
}

void ScriptMain()
{
    LoadSettings();
    while (true)
    {
        Tick();
        WAIT(0);
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_module = module;
        scriptRegister(module, ScriptMain);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (g_relaxing)
            ResetTimeScale();
        scriptUnregister(module);
    }
    return TRUE;
}
