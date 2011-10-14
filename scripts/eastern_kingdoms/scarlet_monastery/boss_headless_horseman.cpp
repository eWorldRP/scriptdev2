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
SDName: boss_headless_horseman
SD%Complete: 95%
SDComment: by Enturion's Staff
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#include "precompiled.h"
#include "scarlet_monastery.h"

enum
{
    SAY_ENTRANCE           = -1189022,
    SAY_REJOINED           = -1189023,
    SAY_BODY_DEFEAT        = -1189024,
    SAY_LOST_HEAD          = -1189025,
    SAY_CONFLAGRATION      = -1189026,
    SAY_SPROUTING_PUMPKINS = -1189027,
    SAY_SLAY               = -1189028,
    SAY_DEATH              = -1189029,

    EMOTE_LAUGH            = -1189030,

    SAY_PLAYER1            = -1189031,
    SAY_PLAYER2            = -1189032,
    SAY_PLAYER3            = -1189033,
    SAY_PLAYER4            = -1189034,

    //Spells
    SPELL_CLEAVE            = 42587,
    SPELL_CONFLAGRATION     = 42380,        //Phase 2, can't find real spell(Dim Fire?)
    SPELL_SUMMON_PUMPKIN    = 42394,        //summon 4 pumpkin or 52236, Phase 3(requres summon_spells.patch - TARGET_EFFECT_SELECT not implemened in core)

    SPELL_WHIRLWIND         = 43116,        //or  ?41194
    SPELL_CLEAVE_PROC       = 42587,        //procs after "killing" body(at each end of phase or when he really dies?)
    SPELL_IMMUNE            = 42556,
    SPELL_BODY_REGEN        = 42403,        //regenerates 4% of total hp per 1sec;not used, because this, body regen and whirlwind non stackable with each other
    SPELL_BODY_CONFUSE      = 43105,        //confused movement

    SPELL_FLYING_HEAD       = 42399,        //visual flying head
    SPELL_HEAD              = 42413,        //visual buff, "head"
    SPELL_HEAD_IS_DEAD      = 42428,        //at killing head, Phase 3
    SPELL_HEAD_INVIS        = 44312,        //not used

    SPELL_PUMPKIN_AURA       = 42280,
    SPELL_PUMPKIN_AURA_GREEN = 42294,
    SPELL_PUMPKIN_DEATH      = 42291,
    SPELL_SQUASH_SOUL        = 42514,        //pumpkin debuff
    SPELL_SPROUTING          = 42281,        //process bar + heal
    SPELL_SPROUT_BODY        = 42285,        //transforms itself into fiend

    //Effects
    SPELL_RHYME_BIG          = 42909,        //summoning rhyme
    SPELL_RHYME_SMALL        = 42910,
    SPELL_HEAD_SPEAKS        = 43129,
    SPELL_HEAD_LANDS         = 42400,
    SPELL_BODY_FLAME         = 42074,
    SPELL_ON_KILL_PROC       = 43877,        //procs after killing players?
    SPELL_ENRAGE_VISUAL      = 42438,        // he uses this spell?
    SPELL_WISP_BLUE          = 42821,
    SPELL_WISP_FLIGHT_PORT   = 42818,
    SPELL_SMOKE              = 42355,
    SPELL_DEATH              = 42566,
    SPELL_LAUGH1             = 43881,
    SPELL_LAUGH2             = 43894,
    SPELL_LAUGH3             = 43885,
    SPELL_CREATE_TREATS      = 42754
};

struct Locations
{
    float x, y, z;
};

static Locations FlightPoint[]=
{
    {1751.00f,1347.00f,19.00f},
    {1765.00f,1347.00f,19.00f},
    {1784.00f,1346.80f,25.40f},
    {1803.30f,1347.60f,33.00f},
    {1824.00f,1350.00f,42.60f},
    {1838.80f,1353.20f,49.80f},
    {1852.00f,1357.60f,55.70f},
    {1861.30f,1364.00f,59.40f},
    {1866.30f,1374.80f,61.70f},
    {1864.00f,1387.30f,63.20f},
    {1854.80f,1399.40f,64.10f},
    {1844.00f,1406.90f,64.10f},
    {1824.30f,1411.40f,63.30f},
    {1801.00f,1412.30f,60.40f},
    {1782.00f,1410.10f,55.50f},
    {1770.50f,1405.20f,50.30f},
    {1765.20f,1400.70f,46.60f},
    {1761.40f,1393.40f,41.70f},
    {1759.10f,1386.70f,36.60f},
    {1757.80f,1378.20f,29.00f},
    {1758.00f,1367.00f,19.51f},
    {1784.00f,1346.80f,25.40f},
    {1797.00f,1341.70f,18.89f}
};

