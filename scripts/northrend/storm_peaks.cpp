/* Copyright (C) 2006 - 2012 ScriptDev2 <http://www.scriptdev2.com/>
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
SDName: Storm_Peaks
SD%Complete: 100
SDComment: Vendor Support (31247). Quest support: 12970, 12684
SDCategory: Storm Peaks
EndScriptData */

/* ContentData
npc_loklira_the_crone
npc_roxi_ramrocket
npc_frostborn_scout
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "Vehicle.h"

/*######
## npc_frostborn_scout
######*/

enum Scout
{
    QUEST_MISSING_SCOUT          = 12864,

    GOSSIP_TEXTID_SCOUT_1        = 13611,
    GOSSIP_TEXTID_SCOUT_2        = 13612,
    GOSSIP_TEXTID_SCOUT_3        = 13613,
    GOSSIP_TEXTID_SCOUT_4        = 13614

};

#define GOSSIP_ITEM_SCOUT_1     "Are you okay? I've come to take you back to Frosthold if you can stand."
#define GOSSIP_ITEM_SCOUT_2     "I'm sorry that I didn't get here sooner. What happened?"
#define GOSSIP_ITEM_SCOUT_3     "I'll go get some help. Hang in there."

bool GossipHello_npc_frostborn_scout(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_MISSING_SCOUT) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SCOUT_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SCOUT_1, pCreature->GetGUID());
        return true;
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_frostborn_scout(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SCOUT_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SCOUT_2, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SCOUT_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SCOUT_3, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SCOUT_4, pCreature->GetGUID());
            pPlayer->AreaExploredOrEventHappens(QUEST_MISSING_SCOUT);
            break;
    }
    return true;
}

/*######
## npc_loklira_the_crone
######*/

#define GOSSIP_ITEM_TELL_ME         "Tell me about this proposal."
#define GOSSIP_ITEM_WHAT_HAPPENED   "What happened then?"
#define GOSSIP_ITEM_YOU_WANT_ME     "You want me to take part in the Hyldsmeet to end the war?"
#define GOSSIP_ITEM_VERY_WELL       "Very well. I'll take part in this competition."

enum
{
    GOSSIP_TEXTID_LOKLIRA1    = 13777,
    GOSSIP_TEXTID_LOKLIRA2    = 13778,
    GOSSIP_TEXTID_LOKLIRA3    = 13779,
    GOSSIP_TEXTID_LOKLIRA4    = 13780,

    QUEST_THE_HYLDSMEET       = 12970,

    CREDIT_LOKLIRA            = 30467
};

bool GossipHello_npc_loklira_the_crone(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(QUEST_THE_HYLDSMEET) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TELL_ME, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOKLIRA1, pCreature->GetGUID());
        return true;
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_loklira_the_crone(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_WHAT_HAPPENED, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOKLIRA2, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_YOU_WANT_ME, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOKLIRA3, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_VERY_WELL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LOKLIRA4, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->TalkedToCreature(CREDIT_LOKLIRA, pCreature->GetGUID());
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}

/*######
## npc_thorim
######*/

#define GOSSIP_ITEM_THORIM1         "Can you tell me what became of Sif?"
#define GOSSIP_ITEM_THORIM2         "He did more than that, Thorim. He controls Ulduar now."
#define GOSSIP_ITEM_THORIM3         "It needn't end this way."

enum
{
    QUEST_SIBLING_RIVALRY           = 13064,

    SPELL_THORIM_STORY_KILL_CREDIT  = 56940,

    GOSSIP_TEXTID_THORIM1           = 13799,
    GOSSIP_TEXTID_THORIM2           = 13801,
    GOSSIP_TEXTID_THORIM3           = 13802,
    GOSSIP_TEXTID_THORIM4           = 13803
};

bool GossipHello_npc_thorim(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(QUEST_SIBLING_RIVALRY) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_THORIM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM1, pCreature->GetGUID());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_thorim(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_THORIM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM2, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_THORIM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM3, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_THORIM4, pCreature->GetGUID());
            pCreature->CastSpell(pPlayer, SPELL_THORIM_STORY_KILL_CREDIT, true);
            break;
    }

    return true;
}

/*######
## npc_roxi_ramrocket
######*/

#define GOSSIP_TEXT_RAMROCKET1  "How do you fly in this cold climate?"
#define GOSSIP_TEXT_RAMROCKET2  "I hear you sell motorcycle parts."

enum
{
    SPELL_MECHANO_HOG           = 60866,
    SPELL_MEKGINEER_CHOPPER     = 60867
};

