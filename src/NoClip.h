#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "World.h"
#include "VMapFactory.h"
#include "VMapMgr2.h"
#include "Geometry.h"
#include "Timer.h"
#include "Object.h"
#include "MapMgr.h"
#include "Creature.h"
#include "Transport.h"
#include "World.h"
#include "Util.h"
#include "GridDefines.h"
#include "condition_variable"
#include "iostream"
#include "mutex"
#include "chrono"
#include "thread"
#include "atomic"
#include "functional"


bool NC_Enable = true;
bool NC_Announce_Enable = false;
uint32 NC_AllowedAccountTypeMax = 2;
float NC_TeleportDistance = 1.0f;

class CustomPlayerInformation : public DataMap::Base
{
public:
    CustomPlayerInformation();
    CustomPlayerInformation(Player* APlayer);

    void SetPlayer(Player* APlayer);
    Player* GetMyPlayer() const;

    void SetTeleportDistance(float ATeleportDistance);
    float GetTeleportDistance() const;

    void SetFlagNoClipEnabled(bool AIsNoClipEnabled);
    bool GetFlagNoClipEnabled() const;

    bool SetThreadHandlerState(int AState);
private:
    Player* player = nullptr;
    float TeleportDistance = 1.0f;

    std::mutex NoClipMutex;
    std::thread NoClipThread;
    std::thread ThreadHandler;

    std::atomic<bool> IsNoClipEnabled = false;

    void HandleNoClipMovement();
    void HandleNoClipThread();

    bool IsPlayerMovingOrFlying();
    bool IsPlayerFlyingUp();
    bool IsPlayerFlyingDown();
    bool IsPlayerFlyingDirections();

    bool IsInvisibleWallInFront(Player* player, float distance);
    bool CollisionInFront(Position xPosPlayer, Position xPosCollision);
    bool IsObstacleInFront();
};

class NoClipConfigLoader : public WorldScript
{
public:
    NoClipConfigLoader() : WorldScript("NoClipConfig") {}

    void OnBeforeConfigLoad(bool /*reload*/) override;
    void SetInitialWorldSettings();
};

class NoClipCommand : public CommandScript
{
public:
    NoClipCommand() : CommandScript("NoClipCommand") {}

    /* Command-Handler */
    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        if (NC_Enable)
        {
            static Acore::ChatCommands::ChatCommandTable xNoClipCommandTable =
            {
                { "on", NoClipCommand::HandleNoClipCommand, NC_AllowedAccountTypeMax, Acore::ChatCommands::Console::No },
                { "off", NoClipCommand::HandleClipCommand, NC_AllowedAccountTypeMax, Acore::ChatCommands::Console::No },
                { "set TeleportDistance", NoClipCommand::HandleSetTeleportDistanceCommand, NC_AllowedAccountTypeMax, Acore::ChatCommands::Console::No }

            };

            static Acore::ChatCommands::ChatCommandTable xNoClipIndividualBaseTable =
            {
                { "noclip",  xNoClipCommandTable }
            };

            return xNoClipIndividualBaseTable;
        }
        return Acore::ChatCommands::ChatCommandTable();
    }
    static bool HandleSetTeleportDistanceCommand(ChatHandler* AChatHandler, float ATeleportDistance);
    static bool HandleNoClipCommand(ChatHandler* AChatHandler);
    static bool HandleClipCommand(ChatHandler* AChatHandler);

};

class NoClipPlayer : public PlayerScript
{
public:
    NoClipPlayer() : PlayerScript("NoClipPlayer") {}

    void OnLogin(Player* APlayer) override;
    void OnBeforeLogout(Player* APlayer) override;
};

void AddNoClipScripts()
{
    new NoClipConfigLoader();
    new NoClipCommand();
    new NoClipPlayer();
}