static Locations Spawn[]=
{
    {1776.27f,1348.74f,19.20f},    //spawn point for pumpkin shrine mob
    {1765.28f,1347.46f,17.55f}     //spawn point for smoke
};

struct Summon
{
    const char* text;
};

static Summon Text[]=
{
    {"Horseman rise..."},
    {"Your time is nigh..."},
    {"You felt death once..."},
    {"Now, know demise!"}
};

/*#####
# Wisp Invis
#####*/
struct MANGOS_DLL_DECL mob_wisp_invisAI : public ScriptedAI
{
    mob_wisp_invisAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_uiCreaturetype = m_uiDelay = m_uiSpell = m_uiSpell2 = 0;
        //that's hack but there are no info about range of this spells in dbc
        SpellEntry *pWisp = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_WISP_BLUE);
        if (pWisp)
            pWisp->rangeIndex = 6; //100 yards
        SpellEntry *pPort = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_WISP_FLIGHT_PORT);
        if (pPort)
            pPort->rangeIndex = 6;

        Reset();
    }

    uint32 m_uiCreaturetype;
    uint32 m_uiDelay;
    uint32 m_uiSpell;
    uint32 m_uiSpell2;

    void Reset()
    {
    }

    void EnterCombat(Unit *pWho)
    {
    }

    void SetType(uint32 _type)
    {
        switch(m_uiCreaturetype = _type)
        {
            case 1:
                m_uiSpell = SPELL_PUMPKIN_AURA_GREEN;
                break;
            case 2:
                m_uiDelay = 15000;
                m_uiSpell = SPELL_BODY_FLAME;
                m_uiSpell2 = SPELL_DEATH;
                break;
            case 3:
                m_uiDelay = 15000;
                m_uiSpell = SPELL_SMOKE;
                break;
            case 4:
                m_uiDelay = 7000;
                m_uiSpell2 = SPELL_WISP_BLUE;
                break;
        }
        if (m_uiSpell)
            DoCast(m_creature, m_uiSpell);
    }

    void SpellHit(Unit* pCaster, const SpellEntry *pSpell)
    {
        if (pSpell->Id == SPELL_WISP_FLIGHT_PORT && m_uiCreaturetype == 4)
            DoSpawnCreature(NPC_SIR_THOMAS, 1, 1, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 240000);
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (!pWho || m_uiCreaturetype != 1 || !pWho->isTargetableForAttack())
            return;

        if (m_creature->IsWithinDist(pWho, 0.1f, false) && !pWho->HasAura(SPELL_SQUASH_SOUL))
            DoCast(pWho, SPELL_SQUASH_SOUL);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiDelay)
        {
            if (m_uiDelay <= uiDiff)
            {
                m_creature->RemoveAurasDueToSpell(SPELL_SMOKE);
                if (m_uiSpell2)
                    DoCastSpellIfCan(m_creature, m_uiSpell2);
                m_uiDelay = 0;
            }
            else
                m_uiDelay -= uiDiff;
        }
    }
};

/*#####
# Horseman's Head
#####*/
struct MANGOS_DLL_DECL mob_headAI : public ScriptedAI
{
    mob_headAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    ObjectGuid m_uiBodyguid;

    uint32 m_uiPhase;
    uint32 m_uiLaugh;
    uint32 m_uiWait;

    bool m_bWithBody;
    bool m_bDie;

    void Reset()
    {
        m_uiPhase = 0;
        m_uiBodyguid = 0;
        m_bDie = false;
        m_bWithBody = true;
        m_uiWait = 1000;
        m_uiLaugh = urand(15000, 30000);
    }

    void EnterCombat(Unit *pWho)
    {
    }

    void SaySound(int32 uiTextEntry, Unit *pTarget = 0)
    {
        DoScriptText(uiTextEntry, m_creature, pTarget);
        //DoCast(m_creature,SPELL_HEAD_SPEAKS,true);
        Creature *pSpeaker = DoSpawnCreature(NPC_HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 1000);
        if (pSpeaker)
            pSpeaker->CastSpell(pSpeaker, SPELL_HEAD_SPEAKS, false);

        m_uiLaugh += 3000;
    }

    void Disappear();

