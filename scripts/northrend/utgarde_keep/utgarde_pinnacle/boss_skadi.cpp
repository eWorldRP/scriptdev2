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
SDName: Boss_Skadi
SD%Complete: 95%
SDComment: TODO: harpoon spells not working 100% blizzlike
SDCategory: Utgarde Pinnacle
EndScriptData */

#include "precompiled.h"
#include "utgarde_pinnacle.h"

enum
{
    SAY_AGGRO                       = -1575019,
    SAY_DRAKEBREATH_1               = -1575020,
    SAY_DRAKEBREATH_2               = -1575021,
    SAY_DRAKEBREATH_3               = -1575022,
    SAY_DRAKE_HARPOON_1             = -1575023,
    SAY_DRAKE_HARPOON_2             = -1575024,
    SAY_KILL_1                      = -1575025,
    SAY_KILL_2                      = -1575026,
    SAY_KILL_3                      = -1575027,
    SAY_DEATH                       = -1575028,
    SAY_DRAKE_DEATH                 = -1575029,
    EMOTE_HARPOON_RANGE             = -1575030,
    EMOTE_DRAKE_BREATH              = -1575041,

    SPELL_CRUSH                     = 50234,
    SPELL_CRUSH_H                   = 59330,

    SPELL_WHIRLWIND                 = 50228,
    SPELL_WHIRLWIND_H               = 59322,

    SPELL_POISONED_SPEAR            = 50255,
    SPELL_POISONED_SPEAR_H          = 59331,
    SPELL_POISONED                  = 50258,
    SPELL_POISONED_H                = 59334,

    // casted with base of creature 22515 (World Trigger), so we must make sure
    // to use the close one by the door leading further in to instance.
    SPELL_SUMMON_GAUNTLET_MOBS      = 48630,                // tick every 30 sec
    SPELL_SUMMON_GAUNTLET_MOBS_H    = 59275,                // tick every 25 sec

    SPELL_GAUNTLET_PERIODIC         = 47546,                // what is this? Unknown use/effect, but probably related

    SPELL_LAUNCH_HARPOON            = 48642,                // this spell hit drake to reduce HP (force triggered from 48641)
    NPC_DUMMY_TARGET                = 22515,

    MODEL_ID_INVISIBLE              = 11686,

    ITEM_HARPOON                    = 37372,
    SPELL_SUMMON_HARPOON            = 56789,

    GO_HARPOON1                     = 192175,
    GO_HARPOON2                     = 192176,
    GO_HARPOON3                     = 192177,

    ACHIEV_LODI_DODI                = 1873,
    ACHIEV_MY_GIRL_LIKES_TO_SKADI   = 2156, // needs script support

};

uint32 goHarpoons[3] =
{
    GO_HARPOON1,
    GO_HARPOON2,
    GO_HARPOON3
};

struct Locations
{
    float x, y, z;
    int id;
};

static Locations MobSpawnLoc[] =
{
    {340.556f, -511.493f, 104.352f},
    {367.741f, -512.865f, 104.828f},
    {399.546f, -512.755f, 104.834f},
    {430.551f, -514.320f, 105.055f},
    {468.931f, -513.555f, 104.723f}
};

static Locations BreathSpawnLocLeft[] =
{
    {481.4f, -517.6f},
    {471.7f, -517.2f},
    {463.2f, -515.7f},
    {453.6f, -517.2f},
    {443.6f, -519.4f},
    {433.5f, -519.4f},
    {424.8f, -520.9f},
    {415.7f, -518.3f},
    {406.1f, -516.9f},
    {396.7f, -514.9f},
    {387.3f, -515.3f},
    {378.2f, -517.0f},
    {368.6f, -517.9f},
    {358.7f, -517.6f},
    {348.3f, -516.6f},
    {338.0f, -516.4f},
    {327.7f, -515.1f},
    {317.7f, -514.7f},
    {307.1f, -514.9f},
};

