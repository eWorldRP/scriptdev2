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
SDName: Boss_Four_Horsemen
SD%Complete: 75
SDComment: Lady Blaumeux, Thane Korthazz, Sir Zeliek, Baron Rivendare
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    //all horsemen
    SPELL_SHIELDWALL        = 29061,
    SPELL_BERSERK           = 26662,

    //lady blaumeux
    SAY_BLAU_AGGRO          = -1533044,
    SAY_BLAU_TAUNT1         = -1533045,
    SAY_BLAU_TAUNT2         = -1533046,
    SAY_BLAU_TAUNT3         = -1533047,
    SAY_BLAU_SPECIAL        = -1533048,
    SAY_BLAU_SLAY           = -1533049,
    SAY_BLAU_DEATH          = -1533050,

    SPELL_MARK_OF_BLAUMEUX  = 28833,
    SPELL_UNYILDING_PAIN    = 57381,
    SPELL_VOIDZONE          = 28863,
    H_SPELL_VOIDZONE        = 57463,
    SPELL_SHADOW_BOLT       = 57374,
    H_SPELL_SHADOW_BOLT     = 57464,

    //baron rivendare
    SAY_RIVE_AGGRO1         = -1533065,
    SAY_RIVE_AGGRO2         = -1533066,
    SAY_RIVE_AGGRO3         = -1533067,
    SAY_RIVE_SLAY1          = -1533068,
    SAY_RIVE_SLAY2          = -1533069,
    SAY_RIVE_SPECIAL        = -1533070,
    SAY_RIVE_TAUNT1         = -1533071,
    SAY_RIVE_TAUNT2         = -1533072,
    SAY_RIVE_TAUNT3         = -1533073,
    SAY_RIVE_DEATH          = -1533074,

    SPELL_MARK_OF_RIVENDARE = 28834,
    SPELL_UNHOLY_SHADOW     = 28882,
    H_SPELL_UNHOLY_SHADOW   = 57369,

    //thane korthazz
    SAY_KORT_AGGRO          = -1533051,
    SAY_KORT_TAUNT1         = -1533052,
    SAY_KORT_TAUNT2         = -1533053,
    SAY_KORT_TAUNT3         = -1533054,
    SAY_KORT_SPECIAL        = -1533055,
    SAY_KORT_SLAY           = -1533056,
    SAY_KORT_DEATH          = -1533057,

    SPELL_MARK_OF_KORTHAZZ  = 28832,
/*    
    SPELL_METEOR            = 26558,
    H_SPELL_METEOR          = 57467,*/
    SPELL_METEOR            = 26553, // spell dbc hack
    H_SPELL_METEOR          = 57451, // spell dbc hack    
    //sir zeliek
    SAY_ZELI_AGGRO          = -1533058,
    SAY_ZELI_TAUNT1         = -1533059,
    SAY_ZELI_TAUNT2         = -1533060,
    SAY_ZELI_TAUNT3         = -1533061,
    SAY_ZELI_SPECIAL        = -1533062,
    SAY_ZELI_SLAY           = -1533063,
    SAY_ZELI_DEATH          = -1533064,

    SPELL_MARK_OF_ZELIEK    = 28835,
    SPELL_HOLY_WRATH        = 28883,
    H_SPELL_HOLY_WRATH      = 57466,
    SPELL_HOLY_BOLT         = 57376,
    H_SPELL_HOLY_BOLT       = 57465,
    SPELL_CONDEMNATION      = 57377,

    // horseman spirits
    NPC_SPIRIT_OF_BLAUMEUX    = 16776,
    NPC_SPIRIT_OF_RIVENDARE   = 0,                          //creature entry not known yet
    NPC_SPIRIT_OF_KORTHAZZ    = 16778,
    NPC_SPIRIT_OF_ZELIREK     = 16777
};

/*walk coords*/
#define WALKX_KORT                2462.112f
#define WALKY_KORT                -2956.598f
#define WALKZ_KORT                241.276f

#define WALKX_BLAU                2529.108f
#define WALKY_BLAU                -3015.303f
#define WALKZ_BLAU                241.32f