    void DamageTaken(Unit* pDoneBy,uint32 &uiDamage)
    {
        if (m_bWithBody)
            return;

        switch(m_uiPhase)
        {
            case 1:
                if (((m_creature->GetHealth() - uiDamage)*100)/m_creature->GetMaxHealth() < 67)
                {
                    m_creature->SetHealth(m_creature->GetMaxHealth()*67/100);
                    uiDamage = 0;
                    Disappear();
                }
                break;
            case 2:
                if (((m_creature->GetHealth() - uiDamage)*100)/m_creature->GetMaxHealth() < 34)
                {
                    m_creature->SetHealth(m_creature->GetMaxHealth()*34/100);
                    uiDamage = 0;
                    Disappear();
                }
                break;
            case 3:
                if (uiDamage >= m_creature->GetHealth())
                {
                    Disappear();
                    m_bDie = true;
                    m_bWithBody = true;
                    m_uiWait = 300;
                    uiDamage = 0;
                    m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    m_creature->StopMoving();
                    //m_creature->GetMotionMaster()->MoveIdle();
                    DoCast(m_creature, SPELL_HEAD_IS_DEAD);
                }
                break;
        }
    }

    void SpellHit(Unit *pCaster, const SpellEntry* pSpell)
    {
        if (!m_bWithBody)
            return;

        if (pSpell->Id == SPELL_FLYING_HEAD)
        {
            if (m_uiPhase < 3)
                ++m_uiPhase;
            else
                m_uiPhase = 3;

            m_bWithBody = false;

            if (!m_uiBodyguid)
                m_uiBodyguid = pCaster->GetObjectGuid();

            m_creature->RemoveAllAuras();
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoCast(m_creature, SPELL_HEAD_LANDS, true);
            DoCast(m_creature, SPELL_HEAD, false);
            SaySound(SAY_LOST_HEAD);
            m_creature->GetMotionMaster()->Clear(false);
            if (pCaster->getVictim())
                m_creature->GetMotionMaster()->MoveFleeing(pCaster->getVictim());
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_bWithBody)
        {
            if (m_uiWait < uiDiff)
            {
                m_uiWait = 5000;
                if (!m_creature->getVictim())
                    return;

                m_creature->GetMotionMaster()->Clear(false);
                m_creature->GetMotionMaster()->MoveFleeing(m_creature->getVictim());
            }
            else
                m_uiWait -= uiDiff;

            if (m_uiLaugh < uiDiff)
            {
                m_uiLaugh = urand(15000, 30000);
                //DoCast(m_creature,SPELL_HEAD_SPEAKS,true); //this pSpell remove buff "head"
                Creature *speaker = DoSpawnCreature(NPC_HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 1000);
                if (speaker)
                    speaker->CastSpell(speaker, SPELL_HEAD_SPEAKS, false);

                DoScriptText(EMOTE_LAUGH, m_creature);
            }
            else
                m_uiLaugh -= uiDiff;
        }
        else
        {
            if (m_bDie)
            {
                if (m_uiWait < uiDiff)
                {
                    m_bDie = false;
                    if (Creature *pBody = m_creature->GetMap()->GetCreature(m_uiBodyguid))
                    {
                        pBody->RemoveAurasDueToSpell(SPELL_IMMUNE);
                        pBody->DealDamage(pBody, pBody->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    }

                    m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }
                else
                    m_uiWait -= uiDiff;
            }
        }
    }
};

/*#####
# Headless Horseman
#####*/
struct MANGOS_DLL_DECL boss_headless_horsemanAI : public ScriptedAI
{
    boss_headless_horsemanAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        SpellEntry *pConfl = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_CONFLAGRATION);

