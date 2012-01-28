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
SDName: boss_grand_champions
SD%Complete: 70%
SDComment: missing yells. hunter AI sucks. no pvp diminuishing returns(is it DB related?). modified by /dev/rsa
SDCategory: Trial Of the Champion
EndScriptData */

#include "precompiled.h"
#include "trial_of_the_champion.h"

enum
{
    // Spells
    SPELL_BERSERK                       = 47008,

    // Warrior
    SPELL_MORTAL_STRIKE                 = 68783,
    SPELL_MORTAL_STRIKE_H               = 68784,
    SPELL_BLADESTORM                    = 63784,
    SPELL_INTERCEPT                     = 67540,
    SPELL_ROLLING_THROW                 = 47115, //need core support for spell 67546, using 47115 instead
    // Mage
    SPELL_FIREBALL                      = 66042,
    SPELL_FIREBALL_H                    = 68310,
    SPELL_BLAST_WAVE                    = 66044,
    SPELL_BLAST_WAVE_H                  = 68312,
    SPELL_HASTE                         = 66045,
    SPELL_POLYMORPH                     = 66043,
    SPELL_POLYMORPH_H                   = 68311,
    // Shaman
    SPELL_CHAIN_LIGHTNING               = 67529,
    SPELL_CHAIN_LIGHTNING_H             = 68319,
    SPELL_EARTH_SHIELD                  = 67530,
    SPELL_HEALING_WAVE                  = 67528,
    SPELL_HEALING_WAVE_H                = 68318,
    SPELL_HEX_OF_MENDING                = 67534,
    // Hunter
    SPELL_DISENGAGE                     = 68340,
    SPELL_LIGHTNING_ARROWS              = 66083,
    SPELL_MULTI_SHOT                    = 66081,
    SPELL_SHOOT                         = 66079,
    // Rogue
    SPELL_EVISCERATE                    = 67709,
    SPELL_EVISCERATE_H                  = 68317,
    SPELL_FAN_OF_KNIVES                 = 67706,
    SPELL_POISON_BOTTLE                 = 67701
};

struct MANGOS_DLL_DECL mob_toc5_warriorAI : public ScriptedAI
{
    mob_toc5_warriorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiMortalStrikeTimer;
    uint32 m_uiBladestormTimer;
    uint32 m_uiRollingThrowTimer;
    uint32 m_uiInterceptCooldown;
    uint32 m_uiInterceptTimer;
    uint32 m_uiBerserkTimer;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_uiMortalStrikeTimer = m_bIsRegularMode ? 9*IN_MILLISECONDS : 6*IN_MILLISECONDS;
        m_uiBladestormTimer = m_bIsRegularMode ? 30*IN_MILLISECONDS : 20*IN_MILLISECONDS;
        m_uiRollingThrowTimer = m_bIsRegularMode ? 45*IN_MILLISECONDS : 30*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        m_uiInterceptCooldown = 0;
        m_uiInterceptTimer = 1*IN_MILLISECONDS;
    }


    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(DATA_CHAMPIONS_COUNT, m_pInstance->GetData(DATA_CHAMPIONS_COUNT) - 1);
        m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

        if (m_pInstance->GetData(DATA_CHAMPIONS_COUNT) < 1)
        {
            m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, DONE);
            uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiMortalStrikeTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_MORTAL_STRIKE : SPELL_MORTAL_STRIKE_H);
            m_uiMortalStrikeTimer = m_bIsRegularMode ? 6*IN_MILLISECONDS : 4*IN_MILLISECONDS;
        }
        else
            m_uiMortalStrikeTimer -= uiDiff;  

        if (m_uiRollingThrowTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_ROLLING_THROW);
            m_uiRollingThrowTimer = m_bIsRegularMode ? 30*IN_MILLISECONDS : 15*IN_MILLISECONDS;
        }
        else
            m_uiRollingThrowTimer -= uiDiff;

        if (m_uiBladestormTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_BLADESTORM);
            m_uiBladestormTimer = m_bIsRegularMode ? 1*MINUTE*IN_MILLISECONDS : 20*IN_MILLISECONDS;
        }
        else
            m_uiBladestormTimer -= uiDiff;

        if (m_uiInterceptTimer < uiDiff)
        {
            if (!m_creature->IsWithinDistInMap(m_creature->getVictim(), 8) && m_creature->IsWithinDistInMap(m_creature->getVictim(), 25) && m_uiInterceptCooldown < uiDiff)
            {
                DoCast(m_creature->getVictim(), SPELL_INTERCEPT);
                m_uiInterceptCooldown = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
            }
            m_uiInterceptTimer = 1*IN_MILLISECONDS;
        }
        else 
        {
            m_uiInterceptTimer -= uiDiff;
            m_uiInterceptCooldown -= uiDiff;
        }

        if (m_uiBerserkTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        }
        else
            m_uiBerserkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_warrior(Creature* pCreature)
{
    return new mob_toc5_warriorAI(pCreature);
}