#define WALKX_ZELI                2579.571f
#define WALKY_ZELI                -2960.945f
#define WALKZ_ZELI                241.32f
    
#define WALKX_RIVE                2521.039f
#define WALKY_RIVE                -2891.633f
#define WALKZ_RIVE                241.276f

#define HIGH_THREAT               50.0f

struct MANGOS_DLL_DECL boss_horsemen_commonAI : public ScriptedAI
{
    boss_horsemen_commonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        memset(&m_auiHorsemenGUIDs, 0, sizeof(m_auiHorsemenGUIDs));
        
        Reset();
    }
    
    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;
    
    ObjectGuid m_auiHorsemenGUIDs[4];
    
    bool Move_Check;
    bool Attack_Check;
    bool ShieldWall1;
    bool ShieldWall2;
    uint32 Berserk_Timer;
    
    void Reset()
    {
        ShieldWall1 = true;
        ShieldWall2 = true;
        Move_Check = true;
        Attack_Check = true;
        Berserk_Timer = 15*100*IN_MILLISECONDS;
        SetCombatMovement(true);
        
        m_creature->SetWalk(false);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);                 
    }
    
    void AcquireGUIDs()
    {
        if (!m_pInstance)
            return;

        m_auiHorsemenGUIDs[0] = m_pInstance->GetSingleCreatureFromStorage(NPC_ZELIEK)->GetObjectGuid();
        m_auiHorsemenGUIDs[1] = m_pInstance->GetSingleCreatureFromStorage(NPC_THANE)->GetObjectGuid();
        m_auiHorsemenGUIDs[2] = m_pInstance->GetSingleCreatureFromStorage(NPC_BLAUMEUX)->GetObjectGuid();
        m_auiHorsemenGUIDs[3] = m_pInstance->GetSingleCreatureFromStorage(NPC_RIVENDARE)->GetObjectGuid();
    }
    
    Unit *PickNearestPlayer()
    {
        Unit *nearp = NULL;
        float neardist = 0.0f;        
        
        ThreatList const& vThreatList = m_creature->getThreatManager().getThreatList();
        if (vThreatList.empty())
            return NULL;

        for (ThreatList::const_iterator itr = vThreatList.begin();itr != vThreatList.end(); ++itr)
        {
            if (Player* pUnit = m_creature->GetMap()->GetPlayer((*itr)->getUnitGuid()))
            {
                float pudist = pUnit->GetDistance((const Creature *)m_creature);
                if (!nearp || (neardist > pudist))
                {
                    nearp = pUnit;
                    neardist = pudist;
                }
            }
        }
                
        return nearp;
    }  
   
    void Cast(int32 spellId1,int32 spellId2)
    {        
        if (Unit* pTarget = PickNearestPlayer())
        {
            AttackStart(pTarget);
            if (pTarget->IsWithinDist(m_creature, 40.0f))
                DoCastSpellIfCan(pTarget, spellId1);
            else if (spellId2)
                DoCastSpellIfCan(m_creature, spellId2, CAST_TRIGGERED);
        }            
    }
    
    void CheckPosition(float x, float y, float z)
    {
        //run on aggro
        if (m_creature->getVictim() && Move_Check)
        {
        
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->GetMotionMaster()->MovePoint(0, x, y, z);
            Move_Check = false;
            return;
        }    
        //when reach position, set possible to attack
        if ((m_creature->GetDistance2d(x, y) <= 10.0f) && Attack_Check)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            if ((m_creature->GetObjectGuid() == m_auiHorsemenGUIDs[3]) ||(m_creature->GetObjectGuid() == m_auiHorsemenGUIDs[1]))
            {
                SetCombatMovement(true);
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            }
            if ((m_creature->GetObjectGuid() == m_auiHorsemenGUIDs[0]) ||(m_creature->GetObjectGuid() == m_auiHorsemenGUIDs[2]))
                SetCombatMovement(false);
            Attack_Check = false;
        }
    }
    
    void JustReachedHome()
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature* pCreature = m_creature->GetMap()->GetCreature(m_auiHorsemenGUIDs[i]))
            {
                if (pCreature == m_creature)
                    continue; // skip this

                if (!pCreature->isAlive())
                    pCreature->Respawn();
                else if (pCreature->isInCombat())
                    pCreature->AI()->EnterEvadeMode();
            }
        }

        if (m_pInstance && m_pInstance->GetData(TYPE_FOUR_HORSEMEN) == IN_PROGRESS)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, NOT_STARTED);
    }
        
    void Aggro(Unit *who)
    {
        AcquireGUIDs();

        for (uint8 i = 0; i < 4; i++) // respawn other horsemen if needed
            if (Creature * pHorseM = m_creature->GetMap()->GetCreature(m_auiHorsemenGUIDs[i]))
                if ((pHorseM != m_creature) && !(pHorseM->isAlive()))
                    pHorseM->Respawn();

        if (m_pInstance && m_pInstance->GetData(TYPE_FOUR_HORSEMEN) != IN_PROGRESS)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);

        m_creature->AddThreat(who, HIGH_THREAT);
        m_creature->CallForHelp(100.0f);
    }

    void JustDied(Unit* Killer)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);

        if (m_pInstance && m_pInstance->GetData(TYPE_FOUR_HORSEMEN) == SPECIAL)
        {
            bool horsemanDone = true;
            for (uint8 i = 0; i < 4; ++i)
            {
                if (Creature* pCreature = m_creature->GetMap()->GetCreature(m_auiHorsemenGUIDs[i]))
                {
                    if (pCreature == m_creature)
                        continue; // skip this

                    if (pCreature->isAlive())
                    {
                        horsemanDone = false;
                        break;
                    }
                }
            }
            if (horsemanDone)
                m_pInstance->SetData(TYPE_FOUR_HORSEMEN, DONE);
        }
    }
        
    void UpdateAI(const uint32 uiDiff)
    {        
        // Shield Wall - All 4 horsemen will shield wall at 50% hp and 20% hp for 20 seconds
        if (ShieldWall1 && (m_creature->GetHealthPercent() < 50.0f))
        {
                DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL, CAST_INTERRUPT_PREVIOUS);
                ShieldWall1 = false;
        }
        
        if (ShieldWall2 && (m_creature->GetHealthPercent() < 20.0f))
        {
                DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL, CAST_INTERRUPT_PREVIOUS);
                ShieldWall2 = false;
        }

        if (Berserk_Timer < uiDiff)
        {
            if (!m_creature->HasAura(SPELL_BERSERK))
                DoCast(m_creature, SPELL_BERSERK, true);
            
            Berserk_Timer = 5000;    
        }
        else
            Berserk_Timer -= uiDiff;

        DoMeleeAttackIfReady();    
    }    
           
};    

