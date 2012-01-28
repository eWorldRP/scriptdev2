/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: boss_black_knight
SD%Complete: 70%
SDComment: missing yells. not sure about timers. modified by /dev/rsa
SDCategory: Trial Of the Champion
EndScriptData */

#include "precompiled.h"
#include "trial_of_the_champion.h"

enum
{
    // Spells
    SPELL_BERSERK                           = 47008,

    // Undead
    SPELL_PLAGUE_STRIKE                     = 67724,
    SPELL_PLAGUE_STRIKE_H                   = 67884,
    SPELL_ICY_TOUCH                         = 67718,
    SPELL_ICY_TOUCH_H                       = 67881,
    SPELL_OBLITERATE                        = 67725,
    SPELL_OBLITERATE_H                      = 67883,
    SPELL_CHOKE                             = 68306,
    // Skeleton
    SPELL_ARMY                              = 42650, //replacing original one, since that one spawns millions of ghouls!!
    // Ghost
    SPELL_DEATH                             = 67808,
    SPELL_DEATH_H                           = 67875,
    SPELL_MARK                              = 67823,
    // Risen ghoul
    SPELL_CLAW                              = 67879,
    SPELL_EXPLODE                           = 67729,
    SPELL_EXPLODE_H                         = 67886,
    SPELL_LEAP                              = 67749,
    SPELL_LEAP_H                            = 67880,

    // Others
    EQUIP_SWORD                             = 40343
};

struct MANGOS_DLL_DECL mob_toc5_risen_ghoulAI : public ScriptedAI
{
    mob_toc5_risen_ghoulAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiAttackTimer;

    void Reset()
    {
        m_uiAttackTimer = 2.5*IN_MILLISECONDS;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiAttackTimer < uiDiff)
        {
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(NPC_BLACK_KNIGHT))
                if (pTemp->isAlive())
                    if ((pTemp->GetHealth()*100 / pTemp->GetMaxHealth()) < 25)
                        DoCast(m_creature, m_bIsRegularMode ? SPELL_EXPLODE : SPELL_EXPLODE_H);

