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
SDName: trial_of_the_crusader
SD%Complete: 80%
SDComment: by /dev/rsa
SDCategory: Crusader Coliseum
EndScriptData */

// Twin pact - heal part not worked now by undefined reason. Need override?
// timers need correct

#include "precompiled.h"
#include "trial_of_the_crusader.h"

enum Equipment
{
    EQUIP_MAIN_1         = 49303,
    EQUIP_OFFHAND_1      = 47146,
    EQUIP_RANGED_1       = 47267,
    EQUIP_MAIN_2         = 45990,
    EQUIP_OFFHAND_2      = 47470,
    EQUIP_RANGED_2       = 47267,
    EQUIP_DONE           = EQUIP_NO_CHANGE,
};

enum Summons
{
    NPC_DARK_ESSENCE     = 34567,
    NPC_LIGHT_ESSENCE    = 34568,

    NPC_DARK_ORB   = 34628,
    NPC_LIGHT_ORB  = 34630,
};

enum BossSpells
{
    SPELL_TWIN_SPIKE_L     = 66075,
    SPELL_LIGHT_SURGE      = 65766,
    SPELL_SHIELD_LIGHT     = 65858,
    SPELL_LIGHT_VORTEX     = 66046,
    SPELL_LIGHT_TOUCH      = 67297,
    SPELL_TWIN_SPIKE_H     = 66069,
    SPELL_DARK_SURGE       = 65768,
    SPELL_SHIELD_DARK      = 65874,
    SPELL_TWIN_PACT_20_F   = 67306,
    SPELL_TWIN_PACT_50_F   = 67307,
    SPELL_TWIN_PACT_20_E   = 67305,
    SPELL_TWIN_PACT_50_E   = 67304,
    SPELL_DARK_VORTEX      = 66058,
    SPELL_DARK_TOUCH       = 67282,
    SPELL_TWIN_POWER       = 65916,
    SPELL_BERSERK          = 64238,
    SPELL_REMOVE_TOUCH     = 68084,
    SPELL_NONE             = 0,
//
    SPELL_UNLEASHED_DARK   = 65808,
    SPELL_UNLEASHED_LIGHT  = 65795,
    SPELL_EMPOWERED_LIGHT_25N   = 67216,
    SPELL_EMPOWERED_DARK_25N    = 67213,
    SPELL_EMPOWERED_LIGHT_25H   = 67218,
    SPELL_EMPOWERED_DARK_25H    = 67215,
    SPELL_EMPOWERED_LIGHT_10N   = 65748,
    SPELL_EMPOWERED_DARK_10N    = 65724,
    SPELL_EMPOWERED_LIGHT_10H   = 67217,
    SPELL_EMPOWERED_DARK_10H    = 67214
};

int GetRightPactId(Difficulty x, uint32 Entry)
{
    switch(x)
    {
        case RAID_DIFFICULTY_10MAN_NORMAL:
        case RAID_DIFFICULTY_25MAN_NORMAL:
            if(Entry == NPC_DARKBANE)
                return SPELL_TWIN_PACT_20_E;
            else if(Entry == NPC_LIGHTBANE)
                return SPELL_TWIN_PACT_20_F;
        case RAID_DIFFICULTY_10MAN_HEROIC:
        case RAID_DIFFICULTY_25MAN_HEROIC:
            if(Entry == NPC_DARKBANE)
                return SPELL_TWIN_PACT_50_E;
            else if(Entry == NPC_LIGHTBANE)
                return SPELL_TWIN_PACT_50_F;
        default:
            return 0;
    }
}

int GetPoweringId(Difficulty x)
{
    switch(x)
    {
        case RAID_DIFFICULTY_10MAN_NORMAL:
            return 67590;
        case RAID_DIFFICULTY_25MAN_NORMAL:
            return 67603;
        case RAID_DIFFICULTY_10MAN_HEROIC:
            return 67602;
        case RAID_DIFFICULTY_25MAN_HEROIC:
            return 67604;
        default:
            return 0;
    }
}

struct MANGOS_DLL_DECL boss_fjolaAI : public BSWScriptedAI
{
    boss_fjolaAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint8 stage;
    uint32 m_uiVortexTimer;
    uint32 m_uiIncrease;
    uint32 m_uiPactTimer;
    uint8 m_uiReply;
    bool m_bIsVortex;
    bool m_bIsPact;

