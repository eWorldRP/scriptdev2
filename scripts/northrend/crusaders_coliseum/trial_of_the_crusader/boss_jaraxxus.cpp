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
// Jaraxxus - Magic aura (from start?) not fully offlike implemented.
// Legion flame visual effect not imlemented

/* ScriptData
SDName: trial_of_the_crusader
SD%Complete: 80%
SDComment: by /dev/rsa
SDCategory: Crusader Coliseum
EndScriptData */

#include "precompiled.h"
#include "trial_of_the_crusader.h"
enum Equipment
{
    EQUIP_MAIN           = 47266,
    EQUIP_OFFHAND        = 46996,
    EQUIP_RANGED         = 47267,
    EQUIP_DONE           = EQUIP_NO_CHANGE,
};

enum Summons
{
    NPC_LEGION_FLAME     = 34784,
    NPC_INFERNAL_VOLCANO = 34813,
    NPC_FEL_INFERNAL     = 34815,
    NPC_NETHER_PORTAL    = 34825,
    NPC_MISTRESS         = 34826,
};

enum BossSpells
{
    SPELL_JARAXXUS_TOUCH    = 66209,
    SPELL_CURSE_OF_NETHER   = 66211,
    SPELL_NETHER_POWER      = 67108,
    SPELL_INFERNAL          = 66258,
    SPELL_INFERNAL_ERUPTION = 66255,
    SPELL_FEL_FIREBALL      = 66532,
    SPELL_FEL_LIGHTING      = 66528,
    SPELL_INCINERATE_FLESH  = 66237,
    SPELL_BURNING_INFERNO   = 66242,
    SPELL_NETHER_PORTAL     = 66264,
    SPELL_LEGION_FLAME_0    = 66877,
    SPELL_LEGION_FLAME_1    = 66197,
    SPELL_SHIVAN_SLASH      = 67098,
    SPELL_SPINNING_STRIKE   = 66316,
    SPELL_FEL_INFERNO       = 67047,
    SPELL_FEL_STREAK        = 66494,
    SPELL_MISTRESS_KISS     = 67078,
    SPELL_BERSERK           = 26662
};

/*######
## Lord Jaraxxus
######*/

struct MANGOS_DLL_DECL boss_jaraxxusAI : public BSWScriptedAI
{
    boss_jaraxxusAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiPowerTimer;
    uint32 m_uiSummonTimer;
    uint32 m_uiFizzlebangSecurityKillTimer;
    uint8 m_uiStackReply;
    bool m_bVolcanoOrPortal;
    bool m_bPowerCheck;

    void Reset()
    {
        if(!m_pInstance)
            return;
        m_pInstance->SetData(TYPE_JARAXXUS, NOT_STARTED);
//        SetEquipmentSlots(false, EQUIP_MAIN, EQUIP_OFFHAND, EQUIP_RANGED);
        m_bVolcanoOrPortal = urand(0,1);
        m_uiPowerTimer = 27000;
        m_uiSummonTimer = 60000;
        // this to avoid Fizzlebang to be saved by players
        m_uiFizzlebangSecurityKillTimer = 7000;
        m_bPowerCheck = false;

        if (currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL || currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC)
            m_uiStackReply = 4;
        else
            m_uiStackReply = 9;

        DoScriptText(-1713517,m_creature);
        m_creature->SetRespawnDelay(DAY);
    }

