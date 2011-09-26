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
SDName: Boss_Faerlina
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_GREET                   = -1533009,
    SAY_AGGRO_1                 = -1533010,
    SAY_AGGRO_2                 = -1533011,
    SAY_AGGRO_3                 = -1533012,
    SAY_AGGRO_4                 = -1533013,
    SAY_SLAY_1                  = -1533014,
    SAY_SLAY_2                  = -1533015,
    SAY_DEATH                   = -1533016,

    EMOTE_BOSS_GENERIC_FRENZY   = -1000005,

    // SOUND_RANDOM_AGGRO       = 8955,                     // soundId containing the 4 aggro sounds, we not using this

    SPELL_POSIONBOLT_VOLLEY     = 28796,
    SPELL_POSIONBOLT_VOLLEY_H   = 54098,
    SPELL_ENRAGE                = 28798,
    SPELL_ENRAGE_H              = 54100,
    SPELL_RAIN_OF_FIRE          = 28794,
    SPELL_RAIN_OF_FIRE_H        = 54099,
    SPELL_WIDOWS_EMBRACE        = 28732,
    SPELL_WIDOWS_EMBRACE_H      = 54097,
    SPELL_FIREBALL              = 54095,
    SPELL_FIREBALL_H            = 54096,
};

struct MANGOS_DLL_DECL boss_faerlinaAI : public ScriptedAI
{
    boss_faerlinaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        m_bHasTaunted = false;
        Reset();
    }

    instance_naxxramas* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiPoisonBoltVolleyTimer;
    uint32 m_uiRainOfFireTimer;
    uint32 m_uiEnrageTimer;
    bool   m_bHasTaunted;

    void Reset()
    {
        m_uiPoisonBoltVolleyTimer = 8000;
        m_uiRainOfFireTimer = 16000;
        m_uiEnrageTimer = 60000;
    }

    void Aggro(Unit* pWho)
    {
        switch(urand(0, 3))
        {
            case 0: DoScriptText(SAY_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO_3, m_creature); break;
            case 3: DoScriptText(SAY_AGGRO_4, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FAERLINA, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_bHasTaunted && m_creature->IsWithinDistInMap(pWho, 60.0f))
        {
            DoScriptText(SAY_GREET, m_creature);
            m_bHasTaunted = true;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY_1 : SAY_SLAY_2, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FAERLINA, DONE);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FAERLINA, FAIL);
    }

    // Widow's Embrace prevents frenzy and poison bolt, if it removes frenzy, next frenzy is sceduled in 60s
    // It is likely that this _should_ be handled with some dummy aura(s) - but couldn't find any
    void SpellHit(Unit* pCaster, const SpellEntry* pSpellEntry)
    {
        // Check if we hit with Widow's Embrave
        if (pSpellEntry->Id == SPELL_WIDOWS_EMBRACE || pSpellEntry->Id == SPELL_WIDOWS_EMBRACE_H)
        {
            bool bIsFrenzyRemove = false;

            // If we remove the Frenzy, the Enrage Timer is reseted to 60s
            if (m_creature->HasAura(m_bIsRegularMode ? SPELL_ENRAGE : SPELL_ENRAGE_H))
            {
                m_uiEnrageTimer = 60000;
                m_creature->RemoveAurasDueToSpell(m_bIsRegularMode ? SPELL_ENRAGE : SPELL_ENRAGE_H);

                pCaster->DealDamage(pCaster, pCaster->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                bIsFrenzyRemove = true;
            }

            // Achievement 'Momma said Knock you out': If we removed OR delayed the frenzy, the criteria is failed
            if ((bIsFrenzyRemove || m_uiEnrageTimer < 30000) && m_pInstance)
                m_pInstance->SetSpecialAchievementCriteria(TYPE_ACHIEV_KNOCK_YOU_OUT, false);

            // In any case we prevent Frenzy and Poison Bolt Volley for Widow's Embrace Duration (30s)
            // We do this be setting the timers to at least bigger than 30s
            m_uiEnrageTimer = std::max(m_uiEnrageTimer, (uint32)30000);
            m_uiPoisonBoltVolleyTimer = std::max(m_uiPoisonBoltVolleyTimer, urand(33000, 38000));
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Poison Bolt Volley
        if (m_uiPoisonBoltVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_POSIONBOLT_VOLLEY : SPELL_POSIONBOLT_VOLLEY_H) == CAST_OK)
                m_uiPoisonBoltVolleyTimer = 11000;
        }
        else
            m_uiPoisonBoltVolleyTimer -= uiDiff;

        // Rain Of Fire
        if (m_uiRainOfFireTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, m_bIsRegularMode ? SPELL_RAIN_OF_FIRE : SPELL_RAIN_OF_FIRE_H) == CAST_OK)
                    m_uiRainOfFireTimer = 16000;
            }
        }
        else
            m_uiRainOfFireTimer -= uiDiff;

        // Enrage Timer
        if (m_uiEnrageTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, m_bIsRegularMode ? SPELL_ENRAGE : SPELL_ENRAGE_H) == CAST_OK)
            {
                DoScriptText(EMOTE_BOSS_GENERIC_FRENZY, m_creature);
                m_uiEnrageTimer = 60000;
            }
        }
        else
            m_uiEnrageTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_faerlina(Creature* pCreature)
{
    return new boss_faerlinaAI(pCreature);
}

struct MANGOS_DLL_DECL mob_worshipperAI : public ScriptedAI
{
    mob_worshipperAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    instance_naxxramas* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiFireballTimer;
    bool   m_bHasTaunted;

    void Reset()
    {
        m_uiFireballTimer = 5000;
    }

    void JustDied(Unit* pKiller)
    {
        if(m_pInstance && m_bIsRegularMode)
        {
			if (Creature* pFaerlina = m_pInstance->GetSingleCreatureFromStorage(NPC_FAERLINA))
            {
                pFaerlina->RemoveAurasDueToSpell(SPELL_ENRAGE);
                pFaerlina->CastSpell(pFaerlina, m_bIsRegularMode ? SPELL_WIDOWS_EMBRACE : SPELL_WIDOWS_EMBRACE_H, true);
            }    
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Only if not charmed
        if(!m_creature->isCharmed())
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;
        
            if (m_uiFireballTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_FIREBALL : SPELL_FIREBALL_H);
                m_uiFireballTimer = 4000 + urand(0,1000);
            }
            else
                m_uiFireballTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_worshipper(Creature* pCreature)
{
    return new mob_worshipperAI(pCreature);
}

void AddSC_boss_faerlina()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_faerlina";
    pNewScript->GetAI = &GetAI_boss_faerlina;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_worshipper";
    pNewScript->GetAI = &GetAI_mob_worshipper;
    pNewScript->RegisterSelf();
}
