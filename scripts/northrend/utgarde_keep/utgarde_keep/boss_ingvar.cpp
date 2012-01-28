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
SDName: Boss_Ingvar_The_Plunderer
SD%Complete: 95
SDComment: Some Problems with Annhylde Movement, Blizzlike Timers
SDCategory: Udgarde Keep
EndScriptData */

#include "precompiled.h"
#include "utgarde_keep.h"

enum eEnums
{
    //Achievements
    ACHIEVEMENT_UTGARDE_KEEP_N                  = 477,
    ACHIEVEMENT_UTGARDE_KEEP_H                  = 489,

    //Yells Ingvar
    YELL_AGGRO_1                                = -1574005,
    YELL_AGGRO_2                                = -1574006,

    YELL_DEAD_1                                 = -1574007,
    YELL_DEAD_2                                 = -1574008,

    YELL_KILL_1                                 = -1574009,
    YELL_KILL_2                                 = -1574010,

    //Ingvar Spells human form
    MOB_INGVAR_HUMAN                            = 23954,
    SPELL_CLEAVE                                = 42724,
    SPELL_SMASH                                 = 42669,
    H_SPELL_SMASH                               = 59706,
    SPELL_STAGGERING_ROAR                       = 42708,
    H_SPELL_STAGGERING_ROAR                     = 59708,
    SPELL_ENRAGE                                = 42705,
    H_SPELL_ENRAGE                              = 59707,

    MOB_ANNHYLDE_THE_CALLER                     = 24068,
    SPELL_INGVAR_FEIGN_DEATH                    = 42795,
    SPELL_SUMMON_BANSHEE                        = 42912,
    SPELL_SCOURG_RESURRECTION_EFFEKTSPAWN       = 42863, //Spawn resurrecteffekt around Ingvar

    MODEL_INGVAR_UNDEAD                         = 26351,
    MODEL_INGVAR_HUMAN                          = 21953,

    //Ingvar Spells undead form
    MOB_INGVAR_UNDEAD                           = 23980,
    SPELL_DARK_SMASH                            = 42723,
    SPELL_DREADFUL_ROAR                         = 42729,
    H_SPELL_DREADFUL_ROAR                       = 59734,
    SPELL_WOE_STRIKE                            = 42730,
    H_SPELL_WOE_STRIKE                          = 59735,

    ENTRY_THROW_TARGET                          = 23996,
    SPELL_SHADOW_AXE_SUMMON                     = 42749
};

struct MANGOS_DLL_DECL boss_ingvarAI : public ScriptedAI
{
    boss_ingvarAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    bool undead;
    bool event_inProgress;

    uint32 Cleave_Timer;
    uint32 Smash_Timer;
    uint32 Enrage_Timer;
    uint32 Roar_Timer;
    uint32 SpawnResTimer;
    uint32 wait_Timer;

    void Reset()
    {
        if (undead) // Visual Hack
            m_creature->SetDisplayId(MODEL_INGVAR_HUMAN);

        undead = false;
        event_inProgress = false;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        Cleave_Timer = 2000;
        Smash_Timer = 5000;
        Enrage_Timer = 10000;
        Roar_Timer = 15000;

        SpawnResTimer = 3000;

        wait_Timer = 0;

        if (m_pInstance)
            m_pInstance->SetData(TYPE_INGVAR, NOT_STARTED);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (damage >= m_creature->GetHealth() && !undead)
        {
            //DoCast(m_creature, SPELL_INGVAR_FEIGN_DEATH, true);  // Dont work ???
            // visuel hack
            m_creature->SetHealth(1);
            m_creature->InterruptNonMeleeSpells(true);
            m_creature->RemoveAllAuras();
            m_creature->SetDisplayId(MODEL_INGVAR_UNDEAD);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
            m_creature->GetMotionMaster()->MovementExpired(false);
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
            // visuel hack end

            event_inProgress = true;
            undead = true;

            DoScriptText(YELL_DEAD_1,m_creature);
        }

        if (event_inProgress)
        {
            damage = 0;
        }
    }

    void StartZombiePhase()
    {
        undead = true;
        event_inProgress = false;
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
        m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
        m_creature->AI()->AttackStart(m_creature->getVictim());

        DoScriptText(YELL_AGGRO_2,m_creature);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO_1,m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_INGVAR, IN_PROGRESS);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(YELL_DEAD_2,m_creature);