#define BREATH_TRIGGER_Z 104.6f
#define OFFSET_RIGHT_X 0.4f
#define OFFSET_RIGHT_Y 8.2f

enum waypoints
{
    END_BREATH  = 0,
    OUTSIDE_1   = 1,
    OUTSIDE_2   = 2,
    OUTSIDE_3   = 3,
    HARPOONS    = 4,
    BREATH_L    = 5,
    BREATH_R    = 6
};

static Locations waypoints[]=
{
    {340.2f, -510.5f, 120.8f, END_BREATH}, 
    {303.3f, -507.4f, 136.8f, OUTSIDE_1},
    {299.8f, -550.2f, 148.1f, OUTSIDE_2},
    {511.9f, -561.3f, 127.9f, OUTSIDE_3},
    {510.2f, -536.2f, 121.1f, HARPOONS},
    {483.5f, -519.8f, 122.9f, BREATH_L},
    {485.3f, -506.3f, 122.0f, BREATH_R}
};

/*######
## boss_skadi
######*/

struct MANGOS_DLL_DECL boss_skadiAI : public ScriptedAI
{
    boss_skadiAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    //Land Phase
    uint32 m_uiCrushTimer;
    uint32 m_uiPoisonedSpearTimer;
    uint32 m_uiWirlwhindTimer;
    bool m_bIsLandPhase;
    
    //Event Phase
    bool m_bIsInHarpoonRange;
    uint32 m_uiIsInHarpoonRangeTimer;

    uint32 m_uiNextWaveTimer;
    uint8 m_uiWaveCounter;

    uint32 m_uiGraufBreathTimer;
    bool m_bGraufBreath;
    uint8 m_uiBreathTrack; 
    uint8 m_uiBreathCount;

    uint32 m_uiEventTimer; // achievement timer
    uint8 m_uiphase;
    uint32 m_uiPlayerCheck_Timer; // check for player death
    uint8 m_uiHarpoonHitsInPass; // count for achievement

    uint8 m_uiHarpoonHits;
    bool m_bAchievement;

