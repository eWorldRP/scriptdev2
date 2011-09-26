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
SDName: boss_argent_challenge
SD%Complete: 50%
SDComment: missing yells. radiance is "wrong". modified by /dev/rsa
SDCategory: Trial Of the Champion
EndScriptData */

#include "precompiled.h"
#include "trial_of_the_champion.h"

enum
{
    // Spells
    SPELL_BERSERK                           = 47008,

    // Eadric
    SPELL_VENGEANCE                         = 66889,
    SPELL_RADIANCE                          = 66862,
    SPELL_RADIANCE_H                        = 67681,
    SPELL_HAMMER_OF_JUSTICE                 = 66940,
    SPELL_HAMMER                            = 67680,
    // Paletress
    SPELL_SMITE                             = 66536,
    SPELL_SMITE_H                           = 67674,
    SPELL_HOLY_FIRE                         = 66538,
    SPELL_HOLY_FIRE_H                       = 67676,
    SPELL_RENEW                             = 66537,
    SPELL_RENEW_H                           = 67675,
    SPELL_HOLY_NOVA                         = 66546,
    SPELL_SHIELD                            = 66515,
    SPELL_CONFESS                           = 66547,
    // Memory
    SPELL_FEAR                              = 66552,
    SPELL_FEAR_H                            = 67677,
    SPELL_SHADOWS                           = 66619,
    SPELL_SHADOWS_H                         = 67678,
    SPELL_OLD_WOUNDS                        = 66620,
    SPELL_OLD_WOUNDS_H                      = 67679,
};

struct MANGOS_DLL_DECL boss_eadricAI : public ScriptedAI
{
    boss_eadricAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiVengeanceTimer;
    uint32 m_uiRadianceTimer;
    uint32 m_uiHammerTimer;
    uint32 m_uiHammerDamagTimer;
    uint32 m_uiBerserkTimer;
    ObjectGuid m_pHammerTargetGuid;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_uiVengeanceTimer = 1*IN_MILLISECONDS;
        m_uiRadianceTimer = m_bIsRegularMode ? 15*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        m_uiHammerTimer = m_bIsRegularMode ? 40*IN_MILLISECONDS : 10*IN_MILLISECONDS;
        m_uiHammerDamagTimer = m_bIsRegularMode ? 45*IN_MILLISECONDS : 20*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
        m_pHammerTargetGuid.Clear();
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        if (m_pInstance->GetData(TYPE_ARGENT_CHALLENGE) != DONE)
            m_pInstance->SetData(TYPE_ARGENT_CHALLENGE, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_ARGENT_CHALLENGE, DONE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiVengeanceTimer < uiDiff)
        {
            DoCast(m_creature, SPELL_VENGEANCE);
            m_uiVengeanceTimer = m_bIsRegularMode ? 12*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        }
        else
            m_uiVengeanceTimer -= uiDiff;  

        if (m_uiRadianceTimer < uiDiff)
        {
            DoCast(m_creature, m_bIsRegularMode ? SPELL_RADIANCE : SPELL_RADIANCE_H);
            m_uiRadianceTimer = m_bIsRegularMode ? 20*IN_MILLISECONDS : 12*IN_MILLISECONDS;
        }
        else
            m_uiRadianceTimer -= uiDiff;

        if (m_uiHammerTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                DoCast(pTarget, SPELL_HAMMER_OF_JUSTICE);
                m_pHammerTargetGuid = pTarget->GetObjectGuid();
            }
            m_uiHammerTimer = m_bIsRegularMode ? 40*IN_MILLISECONDS : 15*IN_MILLISECONDS;
        }
        else
            m_uiHammerTimer -= uiDiff;

        if (m_uiHammerDamagTimer < uiDiff)
        {
            if (Unit* pHammerTarget = m_creature->GetMap()->GetUnit(m_pHammerTargetGuid))
                DoCast(pHammerTarget, SPELL_HAMMER);
            m_uiHammerDamagTimer = m_bIsRegularMode ? 50*IN_MILLISECONDS : 15*IN_MILLISECONDS;
        }
        else
            m_uiHammerDamagTimer -= uiDiff;

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

CreatureAI* GetAI_boss_eadric(Creature* pCreature)
{
    return new boss_eadricAI(pCreature);
}