    void Reset()
    {
        if(!m_pInstance)
            return;

        m_pInstance->SetData(DATA_HEALTH_FJOLA, m_creature->GetMaxHealth());
        
        m_uiVortexTimer = 1000;

        SetEquipmentSlots(false, EQUIP_MAIN_1, EQUIP_OFFHAND_1, EQUIP_RANGED_1);
        m_creature->SetRespawnDelay(7*DAY);
        m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
        stage = 0;
        m_bIsVortex = false;
        m_bIsPact = false;
        if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL)
            m_uiIncrease = 6;
        else
            m_uiIncrease = 9;
    }

    void JustReachedHome()
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_VALKIRIES, FAIL);
        m_pInstance->SetData(DATA_HEALTH_FJOLA, m_creature->GetMaxHealth());
        m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
        m_creature->ForcedDespawn();
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        DoScriptText(-1713547,m_creature);
        if (Creature* pSister = m_pInstance->GetSingleCreatureFromStorage(NPC_DARKBANE))
        {
            if (!pSister->isAlive())
                m_pInstance->SetData(TYPE_VALKIRIES, DONE);
            else 
                m_pInstance->SetData(TYPE_VALKIRIES, SPECIAL);
        }
        m_pInstance->SetData(DATA_HEALTH_FJOLA, 0);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (!m_pInstance)
            return;
        DoScriptText(-1713544,m_creature,pVictim);
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        m_creature->SetInCombatWithZone();
        m_pInstance->SetData(TYPE_VALKIRIES, IN_PROGRESS);
        DoScriptText(-1713541,m_creature);
        if (m_creature->isAlive())
            m_creature->SummonCreature(NPC_LIGHT_ESSENCE, SpawnLoc[24].x, SpawnLoc[24].y, SpawnLoc[24].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 5000);
       if (m_creature->isAlive())
            m_creature->SummonCreature(NPC_LIGHT_ESSENCE, SpawnLoc[25].x, SpawnLoc[25].y, SpawnLoc[25].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 5000);

        m_pInstance->SetData(DATA_HEALTH_FJOLA, m_creature->GetMaxHealth());
        doCast(SPELL_LIGHT_SURGE);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_pInstance)
            return;

        if (!m_creature || !m_creature->isAlive())
            return;

        if (pDoneBy->GetObjectGuid() == m_creature->GetObjectGuid())
            return;

        if (pDoneBy->GetEntry() == NPC_DARKBANE)
        {
            uiDamage = 0;
            return;
        }

        if (pDoneBy->GetTypeId() == TYPEID_PLAYER)
        {
            if (pDoneBy->HasAura(SPELL_LIGHT_ESSENCE))
                uiDamage /= 2;
            else if (pDoneBy->HasAura(SPELL_DARK_ESSENCE))
                uiDamage += uiDamage/2;
        }

        m_pInstance->SetData(DATA_HEALTH_FJOLA, m_creature->GetHealth() >= uiDamage ? m_creature->GetHealth() - uiDamage : 0);
    }

    //hacky workaround for interrupt Twin's Pact
    void SpellHit(Unit* who, const SpellEntry* spell)
    {
        // don't self interrupt
        if (!who->IsControlledByPlayer())
            return;

        if (m_bIsPact)
        {
            if (spell->EffectMechanic[0] == MECHANIC_INTERRUPT || spell->EffectMechanic[1] == MECHANIC_INTERRUPT || spell->EffectMechanic[2] == MECHANIC_INTERRUPT
                || spell->EffectMechanic[0] == MECHANIC_SILENCE || spell->EffectMechanic[1] == MECHANIC_SILENCE || spell->EffectMechanic[2] == MECHANIC_SILENCE)
            {
                if (!m_creature->HasAura(SPELL_SHIELD_LIGHT) && !m_creature->HasAura(67259))
                {
                    m_creature->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);

                    Creature* pSis = m_pInstance->GetSingleCreatureFromStorage(NPC_DARKBANE);
                    if (pSis && pSis->isAlive())
                    {
                        pSis->RemoveAurasDueToSpell(SPELL_TWIN_POWER);
                        pSis->RemoveAurasDueToSpell(65949);
                    }

                    m_bIsPact = false;
                    m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                }
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance)
            return;
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->getVictim() && m_creature->getVictim()->GetEntry() == NPC_DARKBANE)
        {
            float sisThreat = m_creature->getThreatManager().getThreat(m_creature->getVictim());
            m_creature->getThreatManager().addThreat(m_creature->getVictim(), -sisThreat);
        }

        if (m_creature->GetHealth() >= m_pInstance->GetData(DATA_HEALTH_EYDIS) && m_pInstance->GetData(DATA_HEALTH_EYDIS) != 0)
            m_creature->SetHealth(m_pInstance->GetData(DATA_HEALTH_EYDIS));

        switch (stage)
        {
         case 0:
                timedCast(SPELL_TWIN_SPIKE_L, uiDiff);

                if(currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_25MAN_HEROIC)
                {
                    if (timedQuery(SPELL_LIGHT_TOUCH, uiDiff))
                    {
                        if (Unit* pTarget = doSelectRandomPlayer(SPELL_LIGHT_ESSENCE, false, 50.0f))
                            doCast(SPELL_LIGHT_TOUCH, pTarget);
                    }
                }
                if (m_pInstance->GetData(DATA_CASTING_VALKYRS) == SPELL_NONE )
                {
                    if (timedQuery(SPELL_LIGHT_VORTEX, uiDiff))
                    {
                        m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_LIGHT_VORTEX);
                        DoScriptText(-1713538,m_creature);
                        stage = 1;
                        m_uiReply = 5;
                        m_uiVortexTimer = 8000;
                        break;
                    }
                    if (timedQuery(GetRightPactId(currentDifficulty, m_creature->GetEntry()), uiDiff) && m_creature->GetHealthPercent() <= 50.0f)
                    {
                        m_creature->InterruptNonMeleeSpells(true);
                        doCast(SPELL_SHIELD_LIGHT);
                        m_pInstance->SetData(DATA_CASTING_VALKYRS, GetRightPactId(currentDifficulty, m_creature->GetEntry()));
                        DoScriptText(-1713539,m_creature);
                        stage = 3;
                    }
                }
                if (m_pInstance->GetData(DATA_CASTING_VALKYRS) == SPELL_TWIN_PACT_20_E || m_pInstance->GetData(DATA_CASTING_VALKYRS) == SPELL_TWIN_PACT_50_E) 
                    if (!m_creature->HasAura(SPELL_TWIN_POWER))
                        doCast(SPELL_TWIN_POWER);
                break;
         case 1:
                doCast(SPELL_LIGHT_VORTEX);
                m_bIsVortex = true;
                stage = 2;
                break;
         case 2:
                if (!m_creature->HasAura(SPELL_LIGHT_VORTEX) && timedQuery(SPELL_SHIELD_LIGHT, uiDiff))
                {
                    m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                    stage = 0;
                }
                break;
         case 3:
                DoCast(m_creature, GetRightPactId(currentDifficulty, m_creature->GetEntry()));
                m_uiPactTimer = 15000;
                m_bIsPact = true;
                stage = 4;
                break;
         case 4:
                if (!m_creature->HasAura(SPELL_SHIELD_LIGHT) && timedQuery(SPELL_SHIELD_LIGHT, uiDiff))
                {
                    m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                    stage = 0;
                }
         default:
                 break;
        }

        timedCast(SPELL_BERSERK, uiDiff);
        
        if(m_bIsPact)
        {
            if (m_uiPactTimer < uiDiff)
            {
                uint16 pct = 0;
                switch (currentDifficulty)
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        pct = 20;
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        pct = 50;
                        break;
                }
                uint32 addhealth = m_creature->GetMaxHealth() * pct / 100;

                SpellEntry const *spell = GetSpellStore()->LookupEntry(GetRightPactId(currentDifficulty, m_creature->GetEntry()));
                m_creature->DealHeal(m_creature, addhealth, spell);
                m_pInstance->SetData(DATA_HEALTH_FJOLA, m_creature->GetHealth());

                Creature* pSis = m_pInstance->GetSingleCreatureFromStorage(NPC_DARKBANE);
                if (pSis && pSis->isAlive())
                {
                    pSis->DealHeal(pSis, addhealth, spell);
                    m_pInstance->SetData(DATA_HEALTH_EYDIS, pSis->GetHealth());
                }

                pSis->RemoveAurasDueToSpell(SPELL_TWIN_POWER);
                pSis->RemoveAurasDueToSpell(65949);

                m_bIsPact = false;
                m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                m_uiPactTimer = 15000;
            }
            else m_uiPactTimer -= uiDiff;
        }

        if(m_bIsVortex)
        {
            if (m_uiVortexTimer < uiDiff)
            {
                Map* pMap = m_creature->GetMap();
                Map::PlayerList const &lPlayers = pMap->GetPlayers();
                for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                {
                    Unit* pPlayer = itr->getSource();
                    if (!pPlayer)
                        continue;
                    if (pPlayer->isAlive() && pPlayer->HasAura(SPELL_LIGHT_ESSENCE))
                        for(uint8 i=0; i < m_uiIncrease ; i++)
                            pPlayer->CastSpell(pPlayer, GetPoweringId(currentDifficulty), true);
                }
                m_uiVortexTimer = 1000;

                m_uiReply--;
                if (m_uiReply == 0)
                    m_bIsVortex = false;
            }
            else m_uiVortexTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_fjola(Creature* pCreature)

{
    return new boss_fjolaAI(pCreature);
}

struct MANGOS_DLL_DECL boss_eydisAI : public BSWScriptedAI
{
    boss_eydisAI(Creature* pCreature) : BSWScriptedAI(pCreature) 
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint8 stage;
    uint32 m_uiSummonTimer;
    uint32 m_uiVortexTimer;
    uint32 m_uiIncrease;
    uint32 m_uiPactTimer;
    uint8 m_uiReply;
    bool m_bIsVortex;
    bool m_bIsPact;

    void Reset() 
    {
        if(!m_pInstance)
            return;

        m_pInstance->SetData(DATA_HEALTH_EYDIS, m_creature->GetMaxHealth());
        
        m_uiSummonTimer = 45000;
        m_uiVortexTimer = 1000;

        SetEquipmentSlots(false, EQUIP_MAIN_2, EQUIP_OFFHAND_2, EQUIP_RANGED_2);
        m_creature->SetRespawnDelay(7*DAY);
        m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
        stage = 0;
        m_bIsVortex = false;
        m_bIsPact = false;
        if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL)
            m_uiIncrease = 6;
        else
            m_uiIncrease = 9;
    }

    void JustReachedHome()
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_VALKIRIES, FAIL);
        m_pInstance->SetData(DATA_HEALTH_EYDIS, m_creature->GetMaxHealth());
        m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
        m_creature->ForcedDespawn();
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        DoScriptText(-1713547,m_creature);
        if (Creature* pSister = m_pInstance->GetSingleCreatureFromStorage(NPC_LIGHTBANE))
        {
            if (!pSister->isAlive())
                m_pInstance->SetData(TYPE_VALKIRIES, DONE);
            else m_pInstance->SetData(TYPE_VALKIRIES, SPECIAL);
        }
        m_pInstance->SetData(DATA_HEALTH_EYDIS, 0);
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(-1713543,m_creature,pVictim);
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        m_creature->SetInCombatWithZone();
        m_pInstance->SetData(TYPE_VALKIRIES, IN_PROGRESS);
        DoScriptText(-1713741,m_creature);
        if (m_creature->isAlive())
            m_creature->SummonCreature(NPC_DARK_ESSENCE, SpawnLoc[22].x, SpawnLoc[22].y, SpawnLoc[22].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 5000);

        if (m_creature->isAlive())
            m_creature->SummonCreature(NPC_DARK_ESSENCE, SpawnLoc[23].x, SpawnLoc[23].y, SpawnLoc[23].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 5000);

        m_pInstance->SetData(DATA_HEALTH_EYDIS, m_creature->GetMaxHealth());
        doCast(SPELL_DARK_SURGE);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_pInstance)
            return;
        if (!m_creature || !m_creature->isAlive())
            return;

        if(pDoneBy->GetObjectGuid() == m_creature->GetObjectGuid())
            return;

        if (pDoneBy->GetEntry() == NPC_LIGHTBANE)
        {
            uiDamage = 0;
            return;
        }

        if(pDoneBy->GetTypeId() == TYPEID_PLAYER)
        {
            if(pDoneBy->HasAura(SPELL_DARK_ESSENCE))
                uiDamage /= 2;
            else if(pDoneBy->HasAura(SPELL_LIGHT_ESSENCE))
                uiDamage += uiDamage/2;
        }

        m_pInstance->SetData(DATA_HEALTH_EYDIS, m_creature->GetHealth() >= uiDamage ? m_creature->GetHealth() - uiDamage : 0);
    }

    //hacky workaround for interrupt Twin's Pact
    void SpellHit(Unit* who, const SpellEntry* spell)
    {
        // don't self interrupt
        if (!who->IsControlledByPlayer())
            return;
        if( m_bIsPact)
        {
            if (spell->EffectMechanic[0] == MECHANIC_INTERRUPT || spell->EffectMechanic[1] == MECHANIC_INTERRUPT || spell->EffectMechanic[2] == MECHANIC_INTERRUPT
                || spell->EffectMechanic[0] == MECHANIC_SILENCE || spell->EffectMechanic[1] == MECHANIC_SILENCE || spell->EffectMechanic[2] == MECHANIC_SILENCE)
            {
                if (!m_creature->HasAura(SPELL_SHIELD_LIGHT) && !m_creature->HasAura(67259))
                {
                    m_creature->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);

                    Creature* pSis = m_pInstance->GetSingleCreatureFromStorage(NPC_LIGHTBANE);
                    if (pSis && pSis->isAlive())
                    {
                        pSis->RemoveAurasDueToSpell(SPELL_TWIN_POWER);
                        pSis->RemoveAurasDueToSpell(65949);
                    }

                    m_bIsPact = false;
                    m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                }
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->getVictim() && m_creature->getVictim()->GetEntry() == NPC_LIGHTBANE)
        {
            float sisThreat = m_creature->getThreatManager().getThreat(m_creature->getVictim());
            m_creature->getThreatManager().addThreat(m_creature->getVictim(), -sisThreat);
        }

        if (m_creature->GetHealth() >= m_pInstance->GetData(DATA_HEALTH_FJOLA) && m_pInstance->GetData(DATA_HEALTH_FJOLA) != 0)
            m_creature->SetHealth(m_pInstance->GetData(DATA_HEALTH_FJOLA));

        
        if(m_uiSummonTimer < uiDiff)
        {
            int8 black, white = 0;
            for (int i=0; i<20; i++)
            {
                double offset = urand(0, 17);
                float x = SpawnLoc[1].x + RAGGIO_ARENA * cos(offset + i*18);
                float y = SpawnLoc[1].y + RAGGIO_ARENA * sin(offset + i*18);

                if(white == 10)
                {
                    black++;
                    m_creature->SummonCreature(NPC_DARK_ORB, x, y, SpawnLoc[1].z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                }
                else if (black == 10 || urand(0,1))
                {
                    white++;
                    m_creature->SummonCreature(NPC_LIGHT_ORB, x, y, SpawnLoc[1].z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                }
                else
                {
                    black++;
                    m_creature->SummonCreature(NPC_DARK_ORB, x, y, SpawnLoc[1].z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                }
            }
            m_uiSummonTimer = 45000;
        }
        else 
            m_uiSummonTimer -= uiDiff;

        switch (stage)
        {
         case 0:
                timedCast(SPELL_TWIN_SPIKE_H, uiDiff);
                
                if(currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_25MAN_HEROIC)
                {
                    if (timedQuery(SPELL_DARK_TOUCH, uiDiff))
                    {
                        if (Unit* pTarget = doSelectRandomPlayer(SPELL_DARK_ESSENCE, false, 50.0f))
                            doCast(SPELL_DARK_TOUCH, pTarget);
                    }
                }
                if (m_pInstance->GetData(DATA_CASTING_VALKYRS) == SPELL_NONE )
                {
                    if (timedQuery(SPELL_DARK_VORTEX, uiDiff))
                    {
                        m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_DARK_VORTEX);
                        DoScriptText(-1713540,m_creature);
                        stage = 1;
                        m_uiReply = 5;
                        m_uiVortexTimer = 8000;
                        break;
                    }
                    if (timedQuery(GetRightPactId(currentDifficulty, m_creature->GetEntry()), uiDiff) && m_creature->GetHealthPercent() <= 50.0f)
                    {
                        m_creature->InterruptNonMeleeSpells(true);
                        doCast(SPELL_SHIELD_DARK);
                        m_pInstance->SetData(DATA_CASTING_VALKYRS, GetRightPactId(currentDifficulty, m_creature->GetEntry()));
                        DoScriptText(-1713539,m_creature);
                        stage = 3;
                        break;
                    }
                }
                if (m_pInstance->GetData(DATA_CASTING_VALKYRS) == SPELL_TWIN_PACT_20_F || m_pInstance->GetData(DATA_CASTING_VALKYRS) == SPELL_TWIN_PACT_50_F)
                    if (!m_creature->HasAura(SPELL_TWIN_POWER))
                        doCast(SPELL_TWIN_POWER);
                break;
         case 1:
                doCast(SPELL_DARK_VORTEX);
                m_bIsVortex = true;
                stage = 2;
                break;
         case 2:
                if (!m_creature->HasAura(SPELL_DARK_VORTEX) && timedQuery(SPELL_SHIELD_DARK, uiDiff))
                {
                    m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                    stage = 0;
                }
                break;
         case 3:
                DoCast(m_creature, GetRightPactId(currentDifficulty, m_creature->GetEntry()));
                m_uiPactTimer = 15000;
                m_bIsPact = true;
                stage = 4;
                break;
         case 4:
                if (!m_creature->HasAura(SPELL_SHIELD_DARK) && timedQuery(SPELL_SHIELD_DARK, uiDiff))
                {
                    m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                    stage = 0;
                }
         default:
                break;
        }

        timedCast(SPELL_BERSERK, uiDiff);
        
        if(m_bIsPact)
        {
            if (m_uiPactTimer < uiDiff)
            {
                uint16 pct = 0;
                switch (currentDifficulty)
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        pct = 20;
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        pct = 50;
                        break;
                }
                uint32 addhealth = m_creature->GetMaxHealth() * pct / 100;

                SpellEntry const *spell = GetSpellStore()->LookupEntry(GetRightPactId(currentDifficulty, m_creature->GetEntry()));
                m_creature->DealHeal(m_creature, addhealth, spell);
                m_pInstance->SetData(DATA_HEALTH_EYDIS, m_creature->GetHealth());

                Creature* pSis = m_pInstance->GetSingleCreatureFromStorage(NPC_LIGHTBANE);
                if (pSis && pSis->isAlive())
                {
                    pSis->DealHeal(pSis, addhealth, spell);
                    m_pInstance->SetData(DATA_HEALTH_FJOLA, pSis->GetHealth());
                }
                
                pSis->RemoveAurasDueToSpell(SPELL_TWIN_POWER);
                pSis->RemoveAurasDueToSpell(65949);

                m_bIsPact = false;
                m_pInstance->SetData(DATA_CASTING_VALKYRS, SPELL_NONE);
                m_uiPactTimer = 15000;
            }
            else m_uiPactTimer -= uiDiff;
        }

        if(m_bIsVortex)
        {
            if (m_uiVortexTimer < uiDiff)
            {
                Map* pMap = m_creature->GetMap();
                Map::PlayerList const &lPlayers = pMap->GetPlayers();
                for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                {
                    Unit* pPlayer = itr->getSource();
                    if (!pPlayer)
                        continue;
                    if (pPlayer->isAlive() && pPlayer->HasAura(SPELL_DARK_ESSENCE))
                        for(uint8 i=0; i < m_uiIncrease ; i++)
                            pPlayer->CastSpell(pPlayer, GetPoweringId(currentDifficulty), true);
                }
                m_uiVortexTimer = 1000;

                m_uiReply--;
                if (m_uiReply == 0)
                    m_bIsVortex = false;
            }
            else m_uiVortexTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_eydis(Creature* pCreature)
{
    return new boss_eydisAI(pCreature);
}

struct MANGOS_DLL_DECL mob_light_essenceAI : public ScriptedAI
{
    mob_light_essenceAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset() 
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetWalk(true);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance)
            m_creature->ForcedDespawn();

        if (m_pInstance->GetData(TYPE_VALKIRIES) != IN_PROGRESS)
        {
            Map* pMap = m_creature->GetMap();
            Map::PlayerList const &lPlayers = pMap->GetPlayers();
            for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
            {
                Unit* pPlayer = itr->getSource();
                if (!pPlayer)
                    continue;
                if (pPlayer->isAlive())
                    pPlayer->RemoveAurasDueToSpell(SPELL_LIGHT_ESSENCE);
            }

            m_creature->ForcedDespawn();
        }
        return;
    }
};

CreatureAI* GetAI_mob_light_essence(Creature* pCreature)
{
    return new mob_light_essenceAI(pCreature);
};

bool GossipHello_mob_light_essence(Player *player, Creature* pCreature)
{
    ScriptedInstance *pInstance = (ScriptedInstance *) pCreature->GetInstanceData();
    if(!pInstance)
        return true;

    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
    player->RemoveAurasDueToSpell(SPELL_DARK_ESSENCE);
    player->RemoveAurasDueToSpell(GetPoweringId(pCreature->GetMap()->GetDifficulty()));
//        player->CastSpell(player,SPELL_REMOVE_TOUCH,false); // Not worked now
    player->CastSpell(player,SPELL_LIGHT_ESSENCE,false);
    player->RemoveAurasDueToSpell(SPELL_LIGHT_TOUCH); // Override for REMOVE_TOUCH
    player->CLOSE_GOSSIP_MENU();
    return true;
};

struct MANGOS_DLL_DECL mob_dark_essenceAI : public ScriptedAI
{
    mob_dark_essenceAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset() 
    {
        m_creature->SetRespawnDelay(DAY);
        m_creature->SetWalk(true);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance)
            m_creature->ForcedDespawn();

        if (m_pInstance->GetData(TYPE_VALKIRIES) != IN_PROGRESS)
        {
            Map* pMap = m_creature->GetMap();
            Map::PlayerList const &lPlayers = pMap->GetPlayers();
            for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
            {
                Unit* pPlayer = itr->getSource();
                if (!pPlayer)
                    continue;
                if (pPlayer->isAlive())
                    pPlayer->RemoveAurasDueToSpell(SPELL_DARK_ESSENCE);
            }
            m_creature->ForcedDespawn();
        }
        return;
    }
};