        if(pConfl)
        {
            pConfl->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
            pConfl->EffectBasePoints[0] = 10;
            //confl->EffectBaseDice[0] = 10;
            pConfl->DmgMultiplier[0] = 1;
        }

        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        Reset();
    }

    ScriptedInstance *m_pInstance;

    ObjectGuid m_uiHeadGuid;
    ObjectGuid m_uiPlayerGuid;

    uint32 m_uiPhase;
    uint32 m_uiId;
    uint32 m_uiCount;
    uint32 m_uiSayTimer;

    uint32 m_uiConflagrateTimer;
    uint32 m_uiSummonaddsTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiRegenTimer;
    uint32 m_uiWhirlwindTimer;
    uint32 m_uiLaughTimer;
    uint32 m_uiBurnTimer;

    bool m_bWithhead;
    bool m_bReturned;
    bool m_bIsFlying;
    bool m_bWpReached;
    bool m_bBurned;

    void Reset()
    {
        m_uiPhase = 0;
        m_uiConflagrateTimer = 3000;
        m_uiSummonaddsTimer = 3000;
        m_uiLaughTimer = urand(16000,20000);
        m_uiCleaveTimer = 2000;
        m_uiRegenTimer = 1000;
        m_uiBurnTimer = 6000;
        m_uiCount = 0;
        m_uiSayTimer = 3000;
        m_uiPlayerGuid = 0;

        m_bWithhead = true;
        m_bReturned = true;
        m_bBurned = false;
        m_bIsFlying = false;
        m_bWpReached = false;

        m_creature->SetVisibility(VISIBILITY_OFF);
        DoCast(m_creature, SPELL_HEAD);

        if (m_uiHeadGuid)
        {
            if (Creature* Head = m_creature->GetMap()->GetCreature(m_uiHeadGuid))
                Head->ForcedDespawn();

            m_uiHeadGuid = 0;
        }

        if (m_pInstance && m_pInstance->GetData(DATA_HORSEMAN_EVENT) == NOT_STARTED)
            Intro();
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(DATA_HORSEMAN_EVENT, FAIL);

        m_creature->AddObjectToRemoveList();
    }

    void Intro()
    {
        if (m_pInstance)
            m_pInstance->SetData(DATA_HORSEMAN_EVENT, IN_PROGRESS);

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        //m_creature->AddMonsterMoveFlag(MONSTER_MOVE_LEVITATING);
        //m_creature->AddMonsterMoveFlag(MONSTER_MOVE_TELEPORT2);
        m_creature->SetSpeedRate(MOVE_WALK,5.0f,true);
        m_bWpReached = false;
        m_uiCount = 0;
        m_uiSayTimer = 3000;
        m_uiId = 0;
        m_uiPhase = 0;
    }

    void MovementInform(uint32 type, uint32 i)
    {
        if (type != POINT_MOTION_TYPE || !m_bIsFlying || i != m_uiId)
            return;

        m_bWpReached = true;

        switch (m_uiId)
        {
            case 0:
                if (Creature *smoke = m_creature->SummonCreature(NPC_HELPER, Spawn[1].x, Spawn[1].y, Spawn[1].z, 0, TEMPSUMMON_TIMED_DESPAWN, 20000))
                    ((mob_wisp_invisAI*)smoke->AI())->SetType(3);

                DoCast(m_creature,SPELL_RHYME_BIG);
                break;
            case 1:
            {
                m_creature->SetVisibility(VISIBILITY_ON);
                break;
            }
            case 6:
                if (m_pInstance)
                    m_pInstance->SetData(DATA_PUMPKIN_SHRINE, 0);   //hide gameobject
                break;
            case 19:
                //m_creature->RemoveMonsterMoveFlag(MONSTER_MOVE_LEVITATING);
                //m_creature->RemoveMonsterMoveFlag(MONSTER_MOVE_TELEPORT2);
                //m_creature->SetMonsterMoveFlags(MONSTER_MOVE_WALK);
                break;
            case 22:
            {
                m_uiPhase = 1;
                m_bIsFlying = false;
                m_bWpReached = false;
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                DoScriptText(SAY_ENTRANCE, m_creature);

                if (Player *pWho = m_creature->GetMap()->GetPlayer(m_uiPlayerGuid))
                {
                    m_creature->DeleteThreatList();
                    m_creature->SetInCombatWithZone();
                    m_creature->AddThreat(pWho, 100.0f);
                    DoStartMovement(pWho);
                }
                break;
            }
        }
        ++m_uiId;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_bWithhead || m_uiPhase == 0)
            return;

        if (!m_creature->hasUnitState(UNIT_STAT_STUNNED) && pWho->isTargetableForAttack() &&
            m_creature->IsHostileTo(pWho) && pWho->isInAccessablePlaceFor(m_creature))
        {
            if (!m_creature->CanFly() && m_creature->GetDistanceZ(pWho) > CREATURE_Z_ATTACK_RANGE)
                return;

            if (m_creature->IsWithinDistInMap(pWho, 66) && m_creature->IsWithinLOSInMap(pWho))
            {
                if (!m_creature->getVictim())
                {
                    pWho->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                    AttackStart(pWho);
                }
                else if (m_creature->GetMap()->IsDungeon())
                {
                    pWho->SetInCombatWith(m_creature);
                    m_creature->AddThreat(pWho, 0.0f);
                }
            }
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            if (m_bWithhead)
                DoScriptText(SAY_SLAY, m_creature);
            //maybe possible when player dies from conflagration
            else if (Creature *pHead = m_creature->GetMap()->GetCreature(m_uiHeadGuid))
                ((mob_headAI*)pHead->AI())->SaySound(SAY_PLAYER1);
        }
    }

    void JustDied(Unit* pKiller)
    {
        m_creature->StopMoving();
        //m_creature->GetMotionMaster()->MoveIdle();
        DoScriptText(SAY_DEATH, m_creature);

        Map::PlayerList const& players = m_pInstance->instance->GetPlayers();

        if (!players.isEmpty())
        {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* pPlayer = itr->getSource())
                    pPlayer->CastSpell(pPlayer, SPELL_CREATE_TREATS, true);
            }
        }

        if (Creature *pFlame = DoSpawnCreature(NPC_HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000))
            pFlame->CastSpell(pFlame, SPELL_BODY_FLAME, false);

        if (Creature *pWisp = DoSpawnCreature(NPC_WISP_INVIS, 1, 1, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 240000))
        {
            ((mob_wisp_invisAI*)pWisp->AI())->SetType(4);
        }

        if (m_pInstance)
            m_pInstance->SetData(DATA_HORSEMAN_EVENT, DONE);
    }

    void SpellHitTarget(Unit* pUnit, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_CONFLAGRATION)
            if (pUnit->HasAura(SPELL_CONFLAGRATION, EFFECT_INDEX_0))
                DoScriptText(SAY_CONFLAGRATION, m_creature);
    }

    void SpellHit(Unit *pCaster, const SpellEntry* pSpell)
    {
        if (m_bWithhead)
            return;

        if (pSpell->Id == SPELL_FLYING_HEAD)
        {
            if (m_uiPhase < 3)
                ++m_uiPhase;
            else
                m_uiPhase = 3;
            m_bWithhead = true;
            m_creature->RemoveAllAuras();
            m_creature->SetName("Headless Horseman");
            m_creature->SetHealth(m_creature->GetMaxHealth());
            DoScriptText(SAY_REJOINED, m_creature);
            DoCast(m_creature, SPELL_HEAD);
            pCaster->GetMotionMaster()->Clear(false);
            pCaster->GetMotionMaster()->MoveFollow(m_creature, 6, urand(0, 5));
            //DoResetThreat();//not sure if need
            ThreatList const& tList = m_creature->getThreatManager().getThreatList();
            for (ThreatList::const_iterator itr = tList.begin();itr != tList.end(); ++itr)
            {
                Unit* pUnit = m_creature->GetMap()->GetCreature((*itr)->getUnitGuid());
                if (pUnit && pUnit->isAlive() && pUnit != pCaster)
                    m_creature->AddThreat(pUnit, pCaster->getThreatManager().getThreat(pUnit));
            }
        }
    }

    void DamageTaken(Unit *pDoneBy, uint32 &uiDamage)
    {
        if (!m_bWithhead)
        {
            uiDamage = 0;
            return;
        }

        if (m_creature->GetHealth() <= uiDamage + m_creature->GetMaxHealth()/100)
        {
            m_bWithhead = false;
            m_bReturned = false;
            uiDamage = 0;
            m_creature->SetHealth(m_creature->GetMaxHealth()/100);

            m_creature->RemoveAllAuras();
            m_creature->SetDisplayId(22352);
            m_creature->SetNativeDisplayId(22352);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            if (!m_uiHeadGuid)
            {
                if (Unit* pSummon = DoSpawnCreature(NPC_HEAD, rand()%6, rand()%6, 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0))
                    m_uiHeadGuid = pSummon->GetObjectGuid();
            }

            Creature* pHead = (Creature*) m_creature->GetMap()->GetCreature(m_uiHeadGuid);
            if (pHead && pHead->isAlive())
            {
                //((mob_headAI*)Head->AI())->m_uiPhase = m_uiPhase;
                pHead->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pHead->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //pHead->CastSpell(Head,SPELL_HEAD_INVIS,false);
                m_creature->InterruptNonMeleeSpells(false);
                DoCast(m_creature, SPELL_IMMUNE, true);
                DoCast(m_creature, SPELL_BODY_REGEN, true);
                m_creature->CastSpell(pHead, SPELL_FLYING_HEAD, true);
                DoCast(m_creature, SPELL_BODY_CONFUSE, false);                     //test

                pHead->SetVisibility(VISIBILITY_ON);
                //done_by->ProcDamageAndSpell(m_creature,PROC_FLAG_KILL,PROC_FLAG_KILLED,PROC_EX_NONE,0);

                m_uiWhirlwindTimer = urand(4000, 5000);
                m_uiRegenTimer = 0;
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiPhase == 0)
        {
            if (!m_bIsFlying)
            {
                if (m_uiSayTimer < uiDiff)
                {
                    m_uiSayTimer = 3000;

                    if (m_uiCount < 3)
                    {
                        if (Player* pWho = m_creature->GetMap()->GetPlayer(m_uiPlayerGuid))
                            pWho->Say(Text[m_uiCount].text, LANG_UNIVERSAL);

                        ++m_uiCount;
                    }
                    else
                    {
                        DoCast(m_creature,SPELL_RHYME_BIG);
                        if (Player* pWho = m_creature->GetMap()->GetPlayer(m_uiPlayerGuid))
                            pWho->Yell(Text[m_uiCount].text, LANG_UNIVERSAL);

                        //HandleEmoteCommand(ANIM_EMOTE_SHOUT);

                        m_bWpReached = true;
                        m_bIsFlying = true;
                        m_uiCount = 0;
                    }
                }
                else
                    m_uiSayTimer -= uiDiff;
            }
            else
            {
                if (m_bWpReached)
                {
                    m_bWpReached = false;
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MovePoint(m_uiId, FlightPoint[m_uiId].x, FlightPoint[m_uiId].y, FlightPoint[m_uiId].z);
                }
            }
        }  // m_uiPhase 0

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bWithhead)
        {
            switch(m_uiPhase)
            {
                case 1:
                    if (m_bBurned)
                        break;

                    if (m_uiBurnTimer < uiDiff)
                    {
                        if (Creature *pFlame = m_creature->SummonCreature(NPC_HELPER, Spawn[0].x, Spawn[0].y, Spawn[0].z, 0, TEMPSUMMON_TIMED_DESPAWN, 17000))
                            ((mob_wisp_invisAI*)pFlame->AI())->SetType(2);

                        m_bBurned = true;
                    }
                    else
                        m_uiBurnTimer -= uiDiff;

                    break;
                case 2:
                    if (m_uiConflagrateTimer < uiDiff)
                    {
                        Unit* pTarget = NULL;
                        Unit* pTemp = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1);

                        if (pTemp)
                        {
                            if (pTemp->GetTypeId() != TYPEID_PLAYER)
                                pTarget = pTemp->GetCharmerOrOwnerPlayerOrPlayerItself();
                            else
                                pTarget = pTemp;
                        }
                        else
                            pTarget = m_creature->getVictim();

                        if (pTarget)
                            m_creature->CastSpell(pTarget, SPELL_CONFLAGRATION, false);

                        m_uiConflagrateTimer = urand(5000, 8000);
                    }
                    else
                        m_uiConflagrateTimer -= uiDiff;
                    break;
                case 3:
                    if (m_uiSummonaddsTimer < uiDiff)
                    {
                        m_creature->InterruptNonMeleeSpells(false);
                        DoCast(m_creature, SPELL_SUMMON_PUMPKIN);
                        DoScriptText(SAY_SPROUTING_PUMPKINS, m_creature);
                        m_uiSummonaddsTimer = urand(5000, 10000);
                    }
                    else
                        m_uiSummonaddsTimer -= uiDiff;
                    break;
            } //switch

            if (m_uiLaughTimer < uiDiff)
            {
                m_uiLaughTimer = urand(11000, 22000);
/*
                if (Creature *speaker = DoSpawnCreature(HELPER,0,0,0,0,TEMPSUMMON_TIMED_DESPAWN,1000))
                    speaker->CastSpell(speaker,SPELL_HEAD_SPEAKS,false);
*/
                switch (urand(1,3))
                {
                    case 1:
                        DoCast(m_creature, SPELL_LAUGH1, true);
                        break;
                    case 2:
                        DoCast(m_creature, SPELL_LAUGH2, true);
                        break;
                    case 3:
                        DoCast(m_creature, SPELL_LAUGH3, true);
                        break;
                }

                DoScriptText(EMOTE_LAUGH, m_creature);
            }
            else
                m_uiLaughTimer -= uiDiff;

            if (m_creature->getVictim())
            {
                if (m_uiCleaveTimer < uiDiff)
                {
                    DoCast(m_creature->getVictim(), SPELL_CLEAVE);
                    m_uiCleaveTimer = urand(2000, 6000);       //1 cleave per 2.0-6.0sec
                }
                else
                    m_uiCleaveTimer -= uiDiff;

                DoMeleeAttackIfReady();
            }
        }
        else
        {
            if (m_uiRegenTimer < uiDiff)
            {
                m_uiRegenTimer = 1000;                   //"body calls head"
                if (!m_bReturned)
                {
                    if (m_creature->GetHealth() < m_creature->GetMaxHealth())
                        m_creature->SetHealth(m_creature->GetHealth() + m_creature->GetMaxHealth()*0.04f);// regen rate
                    else
                    {
                        Creature* pHead = m_creature->GetMap()->GetCreature(m_uiHeadGuid);
                        if (pHead && pHead->isAlive())
                        {
                            ((mob_headAI*)pHead->AI())->Disappear();
                            ((mob_headAI*)pHead->AI())->m_uiPhase = m_uiPhase;
                        }
                        m_creature->RemoveAurasDueToSpell(SPELL_BODY_CONFUSE);
                        m_bReturned = true;
                        return;
                    }
                }
            }
            else
                m_uiRegenTimer -= uiDiff;

            if (m_uiWhirlwindTimer < uiDiff)
            {
                m_uiWhirlwindTimer = urand(4000, 5000);
                if (urand(0,2)) // 2 su 3 whirl
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_BODY_CONFUSE);
                    DoCast(m_creature, SPELL_WHIRLWIND, true);
                }
                else
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
                    DoCast(m_creature, SPELL_BODY_CONFUSE);
                }
            }
            else
            m_uiWhirlwindTimer -= uiDiff;
        }
    }
};