    void Reset()
    {
        //Land Phase
        m_uiCrushTimer = urand(5000,10000);
        m_uiPoisonedSpearTimer = urand(5000,10000);
        m_uiWirlwhindTimer = urand(5000,10000);
        m_bIsLandPhase = false;

        //Event Phase
        m_uiGraufBreathTimer = 30000;
        m_uiIsInHarpoonRangeTimer = urand(5000,10000);
        m_uiNextWaveTimer = urand(5000,10000);
        m_uiWaveCounter = 0;
        m_uiBreathCount = 0;
        m_uiphase = 0;
        m_bIsInHarpoonRange = false;
        m_bGraufBreath = false;
        m_uiPlayerCheck_Timer = 1000;
        m_uiHarpoonHits = 0;
        m_uiEventTimer = 0;
        m_uiHarpoonHitsInPass = 0;
        m_bAchievement = false;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->Unmount();

        if(Creature* pGrauf = m_pInstance->GetSingleCreatureFromStorage(NPC_GRAUF))
        {
            pGrauf->Respawn();
            pGrauf->setFaction(14);
            pGrauf->SetVisibility(VISIBILITY_ON);
        }

        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 0);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SKADI, FAIL);
    }

    void HarpoonHit()
    {
        if (m_bIsInHarpoonRange)
        {
            ++m_uiHarpoonHits;
            if (++m_uiHarpoonHitsInPass >= 3) // achievement check, kill grauf(=3 harpoons) in 1 pass
                m_bAchievement = true;
        }
        else 
            return;

        if (m_uiHarpoonHits == 1)
            DoScriptText(SAY_DRAKE_HARPOON_1,m_creature);
        else if (m_uiHarpoonHits == 2)
            DoScriptText(SAY_DRAKE_HARPOON_2,m_creature);
    }

    void AttackStart(Unit* pWho)
    {
        if (m_uiphase < 2)
            return;
        
        if (!pWho || pWho == m_creature)
            return;

        if (m_creature->Attack(pWho, true))
        {
            m_creature->AddThreat(pWho, 0.0f);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
            DoStartMovement(pWho);
        }
    }

    void MoveInLineOfSight(Unit* pWho)
    {
         if (!pWho)
            return;

        if (pWho->isTargetableForAttack() && pWho->isInAccessablePlaceFor(m_creature) && m_creature->IsHostileTo(pWho) &&
        !m_uiphase && pWho->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(pWho, 20))
        {
            if(m_pInstance)
                m_pInstance->SetData(TYPE_SKADI, IN_PROGRESS);

            if(Creature* pGrauf = GetClosestCreatureWithEntry(m_creature,NPC_GRAUF, 20.f))
            {
                pGrauf->setFaction(35);
                pGrauf->SetVisibility(VISIBILITY_OFF);
                pGrauf->DealDamage(pGrauf, pGrauf->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }

            DoScriptText(SAY_AGGRO, m_creature);

            m_creature->SetInCombatWithZone();
            m_creature->Mount(27043);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->SetWalk(false);
            m_creature->SetLevitate(true);
            m_creature->GetMotionMaster()->MovePoint(waypoints[END_BREATH].id, waypoints[END_BREATH].x, waypoints[END_BREATH].y, waypoints[END_BREATH].z);
            m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
            m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
            m_uiphase = 1;
        }
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        if (m_pInstance)
        {
            if (Creature* pGrauf = m_pInstance->GetSingleCreatureFromStorage(NPC_GRAUF))
            {
                m_creature->EnterVehicle(pGrauf->GetVehicleKit(),0);
            }
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_KILL_1, m_creature); break;
            case 1: DoScriptText(SAY_KILL_2, m_creature); break;
            case 2: DoScriptText(SAY_KILL_3, m_creature); break;
        }
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_SKADI, DONE);

        if(!m_bIsRegularMode && m_pInstance)
        {
            if (m_uiEventTimer <= 180000)
                m_pInstance->DoCompleteAchievement(ACHIEV_LODI_DODI);

            if (m_bAchievement)
                m_pInstance->DoCompleteAchievement(ACHIEV_MY_GIRL_LIKES_TO_SKADI);
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if(uiType != POINT_MOTION_TYPE)
                return;

        switch(uiPointId)
        {
            case END_BREATH:
                m_creature->SetSpeedRate(MOVE_FLIGHT, 2.f, true);
                m_bGraufBreath = false;
            case OUTSIDE_1:
            case OUTSIDE_2:
            case OUTSIDE_3:
                m_creature->GetMotionMaster()->MovePoint(++uiPointId,waypoints[uiPointId].x,waypoints[uiPointId].y,waypoints[uiPointId].z); 
                break;
            case HARPOONS: 
                m_uiIsInHarpoonRangeTimer = 8000;
                m_uiHarpoonHitsInPass = 0;
                m_bIsInHarpoonRange = true;
                DoScriptText(EMOTE_HARPOON_RANGE,m_creature);
                break;
            case BREATH_L: 
            case BREATH_R: 
                DoScriptText(EMOTE_DRAKE_BREATH,m_creature);
                switch(urand(0, 2))
                {
                    case 0: DoScriptText(SAY_DRAKEBREATH_1, m_creature); break;
                    case 1: DoScriptText(SAY_DRAKEBREATH_2, m_creature); break;
                    case 2: DoScriptText(SAY_DRAKEBREATH_3, m_creature); break;
                }
                m_creature->SetSpeedRate(MOVE_FLIGHT, 4.0f, true);
                m_creature->GetMotionMaster()->MovePoint(END_BREATH,waypoints[END_BREATH].x,waypoints[END_BREATH].y,waypoints[END_BREATH].z);
                m_uiGraufBreathTimer = 200;
                m_uiBreathCount = 0;
                m_bGraufBreath = true;
                break;
            default: break;
        }
    }

    void SpawnMobs(uint8 uiSpot)
    {
        uint8 maxSpawn = (m_bIsRegularMode ? 4 : 5);
        for(uint8 i = 0; i < maxSpawn; ++i)
        {
            Creature* pTemp;
            switch (rand()%3)
            {
                case 0: pTemp = m_creature->SummonCreature(NPC_YMIRJAR_WARRIOR, MobSpawnLoc[uiSpot].x+rand()%5, MobSpawnLoc[uiSpot].y+rand()%5, MobSpawnLoc[uiSpot].z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000); break;
                case 1: pTemp = m_creature->SummonCreature(NPC_YMIRJAR_WITCH_DOCTOR, MobSpawnLoc[uiSpot].x+rand()%5, MobSpawnLoc[uiSpot].y+rand()%5, MobSpawnLoc[uiSpot].z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000); break;
                case 2: pTemp = m_creature->SummonCreature(NPC_YMIRJAR_HARPOONER, MobSpawnLoc[uiSpot].x+rand()%5, MobSpawnLoc[uiSpot].y+rand()%5, MobSpawnLoc[uiSpot].z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000); break;
            }
            if (pTemp)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    pTemp->AddThreat(pTarget, 0.0f);
                    pTemp->AI()->AttackStart(pTarget);
                }
            }
        }
    }

    void SummonedCreatureJustDied(Creature* pCreature)
    {
        if (pCreature->GetEntry() == NPC_YMIRJAR_HARPOONER)
            if (Creature* pDummyCaster = pCreature->SummonCreature(1921, pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 2000))
            {
                pDummyCaster->SetDisplayId(MODEL_ID_INVISIBLE);
                pDummyCaster->setFaction(35);
                pDummyCaster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                SpellEntry* pTempSpell = const_cast<SpellEntry*>(GetSpellStore()->LookupEntry(SPELL_SUMMON_HARPOON));
                if (pTempSpell)
                {
                    pTempSpell->Effect[EFFECT_INDEX_2] = 0;
                    pDummyCaster->CastSpell(pDummyCaster, pTempSpell, true);
                }
            }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiphase == 0)
             return;
        else
            m_uiEventTimer += uiDiff; //Achievement counter

        // Flying & adds
        if (m_uiphase == 1)
        {
            if (m_uiPlayerCheck_Timer < uiDiff) // check player alive
            {
                Map *map = m_creature->GetMap();
                if (m_pInstance->GetData(TYPE_SKADI) == IN_PROGRESS)
                {
                    Map::PlayerList const &PlayerList = map->GetPlayers();
 
                    if (PlayerList.isEmpty())
                        return;

                    bool bIsAlive = false;
                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        if (i->getSource()->isAlive() && !i->getSource()->isGameMaster())
                         bIsAlive = true;

                    if (!bIsAlive)
                    {
                        m_creature->GetMotionMaster()->Clear(false);
                        m_creature->StopMoving();
                        EnterEvadeMode();
                        return;
                    }
                }
                m_uiPlayerCheck_Timer = 1000;
            } 
            else 
                m_uiPlayerCheck_Timer -= uiDiff;

            if (m_bIsInHarpoonRange)
                if (m_uiIsInHarpoonRangeTimer < uiDiff)
                {
                    m_bIsInHarpoonRange = false;
                    m_uiBreathTrack = urand(BREATH_L,BREATH_R);
                    m_creature->GetMotionMaster()->MovePoint(waypoints[m_uiBreathTrack].id, waypoints[m_uiBreathTrack].x, waypoints[m_uiBreathTrack].y, waypoints[m_uiBreathTrack].z);
                }
                else 
                    m_uiIsInHarpoonRangeTimer -= uiDiff;

            if (m_bGraufBreath)
                if(m_uiGraufBreathTimer < uiDiff)
                {
                    float x = m_uiBreathTrack == BREATH_L ? BreathSpawnLocLeft[m_uiBreathCount].x : BreathSpawnLocLeft[m_uiBreathCount].x + OFFSET_RIGHT_X;
                    float y = m_uiBreathTrack == BREATH_L ? BreathSpawnLocLeft[m_uiBreathCount].y : BreathSpawnLocLeft[m_uiBreathCount].y + OFFSET_RIGHT_Y;
                    if (Creature* pCreature = m_creature->SummonCreature(NPC_FLAME_BREATH_TRIGGER,x,y,BREATH_TRIGGER_Z,0,TEMPSUMMON_TIMED_DESPAWN,15000))
                    {
                        pCreature->setFaction(14);
                        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        pCreature->CastSpell(pCreature,SPELL_FREEZING_CLOUD,true);
                    }

                    if (++m_uiBreathCount > 18)
                        m_bGraufBreath = false;

                    m_uiGraufBreathTimer = 200;
                }
                else
                    m_uiGraufBreathTimer -= uiDiff;

            if(m_uiNextWaveTimer < uiDiff) // spawn mobs
            {
                if (m_uiWaveCounter < 4)
                    ++m_uiWaveCounter;
                SpawnMobs(m_uiWaveCounter);
                m_uiNextWaveTimer = urand(20000,30000);
            }
            else 
                m_uiNextWaveTimer -= uiDiff;

            if (m_uiHarpoonHits >= 3) // start land phase
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->Unmount();
                m_uiphase = 2;
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                DoScriptText(SAY_DRAKE_DEATH, m_creature);
                m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 0);
                m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    AttackStart(pTarget);
            }
        }
        // Land & attack
        else if (m_uiphase == 2)
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;

            if(m_uiPoisonedSpearTimer < uiDiff)
            {
                if(Unit* pPlayer = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pPlayer, m_bIsRegularMode ? SPELL_POISONED_SPEAR : SPELL_POISONED_SPEAR_H, false) == CAST_OK)
                        pPlayer->CastSpell(pPlayer, m_bIsRegularMode ? SPELL_POISONED : SPELL_POISONED_H, true);
                }
                m_uiPoisonedSpearTimer = urand(10000,20000);
            }
            else 
                m_uiPoisonedSpearTimer -= uiDiff;

            if(m_uiCrushTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_CRUSH : SPELL_CRUSH_H, false);
                m_uiCrushTimer = urand(10000,15000);
            }
            else 
                m_uiCrushTimer -= uiDiff;

            if(m_uiWirlwhindTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_WHIRLWIND : SPELL_WHIRLWIND_H, false);
                m_uiWirlwhindTimer = urand(30000,45000);
            }
            else
                m_uiWirlwhindTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_skadi(Creature* pCreature)
{
    return new boss_skadiAI(pCreature);
}

