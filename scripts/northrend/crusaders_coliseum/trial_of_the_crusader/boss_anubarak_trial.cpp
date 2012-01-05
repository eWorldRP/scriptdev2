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
SDName: boss_anubarak_trial
SD%Complete: 70%
SDComment: by /dev/rsa
SDCategory:
EndScriptData */

// Anubarak - underground phase partially not worked, timers need correct
// Burrower - underground phase not implemented, buff not worked.
// Leecheng Swarm spell not worked - awaiting core support
// Anubarak spike aura worked only after 9750

#include "precompiled.h"
#include "trial_of_the_crusader.h"

enum Summons
{
    MAX_ORBS             = 6,
    NPC_FROST_SPHERE     = 34606,
    NPC_BURROWER         = 34607,
    NPC_SCARAB           = 34605,
    NPC_SPIKE            = 34660,
};

enum BossSpells
{
    SPELL_COLD              = 66013,
    SPELL_MARK              = 67574,
    SPELL_LEECHING_SWARM    = 66118,
    SPELL_LEECHING_HEAL     = 66125,
    SPELL_LEECHING_DAMAGE   = 66240,
    SPELL_SPIKE_CALL        = 66169,
    SPELL_POUND             = 66012,
    SPELL_SHOUT             = 67730,
    SPELL_SUBMERGE_0        = 53421,
    SPELL_SUBMERGE_1        = 67322,
    SPELL_SUMMON_BEATLES    = 66339,
    SPELL_BERSERK           = 26662,

    // Scarabs' Spell
    SPELL_DETERMINATION     = 66092,
    SPELL_ACID_MANDIBLE     = 67861,

    //Burrowers' Spells
    SPELL_SPIDER_FRENZY     = 66129,
    SPELL_SHADOW_STRIKE     = 66134,
    SPELL_EXPOSE_WEAKNESS   = 67847,

    // Spikes' Spell
    SPELL_PURSUING_SPIKES_0  = 65920,
    SPELL_PURSUING_SPIKES_1  = 65922,
    SPELL_PURSUING_SPIKES_2  = 65923,


    SUMMON_SCARAB           = NPC_SCARAB,
    SUMMON_BORROWER         = NPC_BURROWER,

    // Permafrost
    SPELL_PERMAFROST_N10    = 66193,
    SPELL_PERMAFROST_N25    = 67855,
    SPELL_PERMAFROST_H10    = 67856,
    SPELL_PERMAFROST_H25    = 67857,
    SPELL_PERMAFROST_VISUAL = 65882,
};

static Locations OrbLoc[]=
{
    {686.162720f,   137.699341f,    142.140900f},
    {716.846436f,   101.386314f,    142.120087f},
    {755.520447f,   99.565300f,     142.126083f},
    {801.836440f,   132.908325f,    142.162261f},
    {763.514221f,   172.092712f,    142.120148f},
    {714.347595f,   168.373795f,    142.207657f}
};

bool HasPermafrostAura(Unit* pWho)
{
    if(pWho->HasAura(SPELL_PERMAFROST_N25)
    || pWho->HasAura(SPELL_PERMAFROST_H10)
    || pWho->HasAura(SPELL_PERMAFROST_H25)
    || pWho->HasAura(SPELL_PERMAFROST_N10))
        return true;

    return false;
}

void RemovePermafrostAura(Unit* pWho)
{
    pWho->RemoveAurasDueToSpell(SPELL_PERMAFROST_N25);
    pWho->RemoveAurasDueToSpell(SPELL_PERMAFROST_H10);
    pWho->RemoveAurasDueToSpell(SPELL_PERMAFROST_H25);
    pWho->RemoveAurasDueToSpell(SPELL_PERMAFROST_N10);
}

