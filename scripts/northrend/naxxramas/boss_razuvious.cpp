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
SDName: Boss_Razuvious
SD%Complete: 85%
SDComment: TODO: Timers and sounds need confirmation - orb handling for normal-mode is missing
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1               = -1533120,
    SAY_AGGRO2               = -1533121,
    SAY_AGGRO3               = -1533122,
    SAY_SLAY1                = -1533123,
    SAY_SLAY2                = -1533124,
    SAY_COMMAND1             = -1533125,
    SAY_COMMAND2             = -1533126,
    SAY_COMMAND3             = -1533127,
    SAY_COMMAND4             = -1533128,
    SAY_DEATH                = -1533129,

    SPELL_UNBALANCING_STRIKE = 55470,
    SPELL_DISRUPTING_SHOUT   = 55543,
    SPELL_DISRUPTING_SHOUT_H = 29107,
    SPELL_JAGGED_KNIFE       = 55550,
    SPELL_HOPELESS           = 29125,
    // Cristalls
    SPELL_FORCE_OBEDIENCE    = 55479,

    NPC_DEATH_KNIGHT_UNDERSTUDY = 16803
};

struct MANGOS_DLL_DECL boss_razuviousAI : public ScriptedAI
{
    boss_razuviousAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    instance_naxxramas* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiUnbalancingStrikeTimer;
    uint32 m_uiDisruptingShoutTimer;
    uint32 m_uiJaggedKnifeTimer;
    uint32 m_uiCommandSoundTimer;

    void Reset()
    {
        m_uiUnbalancingStrikeTimer = 30000;                 // 30 seconds
        m_uiDisruptingShoutTimer   = 15000;                 // 15 seconds
        m_uiJaggedKnifeTimer       = urand(10000, 15000);
        m_uiCommandSoundTimer      = 40000;                 // 40 seconds
    }

    void KilledUnit(Unit* Victim)
    {
        if (urand(0, 3))
            return;

        switch(urand(0, 1))
        {
            case 0: DoScriptText(SAY_SLAY1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY2, m_creature); break;
        }
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        DoCastSpellIfCan(m_creature, SPELL_HOPELESS, CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZUVIOUS, DONE);
    }

    void Aggro(Unit* pWho)
    {
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZUVIOUS, IN_PROGRESS);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZUVIOUS, FAIL);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Unbalancing Strike
        if (m_uiUnbalancingStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_UNBALANCING_STRIKE) == CAST_OK)
                m_uiUnbalancingStrikeTimer = 30000;
        }
        else
            m_uiUnbalancingStrikeTimer -= uiDiff;

        // Disrupting Shout
        if (m_uiDisruptingShoutTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, m_bIsRegularMode ? SPELL_DISRUPTING_SHOUT : SPELL_DISRUPTING_SHOUT_H) == CAST_OK)
                m_uiDisruptingShoutTimer = 25000;
        }
        else
            m_uiDisruptingShoutTimer -= uiDiff;

        // Jagged Knife
        if (m_uiJaggedKnifeTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_JAGGED_KNIFE) == CAST_OK)
                    m_uiJaggedKnifeTimer = 10000;
            }
        }
        else
            m_uiJaggedKnifeTimer -= uiDiff;

        // Random say
        if (m_uiCommandSoundTimer < uiDiff)
        {
            switch(urand(0, 3))
            {
                case 0: DoScriptText(SAY_COMMAND1, m_creature); break;
                case 1: DoScriptText(SAY_COMMAND2, m_creature); break;
                case 2: DoScriptText(SAY_COMMAND3, m_creature); break;
                case 3: DoScriptText(SAY_COMMAND4, m_creature); break;
            }

            m_uiCommandSoundTimer = 40000;
        }
        else
            m_uiCommandSoundTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_razuvious(Creature* pCreature)
{
    return new boss_razuviousAI(pCreature);
}

struct MANGOS_DLL_DECL npc_death_knight_understudyAI : public ScriptedAI
{
    npc_death_knight_understudyAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiBoneArmorTimer;
    uint32 m_uiBloodStrikeTimer;

    void Reset()
    {
        m_uiBoneArmorTimer = 1000;
        m_uiBloodStrikeTimer = 7000;
    }

    void Aggro(Unit* pWho)
    {
        m_creature->HandleEmote(0);
        m_creature->CallForHelp(20.0f);            
    }
    
    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiBoneArmorTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, 29061, CAST_AURA_NOT_PRESENT | CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                m_uiBoneArmorTimer = 30000;
        }
        else
            m_uiBoneArmorTimer -= uiDiff;
            
        if (m_uiBloodStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), 61696) == CAST_OK)
                m_uiBloodStrikeTimer = urand(4000, 6000);
        }
        else
            m_uiBloodStrikeTimer -= uiDiff;
            
        DoMeleeAttackIfReady();                        
    }    
};  

CreatureAI* GetAI_npc_death_knight_understudy(Creature* pCreature)
{
    return new npc_death_knight_understudyAI(pCreature);
}

bool GossipHello_npc_obedience_crystal(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance* pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());

    if (!pInstance || pInstance->GetData(TYPE_RAZUVIOUS) == DONE)
        return true;

    if (!pCreature->GetMap()->IsRegularDifficulty())
        return true;

	Creature* pRazuvious = pInstance->GetSingleCreatureFromStorage(NPC_RAZUVIOUS);
    if (!pRazuvious)
        return false;

    pPlayer->RemoveAurasDueToSpell(55520);
    pPlayer->CastSpell(pCreature, SPELL_FORCE_OBEDIENCE, true);
    
    if (!pRazuvious->isInCombat())
        pRazuvious->SetInCombatWithZone();

    std::list<Creature*> lUnitList;
    GetCreatureListWithEntryInGrid(lUnitList, pCreature, NPC_DEATH_KNIGHT_UNDERSTUDY, 100.0f);
    if (lUnitList.empty())
        return true;

    for(std::list<Creature*>::iterator itr = lUnitList.begin(); itr != lUnitList.end(); ++itr)
    {
        if ((*itr)->isAlive() && (*itr)->HasAura(SPELL_FORCE_OBEDIENCE))
        {
            if (!pRazuvious->isInCombat())
            {
                (*itr)->HandleEmote(0);
                pRazuvious->AI()->AttackStart((*itr));
            }
            
            pRazuvious->AddThreat((*itr), 5000.0f);                    

            return true;            
        }    
    }

    return true;
}

void AddSC_boss_razuvious()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_razuvious";
    pNewScript->GetAI = &GetAI_boss_razuvious;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_obedience_crystal";
    pNewScript->pGossipHello =  &GossipHello_npc_obedience_crystal;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_death_knight_understudy";
    pNewScript->GetAI = &GetAI_npc_death_knight_understudy;
    pNewScript->RegisterSelf(); 
}