struct MANGOS_DLL_DECL boss_lady_blaumeuxAI : public boss_horsemen_commonAI
{
    boss_lady_blaumeuxAI(Creature* pCreature) : boss_horsemen_commonAI(pCreature)
    {
        Reset();
    }
    
    uint32 Cast_Timer;
    uint32 Mark_Timer;
    uint32 VoidZone_Timer;

    void Reset()
    {
        boss_horsemen_commonAI::Reset();
        Mark_Timer = 20000;                                 // First Horsemen Mark is applied at 20 sec.
        VoidZone_Timer = 12000;                             // right
        Cast_Timer = 9000;
    }

    void Aggro(Unit *pWho)
    {
        DoScriptText(SAY_BLAU_AGGRO, m_creature);        
        boss_horsemen_commonAI::Aggro(pWho);
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(SAY_BLAU_SLAY, m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_BLAU_DEATH, m_creature);   
        boss_horsemen_commonAI::JustDied(Killer);       
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        CheckPosition(WALKX_BLAU, WALKY_BLAU, WALKZ_BLAU);

        // won't attack before reaching the corner
        if (Attack_Check)
            return;

        // Mark of Blaumeux
        if (Mark_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_MARK_OF_BLAUMEUX, true);
            Mark_Timer = 15000;
        }
        else
            Mark_Timer -= uiDiff;            
                    
        if (Cast_Timer < uiDiff)
        {
            boss_horsemen_commonAI::Cast(m_bIsRegularMode ? SPELL_SHADOW_BOLT : H_SPELL_SHADOW_BOLT, SPELL_UNYILDING_PAIN);
            Cast_Timer = 2100; 
        }
        else
            Cast_Timer -= uiDiff;