CreatureAI* GetAI_mob_dark_essence(Creature* pCreature)
{
    return new mob_dark_essenceAI(pCreature);
};

bool GossipHello_mob_dark_essence(Player *player, Creature* pCreature)
{
    ScriptedInstance *pInstance = (ScriptedInstance *) pCreature->GetInstanceData();
    if(!pInstance)
        return true;
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
    player->RemoveAurasDueToSpell(SPELL_LIGHT_ESSENCE);
    player->RemoveAurasDueToSpell(pCreature->GetMap()->GetDifficulty());
//        player->CastSpell(player,SPELL_REMOVE_TOUCH,false); // Not worked now
    player->CastSpell(player,SPELL_DARK_ESSENCE,false);
    player->RemoveAurasDueToSpell(SPELL_DARK_TOUCH); // Override for REMOVE_TOUCH
    player->CLOSE_GOSSIP_MENU();
    return true;
}

struct MANGOS_DLL_DECL mob_dark_orbAI : public ScriptedAI
{
    mob_dark_orbAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiRangeCheck_Timer;
    uint32 m_uiIncrease;
    uint32 m_uiDespawnDelay;
    Difficulty currentDifficulty;

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        SetCombatMovement(false);
        m_uiDespawnDelay = 45000;
        m_uiRangeCheck_Timer = 500;
        if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL)
            m_uiIncrease = 8;
        else
            m_uiIncrease = 9;

        double offset = urand(0, 360);
        float x = SpawnLoc[1].x + (RAGGIO_ARENA -5) * cos(offset);
        float y = SpawnLoc[1].y + (RAGGIO_ARENA -5) * sin(offset);
        m_creature->GetMotionMaster()->MovePoint(0, x, y, SpawnLoc[1].z);
    }

    void AttackStart(Unit *pWho)
    {
        return;
    }
    
    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        double offset = urand(0, 360);
        float x = SpawnLoc[1].x + (RAGGIO_ARENA -5) * cos(offset);
        float y = SpawnLoc[1].y + (RAGGIO_ARENA -5) * sin(offset);
        m_creature->GetMotionMaster()->MovePoint(0, x, y, SpawnLoc[1].z);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_VALKIRIES) != IN_PROGRESS) 
              m_creature->ForcedDespawn();
 
        if (m_uiRangeCheck_Timer < uiDiff)
        {
            bool bContactPositive = false;
            bool bContact = false;
            Map* pMap = m_creature->GetMap();
            Map::PlayerList const &lPlayers = pMap->GetPlayers();
            for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
            {
                Unit* pPlayer = itr->getSource();
                if (!pPlayer)
                    continue;
                if (pPlayer->isAlive() && pPlayer->IsWithinDistInMap(m_creature, 2.0f))
                {
                    if (pPlayer->HasAura(SPELL_DARK_ESSENCE))
                        bContactPositive = true;
                    bContact = true;
                    break;
                }
            }
            if (bContact)
            {
                if (!bContactPositive)
                    m_creature->CastSpell(m_creature, SPELL_UNLEASHED_DARK, true);

                for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                {
                    Player* pPlayer = itr->getSource();
                    if (!pPlayer)
                        continue;
                    if (pPlayer->isAlive() && pPlayer->IsWithinDistInMap(m_creature, 6.0f))
                    {
                        if (pPlayer->HasAura(SPELL_DARK_ESSENCE))
                            for(uint8 i=0; i < m_uiIncrease ; i++)
                                pPlayer->CastSpell(pPlayer, GetPoweringId(currentDifficulty), true);
                    }
                }
                if (!bContactPositive)
                    m_uiDespawnDelay = 500;
                else
                    m_creature->ForcedDespawn();
            }
            m_uiRangeCheck_Timer = 500;
        }
        else m_uiRangeCheck_Timer -= uiDiff;

        if(m_uiDespawnDelay <= uiDiff)
            m_creature->ForcedDespawn();
        else 
            m_uiDespawnDelay -= uiDiff;

    }

};