        // complete burg utgarde achievements, not really blizzlike (should complete criterias)
        // normal achievement on both difficulties
        AchievementEntry const *AchievUtgardeKeepNorm = GetAchievementStore()->LookupEntry(ACHIEVEMENT_UTGARDE_KEEP_N);
        if (AchievUtgardeKeepNorm)
        {
            Map* pMap = m_creature->GetMap();
            if (pMap && pMap->IsDungeon())
            {
                Map::PlayerList const &players = pMap->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                itr->getSource()->CompletedAchievement(AchievUtgardeKeepNorm);
            }
        }
        if(!m_bIsRegularMode)
        {
            AchievementEntry const *AchievUtgardeKeepHero = GetAchievementStore()->LookupEntry(ACHIEVEMENT_UTGARDE_KEEP_H);
            if (AchievUtgardeKeepHero)
            {
                Map* pMap = m_creature->GetMap();
                if (pMap && pMap->IsDungeon())
                {
                    Map::PlayerList const &players = pMap->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    itr->getSource()->CompletedAchievement(AchievUtgardeKeepHero);
                }
            }
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_INGVAR, DONE);
    }

    void KilledUnit(Unit *victim)
    {
        if (undead) { DoScriptText(YELL_KILL_1,m_creature); }
        else { DoScriptText(YELL_KILL_2,m_creature); }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (event_inProgress)
        {
            if (SpawnResTimer)
                if (SpawnResTimer <= diff)
                {
                    DoCast(m_creature, SPELL_SUMMON_BANSHEE); // Summons direktly on caster position
                    //DoCast(m_creature, SPELL_SCOURG_RESURRECTION_EFFEKTSPAWN); // Dont needet ?
                    SpawnResTimer = 0;
                } else SpawnResTimer -= diff;

            return;
        }

        // This is used for a spell queue ... the spells should not castet if one spell is already casting
        if (wait_Timer)
            if (wait_Timer <= diff)
            {
                wait_Timer = 0;
            } else wait_Timer -= diff;

        if (Cleave_Timer <= diff)
        {
            if (!wait_Timer)
            {
                if (undead)
                    DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_WOE_STRIKE : H_SPELL_WOE_STRIKE);
                else
                    DoCast(m_creature->getVictim(), SPELL_CLEAVE);
                Cleave_Timer = rand()%5000 + 2000;

                wait_Timer = 1000;
            }
        } else Cleave_Timer -= diff;

        if (Smash_Timer <= diff)
        {
            if (!wait_Timer)
            {
                if (undead)
                    DoCast(m_creature->getVictim(), SPELL_DARK_SMASH);
                else
                    DoCast(m_creature->getVictim(), m_bIsRegularMode ? SPELL_SMASH : H_SPELL_SMASH);
                Smash_Timer = 10000;

                wait_Timer = 5000;
            }
        } else Smash_Timer -= diff;

        if (!undead)
        {
            if (Enrage_Timer <= diff)
            {
                DoCast(m_creature, m_bIsRegularMode ? SPELL_ENRAGE : H_SPELL_ENRAGE);
                Enrage_Timer = 10000;
            } else Enrage_Timer -= diff;
        } else // In Undead form used to summon weapon
        {
            if (Enrage_Timer <= diff)
            {
                if (!wait_Timer)
                {
                    // Spawn target for Axe
                    Unit *pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 1);
                    if (pTarget)
                    {
                        Creature* temp = m_creature->SummonCreature(ENTRY_THROW_TARGET,pTarget->GetPositionX(),pTarget->GetPositionY(),pTarget->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,2000);

                        DoCast(m_creature, SPELL_SHADOW_AXE_SUMMON);
                    }
                    Enrage_Timer = 30000;
                }
            } else Enrage_Timer -= diff;
        }

        if (Roar_Timer <= diff)
        {
            if (!wait_Timer)
            {
                if (undead)
                    DoCast(m_creature, m_bIsRegularMode ? SPELL_DREADFUL_ROAR : H_SPELL_DREADFUL_ROAR);
                else
                    DoCast(m_creature, m_bIsRegularMode ? SPELL_STAGGERING_ROAR : H_SPELL_STAGGERING_ROAR);
                Roar_Timer = 10000;

                wait_Timer = 5000;
            }
        } else Roar_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ingvar(Creature* pCreature)
{
    return new boss_ingvarAI (pCreature);
}

enum eSpells
{
    //we don't have that text in db so comment it until we get this text
    //    YELL_RESSURECT                      = -1574025,

    //Spells for Annhylde
    SPELL_SCOURG_RESURRECTION_HEAL              = 42704, //Heal Max + DummyAura
    SPELL_SCOURG_RESURRECTION_BEAM              = 42857, //Channeling Beam of Annhylde
    SPELL_SCOURG_RESURRECTION_DUMMY             = 42862, //Some Emote Dummy?
    SPELL_INGVAR_TRANSFORM                      = 42796
};

struct MANGOS_DLL_DECL mob_annhylde_the_callerAI : public ScriptedAI
{
    mob_annhylde_the_callerAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    float x,y,z;
    uint32 Resurect_Timer;
    uint32 Resurect_Phase;