        // Void Zone
        if (VoidZone_Timer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {            
                DoCastSpellIfCan(pTarget, m_bIsRegularMode ? SPELL_VOIDZONE : H_SPELL_VOIDZONE, CAST_INTERRUPT_PREVIOUS);
                VoidZone_Timer = 12000;
            }
        }
        else
            VoidZone_Timer -= uiDiff;

        boss_horsemen_commonAI::UpdateAI(uiDiff);
    }       
};

CreatureAI* GetAI_boss_lady_blaumeux(Creature* pCreature)
{
    return new boss_lady_blaumeuxAI(pCreature);
}

struct MANGOS_DLL_DECL boss_sir_zeliekAI : public boss_horsemen_commonAI
{
    boss_sir_zeliekAI(Creature* pCreature) : boss_horsemen_commonAI(pCreature)
    {
        Reset();
    }
    
    uint32 Mark_Timer;
    uint32 HolyWrath_Timer;
    uint32 Cast_Timer;

    void Reset()
    {
        boss_horsemen_commonAI::Reset();
        Mark_Timer = 20000;                                 // First Horsemen Mark is applied at 20 sec.
        HolyWrath_Timer = 12000;                            // right
        Cast_Timer = 9000;
    }

    void Aggro(Unit *pWho)
    {
        DoScriptText(SAY_ZELI_AGGRO, m_creature);
        boss_horsemen_commonAI::Aggro(pWho);
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(SAY_ZELI_SLAY, m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_ZELI_DEATH, m_creature);
        boss_horsemen_commonAI::JustDied(Killer);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        CheckPosition(WALKX_ZELI, WALKY_ZELI, WALKZ_ZELI);

        // won't attack before reaching the corner
        if (Attack_Check)
            return;

        // Mark of Zeliek
        if (Mark_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(),SPELL_MARK_OF_ZELIEK, true);
            Mark_Timer = 15000;
        }
        else
            Mark_Timer -= uiDiff;
            
        // Cast
        if (Cast_Timer < uiDiff)
        {
            Cast(m_bIsRegularMode ? SPELL_HOLY_BOLT : H_SPELL_HOLY_BOLT, SPELL_CONDEMNATION);
            Cast_Timer = 2100;
        }
        else
            Cast_Timer -= uiDiff;

        // Holy Wrath
        if (HolyWrath_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),m_bIsRegularMode ? SPELL_HOLY_WRATH : H_SPELL_HOLY_WRATH, CAST_INTERRUPT_PREVIOUS);
            HolyWrath_Timer = 12000;
        }
        else
            HolyWrath_Timer -= uiDiff;

        boss_horsemen_commonAI::UpdateAI(uiDiff);
    }   
};

CreatureAI* GetAI_boss_sir_zeliek(Creature* pCreature)
{
    return new boss_sir_zeliekAI(pCreature);
}

struct MANGOS_DLL_DECL boss_rivendare_naxxAI : public boss_horsemen_commonAI
{
    boss_rivendare_naxxAI(Creature* pCreature) : boss_horsemen_commonAI(pCreature)
    {
        Reset();
    }

    uint32 Mark_Timer;
    uint32 UnholyShadow_Timer;
    
    void Reset()
    {
        boss_horsemen_commonAI::Reset();
        Mark_Timer = 20000;
        UnholyShadow_Timer = 15000;
    }

    void Aggro(Unit *pWho)
    {
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_RIVE_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_RIVE_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_RIVE_AGGRO3, m_creature); break;
        }
        
        boss_horsemen_commonAI::Aggro(pWho);   
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(urand(0, 1) ? SAY_RIVE_SLAY1 : SAY_RIVE_SLAY2, m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_RIVE_DEATH, m_creature);
        boss_horsemen_commonAI::JustDied(Killer);        
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
        
        CheckPosition(WALKX_RIVE, WALKY_RIVE, WALKZ_RIVE);

        // won't attack before reaching the corner
        if (Attack_Check)
            return;

        // Mark of Rivendare
        if (Mark_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_MARK_OF_RIVENDARE, true);
            Mark_Timer = 15000;
        }
        else
            Mark_Timer -= uiDiff;

        if (UnholyShadow_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_UNHOLY_SHADOW : H_SPELL_UNHOLY_SHADOW, CAST_INTERRUPT_PREVIOUS);
            UnholyShadow_Timer = 15000;
        }
        else
            UnholyShadow_Timer -= uiDiff;

        boss_horsemen_commonAI::UpdateAI(uiDiff);
    }
};

