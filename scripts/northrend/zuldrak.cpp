/* Copyright (C) 2006 - 2011 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Zuldrak
SD%Complete: 100
SDComment: Quest support: 12663, 12664, 12934.
SDCategory: Zuldrak
EndScriptData */

/* ContentData
npc_gurgthock
npc_flying_fiend_vehicle
EndContentData */

#include "precompiled.h"
#include "Vehicle.h"

/*######
## npc_gurgthock
######*/

enum
{
    QUEST_FROM_BEYOND = 12934,

    NPC_AZBARIN       = 30026,
    NPC_DUKE_SINGEN   = 30019,
    NPC_ERATHIUS      = 30025,
    NPC_GARGORAL      = 30024
};

static float m_afSpawnLocation[] = {5768.71f, -2969.29f, 273.816f};
static uint32 m_auiBosses[] = {NPC_AZBARIN, NPC_DUKE_SINGEN, NPC_ERATHIUS, NPC_GARGORAL};

struct MANGOS_DLL_DECL npc_gurgthockAI : public ScriptedAI
{
    npc_gurgthockAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    ObjectGuid m_playerGuid;

    void SetPlayer(Player* pPlayer)
    {
        m_playerGuid = pPlayer->GetObjectGuid();
    }

    void Reset()
    {
        m_playerGuid.Clear();
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        uint32 uiEntry = pSummoned->GetEntry();
        for(uint8 i = 0; i < 4; ++i)
        {
            if (uiEntry == m_auiBosses[i])
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pPlayer->GroupEventHappens(QUEST_FROM_BEYOND, m_creature);

                m_playerGuid.Clear();
                return;
            }
        }
    }
};

bool QuestAccept_npc_gurgthock(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_FROM_BEYOND)
    {
        pCreature->SummonCreature(m_auiBosses[urand(0, 3)], m_afSpawnLocation[0], m_afSpawnLocation[1], m_afSpawnLocation[2], 0.0f, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 600000);

        if (npc_gurgthockAI* pGurthockAI = dynamic_cast<npc_gurgthockAI*>(pCreature->AI()))
            pGurthockAI->SetPlayer(pPlayer);
    }
    return true;
}

CreatureAI* GetAI_npc_gurgthock(Creature* pCreature)
{
    return new npc_gurgthockAI(pCreature);
}

/*######
## npc_ghoul_feeding_bunny
######*/
enum 
{
    NPC_DECAYING_GHOUL              = 28565,
    NPC_GHOUL_FEEDING_KC            = 28591,
    QUEST_FEEDIN_DA_GOOLZ           = 12652
 
};

struct MANGOS_DLL_DECL npc_ghoul_feeding_bunnyAI : public ScriptedAI
{
    npc_ghoul_feeding_bunnyAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 uiKaboomTimer;
    uint32 uiCheckTimer;

    void Reset() 
    {
        uiCheckTimer  = 1000;
        uiKaboomTimer = 11000;
    }

    void UpdateAI(const uint32 uiDiff)
     {
            if (uiCheckTimer <= uiDiff)
            {
                if(Creature *pGhoul = GetClosestCreatureWithEntry(m_creature, NPC_DECAYING_GHOUL, 20.0f))
                    {
                        if(pGhoul->isAlive())
                        {
                            pGhoul->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ());
                            

                        }    
                    }

            } else uiCheckTimer -= uiDiff;

            if( uiKaboomTimer <= uiDiff)
            {
                if(Player *pPlayer = m_creature->GetMap()->GetPlayer(m_creature->GetCreatorGuid()))
                {
                    if(pPlayer->GetQuestStatus(QUEST_FEEDIN_DA_GOOLZ) == QUEST_STATUS_INCOMPLETE)
                    {                        
                         pPlayer->KilledMonsterCredit(NPC_GHOUL_FEEDING_KC);
                    }
                }
                m_creature->ForcedDespawn();
                if(Creature *pGhoul = GetClosestCreatureWithEntry(m_creature, NPC_DECAYING_GHOUL, 10.0f))
                {
                    pGhoul->ForcedDespawn();
                }
            }
            else uiKaboomTimer -= uiDiff;
     }    
};

CreatureAI* GetAI_npc_ghoul_feeding_bunny(Creature* pCreature)
{
    return new npc_ghoul_feeding_bunnyAI(pCreature);
}

/*######
## npc_flying_fiend_vehicle
######*/

enum
{
    SAY_1               = -1999810,
    SAY_2               = -1999809,
    SAY_3               = -1999808,
    SAY_4               = -1999807,
    SAY_5               = -1999806,
    SAY_6               = -1999805,

    SPELL_KILL_CREDIT               = 52220,
    SPELL_SUMMON_GARGOYLE_VEHICLE   = 52194, // force cast 52190
    SPELL_SCOURGE_DISGUISE          = 52193, // force cast 52192
    SPELL_ABANDON_VEHICLE           = 52203,
    NPC_OVERLORD_DRAKURU            = 28717,
    MAX_NODE                        = 35
};

