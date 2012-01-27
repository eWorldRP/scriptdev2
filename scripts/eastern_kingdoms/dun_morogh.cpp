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
SDName: Dun_Morogh
SD%Complete: 50
SDComment: Quest support: 1783
SDCategory: Dun Morogh
EndScriptData */

/* ContentData
npc_narm_faulk
npc_neill_ramstein        Brewfest event
npc_flynn_firebrew        Brewfest event
npc_pol_amberstill        Brewfest event
EndContentData */

#include "precompiled.h"

/*######
## npc_narm_faulk
######*/

#define SAY_HEAL    -1000187

struct MANGOS_DLL_DECL npc_narm_faulkAI : public ScriptedAI
{
    uint32 lifeTimer;
    bool spellHit;

    npc_narm_faulkAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset()
    {
        lifeTimer = 120000;
        m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
        spellHit = false;
    }

    void MoveInLineOfSight(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (m_creature->IsStandState())
        {
            if (lifeTimer < diff)
                m_creature->AI()->EnterEvadeMode();
            else
                lifeTimer -= diff;
        }
    }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if (Spellkind->Id == 8593 && !spellHit)
        {
            DoCastSpellIfCan(m_creature,32343);
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            //m_creature->RemoveAllAuras();
            DoScriptText(SAY_HEAL, m_creature, Hitter);
            spellHit = true;
        }
    }

};
CreatureAI* GetAI_npc_narm_faulk(Creature* pCreature)
{
    return new npc_narm_faulkAI(pCreature);
}

/*#####
# Neill Ramstein
#####*/

enum
{
    QUEST_BACK_AGAIN_A      = 11122,
    QUEST_BARK_FOR_BARLEY   = 11293,
    QUEST_BARK_FOR_THUNDER  = 11294,
    QUEST_NOW_RAM_RACING_A  = 11318,

    SPELL_RENTAL_RAM              = 43883,
    SPELL_RAM_FATIGUE             = 43052,
    SPELL_SPEED_RAM_GALLOP        = 42994,
    SPELL_SPEED_RAM_CANTER        = 42993,
    SPELL_SPEED_RAM_TROT          = 42992,
    SPELL_SPEED_RAM_NORMAL        = 43310,
    SPELL_SPEED_RAM_EXHAUSED      = 43332,
    SPELL_RENTAL_RAM_DND          = 42146
};

bool QuestAccept_npc_neill_ramstein(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    switch (pQuest->GetQuestId())
    {
        case QUEST_BACK_AGAIN_A:
        case QUEST_BARK_FOR_BARLEY:
        case QUEST_BARK_FOR_THUNDER:
        case QUEST_NOW_RAM_RACING_A:
            pPlayer->CastSpell(pPlayer, SPELL_RENTAL_RAM, false);
        break;
    }
    return true;
}

bool QuestRewarded_npc_neill_ramstein(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pPlayer->HasAura(SPELL_RENTAL_RAM))
    {
        pPlayer->RemoveAurasDueToSpell(SPELL_RENTAL_RAM);
        pPlayer->RemoveAurasDueToSpell(SPELL_RAM_FATIGUE);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_GALLOP);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_CANTER);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_TROT);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_NORMAL);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_EXHAUSED);
        pPlayer->RemoveAurasDueToSpell(SPELL_RENTAL_RAM_DND);
    }
    return true;
}

/*#####
# Flynn Firebrew
#####*/
enum
{
    ITEM_PORTABLE_KEG = 33797
};

struct MANGOS_DLL_DECL npc_flynn_firebrew : public ScriptedAI
{
    npc_flynn_firebrew (Creature* pCreature) : ScriptedAI (pCreature)
    {
        m_pMap = pCreature->GetMap();
        Reset();
    }

    Map* m_pMap;

    void Reset () {}

    void MoveInLineOfSight (Unit* pWho)
    {
        // player should be near the npc
        if (m_creature->GetDistance(pWho) > 10.0f)
            return;

        if (pWho->GetTypeId() != TYPEID_PLAYER)
            return;
        Player* pPlayer = m_pMap->GetPlayer(pWho->GetObjectGuid());
        if (!pPlayer)
            return;
        if (pPlayer->GetQuestStatus(QUEST_BACK_AGAIN_A) == QUEST_STATUS_INCOMPLETE)
        {
            // player can only have 1 keg
            if (pPlayer->HasItemCount(ITEM_PORTABLE_KEG, 1))
                return;

            ItemPosCountVec dest;
            uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_PORTABLE_KEG, 1, NULL);
            if (msg == EQUIP_ERR_OK)
            {
                pPlayer->StoreNewItem(dest, ITEM_PORTABLE_KEG, 1, true);
                pPlayer->GetItemByEntry(ITEM_PORTABLE_KEG)->SendCreateUpdateToPlayer(pPlayer);
            }
        }
    }
};

CreatureAI* GetAI_npc_flynn_firebrew(Creature* pCreature)
{
    return new npc_flynn_firebrew(pCreature);
}

/*#####
# Pol Amberstill
#####*/
enum
{
    NPC_CREDIT_CREATURE     = 24337,
};

struct MANGOS_DLL_DECL npc_pol_amberstill : public ScriptedAI
{
    npc_pol_amberstill (Creature* pCreature) : ScriptedAI (pCreature)
    {
        m_pMap = pCreature->GetMap();
        Reset();
    }

    Map* m_pMap;

    void Reset () {}

    void MoveInLineOfSight (Unit* pWho)
    {
        // player should be near the npc
        if (m_creature->GetDistance(pWho) > 10.0f)
            return;

        if (pWho->GetTypeId() != TYPEID_PLAYER)
            return;
        Player* pPlayer = m_pMap->GetPlayer(pWho->GetObjectGuid());
        if (!pPlayer)
            return;
        if (pPlayer->GetQuestStatus(QUEST_BACK_AGAIN_A) == QUEST_STATUS_INCOMPLETE)
        {
            // player can only have 1 keg
            if (pPlayer->HasItemCount(ITEM_PORTABLE_KEG, 1))
            {
                pPlayer->DestroyItemCount(ITEM_PORTABLE_KEG, 1, true);
                pPlayer->KilledMonsterCredit(NPC_CREDIT_CREATURE);
            }
        }
    }
};

CreatureAI* GetAI_npc_pol_amberstill(Creature* pCreature)
{
    return new npc_pol_amberstill(pCreature);
}

void AddSC_dun_morogh()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_narm_faulk";
    newscript->GetAI = &GetAI_npc_narm_faulk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_neill_ramstein";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_neill_ramstein;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_neill_ramstein;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_flynn_firebrew";
    newscript->GetAI = &GetAI_npc_flynn_firebrew;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_pol_amberstill";
    newscript->GetAI = &GetAI_npc_pol_amberstill;
    newscript->RegisterSelf();
}