struct MANGOS_DLL_DECL mob_toc5_mageAI : public ScriptedAI
{
    mob_toc5_mageAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiFireballTimer;
    uint32 m_uiBlastWaveTimer;
    uint32 m_uiHasteTimer;
    uint32 m_uiPolymorphTimer;
    uint32 m_uiBerserkTimer;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_uiFireballTimer = 0;
        m_uiBlastWaveTimer = m_bIsRegularMode ? 20*IN_MILLISECONDS : 12*IN_MILLISECONDS;
        m_uiHasteTimer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 9*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        m_uiPolymorphTimer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 10*IN_MILLISECONDS;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(DATA_CHAMPIONS_COUNT, m_pInstance->GetData(DATA_CHAMPIONS_COUNT) - 1);
        m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

        if (m_pInstance->GetData(DATA_CHAMPIONS_COUNT) < 1)
        {
            m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, DONE);
            uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiFireballTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_FIREBALL : SPELL_FIREBALL_H);
            m_uiFireballTimer = m_bIsRegularMode ? 5*IN_MILLISECONDS : 3*IN_MILLISECONDS;
        }
        else
            m_uiFireballTimer -= uiDiff;

        if (m_uiBlastWaveTimer < uiDiff)
        {
            DoCast(m_creature, m_bIsRegularMode ? SPELL_BLAST_WAVE : SPELL_BLAST_WAVE_H);
            m_uiBlastWaveTimer = m_bIsRegularMode ? 20*IN_MILLISECONDS : 12*IN_MILLISECONDS;
        }
        else
            m_uiBlastWaveTimer -= uiDiff;

        if (m_uiHasteTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_HASTE);
            m_uiHasteTimer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        }
        else
            m_uiHasteTimer -= uiDiff;

        if (m_uiPolymorphTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(pTarget, m_bIsRegularMode ? SPELL_POLYMORPH : SPELL_POLYMORPH_H);
            m_uiPolymorphTimer = m_bIsRegularMode ? 20*IN_MILLISECONDS : 15*IN_MILLISECONDS;
        }
        else
            m_uiPolymorphTimer -= uiDiff;

        if (m_uiBerserkTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        }
        else  m_uiBerserkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_mage(Creature* pCreature)
{
    return new mob_toc5_mageAI(pCreature);
}

struct MANGOS_DLL_DECL mob_toc5_shamanAI : public ScriptedAI
{
    mob_toc5_shamanAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiChainLightningTimer;
    uint32 m_uiEarthShieldTimer;
    uint32 m_uiHealingWaveTimer;
    uint32 m_uiHexTimer;
    uint32 m_uiBerserkTimer;
    float m_fHealthFirstChampion;
    float m_fHealthSecondChampion;
    float m_fHealthThirdChampion;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_uiChainLightningTimer = m_bIsRegularMode ? 2*IN_MILLISECONDS : 1*IN_MILLISECONDS;
        m_uiEarthShieldTimer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 5*IN_MILLISECONDS;
        m_uiHealingWaveTimer = m_bIsRegularMode ? 20*IN_MILLISECONDS : 12*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        m_uiHexTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        m_fHealthFirstChampion = 0;
        m_fHealthSecondChampion = 0;
        m_fHealthThirdChampion = 0;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(DATA_CHAMPIONS_COUNT, m_pInstance->GetData(DATA_CHAMPIONS_COUNT) - 1);
        m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