bool GossipHello_npc_roxi_ramrocket(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isTrainer())
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_RAMROCKET1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

    if (pCreature->isVendor())
    {
        if (pPlayer->HasSpell(SPELL_MECHANO_HOG) || pPlayer->HasSpell(SPELL_MEKGINEER_CHOPPER))
        {
            if (pPlayer->HasSkill(SKILL_ENGINEERING) && pPlayer->GetBaseSkillValue(SKILL_ENGINEERING) >= 450)
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_RAMROCKET2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        }
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_roxi_ramrocket(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_TRAIN:
            pPlayer->SEND_TRAINERLIST(pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_TRADE:
            pPlayer->SEND_VENDORLIST(pCreature->GetGUID());
            break;
    }

    return true;
}

/*######
## npc_freed_protodrake
######*/

enum
{
    ENTRY_LIBERATED_BRUNNHILDAR = 29734,
    ENTRY_FREED_PROTODRAKE = 29709,
};

struct MANGOS_DLL_DECL npc_freed_protodrakeAI : public npc_escortAI
{
    npc_freed_protodrakeAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }

    bool m_bNotOnRoute;

    void Reset()
    {
        m_bNotOnRoute = false;
        if (m_creature->getFaction() != m_creature->GetCreatureInfo()->faction_A)
            m_creature->setFaction(m_creature->GetCreatureInfo()->faction_A);
    }

    void WaypointReached(uint32 uiPointId)
    {

        if (uiPointId == 5) //reached village, give credits
        {
            Unit* pPlayer = m_creature->GetVehicleKit()->GetPassenger(0);
            if (pPlayer && pPlayer->GetTypeId() == TYPEID_PLAYER)
            {
                for (uint8 i = 1; i < 4; ++i)
                    if (Unit* pPrisoner = m_creature->GetVehicleKit()->GetPassenger(i))
                    {
                        ((Player*)pPlayer)->KilledMonsterCredit(ENTRY_LIBERATED_BRUNNHILDAR);
                        pPrisoner->ExitVehicle();
                    }

                ((Player*)pPlayer)->KilledMonsterCredit(ENTRY_FREED_PROTODRAKE);
                pPlayer->ExitVehicle();
            }

            m_creature->SetVisibility(VISIBILITY_OFF);
            m_creature->ForcedDespawn(1000);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_bNotOnRoute && m_creature->GetPositionY() > -2595.0f)
            if (m_creature->isCharmed())
            {
                ((Player*)(m_creature->GetCharmer()))->SetClientControl(m_creature, 0);
                m_bNotOnRoute = true;

                //flight he don't accept so we walk o_O
                m_creature->SetSpeedRate(MOVE_WALK, 3.0f,true);

                m_creature->setFaction(35);

                Start(false, (Player*)(m_creature->GetCharmer()));
            }
        npc_escortAI::UpdateAI(uiDiff);
    }
};

CreatureAI* GetAI_npc_freed_protodrake(Creature* pCreature)
{
    return new npc_freed_protodrakeAI(pCreature);
}

/*######
## npc_brunnhildar_prisoner
######*/

enum
{
    SPELL_ICE_BLOCK = 54894,
    SPELL_ICE_SHARD = 55046,
    SPELL_ICE_SHARD_IMPACT = 55047
};

struct npc_brunnhildar_prisonerAI : public ScriptedAI
{
    npc_brunnhildar_prisonerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiCheckTimer;

    void Reset()
    {
        m_uiCheckTimer = 10000;
        m_creature->CastSpell(m_creature, SPELL_ICE_BLOCK, true);
    }

    void SpellHit(Unit *pCaster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_ICE_SHARD)
        {
            m_creature->CastSpell(m_creature, SPELL_ICE_SHARD_IMPACT, true);

            if (pCaster->IsVehicle())
            {
                m_creature->EnterVehicle(pCaster->GetVehicleKit());
                m_creature->RemoveAurasDueToSpell(SPELL_ICE_BLOCK);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (m_uiCheckTimer < diff)
        {
            if (!m_creature->hasUnitState(UNIT_STAT_ON_VEHICLE))
            {
                //return home if not passenger
                float x;
                float y;
                float z;

                m_creature->GetRespawnCoord(x, y, z);
                
                if(m_creature->GetDistance(x,y,z) > 20.0f)
                {
                    m_creature->SetDeathState(JUST_DIED);
                    m_creature->Respawn();
                }
            }
            m_uiCheckTimer = 10000;
        }
        else
            m_uiCheckTimer -= diff;
    }
};

CreatureAI* GetAI_npc_brunnhildar_prisoner(Creature* pCreature)
{
    return new npc_brunnhildar_prisonerAI(pCreature);
}

/*######
## npc_kirgaraak
######*/

enum
{
    QUEST_THE_WARM_UP       = 12996,
    NPC_KIRGARAAK_CREDIT    = 30221
};

struct MANGOS_DLL_DECL npc_kirgaraakAI : public ScriptedAI
{
    npc_kirgaraakAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    bool m_bKirgaraakBeaten;

