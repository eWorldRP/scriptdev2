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
SDName: boss_coren_direbrew
SD%Complete: 40
SDComment: Placeholder, Epona
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "blackrock_depths.h"
#include "escort_ai.h"

enum
{
        SAY_PAY                 = -1799998,
        SAY_AGGRO               = -1799997,

        SAY_IDLE1               = -1799996,
        SAY_IDLE2               = -1799995,
        SAY_IDLE3               = -1799994,
        SAY_IDLE4               = -1799993,

        SAY_ANTAGONIST1_1       = -1799992,
        SAY_ANTAGONIST1_2       = -1799991,
        SAY_ANTAGONIST1_3       = -1799990,
        SAY_ANTAGONIST2_1       = -1799989,
        SAY_ANTAGONIST2_2       = -1799988,
        SAY_ANTAGONIST2_3       = -1799987,
        SAY_ANTAGONIST4         = -1799986,

        // Coren 
        SPELL_DISARM            = 47310,

        // Ilsa
        ITEM_BREWMAIDEN_BREW    = 36748,
        SPELL_BREWMAIDEN_BREW   = 47371,
        SPELL_STUN              = 35856,

        // Ursula
        SPELL_BARRELED          = 47442,

        MOB_ANTAGONIST          = 23795,
        MOB_ILSA                = 26764,
        MOB_URSULA              = 26822,
        MOB_MINION              = 26776
};

float Coord[6][3] = 
{
    {902.65f, -131.26f, -49.74f}, // Antagonist1
    {902.85f, -134.45f, -49.74f}, // Antagonist2
    {904.72f, -137.24f, -49.75f}, // Antagonist3
    {905.53f, -128.06f, -49.74f}, // Coren noncombat event movement point1  id 1
    {909.47f, -137.38f, -49.74f}, // Coren noncombat event movement point2  id 2
    {898.79f, -140.91f, -49.75f}  // Ursula and Ilsa spawn point
};

