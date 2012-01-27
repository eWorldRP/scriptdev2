/* Copyright (C) 2006 - 2012 ScriptDev2 <http://www.scriptdev2.com/>
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
SDName: Boss_Sapphiron
SD%Complete: 0
SDComment: Place Holder
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    EMOTE_BREATH       = -1533082,
    EMOTE_ENRAGE       = -1533083,

    SPELL_ICEBOLT          = 28522,
    SPELL_FROST_BREATH     = 28524,
    SPELL_FROST_AURA       = 28531,
    SPELL_LIFE_DRAIN       = 28542,
    SPELL_LIFE_DRAIN_H     = 55665,
    SPELL_BLIZZARD         = 28547,
    SPELL_BERSERK          = 26662,
    SPELL_CLEAVE           = 19983,
    SPELL_TAIL_LASH        = 55697,
    SPELL_TAIL_LASH_H      = 55696,
    SPELL_ICEBOLT_VISUAL   = 45776,
    SPELL_WING_BUFFET      = 29328,
    
    SAPPHIRON_X            = 3522,
    SAPPHIRON_Y            = -5236,
    SAPPHIRON_Z            = 137   
};

struct MANGOS_DLL_DECL boss_sapphironAI : public ScriptedAI
{
    boss_sapphironAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Icebolt_Number = m_bIsRegularMode ? 2 : 3;
        Reset();
    }

    instance_naxxramas* m_pInstance;
    bool m_bIsRegularMode;

    uint32 Icebolt_Count;
    uint32 Icebolt_Timer;
    uint32 FrostBreath_Timer;
    uint32 FrostAura_Timer;
    uint32 LifeDrain_Timer;
    uint32 Blizzard_Timer;
    uint32 Fly_Timer;
    uint32 Beserk_Timer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiTailSweepTimer;    
    uint32 phase;
    bool landoff;
    uint32 land_Timer;
    ObjectGuid triggerGuid;
    uint32 deIce_Timer;
    bool deIce;
    uint32 Icebolt_Number;
    
    std::list<ObjectGuid> IceBlockedList;

    void Reset()
    {
        FrostAura_Timer = 2000;
        FrostBreath_Timer = 2500;
        LifeDrain_Timer = 24000;
        Blizzard_Timer = 20000;
        Fly_Timer = 45000;
        Icebolt_Timer = 4000;
        land_Timer = 2000;
        Beserk_Timer = 900000;
        m_uiCleaveTimer = 7000;
        m_uiTailSweepTimer = 20000;
        phase = 1;
        Icebolt_Count = 0;
        landoff = false;
        triggerGuid.Clear();
        deIce_Timer = 1000;
        deIce = false;
        SetCombatMovement(true);
        m_creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 15);
        m_creature->SetFloatValue(UNIT_FIELD_COMBATREACH, 15);
        //m_creature->ApplySpellMod(SPELL_FROST_AURA, SPELLMOD_DURATION, -1);
    }

    void Aggro(Unit* pWho)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SAPPHIRON, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SAPPHIRON, DONE);
    }

    void SpellHitTarget(Unit *target, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_ICEBOLT && target->isAlive())
            target->SetInFront(m_creature);
    }

    void DamageDeal(Unit* pDoneTo, uint32& uiDamage) 
    {
        if (pDoneTo->HasAura(SPELL_ICEBOLT))
            uiDamage = 0;
        else if (uiDamage > 50000 && isInBackIceBlock(pDoneTo))
            uiDamage = 0;
    }

    bool isInBackIceBlock(Unit* target)
    {    
        for(std::list<ObjectGuid>::iterator itr = IceBlockedList.begin(); itr != IceBlockedList.end(); ++itr)
        {
            if (Player* pIceBlock = m_creature->GetMap()->GetPlayer(*itr))
            {
                if (pIceBlock->isInBack(target, 20.0f, M_PI_F/1.5f) && pIceBlock->HasAura(SPELL_ICEBOLT))
                    return true;
            }            
        }

        return false;
    }
    
    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SAPPHIRON, FAIL);
        
        if (m_creature->isHover())
            m_creature->SetHover(false);    
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == 16082)
        {
            triggerGuid = pSummoned->GetObjectGuid();
            pSummoned->addUnitState(UNIT_STAT_ROOT);
            DoCastSpellIfCan(pSummoned, 30101, CAST_TRIGGERED);            
        }
    }    
    
    void DoMeleeAttackIfReady()
    {
        //Make sure our attack is ready before checking distance
        if (m_creature->isAttackReady())
        {
            //If we are within range melee the target
            if (m_creature->IsWithinDistInMap(m_creature->getVictim(), 45))
            {
                m_creature->AttackerStateUpdate(m_creature->getVictim());
                m_creature->resetAttackTimer();
            }
        }
    }
    
    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (phase == 1)
        {
            if (deIce)
            {
                if (deIce_Timer < uiDiff)
                {
                    deIce = false;
                    for(std::list<ObjectGuid>::iterator itr = IceBlockedList.begin(); itr != IceBlockedList.end(); ++itr)
                    {
                        if (Player* pIceBlock = m_creature->GetMap()->GetPlayer(*itr))
                        {
                            pIceBlock->RemoveAurasDueToSpell(SPELL_ICEBOLT_VISUAL);
                            pIceBlock->RemoveAurasDueToSpell(SPELL_ICEBOLT);
                        }            
                    }
                    
                    IceBlockedList.clear();
                    deIce_Timer = 1000;
                }
                else
                    deIce_Timer -= uiDiff;
            }        
        
            if (FrostAura_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_FROST_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                FrostAura_Timer = 5000;
            }
            else
                FrostAura_Timer -= uiDiff;

            if (LifeDrain_Timer < uiDiff)
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                    DoCastSpellIfCan(target, m_bIsRegularMode ? SPELL_LIFE_DRAIN : SPELL_LIFE_DRAIN_H);

                LifeDrain_Timer = 24000;
            }
            else
                LifeDrain_Timer -= uiDiff;

            if (Blizzard_Timer < uiDiff)
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(target, SPELL_BLIZZARD);

                Blizzard_Timer = 20000;
            }
            else
                Blizzard_Timer -= uiDiff;
                
            // Cleave
            if (m_uiCleaveTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_CLEAVE, CAST_INTERRUPT_PREVIOUS);
                m_uiCleaveTimer = urand(7000, 10000);
            }
            else
                m_uiCleaveTimer -= uiDiff;

            // Tail Sweep
            if (m_uiTailSweepTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, m_bIsRegularMode ? SPELL_TAIL_LASH : SPELL_TAIL_LASH_H, CAST_INTERRUPT_PREVIOUS);
                m_uiTailSweepTimer = urand(15000, 20000);
            }
            else
                m_uiTailSweepTimer -= uiDiff;                               

            if (m_creature->GetHealthPercent() > 10.0f)
            {
                if (Fly_Timer < uiDiff)
                {
                    phase = 2;
                    m_creature->InterruptNonMeleeSpells(false);
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                    DoCastSpellIfCan(m_creature, SPELL_WING_BUFFET, CAST_TRIGGERED);
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MovePoint(1, SAPPHIRON_X, SAPPHIRON_Y, SAPPHIRON_Z + 5);
                    DoCastSpellIfCan(m_creature, 11010);
                    m_creature->SetHover(true);
                    
                    Icebolt_Timer = 4000;
                    Icebolt_Count = 0;
                    landoff = false;
                    IceBlockedList.clear();
                }
                else
                    Fly_Timer -= uiDiff;
            }
        }

        if (phase == 2)
        {
            if (Icebolt_Timer < uiDiff && Icebolt_Count < Icebolt_Number)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    Player* pPlayer = pTarget->GetCharmerOrOwnerPlayerOrPlayerItself();
                    if (pPlayer && (DoCastSpellIfCan(pPlayer, SPELL_ICEBOLT, CAST_AURA_NOT_PRESENT) == CAST_OK))
                    {
                        IceBlockedList.push_back(pPlayer->GetObjectGuid());
                        ++Icebolt_Count;
                        Icebolt_Timer = 4000;
                        
                        if (Icebolt_Count == Icebolt_Number)
                            DoScriptText(EMOTE_BREATH, m_creature);                    
                    }
                }                
            }
            else
                Icebolt_Timer -= uiDiff;

            if (Icebolt_Count == Icebolt_Number && !landoff)
            {
                if (FrostBreath_Timer < uiDiff)
                {
                    //DoCastSpellIfCan(m_creature, m_bIsRegularMode ? SPELL_FROST_BREATH : SPELL_FROST_BREATH_H, CAST_INTERRUPT_PREVIOUS);                    
                    // workaround effect
                    m_creature->SummonCreature(16082, SAPPHIRON_X, SAPPHIRON_Y, SAPPHIRON_Z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);

                    land_Timer = 9000;
                    landoff = true;
                    FrostBreath_Timer = 5000;
                }
                else
                    FrostBreath_Timer -= uiDiff;
            }

            if (landoff)
            {
                if (land_Timer < uiDiff)
                {
                    phase = 1;
                    if (Creature* pTrigger = m_creature->GetMap()->GetCreature(triggerGuid))
                        pTrigger->CastSpell(pTrigger, SPELL_FROST_BREATH, true, NULL, NULL, m_creature->GetObjectGuid());

                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    m_creature->SetHover(false);
                    m_creature->GetMotionMaster()->Clear(false);                    
                    m_creature->GetMotionMaster()->MovePoint(1, SAPPHIRON_X, SAPPHIRON_Y, SAPPHIRON_Z);
                    SetCombatMovement(false);
                    deIce = true;
                    Fly_Timer = 67000;
                }
                else
                    land_Timer -= uiDiff;
            }
        }
        
        if (Beserk_Timer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BERSERK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT) == CAST_OK)
                DoScriptText(EMOTE_ENRAGE, m_creature);
               
                Beserk_Timer = 5000;
            }
        else
            Beserk_Timer -= uiDiff;

        if (phase != 2)
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_sapphiron(Creature* pCreature)
{
    return new boss_sapphironAI(pCreature);
}

void AddSC_boss_sapphiron()
{
    Script* NewScript;
    NewScript = new Script;
    NewScript->Name = "boss_sapphiron";
    NewScript->GetAI = &GetAI_boss_sapphiron;
    NewScript->RegisterSelf();
}