CreatureAI* GetAI_mob_dark_orb(Creature *pCreature)
{
    return new mob_dark_orbAI(pCreature);
}

struct MANGOS_DLL_DECL mob_light_orbAI : public ScriptedAI
{
    mob_light_orbAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        currentDifficulty = pCreature->GetMap()->GetDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_uiRangeCheck_Timer;
    uint32 m_uiDespawnDelay;
    uint32 m_uiIncrease;
    Difficulty currentDifficulty;

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        SetCombatMovement(false);
        m_uiDespawnDelay = 45000;
        m_uiRangeCheck_Timer = 500;
        if (currentDifficulty == RAID_DIFFICULTY_10MAN_HEROIC || currentDifficulty == RAID_DIFFICULTY_10MAN_NORMAL)
            m_uiIncrease = 8;
        else
            m_uiIncrease = 9;
        
        double offset = urand(0, 360);
        float x = SpawnLoc[1].x + (RAGGIO_ARENA -5) * cos(offset);
        float y = SpawnLoc[1].y + (RAGGIO_ARENA -5) * sin(offset);
        m_creature->GetMotionMaster()->MovePoint(0, x, y, SpawnLoc[1].z);
    }
    
    void AttackStart(Unit *pWho)
    {
        return;
    }
    
    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        double offset = urand(0, 360);
        float x = SpawnLoc[1].x + (RAGGIO_ARENA -5) * cos(offset);
        float y = SpawnLoc[1].y + (RAGGIO_ARENA -5) * sin(offset);
        m_creature->GetMotionMaster()->MovePoint(0, x, y, SpawnLoc[1].z);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_VALKIRIES) != IN_PROGRESS) 
              m_creature->ForcedDespawn();
 
        if (m_uiRangeCheck_Timer < uiDiff)
        {
            bool bContactPositive = false;
            bool bContact = false;
            Map* pMap = m_creature->GetMap();
            Map::PlayerList const &lPlayers = pMap->GetPlayers();
            for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
            {
                Unit* pPlayer = itr->getSource();
                if (!pPlayer)
                    continue;
                if (pPlayer->isAlive() && pPlayer->IsWithinDistInMap(m_creature, 2.0f))
                {
                    if (pPlayer->HasAura(SPELL_LIGHT_ESSENCE))
                        bContactPositive = true;
                    bContact = true;
                    break;
                }
            }
            if (bContact)
            {
                if (!bContactPositive)
                    m_creature->CastSpell(m_creature, SPELL_UNLEASHED_LIGHT, true);

                for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                {
                    Player* pPlayer = itr->getSource();
                    if (!pPlayer)
                        continue;
                    if (pPlayer->isAlive() && pPlayer->IsWithinDistInMap(m_creature, 6.0f))
                    {
                        if (pPlayer->HasAura(SPELL_LIGHT_ESSENCE))
                            for(uint8 i=0; i < m_uiIncrease ; i++)
                                pPlayer->CastSpell(pPlayer, GetPoweringId(currentDifficulty), true);
                    }
                }
                if (!bContactPositive)
                    m_uiDespawnDelay = 500;
                else
                    m_creature->ForcedDespawn();
            }
            m_uiRangeCheck_Timer = 500;
        }
        else m_uiRangeCheck_Timer -= uiDiff;
        
        if(m_uiDespawnDelay <= uiDiff)
            m_creature->ForcedDespawn();
        else 
            m_uiDespawnDelay -= uiDiff;
    }

};

CreatureAI* GetAI_mob_light_orb(Creature *pCreature)
{
    return new mob_light_orbAI(pCreature);
}

void AddSC_twin_valkyr()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_fjola";
    newscript->GetAI = &GetAI_boss_fjola;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_eydis";
    newscript->GetAI = &GetAI_boss_eydis;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_unleashed_light";
    newscript->GetAI = &GetAI_mob_light_orb;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_unleashed_dark";
    newscript->GetAI = &GetAI_mob_dark_orb;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_light_essence";
    newscript->GetAI = &GetAI_mob_light_essence;
    newscript->pGossipHello = &GossipHello_mob_light_essence;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dark_essence";
    newscript->GetAI = &GetAI_mob_dark_essence;
    newscript->pGossipHello = &GossipHello_mob_dark_essence;
    newscript->RegisterSelf();
}