void mob_headAI::Disappear()
{
    if (m_bWithBody)
        return;

    if (m_uiBodyguid)
    {
        Creature *pBody = m_creature->GetMap()->GetCreature(m_uiBodyguid);
        if (pBody && pBody->isAlive())
        {
            m_bWithBody = true;
            pBody->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            pBody->RemoveAurasDueToSpell(SPELL_IMMUNE);//hack, SpellHit doesn't calls if body has immune aura
            pBody->SetDisplayId(22351);
            pBody->SetNativeDisplayId(22351);
            DoCast(pBody, SPELL_FLYING_HEAD);

            m_creature->RemoveAllAuras();
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->SetVisibility(VISIBILITY_OFF);
            ((boss_headless_horsemanAI*)pBody->AI())->m_bReturned = true;
        }
    }
}
/*#####
# Pulsing Pumpkin
#####*/
struct MANGOS_DLL_DECL mob_pulsing_pumpkinAI : public ScriptedAI
{
    mob_pulsing_pumpkinAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    bool m_bSprouted;
    ObjectGuid m_uiDebuffGuid;

    void Reset()
    {
        float x, y, z;
        m_creature->GetPosition(x, y, z);    //this visual aura some under ground
        m_creature->Relocate(x, y, z + 0.35f);
        Despawn();
        if (Creature *pDebuff = DoSpawnCreature(NPC_HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 14500))
        {
            pDebuff->SetDisplayId(m_creature->GetDisplayId());
            pDebuff->CastSpell(pDebuff, SPELL_PUMPKIN_AURA_GREEN ,false);
            pDebuff->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

            ((mob_wisp_invisAI*)pDebuff->AI())->SetType(1);
            m_uiDebuffGuid = pDebuff->GetObjectGuid();
        }

        m_bSprouted = false;
        DoCast(m_creature, SPELL_PUMPKIN_AURA, true);
        DoCast(m_creature, SPELL_SPROUTING);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    void Aggro(Unit *pWho){}

    void SpellHit(Unit *pCaster, const SpellEntry *pSpell)
    {
        if (pSpell->Id == SPELL_SPROUTING)
        {
            m_bSprouted = true;
            m_creature->RemoveAllAuras();
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
            DoCast(m_creature, SPELL_SPROUT_BODY, true);
            m_creature->UpdateEntry(NPC_PUMPKIN_FIEND);
            DoStartMovement(m_creature->getVictim());
        }
    }

    void Despawn()
    {
        if (!m_uiDebuffGuid)
            return;

        if(Creature *pDebuff = m_creature->GetMap()->GetCreature(m_uiDebuffGuid))
            pDebuff->SetVisibility(VISIBILITY_OFF);

        m_uiDebuffGuid = 0;
    }

    void JustDied(Unit *killer)
    {
        if(!m_bSprouted)
            Despawn();
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (!pWho || !pWho->isTargetableForAttack() || !m_creature->IsHostileTo(pWho) || m_creature->getVictim())
            return;

        m_creature->AddThreat(pWho, 0.0f);

        if(m_bSprouted)
            DoStartMovement(pWho);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_bSprouted)
        {
                if (m_creature->SelectHostileTarget() || m_creature->getVictim())
                DoMeleeAttackIfReady();
        }
        else if (!m_creature->HasAura(SPELL_PUMPKIN_AURA, EFFECT_INDEX_0))
            DoCast(m_creature, SPELL_PUMPKIN_AURA, true);
    }
};