CreatureAI* GetAI_boss_rivendare_naxx(Creature* pCreature)
{
    return new boss_rivendare_naxxAI(pCreature);
}

struct MANGOS_DLL_DECL boss_thane_korthazzAI : public boss_horsemen_commonAI
{
    boss_thane_korthazzAI(Creature* pCreature) : boss_horsemen_commonAI(pCreature)
    {
        Reset();
    }

    uint32 Mark_Timer;
    uint32 Meteor_Timer;

    void Reset()
    {
        boss_horsemen_commonAI::Reset();
        Mark_Timer = 20000;                                 // First Horsemen Mark is applied at 20 sec.
        Meteor_Timer = 30000;                               // wrong
    }

    void Aggro(Unit *pWho)
    {
        DoScriptText(SAY_KORT_AGGRO, m_creature);
        boss_horsemen_commonAI::Aggro(pWho);     
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(SAY_KORT_SLAY, m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_KORT_DEATH, m_creature);        
        boss_horsemen_commonAI::JustDied(Killer);    
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        CheckPosition(WALKX_KORT, WALKY_KORT, WALKZ_KORT);

        // won't attack before reaching the corner
        if (Attack_Check)
            return;

        // Mark of Korthazz
        if (Mark_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_MARK_OF_KORTHAZZ, true);
            Mark_Timer = 12000;
        }
        else
            Mark_Timer -= uiDiff;

        // Meteor
        if (Meteor_Timer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {            
                DoCastSpellIfCan(pTarget, m_bIsRegularMode ? SPELL_METEOR : H_SPELL_METEOR, CAST_INTERRUPT_PREVIOUS);
                Meteor_Timer = 20000;                           // wrong
            }
        }
        else
            Meteor_Timer -= uiDiff;

        boss_horsemen_commonAI::UpdateAI(uiDiff);
    }
};

CreatureAI* GetAI_boss_thane_korthazz(Creature* pCreature)
{
    return new boss_thane_korthazzAI(pCreature);
}

struct MANGOS_DLL_DECL npc_void_zoneAI : public Scripted_NoMovementAI
{
    npc_void_zoneAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        Reset();
    }

    uint32 Effect_Timer;
    uint32 Despawn_Timer;

    void Reset()
    {
        Effect_Timer = 1000;
        Despawn_Timer = 75000;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (Effect_Timer < uiDiff)
        {
            if (!m_creature->HasAura(46262))
                DoCast(m_creature, 46262, true);
            
            Effect_Timer = 2000;
        }
        else
            Effect_Timer -= uiDiff;
            
        if (Despawn_Timer < uiDiff)
        {
            m_creature->ForcedDespawn();
        }
        else
            Despawn_Timer -= uiDiff;            
    }
};

CreatureAI* GetAI_npc_void_zone(Creature* pCreature)
{
    return new npc_void_zoneAI(pCreature);
}

void AddSC_boss_four_horsemen()
{
    Script* NewScript;

    NewScript = new Script;
    NewScript->Name = "boss_lady_blaumeux";
    NewScript->GetAI = &GetAI_boss_lady_blaumeux;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "boss_rivendare_naxx";
    NewScript->GetAI = &GetAI_boss_rivendare_naxx;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "boss_thane_korthazz";
    NewScript->GetAI = &GetAI_boss_thane_korthazz;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "boss_sir_zeliek";
    NewScript->GetAI = &GetAI_boss_sir_zeliek;
    NewScript->RegisterSelf();
    
    NewScript = new Script;
    NewScript->Name = "npc_void_zone";
    NewScript->GetAI = &GetAI_npc_void_zone;
    NewScript->RegisterSelf();    
}
