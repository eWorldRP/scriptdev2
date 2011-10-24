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
// Gormok - Firebomb not implemented, timers need correct
// Snakes - Underground phase not worked, timers need correct
// Icehowl - Trample&Crash event not implemented, timers need correct

/* ScriptData
SDName: northrend_beasts
SD%Complete: 90% 
SDComment: by /dev/rsa
SDCategory:
EndScriptData */

// not implemented:
// snobolds link
// snakes underground cast (not support in core)

#include "precompiled.h"
#include "trial_of_the_crusader.h"

enum Equipment
{
    EQUIP_MAIN           = 50760,
    EQUIP_OFFHAND        = 48040,
    EQUIP_RANGED         = 47267,
    EQUIP_DONE           = EQUIP_NO_CHANGE,
};

enum Summons
{
    NPC_SNOBOLD_VASSAL   = 34800,
    NPC_SLIME_POOL       = 35176,
    NPC_FIRE_BOMB        = 34854,
};

enum BossSpells
{
SPELL_IMPALE           = 66331,
SPELL_STAGGERING_STOMP = 67648,
SPELL_RISING_ANGER     = 66636,
SUMMON_SNOBOLD         = NPC_SNOBOLD_VASSAL,
SPELL_ACID_SPIT        = 66880,
SPELL_PARALYTIC_SPRAY  = 66901,
SPELL_ACID_SPEW        = 66819,
SPELL_PARALYTIC_BITE   = 66824,
SPELL_SWEEP_0          = 66794,
SPELL_SLIME_POOL       = 66883,
SPELL_FIRE_SPIT        = 66796,
SPELL_MOLTEN_SPEW      = 66821,
SPELL_BURNING_BITE     = 66879,
SPELL_BURNING_SPRAY    = 66902,
SPELL_SWEEP_1          = 67646,
SPELL_FEROCIOUS_BUTT   = 66770,
SPELL_MASSIVE_CRASH    = 66683,
SPELL_WHIRL            = 67345,
SPELL_ARCTIC_BREATH    = 66689,
SPELL_TRAMPLE          = 66734,
SPELL_ADRENALINE       = 68667,
SPELL_SNOBOLLED        = 66406,
SPELL_BATTER           = 66408,
SPELL_FIRE_BOMB        = 66313,
SPELL_FIRE_BOMB_1      = 66317,
SPELL_FIRE_BOMB_DOT    = 66318,
SPELL_HEAD_CRACK       = 66407,
SPELL_SUBMERGE_0       = 53421,
SPELL_ENRAGE           = 68335,
SPELL_FROTHING_RAGE    = 66759,
SPELL_STAGGERED_DAZE   = 66758,
SPELL_SLIME_POOL_1     = 66881,
SPELL_SLIME_POOL_2     = 66882,
SPELL_SLIME_POOL_VISUAL  = 63084,
};
/*#####
# Gormok the Impaler
#####*/
struct MANGOS_DLL_DECL boss_gormokAI : public BSWScriptedAI
{
    boss_gormokAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint16 m_uiSnowboldHealtPercent;