struct MANGOS_DLL_DECL boss_paletressAI : public ScriptedAI
{
    boss_paletressAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    bool m_bIsSummoned;
    bool m_bIsShielded;
    uint32 m_uiSmiteTimer;
    uint32 m_uiHolyFireTimer;
    uint32 m_uiRenewTimer;
    uint32 m_uiShieldDelay;
    uint32 m_uiShieldCheck;
    uint32 m_uiBerserkTimer;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->RemoveAurasDueToSpell(SPELL_SHIELD);
        m_creature->GetMotionMaster()->MovePoint(0, 746, 614, m_creature->GetPositionZ());
        m_creature->SetWalk(true);
        m_creature->setFaction(14);

        m_bIsSummoned = false;
        m_bIsShielded = false;
        m_uiSmiteTimer = 5*IN_MILLISECONDS;
        m_uiHolyFireTimer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 8*IN_MILLISECONDS;
        m_uiRenewTimer = m_bIsRegularMode ? 7*IN_MILLISECONDS : 5*IN_MILLISECONDS;
        m_uiShieldDelay = 0;
        m_uiShieldCheck = 1*IN_MILLISECONDS;
        m_uiBerserkTimer = m_bIsRegularMode ? 5*MINUTE*IN_MILLISECONDS : 3*MINUTE*IN_MILLISECONDS;
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
            pSummoned->AddThreat(pTarget);

        m_bIsSummoned = true;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        if (m_pInstance->GetData(TYPE_ARGENT_CHALLENGE) != DONE)
            m_pInstance->SetData(TYPE_ARGENT_CHALLENGE, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_ARGENT_CHALLENGE, DONE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiSmiteTimer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(target, m_bIsRegularMode ? SPELL_SMITE : SPELL_SMITE_H);
            m_uiSmiteTimer = 2*IN_MILLISECONDS;
        }
        else
            m_uiSmiteTimer -= uiDiff;  

        if (m_uiHolyFireTimer < uiDiff)
        {
            m_creature->CastStop(m_bIsRegularMode ? SPELL_SMITE : SPELL_SMITE_H);
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(target, m_bIsRegularMode ? SPELL_HOLY_FIRE : SPELL_HOLY_FIRE_H);
            m_uiHolyFireTimer = m_bIsRegularMode ? 10*IN_MILLISECONDS : 7*IN_MILLISECONDS;
        }
        else
            m_uiHolyFireTimer -= uiDiff;

        if (m_uiRenewTimer < uiDiff)
        {
            m_creature->CastStop(m_bIsRegularMode ? SPELL_SMITE : SPELL_SMITE_H);
            m_creature->CastStop(m_bIsRegularMode ? SPELL_HOLY_FIRE : SPELL_HOLY_FIRE_H);
            switch(urand(0, 1))
            {
                case 0:
                    if (Creature* pTemp = (m_creature->GetMap()->GetCreature(m_pInstance->GetData64(DATA_MEMORY))))
                        if (pTemp->isAlive())
                            DoCast(pTemp, m_bIsRegularMode ? SPELL_RENEW : SPELL_RENEW_H);
                        else
                            DoCast(m_creature, m_bIsRegularMode ? SPELL_RENEW : SPELL_RENEW_H);
                    break;
                case 1:
                    DoCast(m_creature, m_bIsRegularMode ? SPELL_RENEW : SPELL_RENEW_H);
                break;
            }
            m_uiRenewTimer = 25*IN_MILLISECONDS;
        }
        else
            m_uiRenewTimer -= uiDiff;

