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
SDName: Boss_Svala
SD%Complete: 90%
SDComment: TODO: Call Flames need correct spell, some visual bugs, the way spells for intro works could use more research.
SDCategory: Utgarde Pinnacle
EndScriptData */

#include "precompiled.h"
#include "utgarde_pinnacle.h"

enum
{
    SAY_INTRO_1                 = -1575000,
    SAY_INTRO_2_ARTHAS          = -1575001,
    SAY_INTRO_3                 = -1575002,
    SAY_INTRO_4_ARTHAS          = -1575003,
    SAY_INTRO_5                 = -1575004,

    SAY_AGGRO                   = -1575005,
    SAY_SLAY_1                  = -1575006,
    SAY_SLAY_2                  = -1575007,
    SAY_SLAY_3                  = -1575008,
    SAY_SACRIFICE_1             = -1575009,
    SAY_SACRIFICE_2             = -1575010,
    SAY_SACRIFICE_3             = -1575011,
    SAY_SACRIFICE_4             = -1575012,
    SAY_SACRIFICE_5             = -1575013,
    SAY_DEATH                   = -1575014,

    NPC_SVALA_SORROW            = 26668,
    NPC_ARTHAS_IMAGE            = 29280,
    NPC_SPECTATOR               = 26667,
    NPC_RITUAL_TARGET           = 27327,
    NPC_FLAME_BRAZIER           = 27273,
    NPC_RITUAL_CHANNELER        = 27281,
    NPC_SCOURGE_HULK            = 26555,

    SPELL_ARTHAS_VISUAL         = 54134,

    // don't know how these should work in relation to each other
    SPELL_TRANSFORMING          = 54205,
    SPELL_TRANSFORMING_FLOATING = 54140,
    SPELL_TRANSFORMING_CHANNEL  = 54142,

    SPELL_RITUAL_OF_SWORD       = 48276,
    SPELL_RITUAL_STRIKE_TRIGGER = 48331, // triggers 48277 & 59930, needs NPC_RITUAL_TARGET as spell_script_target
    SPELL_RITUAL_STRIKE_EFF_1   = 48277,
    SPELL_RITUAL_STRIKE_EFF_2   = 59930,
    SPELL_RITUAL_CHANNELER_1    = 48271,
    SPELL_RITUAL_CHANNELER_2    = 48274,
    SPELL_RITUAL_CHANNELER_3    = 48275,

    SPELL_CALL_FLAMES           = 48258, // caster effect only, triggers event 17841
    SPELL_SINISTER_STRIKE       = 15667,
    SPELL_SINISTER_STRIKE_H     = 59409,

    // used by channelers
    SPELL_SHADOWS_IN_THE_DARK   = 59407,
    SPELL_PARALYZE              = 48278,

    ACHIEV_THE_INCREDIBLE_HULK  = 2043
};

/*######
## boss_svala
######*/

static const float spectatorWP[2][3] = 
{
    {267.8f,-346.7f,86.5f},
    {236.2f,-345.2f,84.3f}
};

struct MANGOS_DLL_DECL boss_svalaAI : public ScriptedAI
{
    boss_svalaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        m_bIsIntroDone = false;
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint64 m_uiArthasGUID;
    uint64 m_uiChanneler1GUID;
    uint64 m_uiChanneler2GUID;
    uint64 m_uiChanneler3GUID;
    uint64 m_uiRitualVictimGUID;

    bool m_bIsIntroDone;
    uint32 m_uiIntroTimer;
    uint32 m_uiIntroCount;
    uint32 m_uiSinisterStrikeTimer;
    uint8 m_uiRitualCount;
    uint8 m_uiRitualProgress;
    uint32 m_uiCallFlamesTimer;