/*#####
# Loosely Turned Soil
#####*/
enum HorsemanQuests
{
    QUEST_CALL_HORSEMAN_0   = 11392,
    QUEST_CALL_HORSEMAN_1   = 11404,
    QUEST_CALL_HORSEMAN_2   = 11405
};
bool GOHello_go_loosely_turned_soil(Player* pPlayer, GameObject* pSoil)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)pSoil->GetInstanceData();

    if (!m_pInstance)
        return true;

    if (pSoil->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        pPlayer->PrepareQuestMenu(pSoil->GetObjectGuid());
        pPlayer->SendPreparedQuest(pSoil->GetObjectGuid());
    }
    else
        return true;

    if ((pPlayer->GetQuestStatus(QUEST_CALL_HORSEMAN_0) != QUEST_STATUS_COMPLETE) && !pPlayer->GetQuestRewardStatus(QUEST_CALL_HORSEMAN_0))
        pPlayer->AreaExploredOrEventHappens(QUEST_CALL_HORSEMAN_0);
    else if ((pPlayer->GetQuestStatus(QUEST_CALL_HORSEMAN_1) != QUEST_STATUS_COMPLETE) && !pPlayer->GetQuestRewardStatus(QUEST_CALL_HORSEMAN_1))
        pPlayer->AreaExploredOrEventHappens(QUEST_CALL_HORSEMAN_1);
    else if ((pPlayer->GetQuestStatus(QUEST_CALL_HORSEMAN_2) != QUEST_STATUS_COMPLETE) && !pPlayer->GetQuestRewardStatus(QUEST_CALL_HORSEMAN_2))
        pPlayer->AreaExploredOrEventHappens(QUEST_CALL_HORSEMAN_2);
    m_pInstance->SetData(DATA_HORSEMAN_EVENT, NOT_STARTED);
    return true;
}

