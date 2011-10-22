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
    NPC_FROST_SPHERE_10  = 34606,
    NPC_FROST_SPHERE_25  = 34649,
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
    SPELL_IMPALE            = 65920,
    SPELL_SPIKE_CALL        = 66169,
    SPELL_POUND             = 66012,
    SPELL_SHOUT             = 67730,
    SPELL_SUBMERGE_0        = 53421,
    SPELL_SUBMERGE_1        = 67322,
    SPELL_SUMMON_BEATLES    = 66339,
    SPELL_DETERMINATION     = 66092,
    SPELL_ACID_MANDIBLE     = 67861,
    SPELL_SPIDER_FRENZY     = 66129,
    SPELL_SHADOW_STRIKE     = 66134,
    SPELL_EXPOSE_WEAKNESS   = 67847,
    SUMMON_SCARAB           = NPC_SCARAB,
    SUMMON_BORROWER         = NPC_BURROWER,
    SPELL_BERSERK           = 26662,
    SPELL_PERMAFROST        = 66193,
    SPELL_PERMAFROST_VISUAL = 65882,
};

bool HasPermafrostAura(Unit* pWho)
{
    if(pWho->HasAura(67855) 
    || pWho->HasAura(67856) 
    || pWho->HasAura(67857)
    || pWho->HasAura(66193))
        return true;

    return false;
}

void RemovePermafrostAura(Unit* pWho)
{
    pWho->RemoveAurasDueToSpell(67855);
    pWho->RemoveAurasDueToSpell(67856);
    pWho->RemoveAurasDueToSpell(67857);
    pWho->RemoveAurasDueToSpell(66193);
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
    uint32 m_uiBerserkTimer;
    uint32 m_uiBurrowTimer;
    uint32 m_uiBurrowerSummonTimer;
    uint8 m_uiStage;
    uint8 m_uiFrostSpheres;
    bool m_bIntro;
    Unit* m_pTarget;

    void Reset()
    {
        if(!m_pInstance)
            return;

        m_uiBerserkTimer = 600000;
        m_uiBurrowTimer = urand(45000, 60000);
        m_uiBurrowerSummonTimer = 90000;
        m_uiStage = 0;
        m_uiFrostSpheres = 0;
        m_bIntro = true;
        m_creature->SetRespawnDelay(DAY);
        m_pTarget = NULL;
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

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;
        DoScriptText(-1713564,m_creature);
        m_pInstance->SetData(TYPE_ANUBARAK, DONE);
    }

    void Aggro(Unit* pWho)
    {
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
                    m_uiBurrowTimer = urand(45000, 60000);
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
                if (timedQuery(SPELL_SPIKE_CALL, uiDiff))
                {
                    m_pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
//                         doCast(SPELL_SPIKE_CALL);
//                         This summon not supported in database. Temporary override.
                    Unit* pSpike = doSummon(NPC_SPIKE, TEMPSUMMON_TIMED_DESPAWN, 8000);
                    if (pSpike)
                    {
                        pSpike->AddThreat(m_pTarget, 100000.0f);
                        DoScriptText(-1713558, m_creature, m_pTarget);
                        doCast(SPELL_MARK, m_pTarget);
                        pSpike->GetMotionMaster()->MoveChase(m_pTarget);
                    }
                }
                if (timedQuery(SPELL_SUMMON_BEATLES, uiDiff))
                {
                    doCast(SPELL_SUMMON_BEATLES);
                    doCast(SUMMON_SCARAB);
                    DoScriptText(-1713560,m_creature);
                }

                if (m_uiBurrowTimer < uiDiff)
                {
                    m_uiBurrowTimer = urand(45000, 60000);
                    m_uiStage = 3;
                }
                else m_uiBurrowTimer -= uiDiff;

                break;
            case 3:
                m_uiStage = 0;
                DoScriptText(-1713559, m_creature);
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

        if (m_uiFrostSpheres <= 6)
        {
            timedCast(NPC_FROST_SPHERE_10, uiDiff);

            //if heroic increase de sphere count
            if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_25MAN_HEROIC)
                m_uiFrostSpheres++;
        }

        if (m_uiBurrowerSummonTimer < uiDiff)
        {
            if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_25MAN_HEROIC)
            {
                //two Burrower in each phase in hero mode
                m_uiBurrowerSummonTimer = 90000;
                doCast(SUMMON_BORROWER);
                doCast(SUMMON_BORROWER);
                DoScriptText(-1713556, m_creature);
            }
            else
            {
                if (m_uiStage != 4 || m_uiStage != 5)
                {
                    //one Burrower in the firsts phases in normal mode
                    m_uiBurrowerSummonTimer = 90000;
                    doCast(SUMMON_BORROWER);
                    DoScriptText(-1713556, m_creature);
                }
            }
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
    uint32 m_uiRangeCheckTimer;
    uint32 m_uiStrikeTimer;
    bool m_bSubmerged;

    void Reset()
    {
        m_creature->SetInCombatWithZone();
        m_creature->SetRespawnDelay(DAY);
        m_bSubmerged = false;
        m_uiRangeCheckTimer = 500;
        m_uiStrikeTimer = 35000;
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

        timedCast(SPELL_EXPOSE_WEAKNESS, uiDiff);

        if (m_uiRangeCheckTimer < uiDiff)
        {
            doRemove(SPELL_SPIDER_FRENZY);
            std::list<Creature*> pBurrowers;
            GetCreatureListWithEntryInGrid(pBurrowers, m_creature, NPC_BURROWER, 12.0f);
            if(!pBurrowers.empty())
            {
                for (std::list<Creature*>::iterator iter = pBurrowers.begin(); iter != pBurrowers.end(); ++iter)
                {
                    if ((*iter)->GetObjectGuid() != m_creature->GetObjectGuid())
                        doCast(SPELL_SPIDER_FRENZY);
                }
            }
            m_uiRangeCheckTimer = 500;
        }
        else m_uiRangeCheckTimer -= uiDiff;

        if (m_creature->GetHealthPercent() < 20.0f && timedQuery(SPELL_SUBMERGE_1, uiDiff) && !m_bSubmerged)
        {
            doCast(SPELL_SUBMERGE_1);
            m_bSubmerged = true;
            DoScriptText(-1713557, m_creature);
        }

        if (m_creature->GetHealthPercent() > 50.0f && m_bSubmerged)
        {
             doRemove(SPELL_SUBMERGE_1, m_creature);
             m_bSubmerged = false;
             DoScriptText(-1713559, m_creature);
         }

        // only hero ability and only if not-submerged
        if (!m_bSubmerged && (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_25MAN_HEROIC))
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
struct MANGOS_DLL_DECL mob_frost_sphereAI : public BSWScriptedAI
{
    mob_frost_sphereAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bMustDie;
    bool m_bCanCast;
    bool m_bFirstTime;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetSpeedRate(MOVE_RUN, 0.1f);
        m_creature->SetWalk(true);
        m_creature->GetMotionMaster()->MoveRandom();
        m_bCanCast = false;
        m_bFirstTime = true;
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
            uiDamage = 0;
            m_creature->setFaction(pDoneBy->getFaction());
            m_creature->GetMotionMaster()->MoveIdle();
            m_bCanCast = true;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_ANUBARAK) != IN_PROGRESS) 
           m_creature->ForcedDespawn();

        if (m_bCanCast && m_bFirstTime)
        {
            DoCast(m_creature, SPELL_PERMAFROST_VISUAL);
            uint32 PermafrostId = 0;
            if (currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL || currentDifficulty == RAID_DIFFICULTY_25MAN_NORMAL) 
                PermafrostId = 66193;
            else
                PermafrostId = 67856;
            DoCast(m_creature, PermafrostId, true);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_bFirstTime = false;
        }
    }
};

