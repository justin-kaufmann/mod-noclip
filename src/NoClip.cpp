#include "NoClip.h"

/* Class: CustomPlayerInformation */

CustomPlayerInformation::CustomPlayerInformation()
{
    this->IsNoClipEnabled = false;
}
CustomPlayerInformation::CustomPlayerInformation(Player* APlayer)
{
    if (APlayer)
    {
        this->player = APlayer;
    }

    this->IsNoClipEnabled = false;
}

void CustomPlayerInformation::SetPlayer(Player* APlayer)
{
    if (APlayer)
    {
        this->player = APlayer;
    }
    return;
}
Player* CustomPlayerInformation::GetMyPlayer() const
{
    return this->player;
}

void CustomPlayerInformation::SetTeleportDistance(float ATeleportDistance)
{
    if (ATeleportDistance > 0.0f)
    {
        {
            std::lock_guard<std::mutex> lockGuard(NoClipMutex);
            this->TeleportDistance = ATeleportDistance;
        }
    }

    return;
}
float CustomPlayerInformation::GetTeleportDistance() const
{
    return this->TeleportDistance;
}

void CustomPlayerInformation::SetFlagNoClipEnabled(bool AIsNoClipEnabled)
{
    this->IsNoClipEnabled = AIsNoClipEnabled;

    return;
}
bool CustomPlayerInformation::GetFlagNoClipEnabled() const
{
    return this->IsNoClipEnabled;
}

bool CustomPlayerInformation::SetThreadHandlerState(int AState)
{
    bool xResult = false;

    if (this->player)
    {
        switch (AState)
        {
            case 1:
            {
                if (!this->IsNoClipEnabled)
                {
                    {
                        std::lock_guard<std::mutex> lockGuard(NoClipMutex);

                        this->IsNoClipEnabled = true;

                        ThreadHandler = std::thread([this]()
                            {
                                while (this->IsNoClipEnabled)
                                {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                    CustomPlayerInformation::HandleNoClipMovement();
                                }
                            });
                        ThreadHandler.detach();

                        NoClipThread = std::thread([this]()
                            {
                                while (this->IsNoClipEnabled)
                                {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                    CustomPlayerInformation::HandleNoClipThread();
                                }
                            });
                        NoClipThread.detach();

                        xResult = true;
                    }
                }
                break;
            }
            case 2:
            {
                if (this->IsNoClipEnabled)
                {
                    {
                        std::lock_guard<std::mutex> lockGuard(NoClipMutex);

                        this->IsNoClipEnabled = false;

                        xResult = true;
                    }
                }
                break;
            }
            default:
            {
                xResult = false;
                break;
            }
        }
    }

    return xResult;
}
void CustomPlayerInformation::HandleNoClipMovement()
{
    if (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        if (!this->player->CanFly())
        {
            this->player->SetCanFly(true);
        }

        while (this->IsNoClipEnabled)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (this->player->CanFly())
        {
            this->player->SetCanFly(false);
        }
    }
}
void CustomPlayerInformation::HandleNoClipThread()
{
    while (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        if (this->TeleportDistance < 0.0f)
        {
            this->TeleportDistance = NC_TeleportDistance;
        }

        float xOrientation = this->player->GetOrientation();
        float xPosX = this->player->GetPositionX();
        float xPosY = this->player->GetPositionY();
        float xPosZ = this->player->GetPositionZ();
        uint32 xMapId = this->player->GetMapId();

        float xNewPosX = xPosX;
        float xNewPosY = xPosY;
        float xNewPosZ = xPosZ;

        if (IsPlayerMovingOrFlying())
        {
            xNewPosX += this->TeleportDistance * cos(xOrientation);
            xNewPosY += this->TeleportDistance * sin(xOrientation);
            Position xCollisionPos = player->GetFirstCollisionPosition(xPosX, xPosY, xPosZ, xNewPosX, xNewPosY);
            Position xPlayerPos = Position(xPosX, xPosY, xPosZ, xOrientation);

            if (this->player->isMoving() || IsPlayerFlyingDirections() || IsPlayerFlyingDown() || IsPlayerFlyingUp())
            {
                if ((IsObstacleInFront() || (!PathGenerator::IsWalkableClimb(xPosX, xPosY, xPosZ, xNewPosX, xNewPosY, xPosZ, this->player->GetCollisionHeight()) ||
                    (CollisionInFront(xPlayerPos, xCollisionPos)))))
                {
                    this->player->TeleportTo(xMapId, xNewPosX, xNewPosY, xPosZ, xOrientation);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return;
}

bool CustomPlayerInformation::IsPlayerMovingOrFlying()
{
    //if (player && NC_Enable && NoClipActivated)
    if (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        return (IsPlayerFlyingDirections() || IsPlayerFlyingDown() || IsPlayerFlyingUp()) ||
            player->isMoving();
    }
    return false;
}
bool CustomPlayerInformation::IsPlayerFlyingUp()
{
    //if (player && NC_Enable && NoClipActivated)
    if (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        return player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_ASCENDING) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_PITCH_UP) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_ASCENDING) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_PITCH_UP);
    }
    return false;
}
bool CustomPlayerInformation::IsPlayerFlyingDown()
{
    //if (player && NC_Enable && NoClipActivated)
    if (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        return player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DESCENDING) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_PITCH_DOWN) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_DESCENDING) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_PITCH_DOWN);
    }
    return false;
}
bool CustomPlayerInformation::IsPlayerFlyingDirections()
{
    //if (player && NC_Enable && NoClipActivated)
    if (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        return player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_FORWARD) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_BACKWARD) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_LEFT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_RIGHT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_STRAFE_LEFT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_STRAFE_RIGHT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FORWARD) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_BACKWARD) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_LEFT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_RIGHT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_STRAFE_LEFT) ||
            player->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_STRAFE_RIGHT);
    }
    return false;
}