/*#####
# Anub'Arak
#####*/
struct MANGOS_DLL_DECL boss_anubarak_trialAI : public BSWScriptedAI
{
    boss_anubarak_trialAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    Creature* m_pSpike;
    uint32 m_uiBerserkTimer;
    uint32 m_uiBurrowTimer;
    uint32 m_uiBurrowerSummonTimer;
    uint32 m_uiSpikeCallTimer;
    uint8 m_uiStage;
    bool m_bSpikeCalled;
    bool m_bIntro;

    void Reset()
    {
        if(!m_pInstance)
            return;

        m_uiSpikeCallTimer = 3000;
        m_bSpikeCalled = false;
        m_uiBerserkTimer = 600000;
        m_uiBurrowTimer = 90000;
        m_uiBurrowerSummonTimer = 15000;
        m_uiStage = 0;
        m_bIntro = true;
        m_pSpike = NULL;
        m_creature->SetRespawnDelay(DAY);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }


    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(-1713563,m_creature);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_bIntro)
            return;

        DoScriptText(-1713554, m_creature);
        m_bIntro = false;
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ANUBARAK, FAIL);
//            m_creature->ForcedDespawn();
    }

    void SummonOrbs()
    {
        for (uint8 i = 0; i <= MAX_ORBS; i++)
            m_creature->SummonCreature(NPC_FROST_SPHERE, OrbLoc[i].x, OrbLoc[i].y, OrbLoc[i].z, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 15000);
    }

    void SummonBurrower()
    {
        float fOffset;

        switch (currentDifficulty)
        {
            case RAID_DIFFICULTY_25MAN_HEROIC:
                fOffset = urand(0, 17);
                DoSpawnCreature(NPC_BURROWER, (20 * cos(fOffset)), (20 * sin(fOffset)), 0.0f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                fOffset = urand(0, 17);
                DoSpawnCreature(NPC_BURROWER, (20 * cos(fOffset)), (20 * sin(fOffset)), 0.0f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
            case RAID_DIFFICULTY_10MAN_HEROIC:
            case RAID_DIFFICULTY_25MAN_NORMAL:
                fOffset = urand(0, 17);
                DoSpawnCreature(NPC_BURROWER, (20 * cos(fOffset)), (20 * sin(fOffset)), 0.0f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
            case RAID_DIFFICULTY_10MAN_NORMAL:
                fOffset = urand(0, 17);
                DoSpawnCreature(NPC_BURROWER, (20 * cos(fOffset)), (20 * sin(fOffset)), 0.0f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                break;
        }
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;
        DoScriptText(-1713564, m_creature);
        m_pInstance->SetData(TYPE_ANUBARAK, DONE);
    }

    void Aggro(Unit* pWho)
    {
        SummonOrbs();

        if (!m_bIntro)
            DoScriptText(-1713555, m_creature);

        m_creature->SetInCombatWithZone();
        m_pInstance->SetData(TYPE_ANUBARAK, IN_PROGRESS);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch(m_uiStage)
        {
            case 0:
                timedCast(SPELL_POUND, uiDiff);
                timedCast(SPELL_COLD, uiDiff);

                if (m_uiBurrowTimer < uiDiff)
                {
                    m_uiBurrowTimer = 60000;
                    m_uiStage = 1;
                }
                else m_uiBurrowTimer -= uiDiff;

                break;
            case 1:
                doCast(SPELL_SUBMERGE_0);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_uiStage = 2;
                DoScriptText(-1713557, m_creature);
                break;
            case 2:
                if (!m_bSpikeCalled)
                {
                    if (m_uiSpikeCallTimer < uiDiff)
                    {
                        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        {
//                          doCast(SPELL_SPIKE_CALL);
//                          This summon not supported in database. Temporary override.
                            if(m_pSpike = DoSpawnCreature(NPC_SPIKE, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000))
                            {
                                m_pSpike->SetInCombatWith(pTarget);
                                m_pSpike->AddThreat(pTarget, 100000.0f);
                                DoScriptText(-1713558, m_creature, pTarget);
                                doCast(SPELL_MARK, pTarget);
                                m_pSpike->GetMotionMaster()->MoveChase(pTarget);
                                m_bSpikeCalled = true;
                                m_uiSpikeCallTimer = 3000;
                            }
                        }
                    }
                    else m_uiSpikeCallTimer -= uiDiff;
                }
                if (timedQuery(SPELL_SUMMON_BEATLES, uiDiff))
                {
                    doCast(SPELL_SUMMON_BEATLES);
                    doCast(SUMMON_SCARAB);
                    DoScriptText(-1713560, m_creature);
                }

                if (m_uiBurrowTimer < uiDiff)
                {
                    m_uiBurrowTimer = 90000;
                    m_uiStage = 3;
                    m_bSpikeCalled = false;
                }
                else m_uiBurrowTimer -= uiDiff;

                break;
            case 3:
                m_uiStage = 0;
                DoScriptText(-1713559, m_creature);

                if (m_pSpike)
                {
                    m_pSpike->ForcedDespawn();
                    m_pSpike = NULL;
                }

                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                doRemove(SPELL_SUBMERGE_0, m_creature);
                break;
            case 4:
                doCast(SPELL_LEECHING_SWARM);
                DoScriptText(-1713561, m_creature);
                m_uiStage = 5;
                break;
            case 5:
                timedCast(SPELL_POUND, uiDiff);
                timedCast(SPELL_COLD, uiDiff);
                break;
        }

        if (m_uiBurrowerSummonTimer < uiDiff)
        {
            if (currentDifficulty >= RAID_DIFFICULTY_10MAN_HEROIC)
                SummonBurrower();
            else
            {
                if (m_uiStage != 5)
                {
                    SummonBurrower();
                }
            }
            DoScriptText(-1713556, m_creature);
            m_uiBurrowerSummonTimer = 65000;
        }
        else m_uiBurrowerSummonTimer -= uiDiff;

        if (m_uiBerserkTimer < uiDiff)
        {
            m_uiBerserkTimer = 600000;
            doCast(SPELL_BERSERK);
        }
        else m_uiBerserkTimer -= uiDiff;

        if (m_creature->GetHealthPercent() < 30.0f && m_uiStage == 0)
            m_uiStage = 4;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_anubarak_trial(Creature* pCreature)
{
    return new boss_anubarak_trialAI(pCreature);
}

/*#####
# Swarm Scarab
#####*/
struct MANGOS_DLL_DECL mob_swarm_scarabAI : public BSWScriptedAI
{
    mob_swarm_scarabAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset()
    {
        m_creature->SetInCombatWithZone();
        m_creature->SetRespawnDelay(DAY);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;
    }

    void JustDied(Unit* Killer)
    {
    }

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_ANUBARAK) != IN_PROGRESS)
            m_creature->ForcedDespawn();

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        timedCast(SPELL_DETERMINATION, uiDiff);

        timedCast(SPELL_ACID_MANDIBLE, uiDiff);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_swarm_scarab(Creature* pCreature)
{
    return new mob_swarm_scarabAI(pCreature);
};

/*#####
# Nerubian Borrower
#####*/
struct MANGOS_DLL_DECL mob_nerubian_borrowerAI : public BSWScriptedAI
{
    mob_nerubian_borrowerAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiStrikeTimer;
    uint32 m_uiRangeCheckTimer;
    uint8 m_uiAllies;
    bool m_bSubmerged;

    void Reset()
    {
        m_creature->SetInCombatWithZone();
        m_creature->SetRespawnDelay(DAY);
        m_bSubmerged = false;
        m_uiStrikeTimer = 35000;
        m_uiRangeCheckTimer = 1000;
        m_uiAllies = 0;
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;
    }

    void JustDied(Unit* Killer)
    {
    }

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_ANUBARAK) != IN_PROGRESS) 
            m_creature->ForcedDespawn();

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiRangeCheckTimer < uiDiff)
        {
            std::list<Creature*> pBurrowers;
            GetCreatureListWithEntryInGrid(pBurrowers, m_creature, NPC_BURROWER, 12.0f);
            uint8 uiAllies = 0;
            if(!pBurrowers.empty())
            {
                for (std::list<Creature*>::iterator iter = pBurrowers.begin(); iter != pBurrowers.end(); ++iter)
                {
                    if ((*iter)->GetObjectGuid() != m_creature->GetObjectGuid())
                        uiAllies++;
                }
            }

            if (uiAllies)
            {
                if (m_uiAllies != uiAllies)
                {
                    DoCast(m_creature, SPELL_SPIDER_FRENZY);

                    if (SpellAuraHolderPtr pFrenzy = m_creature->GetSpellAuraHolder(SPELL_SPIDER_FRENZY))
                    {
                        if (pFrenzy->GetStackAmount() < uiAllies)
                            pFrenzy->SetStackAmount(uiAllies);
                    }

                    m_uiAllies = uiAllies;
                }
            }

            m_uiRangeCheckTimer = 1000;
        }
        else m_uiRangeCheckTimer -= uiDiff;

        timedCast(SPELL_EXPOSE_WEAKNESS, uiDiff);

        if (!m_bSubmerged && m_creature->GetHealthPercent() < 20.0f && timedQuery(SPELL_SUBMERGE_1, uiDiff))
        {
            doCast(SPELL_SUBMERGE_1);
            m_bSubmerged = true;
            DoScriptText(-1713557, m_creature);
        }

        if (m_bSubmerged && m_creature->GetHealthPercent() > 50.0f)
        {
             doRemove(SPELL_SUBMERGE_1, m_creature);
             m_bSubmerged = false;
             DoScriptText(-1713559, m_creature);
         }

        // only hero ability and only if not-submerged
        if (!m_bSubmerged && (currentDifficulty >= RAID_DIFFICULTY_10MAN_HEROIC))
        {
            if (m_uiStrikeTimer < uiDiff)
            {
                m_uiStrikeTimer = 35000;
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                if (pTarget)
                    DoCast(pTarget, SPELL_SHADOW_STRIKE);
            }
            else m_uiStrikeTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_nerubian_borrower(Creature* pCreature)
{
    return new mob_nerubian_borrowerAI(pCreature);
};

/*#####
# Forst Sphere
#####*/
struct MANGOS_DLL_DECL mob_frost_sphereAI : public ScriptedAI
{
    mob_frost_sphereAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetSpeedRate(MOVE_RUN, 0.1f);
        m_creature->SetWalk(true);
        m_creature->GetMotionMaster()->MoveRandom();
    }

    void Aggro(Unit* pWho)
    {
        return;
    }

    void AttackStart(Unit* pWho)
    {
        return;
    }


    void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
    {
        if (uiDamage >= m_creature->GetHealth())
        {
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->SetHealth(m_creature->GetMaxHealth());
            DoCast(m_creature, SPELL_PERMAFROST_VISUAL);
            DoCast(m_creature, SPELL_PERMAFROST_N10, true);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_ANUBARAK) != IN_PROGRESS)
           m_creature->ForcedDespawn();
    }
};

CreatureAI* GetAI_mob_frost_sphere(Creature* pCreature)
{
    return new mob_frost_sphereAI(pCreature);
};

/*#####
# Spike
#####*/
struct MANGOS_DLL_DECL mob_anubarak_spikeAI : public ScriptedAI
{
    mob_anubarak_spikeAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_uiDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    Difficulty m_uiDifficulty;
    ScriptedInstance* m_pInstance;
    uint32 m_uiCheckTimer;
    uint32 m_uiAuraTimer;
    uint32 m_uiChangeTimer;
    bool m_bSpeed;
    bool m_bChange;

    void Reset()
    {
        m_uiChangeTimer = 5000;
        m_uiCheckTimer = 500;
        m_uiAuraTimer = 7000;
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetSpeedRate(MOVE_RUN, 1.5f);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_bSpeed = false;
        m_bChange = false;
    }

    void Aggro(Unit *pWho)
    {
        if (!m_pInstance)
            return;

        DoCast(m_creature, SPELL_PURSUING_SPIKES_0);
    }

    void ChangeTarget()
    {
        Unit* pVictim = m_creature->getVictim();
        Unit* pTemp = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);

        if (pVictim)
        {
            pVictim->RemoveAurasDueToSpell(SPELL_MARK);
            m_creature->AddThreat(pVictim, -100000.0f);
        }

        m_creature->AddThreat(pTemp, 100000.0f);
        DoScriptText(-1713558, m_creature, pTemp);
        m_creature->CastSpell(pTemp, SPELL_MARK, false);
        m_creature->GetMotionMaster()->MoveChase(pTemp);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_ANUBARAK) != IN_PROGRESS)
            m_creature->ForcedDespawn();

        if (m_bChange)
        {
            if (m_uiChangeTimer < uiDiff)
            {
                ChangeTarget();
                m_creature->RemoveAurasDueToSpell(SPELL_PURSUING_SPIKES_0);
                m_creature->RemoveAurasDueToSpell(SPELL_PURSUING_SPIKES_1);
                m_creature->RemoveAurasDueToSpell(SPELL_PURSUING_SPIKES_2);
                DoCast(m_creature, SPELL_PURSUING_SPIKES_0);
                m_uiChangeTimer = 5000;
                m_bChange = false;
                m_bSpeed = false;
            }
            else m_uiChangeTimer -= uiDiff;

            return;
        }

        if (m_uiAuraTimer < uiDiff)
        {
            if (!m_bSpeed)
            {
                DoCast(m_creature, SPELL_PURSUING_SPIKES_1);
                m_bSpeed = true;
            }
            else
                DoCast(m_creature, SPELL_PURSUING_SPIKES_2);

            m_uiAuraTimer = 7000;
        }
        else m_uiAuraTimer -= uiDiff;

        if (m_uiCheckTimer < uiDiff)
        {
            if (m_creature->getVictim() && m_creature->getVictim()->IsWithinDist(m_creature, 1.0))
            {
                m_bChange = true;
                return;
            }

            std::list<Creature*> lOrbs;
            GetCreatureListWithEntryInGrid(lOrbs, m_creature, NPC_FROST_SPHERE, 10.0f);
            if (!lOrbs.empty())
            {
                float fDist = 999.99f;
                Creature* pNearest = NULL;

                // get nearest dead Frost Sphere
                for (std::list<Creature*>::iterator iter = lOrbs.begin(); iter != lOrbs.end(); ++iter)
                {
                    if ((*iter) && !(*iter)->isAlive())
                    {
                        if ((*iter)->GetDistance(m_creature) <= fDist)
                        {
                            pNearest = (*iter);
                            fDist = pNearest->GetDistance(m_creature);
                        }
                    }
                }

                if (pNearest)
                {
                    if(m_uiDifficulty <= RAID_DIFFICULTY_25MAN_NORMAL)
                        pNearest->Respawn();

                    RemovePermafrostAura(m_creature);
                    ChangeTarget();
                }
            }
            m_uiCheckTimer = 500;
        }
        else m_uiCheckTimer -= uiDiff;
    }
};

CreatureAI* GetAI_mob_anubarak_spike(Creature* pCreature)
{
    return new mob_anubarak_spikeAI(pCreature);
};

void AddSC_boss_anubarak_trial()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_anubarak_trial";
    newscript->GetAI = &GetAI_boss_anubarak_trial;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_swarm_scarab";
    newscript->GetAI = &GetAI_mob_swarm_scarab;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_nerubian_borrower";
    newscript->GetAI = &GetAI_mob_nerubian_borrower;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_anubarak_spike";
    newscript->GetAI = &GetAI_mob_anubarak_spike;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_frost_sphere";
    newscript->GetAI = &GetAI_mob_frost_sphere;
    newscript->RegisterSelf();

}