bool AreaTrigger_at_skadi(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_SKADI) == NOT_STARTED)
            pInstance->SetData(TYPE_SKADI, SPECIAL);
    }

    return false;
}

bool GOHello_go_skadi_harpoonlauncher(Player* pPlayer, GameObject* pGo)
{
    if (Creature* pSkadi = GetClosestCreatureWithEntry(pGo, NPC_SKADI, 100))
    {
        if (boss_skadiAI* pSkadiAI = dynamic_cast<boss_skadiAI*>(pSkadi->AI()))
            pSkadiAI->HarpoonHit();
        pPlayer->CastSpell(pSkadi, SPELL_LAUNCH_HARPOON, true);
    }
    return true;
}

// harpoons
bool GOUse_skadi_harpoon(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer)
       if (Item* pItem = pPlayer->StoreNewItemInInventorySlot(ITEM_HARPOON, 1))
       {
           pPlayer->SendNewItem(pItem, 1, true, false);
           pGo->Delete();
       }

    return true;
}

void AddSC_boss_skadi()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_skadi";
    newscript->GetAI = &GetAI_boss_skadi;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_skadi";
    newscript->pAreaTrigger = &AreaTrigger_at_skadi;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_skadi_harpoon";
    newscript->pGOUse=&GOUse_skadi_harpoon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_skadi_harpoonlauncher";
    newscript->pGOUse = &GOHello_go_skadi_harpoonlauncher;
    newscript->RegisterSelf();
}