    void Reset()
    {
        if(!m_pInstance) return;
        SetEquipmentSlots(false, EQUIP_MAIN, EQUIP_OFFHAND, EQUIP_RANGED);
        m_creature->SetRespawnDelay(7*DAY);
        m_creature->SetInCombatWithZone();
        m_uiSnowboldHealtPercent = 80;
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, GORMOK_DONE);
    }

    void JustReachedHome()
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, FAIL);
        m_creature->ForcedDespawn();
    }

    void Aggro(Unit* pWho)
    {
        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, GORMOK_IN_PROGRESS);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        timedCast(SPELL_IMPALE, uiDiff);

        timedCast(SPELL_STAGGERING_STOMP, uiDiff);

        if (m_creature->GetHealthPercent() < m_uiSnowboldHealtPercent)
        {
            doCast(SUMMON_SNOBOLD);
            DoScriptText(-1713601,m_creature);
            m_uiSnowboldHealtPercent -= 20;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gormok(Creature* pCreature)
{
    return new boss_gormokAI(pCreature);
}

/*#####
# Snowbold Vassal
#####*/
struct MANGOS_DLL_DECL mob_snobold_vassalAI : public BSWScriptedAI
{
    mob_snobold_vassalAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    Unit* m_pBoss;
    Unit* m_pDefaultTarget;

    void Reset()
    {
        m_pBoss = NULL;
        m_pDefaultTarget = NULL;
        m_creature->SetInCombatWithZone();
        m_creature->SetRespawnDelay(DAY);
        m_pBoss = m_pInstance->GetSingleCreatureFromStorage(NPC_GORMOK);
        if (m_pBoss)
            doCast(SPELL_RISING_ANGER,m_pBoss);
    }

    void Aggro(Unit *who)
    {
        if (!m_pInstance)
            return;

        m_pDefaultTarget = who;
        doCast(SPELL_SNOBOLLED, m_pDefaultTarget);
    }

    void JustReachedHome()
    {
        if (!m_pInstance)
            return;

        m_creature->ForcedDespawn();
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pDefaultTarget && m_pDefaultTarget->isAlive())
            doRemove(SPELL_SNOBOLLED, m_pDefaultTarget);
//      if (pBoss && pBoss->isAlive()) doRemove(SPELL_RISING_ANGER,pBoss);
//      This string - not offlike, in off this buff not removed! especially for small servers.
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance->GetData(TYPE_BEASTS) != IN_PROGRESS)
            m_creature->ForcedDespawn();

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        timedCast(SPELL_BATTER, uiDiff);

        if (timedCast(SPELL_FIRE_BOMB, uiDiff, m_creature->getVictim()) == CAST_OK)
        {
            doCast(SPELL_FIRE_BOMB_1, m_creature->getVictim());
            doCast(SPELL_FIRE_BOMB_DOT, m_creature->getVictim());
        }

        timedCast(SPELL_HEAD_CRACK, uiDiff);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_snobold_vassal(Creature* pCreature)
{
    return new mob_snobold_vassalAI(pCreature);
}

/*#####
# Acidmaw
#####*/
struct MANGOS_DLL_DECL boss_acidmawAI : public BSWScriptedAI
{
    boss_acidmawAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint8 m_uiStage;
    bool m_bIsSubmerged;
    bool m_bEnraged;
    bool m_bFirstAppear;
    uint32 m_uiAppearTimer;
    uint32 m_uiSpewTimer;


    void Reset()
    {
        m_uiStage = 1;
        m_bEnraged = false;
        m_bIsSubmerged = false;
        m_bFirstAppear = true;
        m_uiAppearTimer = 0;
        m_uiSpewTimer = 30000;
        m_creature->SetInCombatWithZone();
        m_creature->SetRespawnDelay(7*DAY);
        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, ACIDMAW_SUBMERGED);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance) 
            return;
        
        if (Creature* pSister = m_pInstance->GetSingleCreatureFromStorage(NPC_DREADSCALE))
        {
            if (!pSister->isAlive())
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);
            else 
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, SNAKES_SPECIAL);
        }
    }

    void JustReachedHome()
    {
        if (!m_pInstance) 
            return;

        if (m_pInstance->GetData(TYPE_BEASTS) == IN_PROGRESS && m_pInstance->GetData(TYPE_NORTHREND_BEASTS) != FAIL)
            m_pInstance->SetData(TYPE_NORTHREND_BEASTS, FAIL);
        
        m_creature->ForcedDespawn();
    }

    void Aggro(Unit* pWho)
    {
    }

    void UpdateAI(const uint32 uiDiff)
    {

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiStage)
        {
        case 0:
               if (m_bIsSubmerged)
                {
                    if (m_uiAppearTimer < uiDiff)
                    {
                        DoScriptText(-1713559,m_creature);
                        doRemove(SPELL_SUBMERGE_0);
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                        SetCombatMovement(true);
                        m_bIsSubmerged = false;
                    }
                    else
                    {
                        m_uiAppearTimer -= uiDiff;
                        return;
                    }
                }

                if ( m_uiSpewTimer < uiDiff)
                {
                    doCast(SPELL_ACID_SPEW);
                    m_uiSpewTimer = 30000;
                }
                else m_uiSpewTimer -= uiDiff;

                timedCast(SPELL_PARALYTIC_BITE, uiDiff);

                if (timedQuery(SPELL_SLIME_POOL, uiDiff))
                    m_creature->SummonCreature(NPC_SLIME_POOL, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000);

                if (m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == ACIDMAW_SUBMERGED)
                     m_uiStage = 1;

                break;
        case 1:
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->InterruptNonMeleeSpells(true);
                doCast(SPELL_SUBMERGE_0);
                m_uiStage = 2;
                DoScriptText(-1713557,m_creature);
                if (!m_bFirstAppear)
                {
                    if (Creature* pSister = m_pInstance->GetSingleCreatureFromStorage(NPC_DREADSCALE))
                    {
                        float fSisx, fSisy, fSisz;
                        float fBrox, fBroy, fBroz;
                        pSister->GetPosition(fSisx, fSisy, fSisz);
                        m_creature->GetPosition(fBrox, fBroy, fBroz);
                        m_creature->SetPosition(fSisx, fSisy, fSisz, 0, true);
                        pSister->SetPosition(fBrox, fBroy, fBroz, 0, true);
                    }
                }
                else
                {
                    m_creature->SetPosition(546.347839f, 162.338888f, 395.14f, 0, true);
                    m_bFirstAppear = false;
                }
                m_uiAppearTimer = 2000;
                m_bIsSubmerged = true;
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, ACIDMAW_SUBMERGED);
                break;
        case 2:
                if (m_bIsSubmerged)
                {
                    if (m_uiAppearTimer < uiDiff)
                    {
                        DoScriptText(-1713559,m_creature);
                        doRemove(SPELL_SUBMERGE_0);
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        m_creature->GetMotionMaster()->MoveIdle();
                        SetCombatMovement(false);
                        m_bIsSubmerged = false;
                    }
                    else
                    {
                        m_uiAppearTimer -= uiDiff;
                        return;
                    }
                }

                timedCast(SPELL_ACID_SPIT, uiDiff);

                timedCast(SPELL_PARALYTIC_SPRAY, uiDiff);

                timedCast(SPELL_SWEEP_0, uiDiff);

                if ((timedQuery(SPELL_SUBMERGE_0, uiDiff) && m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == ACIDMAW_SUBMERGED) || m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == DREADSCALE_SUBMERGED)
                    m_uiStage = 3;
                break;
        case 3:
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->InterruptNonMeleeSpells(true);
                doCast(SPELL_SUBMERGE_0);
                DoScriptText(-1713557,m_creature);
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, DREADSCALE_SUBMERGED);
                if (Creature* pSister = m_pInstance->GetSingleCreatureFromStorage(NPC_DREADSCALE))
                {
                    float fSisx, fSisy, fSisz;
                    float fBrox, fBroy, fBroz;
                    pSister->GetPosition(fSisx, fSisy, fSisz);
                    m_creature->GetPosition(fBrox, fBroy, fBroz);
                    m_creature->SetPosition(fSisx, fSisy, fSisz, 0, true);
                    pSister->SetPosition(fBrox, fBroy, fBroz, 0, true);
                }
                m_uiSpewTimer += 1000;
                m_uiAppearTimer = 2000;
                m_bIsSubmerged = true;
                m_uiStage = 0;
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, DREADSCALE_SUBMERGED);
                break;
        }

        if (m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == SNAKES_SPECIAL && !m_bEnraged)
        {
            DoScriptText(-1713559,m_creature);
            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            doCast(SPELL_ENRAGE);
            doRemove(SPELL_SUBMERGE_0);
            m_bEnraged = true;
            SetCombatMovement(true);
            m_bIsSubmerged = false;
            m_uiStage = 0;
            DoScriptText(-1713504,m_creature);
         }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_acidmaw(Creature* pCreature)
{
    return new boss_acidmawAI(pCreature);
}