/*#####
# Coren
#####*/
struct MANGOS_DLL_DECL boss_coren_direbrewAI : public ScriptedAI
{
    boss_coren_direbrewAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
       m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
       Reset();
    }

    ScriptedInstance* m_pInstance;

    bool m_bEventStarted;
    bool m_bAntagonistAttacked;
    bool m_bIlsaSpawned;
    bool m_bUrsulaSpawned;

    // Adds GUIDs
    ObjectGuid m_Antagonist1Guid;
    ObjectGuid m_Antagonist2Guid;
    ObjectGuid m_Antagonist3Guid;
    ObjectGuid m_UrsulaGuid;
    ObjectGuid m_IlsaGuid;

    Player* m_pInsulter;

    uint32 m_uiAggroYellTimer;
    uint32 m_uiWalkTimer;
    uint32 m_uiSpeachTimer;
    uint32 m_uiMinionTimer;
    uint32 m_uiDisarmTimer;

    uint8 m_uiSpeach;
    uint8 m_uiWalk;
    uint8 m_uiMinionSummoningPhase;
   
    void Reset() 
    {
        DespawnAdds();
        // Coren cannot be attacked before reaching the final position
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        m_creature->setFaction(35);

        m_bEventStarted = false;
        m_bAntagonistAttacked = false;
        m_bIlsaSpawned = false;
        m_bUrsulaSpawned = false;

        m_pInsulter = NULL;

        m_uiDisarmTimer = 12000;
        m_uiAggroYellTimer = 10000;
        m_uiWalkTimer = 0;
        m_uiSpeachTimer = 0;
        m_uiSpeach = 0;
        m_uiWalk = 0;
        m_uiMinionSummoningPhase = 0;

        m_uiMinionTimer = 10000;
    }
   
   void SaySay(ObjectGuid guid, int32 text)
   {
        Creature* pPointer = m_creature->GetMap()->GetCreature(guid);
        if (pPointer && pPointer->isAlive())
            DoScriptText(text, pPointer, NULL);
   }

   void Despawn(ObjectGuid guid)
   {
       if (Creature* pPointer = m_creature->GetMap()->GetCreature(guid))
            pPointer->ForcedDespawn();
   }

   void Assault(ObjectGuid guid, uint32 faction, Unit* pTarget)
   {
       Creature* pPointer = m_creature->GetMap()->GetCreature(guid);
       if (pTarget && pPointer)
       {
           pPointer->setFaction(faction);
           pPointer->AI()->AttackStart(pTarget);
       }
   }

 
    void DespawnAdds()
    {
        Despawn(m_UrsulaGuid);
        Despawn(m_IlsaGuid);
        Despawn(m_Antagonist1Guid);
        Despawn(m_Antagonist2Guid);
        Despawn(m_Antagonist3Guid);
        if (m_pInstance)
            if (GameObject* pMinionSummoner = m_pInstance->GetSingleGameObjectFromStorage(GO_MINIONSUMMONER))
                pMinionSummoner->SetGoState(GO_STATE_READY);
    }

    void JustDied(Unit* pKiller)
    {
        DespawnAdds();

        Map::PlayerList const &PlayerList = m_creature->GetMap()->GetPlayers();
        if (!PlayerList.isEmpty())
        {
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* pPlayer = i->getSource())
                {
                    if (pPlayer->GetQuestStatus(113021) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->KilledMonsterCredit(428569, 0);       
                }
            }
        }        
    }

    void UpdateAI(const uint32 uiDiff) 
    {
        if (!m_creature->isInCombat() || !m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            // After insulting coren a small noncombat event begins
            if (m_bEventStarted)
            {
                // Coren Walking in fornt of Antagonists
                if (m_uiWalkTimer < uiDiff)
                {
                    m_creature->SetSpeedRate(MOVE_WALK, 1.0f);
                    switch(m_uiWalk)
                    {
                        case 0:
                            m_creature->GetMotionMaster()->MovePoint(1, Coord[3][0], Coord[3][1], Coord[3][2]);
                            m_uiWalk = 1;
                            break;
                        case 1:
                            m_creature->GetMotionMaster()->MovePoint(2, Coord[4][0], Coord[4][1], Coord[4][2]);
                            m_uiWalk = 0;
                            break;
                    }
                    m_uiWalkTimer = 6000;
                }
                else m_uiWalkTimer -= uiDiff;

                // Coren Yell and Antagonists response
                if (m_uiSpeachTimer < uiDiff)
                {
                    switch(m_uiSpeach)
                    {
                    case 0:
                        DoScriptText(SAY_IDLE1, m_creature);
                        m_uiSpeach = 1;
                        m_uiSpeachTimer = 2000;
                        break;
                    case 1:
                        SaySay(m_Antagonist1Guid, SAY_ANTAGONIST1_1);
                        SaySay(m_Antagonist2Guid, SAY_ANTAGONIST1_2);
                        SaySay(m_Antagonist3Guid, SAY_ANTAGONIST1_3);
                        m_uiSpeach = 2;
                        m_uiSpeachTimer = 4000;
                        break;
                    case 2:
                        DoScriptText(SAY_IDLE2, m_creature);
                        m_uiSpeach = 3;
                        m_uiSpeachTimer = 2000;
                        break;
                    case 3:
                        SaySay(m_Antagonist1Guid, SAY_ANTAGONIST2_2);
                        SaySay(m_Antagonist2Guid, SAY_ANTAGONIST2_3);
                        SaySay(m_Antagonist3Guid, SAY_ANTAGONIST2_1);
                        m_uiSpeach = 4;
                        m_uiSpeachTimer = 4000;
                        break;
                    case 4:
                        DoScriptText(SAY_IDLE3, m_creature);
                        m_uiSpeach = 5;
                        m_uiSpeachTimer = 2000;
                        break;
                    case 5:
                        SaySay(m_Antagonist1Guid, SAY_ANTAGONIST1_1);
                        SaySay(m_Antagonist2Guid, SAY_ANTAGONIST1_2);
                        SaySay(m_Antagonist3Guid, SAY_ANTAGONIST1_3);
                        m_uiSpeach = 6;
                        m_uiSpeachTimer = 4000;
                        break;
                    case 6:
                        DoScriptText(SAY_IDLE4, m_creature);
                        m_uiSpeach = 7;
                        m_uiSpeachTimer = 2000;
                        break;
                    case 7:
                        SaySay(m_Antagonist1Guid, SAY_ANTAGONIST4);
                        SaySay(m_Antagonist2Guid, SAY_ANTAGONIST4);
                        SaySay(m_Antagonist3Guid, SAY_ANTAGONIST4);
                        DoScriptText(SAY_PAY, m_creature);
                        m_creature->setFaction(16);
                        m_creature->SetInCombatWithZone();
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        AttackStart(m_pInsulter);
                        break;
                    }
                }
                else m_uiSpeachTimer -= uiDiff;
            }
        }
        else
        {

            //Force Antagonist to assist Coren in combat
            if (!m_bAntagonistAttacked && m_creature->getVictim())
            {
                Assault(m_Antagonist1Guid, m_creature->getFaction(), m_creature->getVictim());
                Assault(m_Antagonist2Guid, m_creature->getFaction(), m_creature->getVictim());
                Assault(m_Antagonist3Guid, m_creature->getFaction(), m_creature->getVictim());
                m_bAntagonistAttacked = true;
            }

            if (!m_bIlsaSpawned &&  m_creature->GetHealthPercent() <= 66.0f)
            {
                Creature* pIlsa = m_creature->SummonCreature(MOB_ILSA, Coord[5][0], Coord[5][1], Coord[5][2], 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                if (pIlsa && m_creature->getVictim()) // i know if(victim) was checked at the top but once got crash with multithreaded mangos
                {
                    pIlsa->AI()->AttackStart(m_creature->getVictim());
                    m_IlsaGuid = pIlsa->GetObjectGuid();
                    m_bIlsaSpawned = true;
                }
            }

            if (!m_bUrsulaSpawned && m_creature->GetHealthPercent() <= 33.0f)
            {
                Creature* pUrsula = m_creature->SummonCreature(MOB_URSULA, Coord[5][0], Coord[5][1], Coord[5][2], 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                if (pUrsula && m_creature->getVictim())// i know if(victim) was checked at the top but once got crash with multithreaded mangos
                {
                    pUrsula->AI()->AttackStart(m_creature->getVictim());
                    m_UrsulaGuid = pUrsula->GetObjectGuid();
                    m_bUrsulaSpawned = true;
                }
            }

            if (m_uiMinionTimer < uiDiff)
            {
                if (m_pInstance)
                {
                    if (GameObject* pMinionSummoner = m_pInstance->GetSingleGameObjectFromStorage(GO_MINIONSUMMONER))
                    {
                        switch(m_uiMinionSummoningPhase)
                        {
                        case 0:
                            pMinionSummoner->SetGoState(GO_STATE_ACTIVE);
                            if (m_creature->getVictim())
                                if (Creature* pMinion = m_creature->SummonCreature(MOB_MINION, pMinionSummoner->GetPositionX(), pMinionSummoner->GetPositionY(), pMinionSummoner->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000))
                                {
                                    pMinion->setFaction(m_creature->getFaction());
                                    pMinion->AI()->AttackStart(m_creature->getVictim());
                                }
                            m_uiMinionTimer = 3000;
                            m_uiMinionSummoningPhase = 1;
                            break;
                        case 1:
                            pMinionSummoner->SetGoState(GO_STATE_READY);
                            m_uiMinionTimer = 20000;
                            m_uiMinionSummoningPhase = 0;
                            break;
                        }
                    }
                }
                else m_uiMinionTimer = 20000;
            }
            else m_uiMinionTimer -= uiDiff;

            if (m_uiAggroYellTimer < uiDiff)
            {
                DoScriptText(SAY_AGGRO, m_creature);
                m_uiAggroYellTimer = 45000 + (rand()%45000);
            }
            else m_uiAggroYellTimer -= uiDiff;

            if (m_uiDisarmTimer < uiDiff)
            {
                DoCast(m_creature->getVictim(), SPELL_DISARM);
                m_uiDisarmTimer = 12000;
            }
            else m_uiDisarmTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    }
};
CreatureAI* GetAI_boss_coren_direbrew(Creature* pCreature)
{
    return new boss_coren_direbrewAI(pCreature);
}

bool GossipSelect_boss_coren_direbrew(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{

    if (pPlayer)
    {
        if (boss_coren_direbrewAI* pCorenAI = dynamic_cast<boss_coren_direbrewAI*>(pCreature->AI()))
        {
            if( pCorenAI->m_creature == pCreature)
            {
                Creature* temp;
                temp = pCreature->SummonCreature(MOB_ANTAGONIST, Coord[0][0], Coord[0][1], Coord[0][2], 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (temp != NULL)
                {
                    pCorenAI->m_Antagonist1Guid = temp->GetObjectGuid();
                    temp = NULL;
                }
                temp = pCreature->SummonCreature(MOB_ANTAGONIST, Coord[1][0], Coord[1][1], Coord[1][2], 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (temp != NULL)
                {
                    pCorenAI->m_Antagonist2Guid = temp->GetObjectGuid();
                    temp = NULL;
                }
                temp = pCreature->SummonCreature(MOB_ANTAGONIST, Coord[2][0], Coord[2][1], Coord[2][2], 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (temp != NULL)
                {
                    pCorenAI->m_Antagonist3Guid = temp->GetObjectGuid();
                    temp = NULL;
                }
                pCorenAI->m_bEventStarted = true;
                pCorenAI->m_pInsulter = pPlayer;
            }
        }
    }
    return true;
}

/*#####
# Ilsa
#####*/
struct MANGOS_DLL_DECL mob_ilsaAI : public ScriptedAI
{
    mob_ilsaAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
       m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
       Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiLaunchTimer;
    uint32 m_uiCheckTimer;

    bool m_bCheck;

    Player* m_pBrewed;

    void Reset ()
    {
        m_uiLaunchTimer = 10000;

        m_bCheck = false;

        m_pBrewed = NULL;
    }

    void UpdateAI (const uint32 uiDiff)
    {
        if (m_uiLaunchTimer < uiDiff)
        {
            if(Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (pTarget->GetTypeId() == TYPEID_PLAYER)
                {
                    if (m_pBrewed = m_creature->GetMap()->GetPlayer(pTarget->GetObjectGuid()))
                    {
                        ItemPosCountVec dest;
                        uint8 msg = m_pBrewed->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_BREWMAIDEN_BREW, 1, NULL);
                        if (msg == EQUIP_ERR_OK)
                        {
                            m_pBrewed->StoreNewItem(dest, ITEM_BREWMAIDEN_BREW, 1, true);
                            m_bCheck = true;
                            m_uiCheckTimer = 6000;
                        }
                    }
                }
            }
            m_uiLaunchTimer = urand(8000, 10000);
        }
        else m_uiLaunchTimer -= uiDiff;

        if (m_bCheck)
        {
            if (m_uiCheckTimer < uiDiff)
            {
                m_bCheck = false;
                if (m_pBrewed->HasItemCount(ITEM_BREWMAIDEN_BREW, 1))
                    m_pBrewed->CastSpell(m_pBrewed, SPELL_STUN, false);
                m_pBrewed = NULL;
            }
            else m_uiCheckTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_ilsa(Creature* pCreature)
{
    return new mob_ilsaAI(pCreature);
}

/*#####
# Ursula
#####*/
struct MANGOS_DLL_DECL mob_ursulaAI : public ScriptedAI
{
    mob_ursulaAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
       m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
       Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiBarrelTimer;

    void Reset ()
    {
        m_uiBarrelTimer = 10000;
    }

    void UpdateAI (const uint32 uiDiff)
    {
        if (m_uiBarrelTimer < uiDiff)
        {
            if(Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                m_creature->CastSpell(pTarget, SPELL_BARRELED, false);
            }
            m_uiBarrelTimer = urand(8000, 10000);
        }
        else m_uiBarrelTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_ursula(Creature* pCreature)
{
    return new mob_ursulaAI(pCreature);
}
void AddSC_boss_coren_direbrew()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_coren_direbrew";
    newscript->GetAI = &GetAI_boss_coren_direbrew;
    newscript->pGossipSelect = &GossipSelect_boss_coren_direbrew;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ilsa";
    newscript->GetAI = &GetAI_mob_ilsa;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ursula";
    newscript->GetAI = &GetAI_mob_ursula;
    newscript->RegisterSelf();
}