    void JustReachedHome()
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_JARAXXUS, FAIL);
        m_creature->ForcedDespawn();
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        DoScriptText(-1713525, m_creature);
        m_pInstance->SetData(TYPE_JARAXXUS, DONE);
        m_pInstance->SetData(TYPE_EVENT, 2000);
        m_pInstance->SetData(TYPE_STAGE, 0);
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        m_creature->SetInCombatWithZone();
        m_pInstance->SetData(TYPE_JARAXXUS, IN_PROGRESS);
        DoScriptText(-1713514, m_creature);
        doCast(SPELL_NETHER_POWER);
    }

    //hacky workaround for interrupt Fel Fireball (need core fix)
    void SpellHit(Unit* who, const SpellEntry* spell)
    {
        // don't self interrupt
        if (!who->IsControlledByPlayer())
            return;

        if (spell->EffectMechanic[0] == MECHANIC_INTERRUPT || spell->EffectMechanic[1] == MECHANIC_INTERRUPT || spell->EffectMechanic[2] == MECHANIC_INTERRUPT)
        {
            m_creature->InterruptNonMeleeSpells(false, SPELL_FEL_FIREBALL);
            m_creature->InterruptNonMeleeSpells(false, 66963); // 25 man
            m_creature->InterruptNonMeleeSpells(false, 66964); // 10 man hero
            m_creature->InterruptNonMeleeSpells(false, 66965); // 25 man hero
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_bPowerCheck)
        {
            if (Creature* pFizzle = m_pInstance->GetSingleCreatureFromStorage(NPC_FIZZLEBANG))
            {
                if (!pFizzle->isAlive())
                {
                    //little workaround to give initial stack amount of Nether Power
                    doCast(SPELL_NETHER_POWER);
                    m_uiStackReply--;
                    if (m_uiStackReply <= 0)
                        m_bPowerCheck = true;
                }
                else if (pFizzle->isAlive()) // don't cast if Fizzlebang is alive
                {
                    DoMeleeAttackIfReady();

                    //kill Fizzlebang if is elapsed too much time
                    if (m_uiFizzlebangSecurityKillTimer < uiDiff)
                        m_creature->DealDamage(pFizzle, pFizzle->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    else m_uiFizzlebangSecurityKillTimer -= uiDiff;
                    return;
                }
            }
        }

        timedCast(SPELL_FEL_FIREBALL, uiDiff);

        timedCast(SPELL_FEL_LIGHTING, uiDiff);

        if (timedQuery(SPELL_INCINERATE_FLESH, uiDiff))
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                DoScriptText(-1713522, m_creature, pTarget);
                doCast(SPELL_INCINERATE_FLESH, pTarget);
             }
        }

        if (timedQuery(SPELL_LEGION_FLAME_1, uiDiff))
        {
            DoScriptText(-1713518, m_creature);
            doCast(SPELL_LEGION_FLAME_1);
        }

        if (m_uiSummonTimer < uiDiff)
        {
            float fPosX, fPosY, fPosZ;
            fPosZ = m_creature->GetPositionZ();
            double dOffset = urand(0, 17);
            int i = urand(0, 19);
            fPosX = SpawnLoc[1].x + 20 * cos(dOffset + i*18);
            fPosY = SpawnLoc[1].y + 20 * sin(dOffset + i*18);

            if (m_bVolcanoOrPortal)
            {
                    DoScriptText(-1713520, m_creature);
                    m_creature->SummonCreature(NPC_INFERNAL_VOLCANO, fPosX, fPosY, fPosZ, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                    m_bVolcanoOrPortal = false;
            }
            else
            {
                    DoScriptText(-1713519, m_creature);
                    m_creature->SummonCreature(NPC_NETHER_PORTAL, fPosX, fPosY, fPosZ, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                    m_bVolcanoOrPortal = true;
            }
            m_uiSummonTimer = 60000;
        }
        else m_uiSummonTimer -= uiDiff;

        if (m_uiPowerTimer < uiDiff)
        {
            doCast(SPELL_NETHER_POWER);
            m_uiPowerTimer = 27000;
        }
        else
            m_uiPowerTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_jaraxxus(Creature* pCreature)
{
    return new boss_jaraxxusAI(pCreature);
}

/*#####
# Legion Flame
#####*/
struct MANGOS_DLL_DECL mob_legion_flameAI : public BSWScriptedAI
{
    mob_legion_flameAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiRangeCheckTimer;

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetInCombatWithZone();
        m_creature->SetDisplayId(27625);
        m_creature->SetRespawnDelay(DAY);
        m_uiRangeCheckTimer = 1000;
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;
    }

    void JustDied(Unit* Killer){}

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance->GetData(TYPE_JARAXXUS) != IN_PROGRESS) 
            m_creature->ForcedDespawn();

        if (m_uiRangeCheckTimer < uiDiff)
        {
           doCast(m_creature,SPELL_LEGION_FLAME_0);
           m_uiRangeCheckTimer = 1000;
        }
        else m_uiRangeCheckTimer -= uiDiff;

    }
};

CreatureAI* GetAI_mob_legion_flame(Creature* pCreature)
{
    return new mob_legion_flameAI(pCreature);
}

/*#####
# Infernal Vulcano
#####*/
struct MANGOS_DLL_DECL mob_infernal_volcanoAI : public BSWScriptedAI
{
    mob_infernal_volcanoAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiCount;
    uint32 m_uiTimer;