/*#####
# Dreadscale
#####*/
struct MANGOS_DLL_DECL boss_dreadscaleAI : public BSWScriptedAI
{
    boss_dreadscaleAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint8 m_uiStage;
    bool m_bEnraged;
    bool m_bIsSubmerged;
    uint32 m_uiAppearTimer;
    uint32 m_uiSpewTimer;

    void Reset()
    {
        m_uiStage = 0;
        m_bEnraged = false;
        m_bIsSubmerged = false;
        m_uiAppearTimer = 0;
        m_uiSpewTimer = 30000;
        m_creature->SetInCombatWithZone();
        m_creature->SetRespawnDelay(7*DAY);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance) 
            return;
        
        if (Creature* pSister = m_pInstance->GetSingleCreatureFromStorage(NPC_ACIDMAW))
            if (!pSister->isAlive())
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);
            else
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, SNAKES_SPECIAL);
    }

    void JustReachedHome()
    {
        if (!m_pInstance) 
            return;
        
        if (m_pInstance->GetData(TYPE_BEASTS) == IN_PROGRESS && m_pInstance->GetData(TYPE_NORTHREND_BEASTS) != FAIL)
            m_pInstance->SetData(TYPE_NORTHREND_BEASTS, FAIL);
        
        m_creature->ForcedDespawn();
    }

    void Aggro(Unit* pWho)
    {
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiStage)
        {
        case 0:
               if (m_bIsSubmerged)
                {
                    if (m_uiAppearTimer < uiDiff)
                    {
                        DoScriptText(-1713559,m_creature);
                        doRemove(SPELL_SUBMERGE_0);
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                        SetCombatMovement(true);
                        m_bIsSubmerged = false;
                    }
                    else
                    {
                        m_uiAppearTimer -= uiDiff;
                        return;
                    }
                }
                timedCast(SPELL_BURNING_BITE, uiDiff);

                if ( m_uiSpewTimer < uiDiff)
                {
                    doCast(SPELL_MOLTEN_SPEW);
                    m_uiSpewTimer = 30000;
                }
                else m_uiSpewTimer -= uiDiff;
                if (timedQuery(SPELL_SLIME_POOL, uiDiff))
                    m_creature->SummonCreature(NPC_SLIME_POOL, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000);

                if (m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == DREADSCALE_SUBMERGED)
                     m_uiStage = 1;

                break;
        case 1:
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->InterruptNonMeleeSpells(true);
                doCast(SPELL_SUBMERGE_0);
                m_uiStage = 2;
                m_uiAppearTimer = 2000;
                m_bIsSubmerged = true;
                DoScriptText(-1713557,m_creature);
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, DREADSCALE_SUBMERGED);
                break;
        case 2:
                if (m_bIsSubmerged)
                {
                    if (m_uiAppearTimer < uiDiff)
                    {
                        DoScriptText(-1713559,m_creature);
                        doRemove(SPELL_SUBMERGE_0);
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        m_creature->GetMotionMaster()->MoveIdle();
                        SetCombatMovement(false);
                        m_bIsSubmerged = false;
                    }
                    else
                    {
                        m_uiAppearTimer -= uiDiff;
                        return;
                    }
                }
                timedCast(SPELL_FIRE_SPIT, uiDiff);

                timedCast(SPELL_BURNING_SPRAY, uiDiff);

                timedCast(SPELL_SWEEP_0, uiDiff);

                if ((timedQuery(SPELL_SUBMERGE_0, uiDiff) && m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == DREADSCALE_SUBMERGED) || m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == ACIDMAW_SUBMERGED)
                    m_uiStage = 3;
                break;
        case 3:
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->InterruptNonMeleeSpells(true);
                doCast(SPELL_SUBMERGE_0);
                DoScriptText(-1713559,m_creature);
                m_pInstance->SetData(TYPE_NORTHREND_BEASTS, ACIDMAW_SUBMERGED);
                m_uiAppearTimer = 2000;
                m_uiSpewTimer += 1000;
                m_bIsSubmerged = true;
                m_uiStage = 0;
                break;
        }

        if (m_pInstance->GetData(TYPE_NORTHREND_BEASTS) == SNAKES_SPECIAL && !m_bEnraged)
        {
            DoScriptText(-1713559,m_creature);
            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            doCast(SPELL_ENRAGE);
            doRemove(SPELL_SUBMERGE_0);
            m_bEnraged = true;
            SetCombatMovement(true);
            m_bIsSubmerged = false;
            m_uiStage = 0;
            DoScriptText(-1713504,m_creature);
         }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_dreadscale(Creature* pCreature)
{
    return new boss_dreadscaleAI(pCreature);
}