    void Reset()
    {
        m_uiArthasGUID = 0;
        m_uiChanneler1GUID = 0;
        m_uiChanneler2GUID = 0;
        m_uiChanneler3GUID = 0;
        m_uiRitualVictimGUID = 0;

        m_uiIntroTimer = 2500;
        m_uiIntroCount = 0;
        m_uiSinisterStrikeTimer = 2000;
        m_uiRitualCount = 0;
        m_uiRitualProgress = 0;
        m_uiCallFlamesTimer = 5000;

        if (m_pInstance && m_pInstance->GetData(TYPE_SVALA) > IN_PROGRESS)
        {
            m_bIsIntroDone = true;

            if (m_creature->GetEntry() != NPC_SVALA_SORROW)
                m_creature->UpdateEntry(NPC_SVALA_SORROW);
            m_creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_UNK_2);
            m_creature->SetLevitate(true);

            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        else
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SVALA, FAIL);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_bIsIntroDone)
        {
            if (m_pInstance && m_pInstance->GetData(TYPE_SVALA) == IN_PROGRESS)
            {
                m_pInstance->SetData(TYPE_SVALA, SPECIAL);

                float fX, fY, fZ;
                m_creature->GetClosePoint(fX, fY, fZ, m_creature->GetObjectBoundingRadius(), 16.0f, 0.0f);

                // we assume m_creature is spawned in proper location
                m_creature->SummonCreature(NPC_ARTHAS_IMAGE, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);

                // spectators flee event
                std::list<Creature*> lspectatorList;
                GetCreatureListWithEntryInGrid(lspectatorList, m_creature, NPC_SPECTATOR, 100.0f);
                for(std::list<Creature*>::iterator itr = lspectatorList.begin(); itr != lspectatorList.end(); ++itr)
                {
                    if ((*itr)->isAlive())
                    {
                        (*itr)->SetWalk(false);
                        (*itr)->GetMotionMaster()->MovePoint(1,spectatorWP[0][0],spectatorWP[0][1],spectatorWP[0][2]);
                    }
                }
            }
            return;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void Aggro(Unit* pWho)
    {
        m_creature->SetLevitate(false);

        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_ARTHAS_IMAGE)
        {
            pSummoned->CastSpell(pSummoned, SPELL_ARTHAS_VISUAL, true);
            m_uiArthasGUID = pSummoned->GetGUID();
            pSummoned->SetFacingToObject(m_creature);
        }
    }

    void SummonedCreatureDespawn(Creature* pDespawned)
    {
        if (pDespawned->GetEntry() == NPC_ARTHAS_IMAGE)
            m_uiArthasGUID = 0;
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_TRANSFORMING)
        {
            if (Creature* pArthas = m_creature->GetMap()->GetCreature(m_uiArthasGUID))
                pArthas->InterruptNonMeleeSpells(true);

            m_creature->UpdateEntry(NPC_SVALA_SORROW);
            m_creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_UNK_2);
            m_creature->SetLevitate(true);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_SLAY_1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY_2, m_creature); break;
            case 2: DoScriptText(SAY_SLAY_3, m_creature); break;
        }

        if (!m_bIsRegularMode && pVictim->GetEntry() == NPC_SCOURGE_HULK && m_pInstance)
            m_pInstance->DoCompleteAchievement(ACHIEV_THE_INCREDIBLE_HULK);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_SVALA, DONE);
    }

    void DoMoveToPosition()
    {
        float fX, fZ, fY;
        m_creature->GetRespawnCoord(fX, fY, fZ);

        m_creature->SetLevitate(true);

        if (m_uiRitualProgress == 1)
            m_creature->GetMotionMaster()->MovePoint(1,fX,fY,fZ+8.f);
        else
        {
//          m_creature->MonsterMoveWithSpeed(fX, fY, fZ + 5.0f, m_uiIntroTimer);
            m_creature->MonsterMoveWithSpeed(fX, fY, fZ + 5.0f, 3);
            m_creature->GetMap()->CreatureRelocation(m_creature, fX, fY, fZ + 5.0f, m_creature->GetOrientation());
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (m_uiRitualProgress == 1 && uiMotionType == POINT_MOTION_TYPE && uiPointId == 1)
        {
            m_uiRitualProgress = 2;
        }
    }

    void SpellHitTarget (Unit* pUnit, const SpellEntry* pSpellEntry)
    { 
        if (pSpellEntry->Id == SPELL_RITUAL_STRIKE_EFF_1)
        {
            m_uiRitualProgress = 4;
            if (Unit* pVictim = m_creature->GetMap()->GetUnit(m_uiRitualVictimGUID))
            {
                pVictim->DealDamage(pVictim, pVictim->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                m_uiRitualVictimGUID = 0;
            }
            if (Creature* pChanneler1 = m_creature->GetMap()->GetCreature(m_uiChanneler1GUID))
                pChanneler1->ForcedDespawn();
            if (Creature* pChanneler2 = m_creature->GetMap()->GetCreature(m_uiChanneler2GUID))
                pChanneler2->ForcedDespawn();
            if (Creature* pChanneler3 = m_creature->GetMap()->GetCreature(m_uiChanneler3GUID))
                pChanneler3->ForcedDespawn();
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {

            if (m_bIsIntroDone)
                return;

            if (Creature* pArthas = m_creature->GetMap()->GetCreature(m_uiArthasGUID))
            {
                if (m_uiIntroTimer < uiDiff)
                {
                    m_uiIntroTimer = 10000;

                    switch(m_uiIntroCount)
                    {
                        case 0:
                            DoScriptText(SAY_INTRO_1, m_creature);
                            break;
                        case 1:
                            DoScriptText(SAY_INTRO_2_ARTHAS, pArthas);
                            break;
                        case 2:
                            pArthas->CastSpell(m_creature, SPELL_TRANSFORMING_CHANNEL, false);
                            m_creature->CastSpell(m_creature, SPELL_TRANSFORMING_FLOATING, false);
                            DoMoveToPosition();
                            break;
                        case 3:
                            m_creature->CastSpell(m_creature, SPELL_TRANSFORMING, false);
                            DoScriptText(SAY_INTRO_3, m_creature);
                            break;
                        case 4:
                            DoScriptText(SAY_INTRO_4_ARTHAS, pArthas);
                            break;
                        case 5:
                            DoScriptText(SAY_INTRO_5, m_creature);
                            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            m_bIsIntroDone = true;
                            break;
                    }

                    ++m_uiIntroCount;
                }
                else
                    m_uiIntroTimer -= uiDiff;
            }

            return;
        }

        if ((m_uiRitualCount == 0 && m_creature->GetHealthPercent() < 75.f) ||
            (m_uiRitualCount == 1 && m_creature->GetHealthPercent() < 50.f) ||
            (m_uiRitualCount == 2 && m_creature->GetHealthPercent() < 25.f))
        {
            ++m_uiRitualCount;
            m_uiRitualProgress = 1;
            DoMoveToPosition();

        }

        if (m_uiRitualProgress == 2)
        {
            m_creature->StopMoving();
            m_creature->GetMotionMaster()->MoveIdle();

            // crap to select a random player
            Map::PlayerList const &pPlayers = m_creature->GetMap()->GetPlayers();
            std::vector<Player*> list;
            list.clear();

            for(Map::PlayerList::const_iterator i = pPlayers.begin(); i != pPlayers.end(); ++i)
            if (Player* player = i->getSource())
            {
                if (!player->isAlive() || player->isGameMaster())
                    continue;

                list.push_back(player);
            }

            Player* pVictim;
            if (!list.empty())
            {
                pVictim = list[urand(0,list.size() - 1)];
            }

            if (!pVictim)
                return;

            // finally we have our victim..
            m_uiRitualVictimGUID = pVictim->GetGUID();

            // spawn ritual channelers
            m_creature->CastSpell(m_creature, SPELL_RITUAL_CHANNELER_1, true);
            m_creature->CastSpell(m_creature, SPELL_RITUAL_CHANNELER_2, true);
            m_creature->CastSpell(m_creature, SPELL_RITUAL_CHANNELER_3, true);

            // teleport selected player
            float fX,fY,fZ;
            m_creature->GetRespawnCoord(fX,fY,fZ);
            pVictim->NearTeleportTo(fX,fY,fZ,pVictim->GetOrientation());

            // cast ritual channelers spells
            int count = 0;
            std::list<Creature*> lChannelerList;
            GetCreatureListWithEntryInGrid(lChannelerList, m_creature, NPC_RITUAL_CHANNELER, 100.0f);
            for(std::list<Creature*>::iterator itr = lChannelerList.begin(); itr != lChannelerList.end(); ++itr)
            {
                if (!m_bIsRegularMode)
                    (*itr)->CastSpell((*itr),SPELL_SHADOWS_IN_THE_DARK, true);
                (*itr)->CastSpell(pVictim,SPELL_PARALYZE,true);

                switch(++count)
                {
                    case 1: m_uiChanneler1GUID = (*itr)->GetGUID(); break;
                    case 2: m_uiChanneler2GUID = (*itr)->GetGUID(); break;
                    case 3: m_uiChanneler3GUID = (*itr)->GetGUID(); break;
                }
            }

            m_creature->CastSpell(pVictim,SPELL_RITUAL_STRIKE_TRIGGER,true);
            m_uiRitualProgress = 3;
        }
        else if (m_uiRitualProgress == 3)
        {
            m_creature->StopMoving();
            m_creature->GetMotionMaster()->MoveIdle();

            bool allChannelersDead = true;

            std::list<Creature*> lChannelerList;
            GetCreatureListWithEntryInGrid(lChannelerList, m_creature, NPC_RITUAL_CHANNELER, 100.0f);
            if (!lChannelerList.empty())
                for (std::list<Creature*>::iterator itr = lChannelerList.begin(); itr != lChannelerList.end(); ++itr)
                {
                    if((*itr)->isAlive())
                    {
                        allChannelersDead = false; break;
                    }
                }

            if (allChannelersDead)
                m_uiRitualProgress = 4;

            return;
        } 
        else if (m_uiRitualProgress == 4)
        {
            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());    
            m_uiRitualProgress = 0;
        }

        if (m_uiSinisterStrikeTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),m_bIsRegularMode ? SPELL_SINISTER_STRIKE : SPELL_SINISTER_STRIKE_H);
            m_uiSinisterStrikeTimer = urand(4000,7000);
        }
        else
            m_uiSinisterStrikeTimer -= uiDiff;

        if (m_uiCallFlamesTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature,SPELL_CALL_FLAMES) == CAST_OK) // hack for spell damage.. spell that is really used is unknown to me
            {
                std::list<Creature*> lBrazierList;
                GetCreatureListWithEntryInGrid(lBrazierList, m_creature, NPC_FLAME_BRAZIER, 120.0f);
                if (!lBrazierList.empty())
                    for(std::list<Creature*>::iterator itr = lBrazierList.begin(); itr != lBrazierList.end(); ++itr)
                    {
                        uint32 dmg = urand(1900,2600); 
                        if (!m_bIsRegularMode)
                            dmg *= 2;
                        if (Unit* pVictim = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                            m_creature->DealDamage(pVictim,dmg,NULL,SPELL_DIRECT_DAMAGE,SPELL_SCHOOL_MASK_FIRE,NULL, false);
                    }
            }
            m_uiCallFlamesTimer = urand(5000,8000);
        }
        else
            m_uiCallFlamesTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_svala(Creature* pCreature)
{
    return new boss_svalaAI(pCreature);
}

bool AreaTrigger_at_svala_intro(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_SVALA) == NOT_STARTED)
            pInstance->SetData(TYPE_SVALA, IN_PROGRESS);
    }

    return false;
}

struct MANGOS_DLL_DECL npc_spectatorAI : public ScriptedAI
{
    npc_spectatorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    void Reset()
    {
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType == POINT_MOTION_TYPE)
        {
            if (uiPointId == 1)
                m_creature->GetMotionMaster()->MovePoint(2,spectatorWP[1][0],spectatorWP[1][1],spectatorWP[1][2]);
            else if (uiPointId == 2)
                m_creature->ForcedDespawn();
        }
    }
};

CreatureAI* GetAI_npc_spectator(Creature* pCreature)
{
    return new npc_spectatorAI(pCreature);
}

void AddSC_boss_svala()
{
    Script *pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_svala";
    pNewscript->GetAI = &GetAI_boss_svala;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_spectator";
    pNewscript->GetAI = &GetAI_npc_spectator;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "at_svala_intro";
    pNewscript->pAreaTrigger = &AreaTrigger_at_svala_intro;
    pNewscript->RegisterSelf();
}