struct NodeData
{
    float fPosX;
    float fPosY;
    float fPosZ;
    int32 uiSay;
};

NodeData NodeInfo[MAX_NODE] =
{
    {6067.32f, -2091.65f, 435.74f},         // tarrace
    {6034.02f, -2108.75f, 439.35f},
    {5943.64f, -2194.36f, 395.83f, SAY_1},  // SAY_1
    {5389.98f, -2298.10f, 400.00f, SAY_2},  // SAY2
    {5338.34f, -2559.86f, 354.92f},
    {5572.49f, -3228.05f, 437.08f},
    {5749.56f, -3505.60f, 416.47f},         // Zim'Torga
    {5790.66f, -3675.21f, 407.28f},
    {6192.40f, -3871.92f, 502.15f},
    {6503.82f, -4182.26f, 491.74f},
    {6658.25f, -4282.02f, 496.58f},
    {6911.33f, -4325.27f, 557.03f, SAY_3},  // SAY3
    {7009.48f, -4345.52f, 549.51f},
    {7017.40f, -4407.66f, 539.99f},
    {6723.51f, -4696.85f, 558.85f},
    {6667.91f, -4693.66f, 543.37f, SAY_4},  // SAY4
    {6245.38f, -4051.44f, 512.56f},
    {6045.13f, -4067.68f, 468.06f},         // rynna
    {5592.85f, -3620.97f, 456.91f},
    {5399.42f, -3367.07f, 410.16f},
    {5212.14f, -3381.96f, 362.84f, SAY_5},  //SAY5
    {5032.75f, -3191.35f, 358.82f},
    {5016.12f, -3036.15f, 330.83f},
    {5367.02f, -2677.88f, 342.06f},
    {5430.66f, -2589.67f, 357.61f},         // The Argent Stand
    {5452.66f, -2287.13f, 357.07f},
    {5650.79f, -2150.54f, 299.41f},
    {5779.82f, -2095.38f, 276.80f},
    {6018.34f, -2082.39f, 278.90f},
    {6079.77f, -1922.40f, 369.19f, SAY_6},  // SAY6
    {5952.33f, -1899.84f, 485.97f},
    {5962.76f, -2083.12f, 469.24f},
    {6034.02f, -2108.75f, 439.35f},
    {6057.18f, -2100.97f, 429.08f},
    {6071.75f, -2087.93f, 426.97f}    
};

struct MANGOS_DLL_DECL npc_flying_fiend_vehicleAI : public ScriptedAI
{
    npc_flying_fiend_vehicleAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        Reset();
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetOwnerGuid(m_creature->GetCreatorGuid());
        m_creature->SetSpeedRate(MOVE_FLIGHT, 2.0f);
        m_creature->SetWalk(false);
        DoCastSpellIfCan(m_creature, SPELL_SCOURGE_DISGUISE);
        uiNode = 0;
        FlyToNextNode();
    }

    uint32 uiNode;

    void Reset(){}

    void FlyToNextNode()
    {
        m_creature->GetMotionMaster()->MovePoint(uiNode, NodeInfo[uiNode].fPosX, NodeInfo[uiNode].fPosY, NodeInfo[uiNode].fPosZ);
    }

    void MovementInform(uint32 uiType, uint32 uiPoint)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        if (uiPoint == uiNode)
        {
            Unit* pPassinger = m_creature->GetMap()->GetUnit(m_creature->GetCreatorGuid());
            if (!pPassinger)
            {
                m_creature->ForcedDespawn();
                return;
            }

            if (NodeInfo[uiNode].uiSay)
                if (Creature* pOverlord = GetClosestCreatureWithEntry(m_creature, NPC_OVERLORD_DRAKURU, DEFAULT_VISIBILITY_DISTANCE))
                    DoScriptText(NodeInfo[uiNode].uiSay, pOverlord, pPassinger);

            if (++uiNode < MAX_NODE)
                FlyToNextNode();
            else
            {
                pPassinger->RemoveAurasDueToSpell(52192);                
                DoCastSpellIfCan(m_creature, SPELL_ABANDON_VEHICLE,CAST_TRIGGERED);
                DoCastSpellIfCan(m_creature, SPELL_KILL_CREDIT, CAST_TRIGGERED);
                m_creature->ForcedDespawn(3000);
            }
        }
    }

};

CreatureAI* GetAI_npc_flying_fiend_vehicle(Creature* pCreature)
{
    return new npc_flying_fiend_vehicleAI(pCreature);
}

void AddSC_zuldrak()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_gurgthock";
    pNewScript->GetAI = &GetAI_npc_gurgthock;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_gurgthock;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_ghoul_feeding_bunny";
    pNewScript->GetAI = &GetAI_npc_ghoul_feeding_bunny;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_flying_fiend_vehicle";
    pNewScript->GetAI = &GetAI_npc_flying_fiend_vehicle;
    pNewScript->RegisterSelf();
}