	if (((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 35 ) && !m_bIsSummoned )
	{
            m_creature->CastStop(m_bIsRegularMode ? SPELL_SMITE : SPELL_SMITE_H);
            m_creature->CastStop(m_bIsRegularMode ? SPELL_HOLY_FIRE : SPELL_HOLY_FIRE_H);
            DoCast(m_creature, SPELL_HOLY_NOVA);
            switch(urand(0, 24))
            {
                case 0:
                    m_creature->SummonCreature(MEMORY_ALGALON, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 1:
                    m_creature->SummonCreature(MEMORY_CHROMAGGUS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 2:
                    m_creature->SummonCreature(MEMORY_CYANIGOSA, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 3:
                    m_creature->SummonCreature(MEMORY_DELRISSA, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 4:
                    m_creature->SummonCreature(MEMORY_ECK, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                case 5:
                     m_creature->SummonCreature(MEMORY_ENTROPIUS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 6:
                     m_creature->SummonCreature(MEMORY_GRUUL, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 7:
                     m_creature->SummonCreature(MEMORY_HAKKAR, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 8:
                     m_creature->SummonCreature(MEMORY_HEIGAN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 9:
                     m_creature->SummonCreature(MEMORY_HEROD, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 10:
                     m_creature->SummonCreature(MEMORY_HOGGER, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 11:
                     m_creature->SummonCreature(MEMORY_IGNIS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 12:
                     m_creature->SummonCreature(MEMORY_ILLIDAN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 13:
                     m_creature->SummonCreature(MEMORY_INGVAR, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 14:
                     m_creature->SummonCreature(MEMORY_KALITHRESH, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 15:
                     m_creature->SummonCreature(MEMORY_LUCIFRON, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 16:
                     m_creature->SummonCreature(MEMORY_MALCHEZAAR, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 17:
                     m_creature->SummonCreature(MEMORY_MUTANUS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 18:
                     m_creature->SummonCreature(MEMORY_ONYXIA, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 19:
                     m_creature->SummonCreature(MEMORY_THUNDERAAN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 20:
                     m_creature->SummonCreature(MEMORY_VANCLEEF, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 21:
                     m_creature->SummonCreature(MEMORY_VASHJ, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 22:
                     m_creature->SummonCreature(MEMORY_VEKNILASH, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 23:
                     m_creature->SummonCreature(MEMORY_VEZAX, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
                case 24:
                     m_creature->SummonCreature(MEMORY_ARCHIMONDE, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                     break;
            }
            m_uiShieldDelay = 1*IN_MILLISECONDS;
        }

        if (m_uiShieldDelay < uiDiff && !m_bIsShielded && m_bIsSummoned)
        {
            m_creature->CastStop(m_bIsRegularMode ? SPELL_SMITE : SPELL_SMITE_H);
            m_creature->CastStop(m_bIsRegularMode ? SPELL_HOLY_FIRE : SPELL_HOLY_FIRE_H);
            DoCast(m_creature, SPELL_SHIELD);
            m_bIsShielded = true;
            m_uiShieldCheck = m_bIsRegularMode ? 3*IN_MILLISECONDS : 5*IN_MILLISECONDS;
        }
        else
            m_uiShieldDelay -= uiDiff;

        if (m_uiShieldCheck < uiDiff && m_bIsShielded)
        {
            if (Creature* pTemp = (m_creature->GetMap()->GetCreature(m_pInstance->GetData64(DATA_MEMORY))))
                if (!pTemp->isAlive())
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_SHIELD);
                    m_bIsShielded = false;
                }
                else
                    m_uiShieldCheck = 1*IN_MILLISECONDS;
        }
        else
            m_uiShieldCheck -= uiDiff;

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

CreatureAI* GetAI_boss_paletress(Creature* pCreature)
{
    return new boss_paletressAI(pCreature);
}

struct MANGOS_DLL_DECL mob_toc5_memoryAI : public ScriptedAI
{
    mob_toc5_memoryAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiOldWoundsTimer;
    uint32 m_uiShadowsTimer;
    uint32 m_uiFearTimer;

    void Reset()
    {
        m_creature->setFaction(14);

        m_uiOldWoundsTimer = 5*IN_MILLISECONDS;
        m_uiShadowsTimer = 8*IN_MILLISECONDS;
        m_uiFearTimer = 13*IN_MILLISECONDS;
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiOldWoundsTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(pTarget, m_bIsRegularMode ? SPELL_OLD_WOUNDS : SPELL_OLD_WOUNDS_H);
            m_uiOldWoundsTimer = 10*IN_MILLISECONDS;
        }
        else
            m_uiOldWoundsTimer -= uiDiff;  

        if (m_uiFearTimer < uiDiff)
        {
            DoCast(m_creature, m_bIsRegularMode ? SPELL_FEAR : SPELL_FEAR_H);
            m_uiFearTimer = 40*IN_MILLISECONDS;
        }
        else
            m_uiFearTimer -= uiDiff; 

        if (m_uiShadowsTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                DoCast(pTarget, m_bIsRegularMode ? SPELL_SHADOWS : SPELL_SHADOWS_H);
            m_uiShadowsTimer = 10*IN_MILLISECONDS;
        }
        else
            m_uiShadowsTimer -= uiDiff; 
		
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_toc5_memory(Creature* pCreature)
{
    return new mob_toc5_memoryAI(pCreature);
}

void AddSC_boss_argent_challenge()
{
    Script* NewScript;

    NewScript = new Script;
    NewScript->Name = "boss_eadric";
    NewScript->GetAI = &GetAI_boss_eadric;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "boss_paletress";
    NewScript->GetAI = &GetAI_boss_paletress;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_toc5_memory";
    NewScript->GetAI = &GetAI_mob_toc5_memory;
    NewScript->RegisterSelf();
}