/*#####
# Slime Pool
#####*/
struct MANGOS_DLL_DECL mob_slime_poolAI : public BSWScriptedAI
{
    mob_slime_poolAI(Creature *pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance *m_pInstance;
    uint32 m_uiDespawnTimer;
    uint32 m_uiSizeTimer;
    uint32 m_uiPoolTimer;
    float m_fSize;
    bool m_bCloudCasted;

    void Reset()
    {
        if(!m_pInstance)
            return;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetInCombatWithZone();
        SetCombatMovement(false);
        m_uiDespawnTimer = 30000;
        m_bCloudCasted = false;
        m_uiPoolTimer = 1000;
        m_fSize = m_creature->GetFloatValue(OBJECT_FIELD_SCALE_X);
        m_uiSizeTimer = 500;
    }

    void AttackStart(Unit *pWho)
    {
        return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_pInstance->GetData(TYPE_BEASTS) != IN_PROGRESS)
            m_creature->ForcedDespawn();

        if (!m_bCloudCasted)
        {
            DoCast(m_creature, SPELL_SLIME_POOL_VISUAL);
            m_bCloudCasted = true;
        }

        if (m_uiPoolTimer < uiDiff)
        {
            doCast(SPELL_SLIME_POOL_1);
            m_uiPoolTimer = 1000;
        }
        else m_uiPoolTimer -= uiDiff;

        if (m_uiSizeTimer < uiDiff)
        {
                m_fSize = m_fSize*1.035;
                m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, m_fSize);
                m_uiSizeTimer = 500;
        }
        else m_uiSizeTimer -= uiDiff;