            if (m_creature->IsWithinDistInMap(m_creature->getVictim(), 4))
            {
                DoCast(m_creature->getVictim(), SPELL_CLAW);
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                    m_creature->AI()->AttackStart(pTarget);
                m_uiAttackTimer = 2.5*IN_MILLISECONDS;
            }
            else if (m_creature->IsWithinDistInMap(m_creature->getVictim(), 30))
            {
                DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_LEAP : SPELL_LEAP_H);
                m_uiAttackTimer = 2.5*IN_MILLISECONDS;
            }
        }
        else
            m_uiAttackTimer -= uiDiff;

        if ((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 25)
            DoCast(m_creature, m_bIsRegularMode ? SPELL_EXPLODE : SPELL_EXPLODE_H);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_risen_ghoul(Creature* pCreature)
{
    return new mob_toc5_risen_ghoulAI(pCreature);
}

struct MANGOS_DLL_DECL boss_black_knightAI : public ScriptedAI
{
    boss_black_knightAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    bool m_bGhoul;
    uint8 m_uiPhase;
    uint32 m_uiPlagueStrikeTimer;
    uint32 Icy_Touch_Timer;
    uint32 m_uiObliterateTimer;
    uint32 m_uiChokeTimer;
    uint32 m_uiDeathTimer;
    uint32 m_uiMarkTimer;
    uint32 Phase_Delay;
    uint32 m_uiSummonGhoulTimer;
    uint32 m_uiBerserk_Timer;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetDisplayId(29837);
        SetEquipmentSlots(false, EQUIP_SWORD, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_bGhoul = false;
        m_uiPhase = 1;
        m_uiPlagueStrikeTimer = m_bIsRegularMode ? 5*IN_MILLISECONDS : 4*IN_MILLISECONDS;
        Icy_Touch_Timer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 7*IN_MILLISECONDS;
        m_uiObliterateTimer = m_bIsRegularMode ? 16*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        m_uiChokeTimer = 15*IN_MILLISECONDS;
        m_uiSummonGhoulTimer = 4*IN_MILLISECONDS;
        m_uiBerserk_Timer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
    }


    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        if (m_pInstance->GetData(TYPE_BLACK_KNIGHT) != DONE)
            m_pInstance->SetData(TYPE_BLACK_KNIGHT, IN_PROGRESS);
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
    {
        if ((uiDamage > m_creature->GetHealth() || m_creature->GetHealth()/m_creature->GetHealth() <= 0.1 ) && m_uiPhase != 3)
        {
            uiDamage = 0;

            if (m_uiPhase == 2)
                StartPhase3();
            else if (m_uiPhase == 1)
                StartPhase2();
        }
    }


    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        if (m_uiPhase == 3)
            m_pInstance->SetData(TYPE_BLACK_KNIGHT, DONE);
    }

    void StartPhase2()
    {
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->SetDisplayId(27550);
        DoCast(m_creature, SPELL_ARMY);
        m_uiPhase = 2;
        m_uiPlagueStrikeTimer = m_bIsRegularMode ? 14*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        Icy_Touch_Timer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 7*IN_MILLISECONDS;
        m_uiObliterateTimer = m_bIsRegularMode ? 18*IN_MILLISECONDS : 10*IN_MILLISECONDS;
    }

    void StartPhase3()
    {
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->SetDisplayId(14560);
        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
        m_uiPhase = 3;
        m_uiDeathTimer = m_bIsRegularMode ? 5*IN_MILLISECONDS : 3*IN_MILLISECONDS;
        m_uiMarkTimer = m_bIsRegularMode ? 9*IN_MILLISECONDS : 7*IN_MILLISECONDS;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiPhase != 3)
        {
            if (m_uiPhase == 1)
            {
                if (m_uiChokeTimer < uiDiff)
                {               
                    DoCast(m_creature->getVictim(), SPELL_CHOKE);
                    m_uiChokeTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
                }
                else
                    m_uiChokeTimer -= uiDiff;

                if (m_uiSummonGhoulTimer < uiDiff && !m_bGhoul)
                {               
                    if (m_pInstance->GetData(DATA_TOC5_ANNOUNCER) == NPC_JAEREN)
                        m_creature->SummonCreature(NPC_RISEN_JAEREN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    else
                        m_creature->SummonCreature(NPC_RISEN_ARELAS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    m_bGhoul = true;
                }
                else
                    m_uiSummonGhoulTimer -= uiDiff;
            }

            if (m_uiPlagueStrikeTimer < uiDiff)
            {
                DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_PLAGUE_STRIKE : SPELL_PLAGUE_STRIKE_H);
                m_uiPlagueStrikeTimer = m_bIsRegularMode ? 10.5*IN_MILLISECONDS : 7*IN_MILLISECONDS;
            }
            else
                m_uiPlagueStrikeTimer -= uiDiff;  

            if (Icy_Touch_Timer < uiDiff)
            {
                DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_ICY_TOUCH : SPELL_ICY_TOUCH_H);
                Icy_Touch_Timer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 8*IN_MILLISECONDS;
            }
            else
                Icy_Touch_Timer -= uiDiff;

            if (m_uiObliterateTimer < uiDiff)
            {
                DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_OBLITERATE : SPELL_OBLITERATE_H);
                m_uiObliterateTimer = m_bIsRegularMode ? 11*IN_MILLISECONDS : 8*IN_MILLISECONDS;
            }
            else
                m_uiObliterateTimer -= uiDiff;
        }
        else
        {
            if (m_uiMarkTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                    DoCast(pTarget, SPELL_MARK);
                m_uiMarkTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
            }
            else
                m_uiMarkTimer -= uiDiff;

            if (m_uiDeathTimer < uiDiff)
            {
                DoCast(m_creature, m_bIsRegularMode ? SPELL_DEATH : SPELL_DEATH_H);
                m_uiDeathTimer = 3.5*IN_MILLISECONDS;
            }
            else
                m_uiDeathTimer -= uiDiff;
        }

        if (m_uiBerserk_Timer < uiDiff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            m_uiBerserk_Timer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        }
        else
            m_uiBerserk_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_black_knight(Creature* pCreature)
{
    return new boss_black_knightAI(pCreature);
}

void AddSC_boss_black_knight()
{
    Script* NewScript;

    NewScript = new Script;
    NewScript->Name = "mob_toc5_risen_ghoul";
    NewScript->GetAI = &GetAI_mob_toc5_risen_ghoul;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "boss_black_knight";
    NewScript->GetAI = &GetAI_boss_black_knight;
    NewScript->RegisterSelf();
}