    void Reset()
    {
        m_bKirgaraakBeaten =  false;
        m_creature->setFaction(m_creature->GetCreatureInfo()->faction_A);
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
    {
        if (uiDamage > m_creature->GetHealth() || (m_creature->GetHealth() - uiDamage)*100 / m_creature->GetMaxHealth() < 10)
        {
            if (Player* pPlayer = pDoneBy->GetCharmerOrOwnerPlayerOrPlayerItself())
            {
                if (!m_bKirgaraakBeaten && pPlayer->GetQuestStatus(QUEST_THE_WARM_UP) == QUEST_STATUS_INCOMPLETE)
                {
                    uiDamage = 0;
                    
                    m_creature->setFaction(35);
                    m_creature->CombatStop(true);
                    m_creature->RemoveAllAuras();
                    m_creature->DeleteThreatList();
                    pPlayer->KilledMonsterCredit(NPC_KIRGARAAK_CREDIT);
                    m_bKirgaraakBeaten = true;
                }
            }
        }
    }

    void MoveInLineOfSight(Unit* pWho) 
    {
        if (pWho->GetTypeId() == TYPEID_PLAYER)
        {
            if (pWho->hasUnitState(UNIT_STAT_ON_VEHICLE) && ((Player*)pWho)->GetQuestStatus(QUEST_THE_WARM_UP) == QUEST_STATUS_INCOMPLETE)
            {
                m_creature->setFaction(90);
            }
        }
    }
};

CreatureAI* GetAI_npc_kirgaraak(Creature* pCreature)
{
    return new npc_kirgaraakAI(pCreature);
}

/*#####
## npc_harnessed_icemaw_matriarch
#####*/

enum
{
    NPC_ASTRID                 = 29839,
    KC_ICEMAW_MATRIARCH        = 29563,
    QUEST_THE_LAST_OF_HER_KIND = 12983
};

struct MANGOS_DLL_DECL npc_harnessed_icemaw_matriarchAI : public ScriptedAI
{
    npc_harnessed_icemaw_matriarchAI(Creature*pCreature) : ScriptedAI(pCreature){}
    void Reset(){}
    void UpdateAI(const uint32 uiDiff){}

    void MoveInLineOfSight(Unit *pWho)
    {
        if (pWho->GetEntry() == NPC_ASTRID)
        {
            if (pWho->GetDistance2d(m_creature) < 30.0f)
            {
                if (Player *pPlayer = m_creature->GetMap()->GetPlayer(m_creature->GetCreatorGuid()))
                {
                    pPlayer->KilledMonsterCredit(KC_ICEMAW_MATRIARCH, m_creature->GetObjectGuid());
                    pPlayer->CompleteQuest(QUEST_THE_LAST_OF_HER_KIND); // hack: kill credit alone doesn't allow turning the quest in :/
                    //pPlayer->ExitVehicle();
                }
                m_creature->ForcedDespawn(1000);
            }
        }
    }
};

CreatureAI* GetAI_npc_harnessed_icemaw_matriarch(Creature* pCreature)
{
    return new npc_harnessed_icemaw_matriarchAI(pCreature);
}

/*#####
## npc_time_lost_proto_drake
#####*/

struct MANGOS_DLL_DECL npc_time_lost_proto_drakeAI : public ScriptedAI
{
    npc_time_lost_proto_drakeAI(Creature*pCreature) : ScriptedAI(pCreature)
    {
        m_bWasVisible = false;
        Reset();
    }

    uint32 m_uiTime2Spawn;
    bool m_bNeedInvisible;
    bool m_bWasVisible;

    void Reset()
    {
        if (m_bWasVisible)
            return;

        //set appearance  between 6h and 6h 30m
        m_uiTime2Spawn = 21600000 + rand()%1800000;
        //set drake invisible and not selectable
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_bNeedInvisible = true;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_bNeedInvisible)
        {
            if (m_uiTime2Spawn <= uiDiff)
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->SetVisibility(VISIBILITY_ON);
                m_bNeedInvisible = false;
                m_bWasVisible = true;
            }
            else 
                m_uiTime2Spawn -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_time_lost_proto_drake(Creature* pCreature)
{
    return new npc_time_lost_proto_drakeAI(pCreature);
}

void AddSC_storm_peaks()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_frostborn_scout";
    newscript->pGossipHello = &GossipHello_npc_frostborn_scout;
    newscript->pGossipSelect = &GossipSelect_npc_frostborn_scout;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_loklira_the_crone";
    newscript->pGossipHello = &GossipHello_npc_loklira_the_crone;
    newscript->pGossipSelect = &GossipSelect_npc_loklira_the_crone;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_thorim";
    newscript->pGossipHello = &GossipHello_npc_thorim;
    newscript->pGossipSelect = &GossipSelect_npc_thorim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_roxi_ramrocket";
    newscript->pGossipHello = &GossipHello_npc_roxi_ramrocket;
    newscript->pGossipSelect = &GossipSelect_npc_roxi_ramrocket;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_freed_protodrake";
    newscript->GetAI = &GetAI_npc_freed_protodrake;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_brunnhildar_prisoner";
    newscript->GetAI = &GetAI_npc_brunnhildar_prisoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_kirgaraak";
    newscript->GetAI = &GetAI_npc_kirgaraak;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_harnessed_icemaw_matriarch";
    newscript->GetAI = &GetAI_npc_harnessed_icemaw_matriarch;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_time_lost_proto_drake";
    newscript->GetAI = &GetAI_npc_time_lost_proto_drake;
    newscript->RegisterSelf();
}