        if (m_pInstance->GetData(DATA_CHAMPIONS_COUNT) < 1)
        {
            m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, DONE);
            uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiChainLightningTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_CHAIN_LIGHTNING : SPELL_CHAIN_LIGHTNING_H);
            m_uiChainLightningTimer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        }
        else
            m_uiChainLightningTimer -= uiDiff;  

        if (m_uiHexTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_HEX_OF_MENDING);
            m_uiHexTimer = m_bIsRegularMode ? 30*IN_MILLISECONDS : 20*IN_MILLISECONDS;
        }
        else
            m_uiHexTimer -= uiDiff;

        if (m_uiHealingWaveTimer < uiDiff)
        {
            uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                if (pTemp->isAlive())
                    m_fHealthFirstChampion = pTemp->GetHealth()*100 / pTemp->GetMaxHealth();
                else
                    m_fHealthFirstChampion = 100;

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                if (pTemp->isAlive())
                    m_fHealthSecondChampion = pTemp->GetHealth()*100 / pTemp->GetMaxHealth();
                else
                    m_fHealthSecondChampion = 100;
            
            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                if (pTemp->isAlive())
                    m_fHealthThirdChampion = pTemp->GetHealth()*100 / pTemp->GetMaxHealth();
                else
                    m_fHealthThirdChampion = 100;

            if (m_fHealthFirstChampion < m_fHealthSecondChampion && m_fHealthFirstChampion < m_fHealthThirdChampion && m_fHealthFirstChampion < 70)
            {
                uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
                if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                    DoCast(pTemp, m_bIsRegularMode ? SPELL_HEALING_WAVE : SPELL_HEALING_WAVE_H);
            }
            if (m_fHealthFirstChampion > m_fHealthSecondChampion && m_fHealthSecondChampion < m_fHealthThirdChampion && m_fHealthSecondChampion < 70)
            {
                uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
                if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                    DoCast(pTemp, m_bIsRegularMode ? SPELL_HEALING_WAVE : SPELL_HEALING_WAVE_H);
            }
            if (m_fHealthThirdChampion < m_fHealthSecondChampion && m_fHealthFirstChampion > m_fHealthThirdChampion && m_fHealthThirdChampion < 70)
            {
                uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
                if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                    DoCast(pTemp, m_bIsRegularMode ? SPELL_HEALING_WAVE : SPELL_HEALING_WAVE_H);
            }

            m_uiHealingWaveTimer = m_bIsRegularMode ? 8*IN_MILLISECONDS : 6*IN_MILLISECONDS;
        }
        else
            m_uiHealingWaveTimer -= uiDiff;

        if (m_uiEarthShieldTimer < uiDiff)
        {
            uint32 uiOtherId;
            switch(urand(0, 2))
            {
                case 0:
                    uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
                    if (Creature* pTemp =  m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                        if (pTemp->isAlive())
                            DoCast(pTemp, SPELL_EARTH_SHIELD);
                        else
                            DoCast(m_creature, SPELL_EARTH_SHIELD);
                    break;
                case 1:
                    uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
                    if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                        if (pTemp->isAlive())
                            DoCast(pTemp, SPELL_EARTH_SHIELD);
                        else
                            DoCast(m_creature, SPELL_EARTH_SHIELD);
                    break;
                case 2:
                    uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
                    if (Creature* pTemp =  m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                        if (pTemp->isAlive())
                            DoCast(pTemp, SPELL_EARTH_SHIELD);
                        else
                            DoCast(m_creature, SPELL_EARTH_SHIELD);
                    break;
            }
            m_uiEarthShieldTimer = m_bIsRegularMode ? 35*IN_MILLISECONDS : 25*IN_MILLISECONDS;
        }
        else
            m_uiEarthShieldTimer -= uiDiff;

        if (m_uiBerserkTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        }
        else
            m_uiBerserkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_shaman(Creature* pCreature)
{
    return new mob_toc5_shamanAI(pCreature);
}

struct MANGOS_DLL_DECL mob_toc5_hunterAI : public ScriptedAI
{
    mob_toc5_hunterAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiShootTimer;
    uint32 m_uiLightningArrowsTimer;
    uint32 m_uiMultiShotTimer;
    uint32 m_uiDisengageCooldownTimer;
    uint32 m_uiEnemyCheckTimer;
    uint32 m_uiDisengageCheckTimer;
    uint32 m_uiBerserkTimer;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_uiShootTimer = 0;
        m_uiLightningArrowsTimer = m_bIsRegularMode ? 18*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        m_uiMultiShotTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        m_uiDisengageCooldownTimer = 0;
        m_uiEnemyCheckTimer = 1*IN_MILLISECONDS;
        m_uiDisengageCheckTimer = 0;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(DATA_CHAMPIONS_COUNT, m_pInstance->GetData(DATA_CHAMPIONS_COUNT) - 1);
        m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

        if (m_pInstance->GetData(DATA_CHAMPIONS_COUNT) < 1)
        {
            m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, DONE);
            uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiEnemyCheckTimer < uiDiff)
        {
            if (!m_creature->IsWithinDistInMap(m_creature->getVictim(), 8) && m_creature->IsWithinDistInMap(m_creature->getVictim(), 30))
            {
                m_creature->SetSpeedRate(MOVE_RUN, 0.0001);
            }
            else
            {
                m_creature->SetSpeedRate(MOVE_RUN, 1.2);
            }
            m_uiEnemyCheckTimer = 0.1*IN_MILLISECONDS;
        }
        else
            m_uiEnemyCheckTimer -= uiDiff;

        if (m_uiDisengageCooldownTimer > 0)
            m_uiDisengageCooldownTimer -= uiDiff;

        if (m_uiShootTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_SHOOT);
            m_uiShootTimer = m_bIsRegularMode ? 5*IN_MILLISECONDS : 3*IN_MILLISECONDS;
        }
        else
            m_uiShootTimer -= uiDiff;

        if (m_uiMultiShotTimer < uiDiff)
        {
            m_creature->CastStop(SPELL_SHOOT);
            DoCast(m_creature->getVictim(), SPELL_MULTI_SHOT);
            m_uiMultiShotTimer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 5*IN_MILLISECONDS;
        }
        else
            m_uiMultiShotTimer -= uiDiff;

        if (m_uiLightningArrowsTimer < uiDiff)
        {
            m_creature->CastStop(SPELL_SHOOT);
            DoCast(m_creature, SPELL_LIGHTNING_ARROWS);
            m_uiLightningArrowsTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        }
        else
            m_uiLightningArrowsTimer -= uiDiff;

        if (m_uiDisengageCheckTimer < uiDiff)
        {
            if (m_creature->IsWithinDistInMap(m_creature->getVictim(), 5) && m_uiDisengageCooldownTimer == 0)
            {
                DoCast(m_creature, SPELL_DISENGAGE);
                m_uiDisengageCooldownTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
            }
            m_uiDisengageCheckTimer = 1*IN_MILLISECONDS;
        }
        else
            m_uiDisengageCheckTimer -= uiDiff;

        if (m_uiBerserkTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        }
        else
            m_uiBerserkTimer -= uiDiff;


        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_hunter(Creature* pCreature)
{
    return new mob_toc5_hunterAI(pCreature);
}

struct MANGOS_DLL_DECL mob_toc5_rogueAI : public ScriptedAI
{
    mob_toc5_rogueAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiEviscerateTimer;
    uint32 m_uiFanOfKnivesTimer;
    uint32 m_uiPoisonTimer;
    uint32 m_uiBerserkTimer;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_uiEviscerateTimer = m_bIsRegularMode ? 20*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        m_uiFanOfKnivesTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        m_uiPoisonTimer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 5*IN_MILLISECONDS;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
        if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
            if (pTemp->isAlive())
                pTemp->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(DATA_CHAMPIONS_COUNT, m_pInstance->GetData(DATA_CHAMPIONS_COUNT) - 1);
        m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

        if (m_pInstance->GetData(DATA_CHAMPIONS_COUNT) < 1)
        {
            m_pInstance->SetData(TYPE_GRAND_CHAMPIONS, DONE);
            uint32 uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_1);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_2);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            uiOtherId = m_pInstance->GetData(DATA_CHAMPIONID_3);
            if (Creature* pTemp = m_pInstance->GetSingleCreatureFromStorage(uiOtherId))
                pTemp->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiEviscerateTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_EVISCERATE : SPELL_EVISCERATE_H);
            m_uiEviscerateTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        }
        else
            m_uiEviscerateTimer -= uiDiff;  

        if (m_uiFanOfKnivesTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_FAN_OF_KNIVES);
            m_uiFanOfKnivesTimer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 7*IN_MILLISECONDS;
        }
        else
            m_uiFanOfKnivesTimer -= uiDiff;

        if (m_uiPoisonTimer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                DoCast(m_creature, SPELL_POISON_BOTTLE);
            m_uiPoisonTimer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 5*IN_MILLISECONDS;
        }
        else
            m_uiPoisonTimer -= uiDiff;

        if (m_uiBerserkTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        }
        else
            m_uiBerserkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_rogue(Creature* pCreature)
{
    return new mob_toc5_rogueAI(pCreature);
}

void AddSC_boss_grand_champions()
{
    Script* NewScript;

    NewScript = new Script;
    NewScript->Name = "mob_toc5_warrior";
    NewScript->GetAI = &GetAI_mob_toc5_warrior;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_toc5_mage";
    NewScript->GetAI = &GetAI_mob_toc5_mage;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_toc5_shaman";
    NewScript->GetAI = &GetAI_mob_toc5_shaman;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_toc5_hunter";
    NewScript->GetAI = &GetAI_mob_toc5_hunter;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_toc5_rogue";
    NewScript->GetAI = &GetAI_mob_toc5_rogue;
    NewScript->RegisterSelf();
}