        if (m_uiDespawnTimer <= uiDiff)
        {
             m_creature->ForcedDespawn();
        }
        else
            m_uiDespawnTimer -= uiDiff;
    }

};

CreatureAI* GetAI_mob_slime_pool(Creature* pCreature)
{
    return new mob_slime_poolAI(pCreature);
}

/*#####
# Icehowl
#####*/
struct MANGOS_DLL_DECL boss_icehowlAI : public BSWScriptedAI
{
    boss_icehowlAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bMovementStarted;
    bool m_bTrampleCasted;
    uint8 m_uiStage;
    float fPosX, fPosY, fPosZ;
    Unit* m_pTarget;
    Unit* m_pOldTarget;

    void Reset()
    {
        if(!m_pInstance)
            return;
        m_creature->SetRespawnDelay(7*DAY);
        m_bMovementStarted = false;
        m_uiStage = 0;
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, ICEHOWL_DONE);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(!m_pInstance)
            return;

        if(type != POINT_MOTION_TYPE)
            return;

        if(id != 1 && m_bMovementStarted)
        {
             m_creature->GetMotionMaster()->MovePoint(1, fPosX, fPosY, fPosZ);
        }
        else
        {
            m_creature->GetMotionMaster()->MovementExpired();
            m_bMovementStarted = false;
            SetCombatMovement(true);
            if (m_pOldTarget && m_pOldTarget->isAlive())
                m_creature->GetMotionMaster()->MoveChase(m_pOldTarget);
        }
    }

    void JustReachedHome()
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, FAIL);
        m_creature->ForcedDespawn();
    }

    void Aggro(Unit* pWho)
    {
        m_creature->SetInCombatWithZone();
        m_pInstance->SetData(TYPE_NORTHREND_BEASTS, ICEHOWL_IN_PROGRESS);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiStage)
        {
        case 0:
            timedCast(SPELL_FEROCIOUS_BUTT, uiDiff);

            timedCast(SPELL_ARCTIC_BREATH, uiDiff);

            timedCast(SPELL_WHIRL, uiDiff);

            if (timedQuery(SPELL_MASSIVE_CRASH, uiDiff))
            {
                m_creature->SetPosition(SpawnLoc[1].x, SpawnLoc[1].y, SpawnLoc[1].z, 0);
                m_pOldTarget = m_creature->getVictim();
                m_uiStage = 1;
                break;
            }

            timedCast(SPELL_FROTHING_RAGE, uiDiff);

            DoMeleeAttackIfReady();

            break;
        case 1:
            if (doCast(SPELL_MASSIVE_CRASH) == CAST_OK)
            {
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                SetCombatMovement(false);
                m_uiStage = 2;
            }
            break;
        case 2:
            if (m_pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
            {
                m_bTrampleCasted = false;
                m_uiStage = 3;
                resetTimer(SPELL_TRAMPLE);
                DoScriptText(-1713506,m_creature,m_pTarget);
                m_creature->GetMotionMaster()->MoveIdle();
            }
            break;
        case 3:
            if (timedQuery(SPELL_TRAMPLE,uiDiff))
            {
                if (m_pTarget && m_pTarget->isAlive() && (m_pTarget->IsWithinDistInMap(m_creature, 200.0f)))
                {
                    m_pTarget->GetPosition(fPosX, fPosY, fPosZ);
                    float fTarX, fTarY;
                    fTarX =fPosX - SpawnLoc[1].x;
                    fTarY =fPosY - SpawnLoc[1].y;
                    //this to avoid boss going in texture and despawn
                    if (fTarX > 0)
                        fTarX = SpawnLoc[1].x - fTarX +8.0f;
                    else
                        fTarX = SpawnLoc[1].x - fTarX -8.0f;

                    if (fTarY > 0)
                        fTarY = SpawnLoc[1].y - fTarY +8.0f;
                    else
                        fTarY = SpawnLoc[1].y - fTarY -8.0f;

                    m_creature->SetPosition(fTarX, fTarY, fPosZ, 0);
                    m_bTrampleCasted = false;
                    m_bMovementStarted = true;
                    m_creature->GetMotionMaster()->MovePoint(1, fPosX, fPosY, fPosZ);
                    DoScriptText(-1713508,m_creature);
                    m_uiStage = 4;
                    Map* pMap = m_creature->GetMap();
                    Map::PlayerList const &lPlayers = pMap->GetPlayers();
                    for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                    {
                        Unit* pPlayer = itr->getSource();
                        if (!pPlayer)
                            continue;

                        if (pPlayer->isAlive())
                        {
                            pPlayer->RemoveAurasDueToSpell(SPELL_MASSIVE_CRASH);
                            pPlayer->CastSpell(pPlayer, SPELL_ADRENALINE, true);
                        }
                    }
                }
                else
                {
                    m_bTrampleCasted = true;
                    m_uiStage = 5;
                }
            }
            break;
        case 4:
            if (m_bMovementStarted)
            {
                Map* pMap = m_creature->GetMap();
                Map::PlayerList const &lPlayers = pMap->GetPlayers();
                for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                {
                    Unit* pPlayer = itr->getSource();
                    if (!pPlayer)
                        continue;

                    if (pPlayer->isAlive() && pPlayer->IsWithinDistInMap(m_creature, 5.0f))
                    {
                        doCast(SPELL_TRAMPLE, pPlayer);
                        m_bTrampleCasted = true;
                        m_bMovementStarted = false;
                    }
                }
            }
            else
                m_uiStage = 5;

            if (m_bTrampleCasted)
                m_uiStage = 5;

            break;
        case 5:
            if (!m_bTrampleCasted)
            {
                doCast(SPELL_STAGGERED_DAZE);
                DoScriptText(-1713507,m_creature);
            }
            m_bMovementStarted = false;
            m_creature->GetMotionMaster()->MovementExpired();
            if (m_pOldTarget && m_pOldTarget->isAlive())
                m_creature->GetMotionMaster()->MoveChase(m_pOldTarget);
            SetCombatMovement(true);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_uiStage = 0;
            break;
        }

    }
};

CreatureAI* GetAI_boss_icehowl(Creature* pCreature)
{
    return new boss_icehowlAI(pCreature);
}

void AddSC_northrend_beasts()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_gormok";
    newscript->GetAI = &GetAI_boss_gormok;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_acidmaw";
    newscript->GetAI = &GetAI_boss_acidmaw;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_dreadscale";
    newscript->GetAI = &GetAI_boss_dreadscale;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_icehowl";
    newscript->GetAI = &GetAI_boss_icehowl;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_snobold_vassal";
    newscript->GetAI = &GetAI_mob_snobold_vassal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_slime_pool";
    newscript->GetAI = &GetAI_mob_slime_pool;
    newscript->RegisterSelf();

}