    void Reset()
    {
        m_uiTimer = 5000;
        m_creature->SetRespawnDelay(DAY);
        m_uiCount = 0;
        if (currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL || currentDifficulty == RAID_DIFFICULTY_25MAN_NORMAL) 
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        else
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
         }
    }

    void AttackStart(Unit *who)
    {
        return;
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;
    }

    void JustDied(Unit* Killer){}

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (m_pInstance->GetData(TYPE_JARAXXUS) != IN_PROGRESS)
            m_creature->ForcedDespawn();

        if (currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL || currentDifficulty == RAID_DIFFICULTY_25MAN_NORMAL)
        {
            if (m_uiCount >= 3)
            {
                m_creature->ForcedDespawn();
            }
        }

        if (m_uiTimer < diff)
        {
            doCast(SPELL_INFERNAL_ERUPTION);
            DoScriptText(-1713524, m_creature);
            m_uiCount++;
            m_uiTimer = 5000;
        }
        else m_uiTimer -= diff;

        if (m_uiCount >= 3)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

    }
};

CreatureAI* GetAI_mob_infernal_volcano(Creature* pCreature)
{
    return new mob_infernal_volcanoAI(pCreature);
}

/*#####
# Fel Infernal
#####*/
struct MANGOS_DLL_DECL mob_fel_infernalAI : public BSWScriptedAI
{
    mob_fel_infernalAI(Creature* pCreature) : BSWScriptedAI(pCreature)
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

    void JustDied(Unit* Killer){}

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance->GetData(TYPE_JARAXXUS) != IN_PROGRESS) 
            m_creature->ForcedDespawn();

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        timedCast(SPELL_FEL_INFERNO, uiDiff);

        timedCast(SPELL_FEL_STREAK, uiDiff);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_fel_infernal(Creature* pCreature)
{
    return new mob_fel_infernalAI(pCreature);
}

/*#####
# Nether Portal
#####*/

struct MANGOS_DLL_DECL mob_nether_portalAI : public BSWScriptedAI
{
    mob_nether_portalAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiTimer;
    bool m_bHasSummoned;

    void Reset()
    {
        m_uiTimer = 15000;
        m_bHasSummoned = false;
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetDisplayId(29074);
        if (currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL || currentDifficulty == RAID_DIFFICULTY_25MAN_NORMAL)
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        else
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
         }

    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;
    }

    void AttackStart(Unit *who)
    {
        return;
    }

    void JustDied(Unit* Killer){}

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (m_pInstance->GetData(TYPE_JARAXXUS) != IN_PROGRESS )
            m_creature->ForcedDespawn();

        if (currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL || currentDifficulty == RAID_DIFFICULTY_25MAN_NORMAL)
        {
            if (m_bHasSummoned)
            {
                m_creature->ForcedDespawn();
            }
        }

        if (m_uiTimer < diff)
        {
            m_creature->SummonCreature(NPC_MISTRESS, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
            DoScriptText(-1713521, m_creature);
            m_bHasSummoned = true;
            m_uiTimer = 15000;
        }
        else m_uiTimer -= diff;

        if (m_bHasSummoned)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

    }
};

CreatureAI* GetAI_mob_nether_portal(Creature* pCreature)
{
    return new mob_nether_portalAI(pCreature);
}

/*#####
# Misterss of Pain
#####*/
struct MANGOS_DLL_DECL mob_mistress_of_painAI : public BSWScriptedAI
{
    mob_mistress_of_painAI(Creature* pCreature) : BSWScriptedAI(pCreature)
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

    void JustDied(Unit* Killer){}

    void Aggro(Unit *who)
    {
        if (!m_pInstance) return;
        DoScriptText(-1713523,m_creature, who);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance)
            return;

        if (m_pInstance->GetData(TYPE_JARAXXUS) != IN_PROGRESS) 
            m_creature->ForcedDespawn();

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        timedCast(SPELL_SHIVAN_SLASH, uiDiff);

        timedCast(SPELL_SPINNING_STRIKE, uiDiff);

        if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_25MAN_HEROIC)
            timedCast(SPELL_MISTRESS_KISS, uiDiff);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mistress_of_pain(Creature* pCreature)
{
    return new mob_mistress_of_painAI(pCreature);
}

void AddSC_boss_jaraxxus()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_jaraxxus";
    newscript->GetAI = &GetAI_boss_jaraxxus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_legion_flame";
    newscript->GetAI = &GetAI_mob_legion_flame;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_infernal_volcano";
    newscript->GetAI = &GetAI_mob_infernal_volcano;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_fel_infernal";
    newscript->GetAI = &GetAI_mob_fel_infernal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_nether_portal";
    newscript->GetAI = &GetAI_mob_nether_portal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_mistress_of_pain";
    newscript->GetAI = &GetAI_mob_mistress_of_pain;
    newscript->RegisterSelf();
}