CreatureAI* GetAI_mob_frost_sphere(Creature* pCreature)
{
    return new mob_frost_sphereAI(pCreature);
};

/*#####
# Spike
#####*/
struct MANGOS_DLL_DECL mob_anubarak_spikeAI : public BSWScriptedAI
{
    mob_anubarak_spikeAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    Unit* defaultTarget;

    void Reset()
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetSpeedRate(MOVE_RUN, 0.5f);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        defaultTarget = NULL;
    }

    void Aggro(Unit *who)
    {
        if (!m_pInstance) return;
        doCast(SPELL_IMPALE);
        defaultTarget = who;
    }
    
    void DamageDeal(Unit* pDoneTo, uint32& uiDamage) 
    {
        if (HasPermafrostAura(pDoneTo))
        {
            uiDamage = 0;
            m_creature->ForcedDespawn();
            if(Creature* pTemp = GetClosestCreatureWithEntry(pDoneTo, NPC_FROST_SPHERE_10, 10.0f))
                pTemp->ForcedDespawn();
            else if(Creature* pTemp = GetClosestCreatureWithEntry(pDoneTo, NPC_FROST_SPHERE_25, 10.0f))
                pTemp->ForcedDespawn();
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_ANUBARAK) != IN_PROGRESS) 
            m_creature->ForcedDespawn();
        if (defaultTarget)
            if (!defaultTarget->isAlive() || !hasAura(SPELL_MARK,defaultTarget))
                 m_creature->ForcedDespawn();

        if(HasPermafrostAura(m_creature))
        {
            RemovePermafrostAura(m_creature);
        }
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