    void Reset()
    {
        m_creature->SetLevitate(true);
        m_creature->SetSpeedRate(MOVE_SWIM , 1.0f);
        m_creature->SetSpeedRate(MOVE_RUN , 1.0f);
        m_creature->SetSpeedRate(MOVE_WALK , 1.0f);
        //m_creature->SetSpeed(MOVE_FLIGHT , 1.0f);

        m_creature->GetPosition(x,y,z);
        m_creature->NearTeleportTo(x+1,y,z+30,0);

        Unit* ingvar = m_pInstance->GetSingleCreatureFromStorage(NPC_INGVAR);
        if (ingvar)
        {
            m_creature->GetMotionMaster()->MovePoint(1,x,y,z+15);

            //DoScriptText(YELL_RESSURECT,m_creature);
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;
        Unit* ingvar = m_pInstance ? m_pInstance->GetSingleCreatureFromStorage(NPC_INGVAR));
        if (ingvar)
        {
            switch (id)
            {
                case 1:
                    ingvar->RemoveAura(SPELL_SUMMON_BANSHEE, EFFECT_INDEX_0);
                    ingvar->CastSpell(ingvar,SPELL_SCOURG_RESURRECTION_DUMMY,true);
                    DoCast(ingvar, SPELL_SCOURG_RESURRECTION_BEAM);
                    Resurect_Timer = 8000;
                    Resurect_Phase = 1;
                    break;
                case 2:
                    m_creature->DealDamage(m_creature,m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    m_creature->RemoveCorpse();
                    break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Resurect_Timer)
            if (Resurect_Timer <= diff)
            {
                if (Resurect_Phase == 1)
                {
                    Unit* ingvar = m_pInstance->GetSingleCreatureFromStorage(NPC_INGVAR);
                    if (ingvar)
                    {
                        ingvar->SetStandState(UNIT_STAND_STATE_STAND);
                        ingvar->CastSpell(ingvar,SPELL_SCOURG_RESURRECTION_HEAL,false);
                    }
                    Resurect_Timer = 3000;
                    Resurect_Phase = 2;
                } else if (Resurect_Phase == 2)
                {
                    // DA RIVEDERE E CORREGGERE
                    Creature* ingvar = m_pInstance->GetSingleCreatureFromStorage(NPC_INGVAR);
                    if (ingvar)
                    {
                        ingvar->RemoveAurasDueToSpell(SPELL_SCOURG_RESURRECTION_DUMMY);
                        //ingvar->CastSpell(ingvar,SPELL_INGVAR_TRANSFORM,false);
                        //ingvar->SetDisplayId(MODEL_INGVAR_UNDEAD); // Visual Hack - when he dies he becomes human model -> wrong
                        if (Creature* c_ingvar = ingvar)
                            if (boss_ingvarAI* pIngvar = dynamic_cast<boss_ingvarAI*>(c_ingvar->AI()))
                                pIngvar->StartZombiePhase();

                        m_creature->GetMotionMaster()->MovePoint(2,x+1,y,z+30);
                        ++Resurect_Phase;
                    }
                }

            } else Resurect_Timer -= diff;
    }
};

CreatureAI* GetAI_mob_annhylde_the_caller(Creature* pCreature)
{
    return new mob_annhylde_the_callerAI (pCreature);
}

enum eShadowAxe
{
    SPELL_SHADOW_AXE_DAMAGE                     = 42750,
    H_SPELL_SHADOW_AXE_DAMAGE                   = 59719
};

struct MANGOS_DLL_DECL mob_ingvar_throw_dummyAI : public ScriptedAI
{
    mob_ingvar_throw_dummyAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 Despawn_Timer;

    void Reset()
    {
        Unit *pTarget = GetClosestCreatureWithEntry(m_creature, ENTRY_THROW_TARGET,50);
        if (pTarget)
        {
            DoCast(m_creature, m_bIsRegularMode ? SPELL_SHADOW_AXE_DAMAGE : H_SPELL_SHADOW_AXE_DAMAGE);
            float x,y,z;
            pTarget->GetPosition(x,y,z);
            m_creature->GetMotionMaster()->MovePoint(0,x,y,z);
        }
        Despawn_Timer = 7000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (Despawn_Timer <= diff)
        {
            m_creature->DealDamage(m_creature,m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();
            Despawn_Timer = 0;
        } else Despawn_Timer -= diff;
    }
};

CreatureAI* GetAI_mob_ingvar_throw_dummy(Creature* pCreature)
{
    return new mob_ingvar_throw_dummyAI (pCreature);
}

void AddSC_boss_ingvar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_ingvar";
    newscript->GetAI = &GetAI_boss_ingvar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_annhylde";
    newscript->GetAI = &GetAI_mob_annhylde_the_caller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ingvar_throw_dummy";
    newscript->GetAI = &GetAI_mob_ingvar_throw_dummy;
    newscript->RegisterSelf();
}