bool CustomPlayerInformation::IsInvisibleWallInFront(Player* player, float distance)
{
    /*float playerX, playerY, playerZ;
    player->GetPosition(playerX, playerY, playerZ);

    float orientation = player->GetOrientation();
    float checkX = playerX + distance * cos(orientation);
    float checkY = playerY + distance * sin(orientation);

    float distanceToPoint = player->GetDistance2d(checkX, checkY);
    return (distanceToPoint < distance); // Falls die Distanz kleiner als erwartet ist, könnte etwas im Weg sein*/
    return false;
}
bool CustomPlayerInformation::CollisionInFront(Position xPosPlayer, Position xPosCollision)
{
    if (this->player)
    {
        float xPosPlayerX = xPosPlayer.GetPositionX();
        float xPosPlayerY = xPosPlayer.GetPositionY();
        float xPosPlayerZ = xPosPlayer.GetPositionZ();
        float xPosPlayerOrientation = xPosPlayer.GetOrientation();

        float xPosCollisionX = xPosCollision.GetPositionX();
        float xPosCollisionY = xPosCollision.GetPositionY();
        float xPosCollisionZ = xPosCollision.GetPositionZ();
        float xPosCollisionOrientation = xPosCollision.GetOrientation();

        bool isWaterNext = player->GetMap()->IsInWater(this->player->GetPhaseMask(), xPosPlayerX, xPosPlayerY, xPosPlayerZ, this->player->GetCollisionHeight());

        PathGenerator path(this->player);
        path.SetUseRaycast(true);

        bool result = path.CalculatePath(xPosPlayerX, xPosPlayerY, xPosPlayerZ,
            xPosCollisionX, xPosCollisionY, xPosCollisionZ, false);

        bool notOnGround = path.GetPathType() & PATHFIND_NOT_USING_PATH
            || isWaterNext || (this->player && this->player->IsFlying());

        // Check for valid path types before we proceed
        if (!result || (!notOnGround && path.GetPathType() & ~(PATHFIND_NORMAL | PATHFIND_SHORTCUT | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY_END)))
        {
            return true;
        }

        G3D::Vector3 endPos = path.GetPath().back();
        xPosCollisionX = endPos.x;
        xPosCollisionY = endPos.y;
        xPosCollisionZ = endPos.z;

        // check static LOS
        float halfHeight = this->player->GetCollisionHeight() * 0.5f;

        bool col = VMAP::VMapFactory::createOrGetVMapMgr()->GetObjectHitPos(this->player->GetMapId(),
            xPosPlayerX, xPosPlayerY, xPosPlayerZ + halfHeight,
            xPosCollisionX, xPosCollisionY, xPosCollisionZ + halfHeight,
            xPosCollisionX, xPosCollisionY, xPosCollisionZ, -CONTACT_DISTANCE);

        // Collided with static LOS object, move back to collision point
        if (col)
        {
            return true;
        }
        xPosCollisionZ -= halfHeight;

        // check dynamic collision
        col = this->player->GetMap()->GetObjectHitPos(this->player->GetPhaseMask(),
            xPosPlayerX, xPosPlayerY, xPosPlayerZ + halfHeight,
            xPosCollisionX, xPosCollisionY, xPosCollisionZ + halfHeight,
            xPosCollisionX, xPosCollisionY, xPosCollisionZ, -CONTACT_DISTANCE);

        xPosCollisionZ -= halfHeight;

        // Collided with a gameobject, move back to collision point
        if (col)
        {
            return true;
        }

        float xCurMapHeight = this->player->GetMapHeight(xPosPlayerX, xPosPlayerY, xPosPlayerZ);
        float xNewMapHeight = this->player->GetMapHeight(xPosCollisionX, xPosCollisionY, xPosCollisionZ);

        if (xCurMapHeight != xNewMapHeight)
        {
            if (this->player->isMoving() && !(IsPlayerFlyingDirections() || IsPlayerFlyingDown() || IsPlayerFlyingUp()))
            {
                return true;
            }
        }
    }

    return false;
}
bool CustomPlayerInformation::IsObstacleInFront()
{
    //if (player && NC_Enable && NoClipActivated)
    if (this->player && NC_Enable && this->IsNoClipEnabled)
    {
        float xOrientation = player->GetOrientation();
        float xPosX = player->GetPositionX();
        float xPosY = player->GetPositionY();
        float xPosZ = player->GetPositionZ();
        float xCheckPosX = xPosX + cos(xOrientation);
        float xCheckPosY = xPosY + sin(xOrientation);

        return (!player->IsWithinLOS(xCheckPosX, xCheckPosY, xPosZ, VMAP::ModelIgnoreFlags::Nothing, LINEOFSIGHT_ALL_CHECKS));
    }
    return false;
}