bool GOChooseReward_go_loosely_turned_soil(Player* pPlayer, GameObject* pSoil, const Quest* pQuest)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)pSoil->GetInstanceData();

    if (!m_pInstance)
        return true;

    if (m_pInstance->GetData(DATA_HORSEMAN_EVENT) == NOT_STARTED)
    {
        outstring_log("SD2: Headless Horseman summoned by: %s",  pPlayer->GetName());

        if (Creature* horseman = pSoil->SummonCreature(NPC_HH_MOUNTED, FlightPoint[0].x, FlightPoint[0].y, FlightPoint[0].z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300000))
        {
            ((boss_headless_horsemanAI*)horseman->AI())->m_uiPlayerGuid = pPlayer->GetObjectGuid();
        }
    }
    return true;
}

CreatureAI* GetAI_mob_head(Creature* pCreature)
{
    return new mob_headAI (pCreature);
}

CreatureAI* GetAI_boss_headless_horseman(Creature* pCreature)
{
    return new boss_headless_horsemanAI (pCreature);
}

CreatureAI* GetAI_mob_pulsing_pumpkin(Creature* pCreature)
{
    return new mob_pulsing_pumpkinAI (pCreature);
}

CreatureAI* GetAI_mob_wisp_invis(Creature* pCreature)
{
    return new mob_wisp_invisAI (pCreature);
}


void AddSC_boss_headless_horseman()
{
    Script* NewScript;

    NewScript = new Script;
    NewScript->Name = "boss_headless_horseman";
    NewScript->GetAI = GetAI_boss_headless_horseman;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_head";
    NewScript->GetAI = &GetAI_mob_head;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_pulsing_pumpkin";
    NewScript->GetAI = &GetAI_mob_pulsing_pumpkin;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "mob_wisp_invis";
    NewScript->GetAI = &GetAI_mob_wisp_invis;
    NewScript->RegisterSelf();

    NewScript = new Script;
    NewScript->Name = "go_loosely_turned_soil";
    NewScript->pGossipHelloGO = &GOHello_go_loosely_turned_soil;
    NewScript->pQuestRewardedGO = &GOChooseReward_go_loosely_turned_soil;
    NewScript->RegisterSelf();
}