/* Class: NoClipConfigLoader */

void NoClipConfigLoader::OnBeforeConfigLoad(bool /*reload*/)
{
    SetInitialWorldSettings();

    return;
}
void NoClipConfigLoader::SetInitialWorldSettings()
{
    NC_Enable = sConfigMgr->GetOption<bool>("Module.Enable", true);
    NC_Announce_Enable = sConfigMgr->GetOption<bool>("Module.Announce.Enable", true && NC_Enable);
    NC_AllowedAccountTypeMax = sConfigMgr->GetOption<int>("Module.AllowedAccountTypeMax", NC_AllowedAccountTypeMax);
    NC_TeleportDistance = sConfigMgr->GetOption<float>("Module.TeleportDistance", NC_TeleportDistance);

    return;
}

/* Class: NoClipCommand */

bool NoClipCommand::HandleSetTeleportDistanceCommand(ChatHandler* AChatHandler, float ATeleportDistance)
{
    if (AChatHandler && NC_Enable && ATeleportDistance > 0.0f)
    {
        WorldSession* xPlayerSession = AChatHandler->GetSession();
        if (xPlayerSession)
        {
            Player* xPlayer = xPlayerSession->GetPlayer();
            if (xPlayer)
            {
                xPlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->SetTeleportDistance(ATeleportDistance);

                return true;
            }
        }
    }
    return false;
}
bool NoClipCommand::HandleNoClipCommand(ChatHandler* AChatHandler)
{
    //if (handler && NC_Enable && !NoClipActivated)
    if (AChatHandler && NC_Enable)
    {
        WorldSession* xPlayerSession = AChatHandler->GetSession();
        if (xPlayerSession)
        {
            Player* xPlayer = xPlayerSession->GetPlayer();
            if (xPlayer)
            {
                if (!xPlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->GetFlagNoClipEnabled())
                {
                    if (xPlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->SetThreadHandlerState(1))
                    {
                        AChatHandler->SendSysMessage("NoClip is now enabled!");
                        return true;
                    }
                    return false;

                }
            }
        }
    }
    return false;
}
bool NoClipCommand::HandleClipCommand(ChatHandler* AChatHandler)
{
    if (AChatHandler && NC_Enable)
    {
        WorldSession* xPlayerSession = AChatHandler->GetSession();
        if (xPlayerSession)
        {
            Player* xPlayer = xPlayerSession->GetPlayer();
            if (xPlayer)
            {
                if (xPlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->GetFlagNoClipEnabled())
                {
                    if (xPlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->SetThreadHandlerState(2))
                    {
                        AChatHandler->SendSysMessage("NoClip is now disabled!");
                        return true;
                    }
                    return false;
                }
            }
        }
    }
    return false;
}

/* Class: NoClipPlayer */

void NoClipPlayer::OnLogin(Player* APlayer)
{
    if (APlayer && NC_Enable && NC_Announce_Enable)
    {
        WorldSession* xPlayerSession = APlayer->GetSession();
        if (xPlayerSession)
        {
            APlayer->CustomData.Set("NoClip", new CustomPlayerInformation(APlayer));
            ChatHandler(xPlayerSession).SendSysMessage("This server is running the |cff4CFF00NoClip |rmodule.");
        }
    }
}
void NoClipPlayer::OnBeforeLogout(Player* APlayer)
{
    if (APlayer && NC_Enable && APlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->GetFlagNoClipEnabled())
    {
        APlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->SetFlagNoClipEnabled(false);

        APlayer->SetCanFly(false);

        APlayer->CustomData.GetDefault<CustomPlayerInformation>("NoClip")->SetThreadHandlerState(2);
    }
}
