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
SDName: Npcs_Special
SD%Complete: 100
SDComment: To be used for special NPCs that are located globally.
SDCategory: NPCs
EndScriptData
*/

#include "precompiled.h"
#include "escort_ai.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "Spell.h"
#include "PetAI.h"
#include "Pet.h"
#include "SpellMgr.h"

/* ContentData
npc_air_force_bots       80%    support for misc (invisible) guard bots in areas where player allowed to fly. Summon guards after a preset time if tagged by spell
npc_chicken_cluck       100%    support for quest 3861 (Cluck!)
npc_dancing_flames      100%    midsummer event NPC
npc_guardian            100%    guardianAI used to prevent players from accessing off-limits areas. Not in use by SD2
npc_garments_of_quests   80%    NPC's related to all Garments of-quests 5621, 5624, 5625, 5648, 5650
npc_injured_patient     100%    patients for triage-quests (6622 and 6624)
npc_doctor              100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_innkeeper            25%    ScriptName not assigned. Innkeepers in general.
npc_mount_vendor        100%    Regular mount vendors all over the world. Display gossip if player doesn't meet the requirements to buy
npc_sayge               100%    Darkmoon event fortune teller, buff player based on answers given
npc_tabard_vendor        50%    allow recovering quest related tabards, achievement related ones need core support
npc_locksmith            75%    list of keys needs to be confirmed
npc_death_knight_gargoyle       AI for summoned gargoyle of deathknights
npc_training_dummy      100%    AI for training dummies
npc_winter_reveler      100%    Winterveil event
npc_metzen              100%    Winterveil event
npc_experience_eliminator       NPC to stop gaining experience
pet_spring_rabbit       100%    Noblegarden event
pet_orphan              100%    Children's Week
npc_wormhole            100%    Creates an unstable wormhole
npc_bunny_bark                  Brewfest event
npc_bunny_fire_training         Hallow's End event
npc_bunny_fire_town             Hallow's End event
npc_bunny_horseman              Hallow's End event
EndContentData */

/*########
# npc_air_force_bots
#########*/

enum SpawnType
{
    SPAWNTYPE_TRIPWIRE_ROOFTOP,                             // no warning, summon creature at smaller range
    SPAWNTYPE_ALARMBOT                                      // cast guards mark and summon npc - if player shows up with that buff duration < 5 seconds attack
};

struct SpawnAssociation
{
    uint32 m_uiThisCreatureEntry;
    uint32 m_uiSpawnedCreatureEntry;
    SpawnType m_SpawnType;
};

enum
{
    SPELL_GUARDS_MARK               = 38067,
    AURA_DURATION_TIME_LEFT         = 5000
};

const float RANGE_TRIPWIRE          = 15.0f;
const float RANGE_GUARDS_MARK       = 50.0f;

SpawnAssociation m_aSpawnAssociations[] =
{
    {2614,  15241, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Alliance)
    {2615,  15242, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Horde)
    {21974, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Area 52)
    {21993, 15242, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Horde - Bat Rider)
    {21996, 15241, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Alliance - Gryphon)
    {21997, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Goblin - Area 52 - Zeppelin)
    {21999, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Alliance)
    {22001, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Horde)
    {22002, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Horde)
    {22003, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Alliance)
    {22063, 21976, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Goblin - Area 52)
    {22065, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Ethereal - Stormspire)
    {22066, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Scryer - Dragonhawk)
    {22068, 22064, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Ethereal - Stormspire)
    {22069, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Stormspire)
    {22070, 22067, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Scryer)
    {22071, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Scryer)
    {22078, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Aldor)
    {22079, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Aldor - Gryphon)
    {22080, 22077, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Aldor)
    {22086, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Sporeggar)
    {22087, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Sporeggar - Spore Bat)
    {22088, 22085, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Sporeggar)
    {22090, 22089, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Toshley's Station - Flying Machine)
    {22124, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Cenarion)
    {22125, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Cenarion - Stormcrow)
    {22126, 22122, SPAWNTYPE_ALARMBOT}                      //Air Force Trip Wire - Rooftop (Cenarion Expedition)
};

struct MANGOS_DLL_DECL npc_air_force_botsAI : public ScriptedAI
{
    npc_air_force_botsAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pSpawnAssoc = NULL;

        // find the correct spawnhandling
        static uint32 uiEntryCount = sizeof(m_aSpawnAssociations)/sizeof(SpawnAssociation);

        for (uint8 i=0; i<uiEntryCount; ++i)
        {
            if (m_aSpawnAssociations[i].m_uiThisCreatureEntry == pCreature->GetEntry())
            {
                m_pSpawnAssoc = &m_aSpawnAssociations[i];
                break;
            }
        }

        if (!m_pSpawnAssoc)
            error_db_log("SD2: Creature template entry %u has ScriptName npc_air_force_bots, but it's not handled by that script", pCreature->GetEntry());
        else
        {
            CreatureInfo const* spawnedTemplate = GetCreatureTemplateStore(m_pSpawnAssoc->m_uiSpawnedCreatureEntry);

            if (!spawnedTemplate)
            {
                error_db_log("SD2: Creature template entry %u does not exist in DB, which is required by npc_air_force_bots", m_pSpawnAssoc->m_uiSpawnedCreatureEntry);
                m_pSpawnAssoc = NULL;
                return;
            }
        }
    }

    SpawnAssociation* m_pSpawnAssoc;
    ObjectGuid m_spawnedGuid;

    void Reset() { }

    Creature* SummonGuard()
    {
        Creature* pSummoned = m_creature->SummonCreature(m_pSpawnAssoc->m_uiSpawnedCreatureEntry, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);

        if (pSummoned)
            m_spawnedGuid = pSummoned->GetObjectGuid();
        else
        {
            error_db_log("SD2: npc_air_force_bots: wasn't able to spawn creature %u", m_pSpawnAssoc->m_uiSpawnedCreatureEntry);
            m_pSpawnAssoc = NULL;
        }

        return pSummoned;
    }

    Creature* GetSummonedGuard()
    {
        Creature* pCreature = m_creature->GetMap()->GetCreature(m_spawnedGuid);

        if (pCreature && pCreature->isAlive())
            return pCreature;

        return NULL;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_pSpawnAssoc)
            return;

        if (pWho->isTargetableForAttack() && m_creature->IsHostileTo(pWho))
        {
            Player* pPlayerTarget = pWho->GetTypeId() == TYPEID_PLAYER ? (Player*)pWho : NULL;

            // airforce guards only spawn for players
            if (!pPlayerTarget)
                return;

            Creature* pLastSpawnedGuard = m_spawnedGuid ? GetSummonedGuard() : NULL;

            // prevent calling GetCreature at next MoveInLineOfSight call - speedup
            if (!pLastSpawnedGuard)
                m_spawnedGuid.Clear();

            switch(m_pSpawnAssoc->m_SpawnType)
            {
                case SPAWNTYPE_ALARMBOT:
                {
                    if (!pWho->IsWithinDistInMap(m_creature, RANGE_GUARDS_MARK))
                        return;

                    Aura* pMarkAura = pWho->GetAura(SPELL_GUARDS_MARK, EFFECT_INDEX_0);
                    if (pMarkAura)
                    {
                        // the target wasn't able to move out of our range within 25 seconds
                        if (!pLastSpawnedGuard)
                        {
                            pLastSpawnedGuard = SummonGuard();

                            if (!pLastSpawnedGuard)
                                return;
                        }

                        if (pMarkAura->GetAuraDuration() < AURA_DURATION_TIME_LEFT)
                        {
                            if (!pLastSpawnedGuard->getVictim())
                                pLastSpawnedGuard->AI()->AttackStart(pWho);
                        }
                    }
                    else
                    {
                        if (!pLastSpawnedGuard)
                            pLastSpawnedGuard = SummonGuard();

                        if (!pLastSpawnedGuard)
                            return;

                        pLastSpawnedGuard->CastSpell(pWho, SPELL_GUARDS_MARK, true);
                    }
                    break;
                }
                case SPAWNTYPE_TRIPWIRE_ROOFTOP:
                {
                    if (!pWho->IsWithinDistInMap(m_creature, RANGE_TRIPWIRE))
                        return;

                    if (!pLastSpawnedGuard)
                        pLastSpawnedGuard = SummonGuard();

                    if (!pLastSpawnedGuard)
                        return;

                    // ROOFTOP only triggers if the player is on the ground
                    if (!pPlayerTarget->IsFlying())
                    {
                        if (!pLastSpawnedGuard->getVictim())
                            pLastSpawnedGuard->AI()->AttackStart(pWho);
                    }
                    break;
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_air_force_bots(Creature* pCreature)
{
    return new npc_air_force_botsAI(pCreature);
}

/*########
# npc_chicken_cluck
#########*/

enum
{
    EMOTE_A_HELLO           = -1000204,
    EMOTE_H_HELLO           = -1000205,
    EMOTE_CLUCK_TEXT2       = -1000206,

    QUEST_CLUCK             = 3861,
    FACTION_FRIENDLY        = 35,
    FACTION_CHICKEN         = 31
};

struct MANGOS_DLL_DECL npc_chicken_cluckAI : public ScriptedAI
{
    npc_chicken_cluckAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiResetFlagTimer;

    void Reset()
    {
        m_uiResetFlagTimer = 120000;

        m_creature->setFaction(FACTION_CHICKEN);
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void ReceiveEmote(Player* pPlayer, uint32 uiEmote)
    {
        if (uiEmote == TEXTEMOTE_CHICKEN)
        {
            if (!urand(0, 29))
            {
                if (pPlayer->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE)
                {
                    m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    m_creature->setFaction(FACTION_FRIENDLY);

                    DoScriptText(EMOTE_A_HELLO, m_creature);

                    /* are there any difference in texts, after 3.x ?
                    if (pPlayer->GetTeam() == HORDE)
                        DoScriptText(EMOTE_H_HELLO, m_creature);
                    else
                        DoScriptText(EMOTE_A_HELLO, m_creature);
                    */
                }
            }
        }

        if (uiEmote == TEXTEMOTE_CHEER)
        {
            if (pPlayer->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE)
            {
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                m_creature->setFaction(FACTION_FRIENDLY);
                DoScriptText(EMOTE_CLUCK_TEXT2, m_creature);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Reset flags after a certain time has passed so that the next player has to start the 'event' again
        if (m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
        {
            if (m_uiResetFlagTimer < uiDiff)
                EnterEvadeMode();
            else
                m_uiResetFlagTimer -= uiDiff;
        }

        if (m_creature->SelectHostileTarget() && m_creature->getVictim())
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_chicken_cluck(Creature* pCreature)
{
    return new npc_chicken_cluckAI(pCreature);
}

bool QuestAccept_npc_chicken_cluck(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CLUCK)
    {
        if (npc_chicken_cluckAI* pChickenAI = dynamic_cast<npc_chicken_cluckAI*>(pCreature->AI()))
            pChickenAI->Reset();
    }

    return true;
}

bool QuestRewarded_npc_chicken_cluck(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CLUCK)
    {
        if (npc_chicken_cluckAI* pChickenAI = dynamic_cast<npc_chicken_cluckAI*>(pCreature->AI()))
            pChickenAI->Reset();
    }

    return true;
}

/*######
## npc_dancing_flames
######*/

enum
{
    SPELL_FIERY_SEDUCTION = 47057
};

struct MANGOS_DLL_DECL npc_dancing_flamesAI : public ScriptedAI
{
    npc_dancing_flamesAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() {}

    void ReceiveEmote(Player* pPlayer, uint32 uiEmote)
    {
        m_creature->SetFacingToObject(pPlayer);

        if (pPlayer->HasAura(SPELL_FIERY_SEDUCTION))
            pPlayer->RemoveAurasDueToSpell(SPELL_FIERY_SEDUCTION);

        if (pPlayer->IsMounted())
        {
            pPlayer->Unmount();                             // doesnt remove mount aura
            pPlayer->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
        }

        switch(uiEmote)
        {
            case TEXTEMOTE_DANCE: DoCastSpellIfCan(pPlayer, SPELL_FIERY_SEDUCTION); break;// dance -> cast SPELL_FIERY_SEDUCTION
            case TEXTEMOTE_WAVE:  m_creature->HandleEmote(EMOTE_ONESHOT_WAVE);      break;// wave -> wave
            case TEXTEMOTE_JOKE:  m_creature->HandleEmote(EMOTE_STATE_LAUGH);       break;// silly -> laugh(with sound)
            case TEXTEMOTE_BOW:   m_creature->HandleEmote(EMOTE_ONESHOT_BOW);       break;// bow -> bow
            case TEXTEMOTE_KISS:  m_creature->HandleEmote(TEXTEMOTE_CURTSEY);       break;// kiss -> curtsey
        }
    }
};

CreatureAI* GetAI_npc_dancing_flames(Creature* pCreature)
{
    return new npc_dancing_flamesAI(pCreature);
}

/*######
## Triage quest
######*/

enum
{
    SAY_DOC1                    = -1000201,
    SAY_DOC2                    = -1000202,
    SAY_DOC3                    = -1000203,

    QUEST_TRIAGE_H              = 6622,
    QUEST_TRIAGE_A              = 6624,

    DOCTOR_ALLIANCE             = 12939,
    DOCTOR_HORDE                = 12920,
    ALLIANCE_COORDS             = 7,
    HORDE_COORDS                = 6
};

struct Location
{
    float x, y, z, o;
};

static Location AllianceCoords[]=
{
    {-3757.38f, -4533.05f, 14.16f, 3.62f},                  // Top-far-right bunk as seen from entrance
    {-3754.36f, -4539.13f, 14.16f, 5.13f},                  // Top-far-left bunk
    {-3749.54f, -4540.25f, 14.28f, 3.34f},                  // Far-right bunk
    {-3742.10f, -4536.85f, 14.28f, 3.64f},                  // Right bunk near entrance
    {-3755.89f, -4529.07f, 14.05f, 0.57f},                  // Far-left bunk
    {-3749.51f, -4527.08f, 14.07f, 5.26f},                  // Mid-left bunk
    {-3746.37f, -4525.35f, 14.16f, 5.22f},                  // Left bunk near entrance
};

//alliance run to where
#define A_RUNTOX -3742.96f
#define A_RUNTOY -4531.52f
#define A_RUNTOZ 11.91f

static Location HordeCoords[]=
{
    {-1013.75f, -3492.59f, 62.62f, 4.34f},                  // Left, Behind
    {-1017.72f, -3490.92f, 62.62f, 4.34f},                  // Right, Behind
    {-1015.77f, -3497.15f, 62.82f, 4.34f},                  // Left, Mid
    {-1019.51f, -3495.49f, 62.82f, 4.34f},                  // Right, Mid
    {-1017.25f, -3500.85f, 62.98f, 4.34f},                  // Left, front
    {-1020.95f, -3499.21f, 62.98f, 4.34f}                   // Right, Front
};

//horde run to where
#define H_RUNTOX -1016.44f
#define H_RUNTOY -3508.48f
#define H_RUNTOZ 62.96f

const uint32 AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

const uint32 HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/

struct MANGOS_DLL_DECL npc_doctorAI : public ScriptedAI
{
    npc_doctorAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    ObjectGuid m_playerGuid;

    uint32 SummonPatient_Timer;
    uint32 SummonPatientCount;
    uint32 PatientDiedCount;
    uint32 PatientSavedCount;

    bool Event;

    GUIDList Patients;
    std::vector<Location*> Coordinates;

    void Reset()
    {
        m_playerGuid.Clear();

        SummonPatient_Timer = 10000;
        SummonPatientCount = 0;
        PatientDiedCount = 0;
        PatientSavedCount = 0;

        Patients.clear();
        Coordinates.clear();

        Event = false;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void BeginEvent(Player* pPlayer);
    void PatientDied(Location* Point);
    void PatientSaved(Creature* soldier, Player* pPlayer, Location* Point);
    void UpdateAI(const uint32 diff);
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

struct MANGOS_DLL_DECL npc_injured_patientAI : public ScriptedAI
{
    npc_injured_patientAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    ObjectGuid m_doctorGuid;
    Location* Coord;

    void Reset()
    {
        m_doctorGuid.Clear();
        Coord = NULL;

        //no select
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        //no regen health
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        //to make them lay with face down
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

        uint32 mobId = m_creature->GetEntry();

        switch (mobId)
        {                                                   //lower max health
            case 12923:
            case 12938:                                     //Injured Soldier
                m_creature->SetHealth(uint32(m_creature->GetMaxHealth()*.75));
                break;
            case 12924:
            case 12936:                                     //Badly injured Soldier
                m_creature->SetHealth(uint32(m_creature->GetMaxHealth()*.50));
                break;
            case 12925:
            case 12937:                                     //Critically injured Soldier
                m_creature->SetHealth(uint32(m_creature->GetMaxHealth()*.25));
                break;
        }
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (caster->GetTypeId() == TYPEID_PLAYER && m_creature->isAlive() && spell->Id == 20804)
        {
            if ((((Player*)caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (((Player*)caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
            {
                if (m_doctorGuid)
                {
                    if (Creature* pDoctor = m_creature->GetMap()->GetCreature(m_doctorGuid))
                    {
                        if (npc_doctorAI* pDocAI = dynamic_cast<npc_doctorAI*>(pDoctor->AI()))
                            pDocAI->PatientSaved(m_creature, (Player*)caster, Coord);
                    }
                }
            }
            //make not selectable
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            //regen health
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            //stand up
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);

            switch(urand(0, 2))
            {
                case 0: DoScriptText(SAY_DOC1,m_creature); break;
                case 1: DoScriptText(SAY_DOC2,m_creature); break;
                case 2: DoScriptText(SAY_DOC3,m_creature); break;
            }

            m_creature->SetWalk(false);

            uint32 mobId = m_creature->GetEntry();

            switch (mobId)
            {
                case 12923:
                case 12924:
                case 12925:
                    m_creature->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                    break;
                case 12936:
                case 12937:
                case 12938:
                    m_creature->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                    break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //lower HP on every world tick makes it a useful counter, not officlone though
        uint32 uiHPLose = uint32(0.05f * diff);
        if (m_creature->isAlive() && m_creature->GetHealth() > 1 + uiHPLose)
        {
            m_creature->SetHealth(m_creature->GetHealth() - uiHPLose);
        }

        if (m_creature->isAlive() && m_creature->GetHealth() <= 1 + uiHPLose)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->SetDeathState(JUST_DIED);
            m_creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

            if (m_doctorGuid)
            {
                if (Creature* pDoctor = m_creature->GetMap()->GetCreature(m_doctorGuid))
                {
                    if (npc_doctorAI* pDocAI = dynamic_cast<npc_doctorAI*>(pDoctor->AI()))
                        pDocAI->PatientDied(Coord);
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_injured_patient(Creature* pCreature)
{
    return new npc_injured_patientAI(pCreature);
}

/*
npc_doctor (continue)
*/

void npc_doctorAI::BeginEvent(Player* pPlayer)
{
    m_playerGuid = pPlayer->GetObjectGuid();

    SummonPatient_Timer = 10000;
    SummonPatientCount = 0;
    PatientDiedCount = 0;
    PatientSavedCount = 0;

    switch(m_creature->GetEntry())
    {
        case DOCTOR_ALLIANCE:
            for(uint8 i = 0; i < ALLIANCE_COORDS; ++i)
                Coordinates.push_back(&AllianceCoords[i]);
            break;
        case DOCTOR_HORDE:
            for(uint8 i = 0; i < HORDE_COORDS; ++i)
                Coordinates.push_back(&HordeCoords[i]);
            break;
    }

    Event = true;
    m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
}

void npc_doctorAI::PatientDied(Location* Point)
{
    Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);

    if (pPlayer && ((pPlayer->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
    {
        ++PatientDiedCount;

        if (PatientDiedCount > 5 && Event)
        {
            if (pPlayer->GetQuestStatus(QUEST_TRIAGE_A) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_TRIAGE_A);
            else if (pPlayer->GetQuestStatus(QUEST_TRIAGE_H) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_TRIAGE_H);

            Reset();
            return;
        }

        Coordinates.push_back(Point);
    }
    else
        // If no player or player abandon quest in progress
        Reset();
}

void npc_doctorAI::PatientSaved(Creature* soldier, Player* pPlayer, Location* Point)
{
    if (pPlayer && m_playerGuid == pPlayer->GetObjectGuid())
    {
        if ((pPlayer->GetQuestStatus(QUEST_TRIAGE_A) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(QUEST_TRIAGE_H) == QUEST_STATUS_INCOMPLETE))
        {
            ++PatientSavedCount;

            if (PatientSavedCount == 15)
            {
                for(GUIDList::const_iterator itr = Patients.begin(); itr != Patients.end(); ++itr)
                {
                    if (Creature* Patient = m_creature->GetMap()->GetCreature(*itr))
                        Patient->SetDeathState(JUST_DIED);
                }

                if (pPlayer->GetQuestStatus(QUEST_TRIAGE_A) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->GroupEventHappens(QUEST_TRIAGE_A, m_creature);
                else if (pPlayer->GetQuestStatus(QUEST_TRIAGE_H) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->GroupEventHappens(QUEST_TRIAGE_H, m_creature);

                Reset();
                return;
            }

            Coordinates.push_back(Point);
        }
    }
}

void npc_doctorAI::UpdateAI(const uint32 diff)
{
    if (Event && SummonPatientCount >= 20)
    {
        Reset();
        return;
    }

    if (Event)
    {
        if (SummonPatient_Timer < diff)
        {
            Creature* Patient = NULL;
            Location* Point = NULL;

            if (Coordinates.empty())
                return;

            std::vector<Location*>::iterator itr = Coordinates.begin()+rand()%Coordinates.size();
            uint32 patientEntry = 0;

            switch(m_creature->GetEntry())
            {
                case DOCTOR_ALLIANCE: patientEntry = AllianceSoldierId[urand(0, 2)]; break;
                case DOCTOR_HORDE:    patientEntry = HordeSoldierId[urand(0, 2)]; break;
                default:
                    error_log("SD2: Invalid entry for Triage doctor. Please check your database");
                    return;
            }

            Point = *itr;

            Patient = m_creature->SummonCreature(patientEntry, Point->x, Point->y, Point->z, Point->o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);

            if (Patient)
            {
                //303, this flag appear to be required for client side item->spell to work (TARGET_SINGLE_FRIEND)
                Patient->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

                Patients.push_back(Patient->GetObjectGuid());

                npc_injured_patientAI* pPatientAI = dynamic_cast<npc_injured_patientAI*>(Patient->AI());

                if (pPatientAI)
                {
                    pPatientAI->m_doctorGuid = m_creature->GetObjectGuid();

                    if (Point)
                        pPatientAI->Coord = Point;
                }

                Coordinates.erase(itr);
            }
            SummonPatient_Timer = 10000;
            ++SummonPatientCount;
        }else SummonPatient_Timer -= diff;
    }
}

bool QuestAccept_npc_doctor(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if ((pQuest->GetQuestId() == QUEST_TRIAGE_A) || (pQuest->GetQuestId() == QUEST_TRIAGE_H))
    {
        if (npc_doctorAI* pDocAI = dynamic_cast<npc_doctorAI*>(pCreature->AI()))
            pDocAI->BeginEvent(pPlayer);
    }

    return true;
}

CreatureAI* GetAI_npc_doctor(Creature* pCreature)
{
    return new npc_doctorAI(pCreature);
}

/*######
## npc_garments_of_quests
######*/

enum
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    SAY_COMMON_HEALED       = -1000231,
    SAY_DG_KEL_THANKS       = -1000232,
    SAY_DG_KEL_GOODBYE      = -1000233,
    SAY_ROBERTS_THANKS      = -1000256,
    SAY_ROBERTS_GOODBYE     = -1000257,
    SAY_KORJA_THANKS        = -1000258,
    SAY_KORJA_GOODBYE       = -1000259,
    SAY_DOLF_THANKS         = -1000260,
    SAY_DOLF_GOODBYE        = -1000261,
    SAY_SHAYA_THANKS        = -1000262,
    SAY_SHAYA_GOODBYE       = -1000263,
};

struct MANGOS_DLL_DECL npc_garments_of_questsAI : public npc_escortAI
{
    npc_garments_of_questsAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    ObjectGuid m_playerGuid;

    bool m_bIsHealed;
    bool m_bCanRun;

    uint32 m_uiRunAwayTimer;

    void Reset()
    {
        m_playerGuid.Clear();

        m_bIsHealed = false;
        m_bCanRun = false;

        m_uiRunAwayTimer = 5000;

        m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
        // expect database to have RegenHealth=0
        m_creature->SetHealth(int(m_creature->GetMaxHealth()*0.7));
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_LESSER_HEAL_R2 || pSpell->Id == SPELL_FORTITUDE_R1)
        {
            // not while in combat
            if (m_creature->isInCombat())
                return;

            // nothing to be done now
            if (m_bIsHealed && m_bCanRun)
                return;

            if (pCaster->GetTypeId() == TYPEID_PLAYER)
            {
                switch(m_creature->GetEntry())
                {
                    case ENTRY_SHAYA:
                        if (((Player*)pCaster)->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (m_bIsHealed && !m_bCanRun && pSpell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_SHAYA_THANKS, m_creature, pCaster);
                                m_bCanRun = true;
                            }
                            else if (!m_bIsHealed && pSpell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                m_playerGuid = pCaster->GetObjectGuid();
                                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, m_creature, pCaster);
                                m_bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_ROBERTS:
                        if (((Player*)pCaster)->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (m_bIsHealed && !m_bCanRun && pSpell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_ROBERTS_THANKS, m_creature, pCaster);
                                m_bCanRun = true;
                            }
                            else if (!m_bIsHealed && pSpell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                m_playerGuid = pCaster->GetObjectGuid();
                                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, m_creature, pCaster);
                                m_bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_DOLF:
                        if (((Player*)pCaster)->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (m_bIsHealed && !m_bCanRun && pSpell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_DOLF_THANKS, m_creature, pCaster);
                                m_bCanRun = true;
                            }
                            else if (!m_bIsHealed && pSpell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                m_playerGuid = pCaster->GetObjectGuid();
                                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, m_creature, pCaster);
                                m_bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_KORJA:
                        if (((Player*)pCaster)->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (m_bIsHealed && !m_bCanRun && pSpell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_KORJA_THANKS, m_creature, pCaster);
                                m_bCanRun = true;
                            }
                            else if (!m_bIsHealed && pSpell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                m_playerGuid = pCaster->GetObjectGuid();
                                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, m_creature, pCaster);
                                m_bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_DG_KEL:
                        if (((Player*)pCaster)->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (m_bIsHealed && !m_bCanRun && pSpell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_DG_KEL_THANKS, m_creature, pCaster);
                                m_bCanRun = true;
                            }
                            else if (!m_bIsHealed && pSpell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                m_playerGuid = pCaster->GetObjectGuid();
                                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, m_creature, pCaster);
                                m_bIsHealed = true;
                            }
                        }
                        break;
                }

                // give quest credit, not expect any special quest objectives
                if (m_bCanRun)
                    ((Player*)pCaster)->TalkedToCreature(m_creature->GetEntry(), m_creature->GetObjectGuid());
            }
        }
    }

    void WaypointReached(uint32 uiPointId) {}

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (m_bCanRun && !m_creature->isInCombat())
        {
            if (m_uiRunAwayTimer <= uiDiff)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                {
                    switch(m_creature->GetEntry())
                    {
                        case ENTRY_SHAYA:   DoScriptText(SAY_SHAYA_GOODBYE, m_creature, pPlayer);   break;
                        case ENTRY_ROBERTS: DoScriptText(SAY_ROBERTS_GOODBYE, m_creature, pPlayer); break;
                        case ENTRY_DOLF:    DoScriptText(SAY_DOLF_GOODBYE, m_creature, pPlayer);    break;
                        case ENTRY_KORJA:   DoScriptText(SAY_KORJA_GOODBYE, m_creature, pPlayer);   break;
                        case ENTRY_DG_KEL:  DoScriptText(SAY_DG_KEL_GOODBYE, m_creature, pPlayer);  break;
                    }

                    Start(true);
                }
                else
                    EnterEvadeMode();                       // something went wrong

                m_uiRunAwayTimer = 30000;
            }
            else
                m_uiRunAwayTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_garments_of_quests(Creature* pCreature)
{
    return new npc_garments_of_questsAI(pCreature);
}

/*######
## npc_guardian
######*/

#define SPELL_DEATHTOUCH                5

struct MANGOS_DLL_DECL npc_guardianAI : public ScriptedAI
{
    npc_guardianAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->isAttackReady())
        {
            m_creature->CastSpell(m_creature->getVictim(),SPELL_DEATHTOUCH, true);
            m_creature->resetAttackTimer();
        }
    }
};

CreatureAI* GetAI_npc_guardian(Creature* pCreature)
{
    return new npc_guardianAI(pCreature);
}

/*########
# npc_innkeeper
#########*/

// Script applied to all innkeepers by npcflag.
// Are there any known innkeepers that does not hape the options in the below?
// (remember gossipHello is not called unless npcflag|1 is present)

enum
{
    TEXT_ID_WHAT_TO_DO              = 1853,

    SPELL_TRICK_OR_TREAT            = 24751,                 // create item or random buff
    SPELL_TRICK_OR_TREATED          = 24755,                 // buff player get when tricked or treated
};

#define GOSSIP_ITEM_TRICK_OR_TREAT  "Trick or Treat!"
#define GOSSIP_ITEM_WHAT_TO_DO      "What can I do at an Inn?"

bool GossipHello_npc_innkeeper(Player* pPlayer, Creature* pCreature)
{
    pPlayer->PrepareGossipMenu(pCreature, pPlayer->GetDefaultGossipMenuForSource(pCreature));

    if (IsHolidayActive(HOLIDAY_HALLOWS_END) && !pPlayer->HasAura(SPELL_TRICK_OR_TREATED, EFFECT_INDEX_0))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TRICK_OR_TREAT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    // Should only apply to innkeeper close to start areas.
    if (AreaTableEntry const* pAreaEntry = GetAreaEntryByAreaID(pCreature->GetAreaId()))
    {
        if (pAreaEntry->flags & AREA_FLAG_LOWLEVEL)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_WHAT_TO_DO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }

    pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetObjectGuid());
    pPlayer->SendPreparedGossip(pCreature);
    return true;
}

bool GossipSelect_npc_innkeeper(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->SEND_GOSSIP_MENU(TEXT_ID_WHAT_TO_DO, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pCreature->CastSpell(pPlayer, SPELL_TRICK_OR_TREAT, true);
            break;
        case GOSSIP_OPTION_VENDOR:
            pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());
            break;
        case GOSSIP_OPTION_INNKEEPER:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->SetBindPoint(pCreature->GetObjectGuid());
            break;
    }

    return true;
}

/*######
## npc_kingdom_of_dalaran_quests
######*/

enum
{
    SPELL_TELEPORT_DALARAN  = 53360,
    ITEM_KT_SIGNET          = 39740,
    QUEST_MAGICAL_KINGDOM_A = 12794,
    QUEST_MAGICAL_KINGDOM_H = 12791,
    QUEST_MAGICAL_KINGDOM_N = 12796
};

#define GOSSIP_ITEM_TELEPORT_TO "I am ready to be teleported to Dalaran."

bool GossipHello_npc_kingdom_of_dalaran_quests(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->HasItemCount(ITEM_KT_SIGNET,1) && (!pPlayer->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_A) ||
        !pPlayer->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_H) || !pPlayer->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_N)))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TELEPORT_TO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_kingdom_of_dalaran_quests(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pPlayer->CastSpell(pPlayer,SPELL_TELEPORT_DALARAN,false);
    }
    return true;
}

/*######
## npc_lunaclaw_spirit
######*/

enum
{
    QUEST_BODY_HEART_A      = 6001,
    QUEST_BODY_HEART_H      = 6002,

    TEXT_ID_DEFAULT         = 4714,
    TEXT_ID_PROGRESS        = 4715
};

#define GOSSIP_ITEM_GRANT   "You have thought well, spirit. I ask you to grant me the strength of your body and the strength of your heart."

bool GossipHello_npc_lunaclaw_spirit(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_BODY_HEART_A) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(QUEST_BODY_HEART_H) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GRANT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(TEXT_ID_DEFAULT, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_lunaclaw_spirit(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->SEND_GOSSIP_MENU(TEXT_ID_PROGRESS, pCreature->GetObjectGuid());
        pPlayer->AreaExploredOrEventHappens((pPlayer->GetTeam() == ALLIANCE) ? QUEST_BODY_HEART_A : QUEST_BODY_HEART_H);
    }
    return true;
}

/*######
## npc_mount_vendor
######*/

bool GossipHello_npc_mount_vendor(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    bool canBuy;
    canBuy = false;
    uint32 vendor = pCreature->GetEntry();
    uint8 race = pPlayer->getRace();

    switch (vendor)
    {
        case 384:                                           //Katie Hunter
        case 1460:                                          //Unger Statforth
        case 2357:                                          //Merideth Carlson
        case 4885:                                          //Gregor MacVince
            if (pPlayer->GetReputationRank(72) != REP_EXALTED && race != RACE_HUMAN)
                pPlayer->SEND_GOSSIP_MENU(5855, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 1261:                                          //Veron Amberstill
            if (pPlayer->GetReputationRank(47) != REP_EXALTED && race != RACE_DWARF)
                pPlayer->SEND_GOSSIP_MENU(5856, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 3362:                                          //Ogunaro Wolfrunner
            if (pPlayer->GetReputationRank(76) != REP_EXALTED && race != RACE_ORC)
                pPlayer->SEND_GOSSIP_MENU(5841, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 3685:                                          //Harb Clawhoof
            if (pPlayer->GetReputationRank(81) != REP_EXALTED && race != RACE_TAUREN)
                pPlayer->SEND_GOSSIP_MENU(5843, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 4730:                                          //Lelanai
            if (pPlayer->GetReputationRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
                pPlayer->SEND_GOSSIP_MENU(5844, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 4731:                                          //Zachariah Post
            if (pPlayer->GetReputationRank(68) != REP_EXALTED && race != RACE_UNDEAD)
                pPlayer->SEND_GOSSIP_MENU(5840, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 7952:                                          //Zjolnir
            if (pPlayer->GetReputationRank(530) != REP_EXALTED && race != RACE_TROLL)
                pPlayer->SEND_GOSSIP_MENU(5842, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 7955:                                          //Milli Featherwhistle
            if (pPlayer->GetReputationRank(54) != REP_EXALTED && race != RACE_GNOME)
                pPlayer->SEND_GOSSIP_MENU(5857, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 16264:                                         //Winaestra
            if (pPlayer->GetReputationRank(911) != REP_EXALTED && race != RACE_BLOODELF)
                pPlayer->SEND_GOSSIP_MENU(10305, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
        case 17584:                                         //Torallius the Pack Handler
            if (pPlayer->GetReputationRank(930) != REP_EXALTED && race != RACE_DRAENEI)
                pPlayer->SEND_GOSSIP_MENU(10239, pCreature->GetObjectGuid());
            else canBuy = true;
            break;
    }

    if (canBuy)
    {
        if (pCreature->isVendor())
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    }
    return true;
}

bool GossipSelect_npc_mount_vendor(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());

    return true;
}

/*######
## npc_rogue_trainer
######*/

bool GossipHello_npc_rogue_trainer(Player* pPlayer, Creature* pCreature)
{
   if (pPlayer->getClass() != CLASS_ROGUE) return false;

   if (pPlayer->getLevel() >= 24 && !pPlayer->HasItemCount(17126,1) && !pPlayer->GetQuestRewardStatus(6681))
        if (pCreature->isQuestGiver())
           {
            pPlayer->PrepareGossipMenu(pCreature,50195);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "<Take the letter>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(5996, pCreature->GetObjectGuid());
            return true;
           }
    return false;
}

bool GossipSelect_npc_rogue_trainer(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
      if (uiAction == GOSSIP_ACTION_INFO_DEF)
      {
            pPlayer->CastSpell(pPlayer,21100,false);
            pPlayer->CLOSE_GOSSIP_MENU();
            return true;
      } else return false;
}

/*######
## npc_sayge
######*/

#define SPELL_DMG       23768                               //dmg
#define SPELL_RES       23769                               //res
#define SPELL_ARM       23767                               //arm
#define SPELL_SPI       23738                               //spi
#define SPELL_INT       23766                               //int
#define SPELL_STM       23737                               //stm
#define SPELL_STR       23735                               //str
#define SPELL_AGI       23736                               //agi
#define SPELL_FORTUNE   23765                               //faire fortune

bool GossipHello_npc_sayge(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->HasSpellCooldown(SPELL_INT) ||
        pPlayer->HasSpellCooldown(SPELL_ARM) ||
        pPlayer->HasSpellCooldown(SPELL_DMG) ||
        pPlayer->HasSpellCooldown(SPELL_RES) ||
        pPlayer->HasSpellCooldown(SPELL_STR) ||
        pPlayer->HasSpellCooldown(SPELL_AGI) ||
        pPlayer->HasSpellCooldown(SPELL_STM) ||
        pPlayer->HasSpellCooldown(SPELL_SPI))
        pPlayer->SEND_GOSSIP_MENU(7393, pCreature->GetObjectGuid());
    else
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Yes", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(7339, pCreature->GetObjectGuid());
    }

    return true;
}

void SendAction_npc_sayge(Player* pPlayer, Creature* pCreature, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Slay the Man",                      GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Turn him over to liege",            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Confiscate the corn",               GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Let him go and have the corn",      GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            pPlayer->SEND_GOSSIP_MENU(7340, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Execute your friend painfully",     GOSSIP_SENDER_MAIN+1, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Execute your friend painlessly",    GOSSIP_SENDER_MAIN+2, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Let your friend go",                GOSSIP_SENDER_MAIN+3, GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(7341, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Confront the diplomat",             GOSSIP_SENDER_MAIN+4, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Show not so quiet defiance",        GOSSIP_SENDER_MAIN+5, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Remain quiet",                      GOSSIP_SENDER_MAIN+2, GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(7361, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Speak against your brother openly", GOSSIP_SENDER_MAIN+6, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Help your brother in",              GOSSIP_SENDER_MAIN+7, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Keep your brother out without letting him know", GOSSIP_SENDER_MAIN+8, GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(7362, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take credit, keep gold",            GOSSIP_SENDER_MAIN+5, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take credit, share the gold",       GOSSIP_SENDER_MAIN+4, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Let the knight take credit",        GOSSIP_SENDER_MAIN+3, GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(7363, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Thanks",                            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            pPlayer->SEND_GOSSIP_MENU(7364, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            pCreature->CastSpell(pPlayer, SPELL_FORTUNE, false);
            pPlayer->SEND_GOSSIP_MENU(7365, pCreature->GetObjectGuid());
            break;
    }
}

bool GossipSelect_npc_sayge(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiSender)
    {
        case GOSSIP_SENDER_MAIN:
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+1:
            pCreature->CastSpell(pPlayer, SPELL_DMG, false);
            pPlayer->AddSpellCooldown(SPELL_DMG,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+2:
            pCreature->CastSpell(pPlayer, SPELL_RES, false);
            pPlayer->AddSpellCooldown(SPELL_RES,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+3:
            pCreature->CastSpell(pPlayer, SPELL_ARM, false);
            pPlayer->AddSpellCooldown(SPELL_ARM,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+4:
            pCreature->CastSpell(pPlayer, SPELL_SPI, false);
            pPlayer->AddSpellCooldown(SPELL_SPI,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+5:
            pCreature->CastSpell(pPlayer, SPELL_INT, false);
            pPlayer->AddSpellCooldown(SPELL_INT,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+6:
            pCreature->CastSpell(pPlayer, SPELL_STM, false);
            pPlayer->AddSpellCooldown(SPELL_STM,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+7:
            pCreature->CastSpell(pPlayer, SPELL_STR, false);
            pPlayer->AddSpellCooldown(SPELL_STR,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+8:
            pCreature->CastSpell(pPlayer, SPELL_AGI, false);
            pPlayer->AddSpellCooldown(SPELL_AGI,0,time(NULL) + 7200);
            SendAction_npc_sayge(pPlayer, pCreature, uiAction);
            break;
    }
    return true;
}

/*######
## npc_tabard_vendor
######*/

enum
{
    QUEST_TRUE_MASTERS_OF_LIGHT                = 9737,
    QUEST_THE_UNWRITTEN_PROPHECY               = 9762,
    QUEST_INTO_THE_BREACH                      = 10259,
    QUEST_BATTLE_OF_THE_CRIMSON_WATCH          = 10781,
    QUEST_SHARDS_OF_AHUNE                      = 11972,

    ACHIEVEMENT_EXPLORE_NORTHREND              = 45,
    ACHIEVEMENT_TWENTYFIVE_TABARDS             = 1021,
    ACHIEVEMENT_THE_LOREMASTER_A               = 1681,
    ACHIEVEMENT_THE_LOREMASTER_H               = 1682,

    ITEM_TABARD_OF_THE_HAND                    = 24344,
    ITEM_TABARD_OF_THE_BLOOD_KNIGHT            = 25549,
    ITEM_TABARD_OF_THE_PROTECTOR               = 28788,
    ITEM_OFFERING_OF_THE_SHATAR                = 31408,
    ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI   = 31404,
    ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI  = 31405,
    ITEM_TABARD_OF_THE_SUMMER_SKIES            = 35279,
    ITEM_TABARD_OF_THE_SUMMER_FLAMES           = 35280,
    ITEM_TABARD_OF_THE_ACHIEVER                = 40643,
    ITEM_LOREMASTERS_COLORS                    = 43300,
    ITEM_TABARD_OF_THE_EXPLORER                = 43348,

    SPELL_TABARD_OF_THE_BLOOD_KNIGHT           = 54974,
    SPELL_TABARD_OF_THE_HAND                   = 54976,
    SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI  = 54977,
    SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 54982,
    SPELL_TABARD_OF_THE_ACHIEVER               = 55006,
    SPELL_TABARD_OF_THE_PROTECTOR              = 55008,
    SPELL_LOREMASTERS_COLORS                   = 58194,
    SPELL_TABARD_OF_THE_EXPLORER               = 58224,
    SPELL_TABARD_OF_SUMMER_SKIES               = 62768,
    SPELL_TABARD_OF_SUMMER_FLAMES              = 62769
};

#define GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT               "I've lost my Tabard of Blood Knight."
#define GOSSIP_LOST_TABARD_OF_THE_HAND                   "I've lost my Tabard of the Hand."
#define GOSSIP_LOST_TABARD_OF_THE_PROTECTOR              "I've lost my Tabard of the Protector."
#define GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI  "I've lost my Green Trophy Tabard of the Illidari."
#define GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Purple Trophy Tabard of the Illidari."
#define GOSSIP_LOST_TABARD_OF_SUMMER_SKIES               "I've lost my Tabard of Summer Skies."
#define GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES              "I've lost my Tabard of Summer Flames."
#define GOSSIP_LOST_LOREMASTERS_COLORS                   "I've lost my Loremaster's Colors."
#define GOSSIP_LOST_TABARD_OF_THE_EXPLORER               "I've lost my Tabard of the Explorer."
#define GOSSIP_LOST_TABARD_OF_THE_ACHIEVER               "I've lost my Tabard of the Achiever."

bool GossipHello_npc_tabard_vendor(Player* pPlayer, Creature* pCreature)
{
    bool m_bLostBloodKnight = false;
    bool m_bLostHand        = false;
    bool m_bLostProtector   = false;
    bool m_bLostIllidari    = false;
    bool m_bLostSummer      = false;

    //Tabard of the Blood Knight
    if (pPlayer->GetQuestRewardStatus(QUEST_TRUE_MASTERS_OF_LIGHT) && !pPlayer->HasItemCount(ITEM_TABARD_OF_THE_BLOOD_KNIGHT, 1, true))
        m_bLostBloodKnight = true;

    //Tabard of the Hand
    if (pPlayer->GetQuestRewardStatus(QUEST_THE_UNWRITTEN_PROPHECY) && !pPlayer->HasItemCount(ITEM_TABARD_OF_THE_HAND, 1, true))
        m_bLostHand = true;

    //Tabard of the Protector
    if (pPlayer->GetQuestRewardStatus(QUEST_INTO_THE_BREACH) && !pPlayer->HasItemCount(ITEM_TABARD_OF_THE_PROTECTOR, 1, true))
        m_bLostProtector = true;

    //Green Trophy Tabard of the Illidari
    //Purple Trophy Tabard of the Illidari
    if (pPlayer->GetQuestRewardStatus(QUEST_BATTLE_OF_THE_CRIMSON_WATCH) &&
        (!pPlayer->HasItemCount(ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
        !pPlayer->HasItemCount(ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
        !pPlayer->HasItemCount(ITEM_OFFERING_OF_THE_SHATAR, 1, true)))
        m_bLostIllidari = true;

    //Tabard of Summer Skies
    //Tabard of Summer Flames
    if (pPlayer->GetQuestRewardStatus(QUEST_SHARDS_OF_AHUNE) &&
        !pPlayer->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_SKIES, 1, true) &&
        !pPlayer->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_FLAMES, 1, true))
        m_bLostSummer = true;

    if (m_bLostBloodKnight || m_bLostHand || m_bLostProtector || m_bLostIllidari || m_bLostSummer)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        if (m_bLostBloodKnight)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +1);

        if (m_bLostHand)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_HAND, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +2);

        if (m_bLostProtector)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_PROTECTOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);

        if (m_bLostIllidari)
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
        }

        if (m_bLostSummer)
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_SKIES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
        }

        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    }
    else
        pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_tabard_vendor(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_TRADE:
            pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_TABARD_OF_THE_BLOOD_KNIGHT, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_TABARD_OF_THE_HAND, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_TABARD_OF_THE_PROTECTOR, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_TABARD_OF_SUMMER_SKIES, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_TABARD_OF_SUMMER_FLAMES, false);
            break;
    }
    return true;
}

/*######
## npc_locksmith
######*/

enum
{
    QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ = 10704,
    QUEST_DARK_IRON_LEGACY                = 3802,
    QUEST_THE_KEY_TO_SCHOLOMANCE_A        = 5505,
    QUEST_THE_KEY_TO_SCHOLOMANCE_H        = 5511,
    QUEST_HOTTER_THAN_HELL_A              = 10758,
    QUEST_HOTTER_THAN_HELL_H              = 10764,
    QUEST_RETURN_TO_KHAGDAR               = 9837,
    QUEST_CONTAINMENT                     = 13159,

    ITEM_ARCATRAZ_KEY                     = 31084,
    ITEM_SHADOWFORGE_KEY                  = 11000,
    ITEM_SKELETON_KEY                     = 13704,
    ITEM_SHATTERED_HALLS_KEY              = 28395,
    ITEM_THE_MASTERS_KEY                  = 24490,
    ITEM_VIOLET_HOLD_KEY                  = 42482,

    SPELL_ARCATRAZ_KEY                    = 54881,
    SPELL_SHADOWFORGE_KEY                 = 54882,
    SPELL_SKELETON_KEY                    = 54883,
    SPELL_SHATTERED_HALLS_KEY             = 54884,
    SPELL_THE_MASTERS_KEY                 = 54885,
    SPELL_VIOLET_HOLD_KEY                 = 67253
};

#define GOSSIP_LOST_ARCATRAZ_KEY         "I've lost my key to the Arcatraz."
#define GOSSIP_LOST_SHADOWFORGE_KEY      "I've lost my key to the Blackrock Depths."
#define GOSSIP_LOST_SKELETON_KEY         "I've lost my key to the Scholomance."
#define GOSSIP_LOST_SHATTERED_HALLS_KEY  "I've lost my key to the Shattered Halls."
#define GOSSIP_LOST_THE_MASTERS_KEY      "I've lost my key to the Karazhan."
#define GOSSIP_LOST_VIOLET_HOLD_KEY      "I've lost my key to the Violet Hold."

bool GossipHello_npc_locksmith(Player* pPlayer, Creature* pCreature)
{

    // Arcatraz Key
    if (pPlayer->GetQuestRewardStatus(QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ) && !pPlayer->HasItemCount(ITEM_ARCATRAZ_KEY, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_ARCATRAZ_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +1);

    // Shadowforge Key
    if (pPlayer->GetQuestRewardStatus(QUEST_DARK_IRON_LEGACY) && !pPlayer->HasItemCount(ITEM_SHADOWFORGE_KEY, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHADOWFORGE_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +2);

    // Skeleton Key
    if ((pPlayer->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_A) || pPlayer->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_H)) &&
        !pPlayer->HasItemCount(ITEM_SKELETON_KEY, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SKELETON_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +3);

    // Shatered Halls Key
    if ((pPlayer->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_A) || pPlayer->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_H)) &&
        !pPlayer->HasItemCount(ITEM_SHATTERED_HALLS_KEY, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHATTERED_HALLS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +4);

    // Master's Key
    if (pPlayer->GetQuestRewardStatus(QUEST_RETURN_TO_KHAGDAR) && !pPlayer->HasItemCount(ITEM_THE_MASTERS_KEY, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_THE_MASTERS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +5);

    // Violet Hold Key
    if (pPlayer->GetQuestRewardStatus(QUEST_CONTAINMENT) && !pPlayer->HasItemCount(ITEM_VIOLET_HOLD_KEY, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_VIOLET_HOLD_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +6);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_locksmith(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_ARCATRAZ_KEY, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_SHADOWFORGE_KEY, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_SKELETON_KEY, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_SHATTERED_HALLS_KEY, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_THE_MASTERS_KEY, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->CastSpell(pPlayer, SPELL_VIOLET_HOLD_KEY, false);
            break;
    }
    return true;
}

/*######
## npc_experience_eliminator
######*/

#define GOSSIP_ITEM_STOP_XP_GAIN "I don't want to gain experience anymore."
#define GOSSIP_CONFIRM_STOP_XP_GAIN "Are you sure you want to stop gaining experience?"
#define GOSSIP_ITEM_START_XP_GAIN "I want to be able to gain experience again."
#define GOSSIP_CONFIRM_START_XP_GAIN "Are you sure you want to be able to gain experience once again?"

bool GossipHello_npc_experience_eliminator(Player* pPlayer, Creature* pCreature)
{
	pPlayer->ADD_GOSSIP_ITEM_EXTENDED(
		GOSSIP_ICON_CHAT,
		pPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_XP_USER_DISABLED) ? GOSSIP_ITEM_START_XP_GAIN : GOSSIP_ITEM_STOP_XP_GAIN,
		GOSSIP_SENDER_MAIN,
		GOSSIP_ACTION_INFO_DEF+1,
		pPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_XP_USER_DISABLED) ? GOSSIP_CONFIRM_START_XP_GAIN : GOSSIP_CONFIRM_STOP_XP_GAIN,
		100000,
		false
	);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
	return true;
}

bool GossipSelect_npc_experience_eliminator(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
	if(uiAction == GOSSIP_ACTION_INFO_DEF+1)
	{
	// cheater(?) passed through client limitations
	if(pPlayer->GetMoney() < 100000)
		return true;

	pPlayer->ModifyMoney(-100000);

	if(pPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_XP_USER_DISABLED))
		pPlayer->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_XP_USER_DISABLED);
	else
		pPlayer->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_XP_USER_DISABLED);

	pPlayer->CLOSE_GOSSIP_MENU();
	return true;
	}
pPlayer->CLOSE_GOSSIP_MENU();
return true;
}

/*########
## npc_mirror_image
######*/

enum MirrorImageSpells
{
    SPELL_CLONE_CASTER    = 45204,
    SPELL_CLONE_CASTER_1  = 69837,
//    SPELL_CLONE_CASTER_1  = 58836,
    SPELL_CLONE_THREAT    = 58838,
    SPELL_FIREBLAST       = 59637,
    SPELL_FROSTBOLT       = 59638,
    SPELL_FROSTSHIELD     = 43008,
    SPELL_FIRESHIELD      = 43046,
};

struct MANGOS_DLL_DECL npc_mirror_imageAI : public ScriptedAI
{
    npc_mirror_imageAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiFrostboltTimer;
    uint32 m_uiFireblastTimer;
    bool inCombat;
    Unit *owner;
    float angle;
    bool blocked;
    bool movement;

    void Reset() 
    {
        owner = m_creature->GetOwner();
        if (!owner) 
            return;

        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_2,owner->GetUInt32Value(UNIT_FIELD_BYTES_2));
        m_creature->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        m_creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE);
        m_creature->SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);

        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID,   owner->GetUInt32Value(PLAYER_VISIBLE_ITEM_16_ENTRYID));
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, owner->GetUInt32Value(PLAYER_VISIBLE_ITEM_17_ENTRYID));
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, owner->GetUInt32Value(PLAYER_VISIBLE_ITEM_18_ENTRYID));
        m_creature->SetSpeedRate(MOVE_RUN, owner->GetSpeedRate(MOVE_RUN), true);

        m_uiFrostboltTimer = 500;
        m_uiFireblastTimer = urand(0,3000);
        inCombat = false;
        blocked = false;
        movement = false;


        if (owner && !m_creature->hasUnitState(UNIT_STAT_FOLLOW))
        {
            angle = m_creature->GetAngle(owner);
            m_creature->GetMotionMaster()->Clear(false);
            m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST + 3.0f, angle);
        }

        if(owner->IsPvP())
            m_creature->SetPvP(true);
        if(owner->IsFFAPvP())
            m_creature->SetFFAPvP(true);
    }

    void AttackStart(Unit* pWho)
    {
      if (!pWho) return;

      if (m_creature->Attack(pWho, true))
        {
            if (owner)
                 m_creature->CastSpell(m_creature, SPELL_CLONE_THREAT, true, NULL, NULL, owner->GetObjectGuid());
            m_creature->clearUnitState(UNIT_STAT_FOLLOW);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
            m_creature->AddThreat(pWho, 100.0f);
            DoStartMovement(pWho, 30.0f);
            SetCombatMovement(true);
            inCombat = true;
        }
    }

    void EnterEvadeMode()
    {
     if (m_creature->IsInEvadeMode() || !m_creature->isAlive())
          return;

        inCombat = false;

        m_creature->AttackStop();
        m_creature->CombatStop(true);
        if (owner && !m_creature->hasUnitState(UNIT_STAT_FOLLOW))
        {
            angle = m_creature->GetAngle(owner);
            m_creature->GetMotionMaster()->Clear(false);
            m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST + 3.0f,angle);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!owner || !owner->isAlive()) 
            m_creature->ForcedDespawn();

        if (owner && !m_creature->HasAura(SPELL_CLONE_CASTER))
            m_creature->CastSpell(m_creature, SPELL_CLONE_CASTER, true, NULL, NULL, owner->GetObjectGuid());

        if (owner && !m_creature->HasAura(SPELL_CLONE_CASTER_1))
            m_creature->CastSpell(m_creature, SPELL_CLONE_CASTER_1, true, NULL, NULL, owner->GetObjectGuid());

        if (owner && owner->HasAura(SPELL_FROSTSHIELD) && !m_creature->HasAura(SPELL_FROSTSHIELD))
            m_creature->CastSpell(m_creature, SPELL_FROSTSHIELD, false);

        if (owner && owner->HasAura(SPELL_FIRESHIELD) && !m_creature->HasAura(SPELL_FIRESHIELD))
            m_creature->CastSpell(m_creature, SPELL_FIRESHIELD, false);

        if (!m_creature->getVictim())
            if (owner && owner->getVictim())
                AttackStart(owner->getVictim());

        if (m_creature->getVictim() && m_creature->getVictim() != owner->getVictim())
            AttackStart(owner->getVictim());

        if (inCombat && !m_creature->getVictim())
        {
            EnterEvadeMode();
            return;
        }

        if (!inCombat) 
            return;

        if (m_creature->IsWithinDistInMap(m_creature->getVictim(),30.0f))
        {
            movement = false;
            if (m_uiFrostboltTimer <= diff)
            {
                DoCastSpellIfCan(m_creature->getVictim(),SPELL_FROSTBOLT);
                m_uiFrostboltTimer = urand(1000,6000);
            } else m_uiFrostboltTimer -= diff;

            if (m_uiFireblastTimer <= diff)
            {
                DoCastSpellIfCan(m_creature->getVictim(),SPELL_FIREBLAST);
                m_uiFireblastTimer = urand(4000,8000);
            } else m_uiFireblastTimer -= diff;
        }
        else
            if (!movement) 
            {
                DoStartMovement(m_creature->getVictim(), 20.0f);
                movement = true;
            }

//        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_mirror_image(Creature* pCreature)
{
    return new npc_mirror_imageAI(pCreature);
};

/*####
 ## npc_snake_trap_serpents - Summonned snake id are 19921 and 19833
 ####*/

#define SPELL_MIND_NUMBING_POISON    25810   //Viper
#define SPELL_CRIPPLING_POISON       30981   //Viper
#define SPELL_DEADLY_POISON          34655   //Venomous Snake

#define MOB_VIPER 19921

struct MANGOS_DLL_DECL npc_snake_trap_serpentsAI : public ScriptedAI
{
    npc_snake_trap_serpentsAI(Creature *c) : ScriptedAI(c) {Reset();}

    uint32 SpellTimer;
    bool IsViper;

    void Reset()
    {
        SpellTimer = 500;

        Unit *Owner = m_creature->GetOwner();
        if (!Owner) return;

        CreatureInfo const *Info = m_creature->GetCreatureInfo();

        if (Info->Entry == MOB_VIPER)
            IsViper = true;
        else
            IsViper = false;
    }

    void UpdateAI(const uint32 diff)
    {
        Unit *Owner = m_creature->GetOwner();

        if (!Owner) return;

        if (!m_creature->getVictim())
        {
            if (m_creature->isInCombat())
                DoStopAttack();

            if (Owner->getAttackerForHelper())
                AttackStart(Owner->getAttackerForHelper());
        }

        if (SpellTimer <= diff)
        {
            if (IsViper) //Viper - 19921
            {
                if (urand(0,2) == 0) //33% chance to cast
                {
                    uint32 spell;
                    if (urand(0,1) == 0)
                        spell = SPELL_MIND_NUMBING_POISON;
                    else
                        spell = SPELL_CRIPPLING_POISON;
                    m_creature->CastSpell(m_creature->getVictim(), spell, true);
                }

                SpellTimer = urand(4000, 7000);
            }
            else //Venomous Snake - 19833
            {
                if (urand(0,1) == 0) //80% chance to cast
                    m_creature->CastSpell(m_creature->getVictim(), SPELL_DEADLY_POISON, true);
                SpellTimer = urand(3500, 7500);
            }
        } 
        else 
        {
            SpellTimer -= diff;
            DoMeleeAttackIfReady();
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_snake_trap_serpents(Creature* pCreature)
{
    return new npc_snake_trap_serpentsAI(pCreature);
}

/*########
# npc_rune_blade AI
#########*/

struct MANGOS_DLL_DECL npc_rune_blade : public ScriptedAI
{
    npc_rune_blade(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    Unit* owner;

    void Reset()
    {
        owner = m_creature->GetOwner();
        if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
            return;

        // Cannot be Selected or Attacked
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        m_creature->SetLevel(owner->getLevel());
        m_creature->setFaction(owner->getFaction());

        // Add visible weapon
        if (Item const * item = ((Player *)owner)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, item->GetProto()->ItemId);

        // Add stats scaling
        int32 damageDone=owner->CalculateDamage(BASE_ATTACK, true); // might be average damage instead ?
        int32 meleeSpeed=owner->m_modAttackSpeedPct[BASE_ATTACK];
        m_creature->CastCustomSpell(m_creature, 51906, &damageDone, &meleeSpeed, NULL, true);

        // Visual Glow
        m_creature->CastSpell(m_creature, 53160, true);

        SetCombatMovement(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!owner) return;

        if (!m_creature->getVictim())
        {
            if (owner->getVictim())
                AttackStart(owner->getVictim());
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_rune_blade(Creature* pCreature)
{
    return new npc_rune_blade(pCreature);
}

/*########
# npc_death_knight_gargoyle AI
#########*/

// UPDATE `creature_template` SET `ScriptName` = 'npc_death_knight_gargoyle' WHERE `entry` = '27829';

enum GargoyleSpells
{
    SPELL_GARGOYLE_STRIKE = 51963      // Don't know if this is the correct spell, it does about 700-800 damage points
};

struct MANGOS_DLL_DECL npc_death_knight_gargoyle : public ScriptedAI
{
    npc_death_knight_gargoyle(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }
    uint32 m_uiGargoyleStrikeTimer;
    bool inCombat;
    Unit *owner;


    void Reset() 
    {
     owner = m_creature->GetOwner();
     if (!owner) return;

     m_creature->SetLevel(owner->getLevel());
     m_creature->setFaction(owner->getFaction());

     m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
     m_creature->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
     m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
     m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
     m_creature->SetLevitate(true);

     inCombat = false;
     m_uiGargoyleStrikeTimer = urand(3000, 5000);

     float fPosX, fPosY, fPosZ;
     owner->GetPosition(fPosX, fPosY, fPosZ);

     m_creature->NearTeleportTo(fPosX, fPosY, fPosZ+10.0f, m_creature->GetAngle(owner));


     if (owner && !m_creature->hasUnitState(UNIT_STAT_FOLLOW))
        {
            m_creature->GetMotionMaster()->Clear(false);
            m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST + 3.0f, m_creature->GetAngle(owner));
        }

      if(owner->IsPvP())
                 m_creature->SetPvP(true);
      if(owner->IsFFAPvP())
                 m_creature->SetFFAPvP(true);
    }

    void EnterEvadeMode()
    {
     if (m_creature->IsInEvadeMode() || !m_creature->isAlive())
          return;

        inCombat = false;

        m_creature->AttackStop();
        m_creature->CombatStop(true);
        if (owner && !m_creature->hasUnitState(UNIT_STAT_FOLLOW))
        {
            m_creature->GetMotionMaster()->Clear(false);
            m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST + 3.0f, m_creature->GetAngle(owner));
        }
    }

    void AttackStart(Unit* pWho)
    {
      if (!pWho) return;

      if (m_creature->Attack(pWho, true))
        {
            m_creature->clearUnitState(UNIT_STAT_FOLLOW);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
            m_creature->AddThreat(pWho, 100.0f);
            DoStartMovement(pWho, 10.0f);
            SetCombatMovement(true);
            inCombat = true;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {

        if (!owner || !owner->IsInWorld())
        {
            m_creature->ForcedDespawn();
            return;
        }

        if (!m_creature->getVictim())
            if (owner && owner->getVictim())
                AttackStart(owner->getVictim());

        if (m_creature->getVictim() && m_creature->getVictim() != owner->getVictim())
                AttackStart(owner->getVictim());

        if (inCombat && !m_creature->getVictim())
        {
            EnterEvadeMode();
            return;
        }

        if (!inCombat) return;

        if (m_uiGargoyleStrikeTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_GARGOYLE_STRIKE);
            m_uiGargoyleStrikeTimer = urand(3000, 5000);
        }
        else m_uiGargoyleStrikeTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_death_knight_gargoyle(Creature* pCreature)
{
    return new npc_death_knight_gargoyle(pCreature);
}

/*######
## npc_training_dummy
######*/

#define OUT_OF_COMBAT_TIME 5000
#define STUN_DURATION      55000
#define SPELL_STUN_4EVER   36877

struct MANGOS_DLL_DECL npc_training_dummyAI : public Scripted_NoMovementAI
{
    npc_training_dummyAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        Reset();
    }

    uint32 combat_timer;
    uint32 stun_timer;

    void Reset()
    {
        m_creature->CastSpell(m_creature, SPELL_STUN_4EVER, true);
        stun_timer = STUN_DURATION;
        combat_timer = 0;
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        combat_timer = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
      
        if (m_creature->GetHealthPercent() < 1.0f) // allow players using finishers
            m_creature->ModifyHealth(m_creature->GetMaxHealth());

        combat_timer += diff;
        if (stun_timer <= diff)
        {
            m_creature->CastSpell(m_creature, SPELL_STUN_4EVER, true);
            stun_timer = STUN_DURATION;
        }
        else
            stun_timer -= diff;

        if (combat_timer > OUT_OF_COMBAT_TIME)
            EnterEvadeMode();
    }
};

CreatureAI* GetAI_npc_training_dummy(Creature* pCreature)
{
    return new npc_training_dummyAI(pCreature);
}

/*######
# npc_risen_ally
#######*/

struct MANGOS_DLL_DECL npc_risen_allyAI : public ScriptedAI
{
    npc_risen_allyAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
    }

    uint32 StartTimer;

    void Reset()
    {
        StartTimer = 2000;
        m_creature->SetSheath(SHEATH_STATE_MELEE);
        m_creature->SetByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_ABANDONED);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
        m_creature->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        m_creature->SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);
        if (Player* creator = m_creature->GetMap()->GetPlayer(m_creature->GetCreatorGuid()))
        {
           m_creature->SetLevel(creator->getLevel());
           m_creature->setFaction(creator->getFaction());
        }
    }

    void JustDied(Unit* killer)
    {
        if (!m_creature)
            return;

        if (Player* creator = m_creature->GetMap()->GetPlayer(m_creature->GetCreatorGuid()))
        {
            creator->RemoveAurasDueToSpell(46619);
            creator->RemoveAurasDueToSpell(62218);
        }
    }

    void AttackStart(Unit* pWho)
    {
        if (!pWho) return;

        if (m_creature->Attack(pWho, true))
        {
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
            DoStartMovement(pWho, 10.0f);
            SetCombatMovement(true);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if(StartTimer > uiDiff)
        {
            StartTimer -= uiDiff;
            return;
        }

        if(!m_creature->isCharmed())
            m_creature->ForcedDespawn();

        if (m_creature->isInCombat())
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_risen_ally(Creature* pCreature)
{
    return new npc_risen_allyAI(pCreature);
}

/*######
# npc_explosive_decoy
######*/

struct MANGOS_DLL_DECL npc_explosive_decoyAI : public ScriptedAI
{
    npc_explosive_decoyAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    Player* p_owner;

    void Reset()
    {
        p_owner = NULL;
        SetCombatMovement(false);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_creature || !m_creature->isAlive())
            return;

        if (uiDamage > 0)
            m_creature->CastSpell(m_creature, 53273, true);
    }

    void JustDied(Unit* killer)
    {
        if (!m_creature || !p_owner)
            return;

        SpellEntry const* createSpell = GetSpellStore()->LookupEntry(m_creature->GetUInt32Value(UNIT_CREATED_BY_SPELL));

        if (createSpell)
            p_owner->SendCooldownEvent(createSpell);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (p_owner)
            return;

        p_owner = m_creature->GetMap()->GetPlayer(m_creature->GetCreatorGuid());

        if (!p_owner) 
            return;

        m_creature->setFaction(p_owner->getFaction());
        m_creature->SetCreatorGuid(ObjectGuid());
    }
};

CreatureAI* GetAI_npc_explosive_decoy(Creature* pCreature)
{
    return new npc_explosive_decoyAI(pCreature);
}

/*######
# npc_eye_of_kilrogg
######*/

struct MANGOS_DLL_DECL npc_eye_of_kilrogg : public ScriptedAI
{
    npc_eye_of_kilrogg(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    Player* p_owner;

    void Reset()
    {
        p_owner = NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if (p_owner)
            return;

        p_owner = (Player*)m_creature->GetCharmerOrOwner();

        if (!p_owner)
            return;

        if (!m_creature->HasAura(2585))
            m_creature->CastSpell(m_creature, 2585, true);

        if (p_owner->HasAura(58081))
            m_creature->CastSpell(m_creature, 58083, true);

    }
};

CreatureAI* GetAI_npc_eye_of_kilrogg(Creature* pCreature)
{
    return new npc_eye_of_kilrogg(pCreature);
}

/*######
# npc_battle_standard
######*/

#define DESPAWN_TIME 120000 // 2 minutes

struct MANGOS_DLL_DECL npc_battle_standard : public ScriptedAI
{
	npc_battle_standard(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

	int32 despawn_timer;

	void Reset()
	{
		despawn_timer = DESPAWN_TIME;
		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
        m_creature->SetSpeedRate(MOVE_WALK, 0.0f);
		m_creature->SetSpeedRate(MOVE_RUN, 0.0f);
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if (despawn_timer <= 0)
			m_creature->ForcedDespawn();
		else
			despawn_timer -= uiDiff;
	}
};

CreatureAI* GetAI_npc_battle_standard(Creature* pCreature)
{
	return new npc_battle_standard(pCreature);
}

namespace Pasqua2011
{

#define GOSSIP_MASTRO_ERMES "Voglio fare lo Zabaione Speciale."
/*##################################
#Quest Zabaione Speciale           #
###################################*/
enum
{
    QUEST_ZABAIONE_SPECIALE = 112644,
    SAY_EVENT_STARTED = -2760000,
    SAY_ADD_BANNANAS  = -2760001,
    SAY_ADD_PAPAYA    = -2760002,
    SAY_GOOD_JOB_1    = -2760003,
    SAY_BRAZIER       = -2760004,
    SAY_GOOD_JOB_2    = -2760005,
    SAY_OPEN_VALVE    = -2760006,
    SAY_GOOD_JOB_3    = -2760007,
    SAY_GOOD_JOB_4    = -2760008,
    SAY_EVENT_END     = -2760009,
    SAY_ADD_ORANGE    = -2760010,
    SAY_FAIL          = -2760011,
    GO_JUNGLE_PUNCH   =  490643
};
int32 GoogJobText[4] = {SAY_GOOD_JOB_1,SAY_GOOD_JOB_2,SAY_GOOD_JOB_3,SAY_GOOD_JOB_4};
struct MANGOS_DLL_DECL npc_mastro_ermesAI : public ScriptedAI
{
    npc_mastro_ermesAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool bEventCanBeStarted;
    bool bEventStarted;
    uint32 m_uiStepTimer;
    uint32 m_uiEventStartTimer;
    uint32 uiPhase;
    uint32 uiObjectToClick;
    uint32 uiCurrentClickedObject;
    uint32 uiPhaseCounter;
    /*
       banana         = 1
       orange         = 2
       papaya         = 3
       pressure valve = 4
       brazier        = 5

    */
    void Reset()
    {
        bEventCanBeStarted      = false;
        bEventStarted           = false;
        m_uiStepTimer           = 10000;
        m_uiEventStartTimer     = 5000;
        uiPhase                 = 0;
        uiObjectToClick         = 0;
        uiCurrentClickedObject  = 0;
        uiPhaseCounter          = 0;
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    bool CheckClickedObject(uint32 uiObjectToClick)
    {
        if(uiObjectToClick == uiCurrentClickedObject)
            return true;

        else return false;

    }
    void SayEventText(uint32 uiTextIdentifier)
    {
        switch(uiTextIdentifier)
        {
            case 1: DoScriptText(SAY_ADD_BANNANAS, m_creature); break;
            case 2: DoScriptText(SAY_ADD_ORANGE, m_creature);   break;
            case 3: DoScriptText(SAY_ADD_PAPAYA, m_creature);   break;
            case 4: DoScriptText(SAY_OPEN_VALVE, m_creature);   break;
            case 5: DoScriptText(SAY_BRAZIER, m_creature);      break;
            default: break;
        }
    }

    void UpdateAI (uint32 const uiDiff)
    {
        if (bEventCanBeStarted)
        {
                if (m_uiEventStartTimer <= uiDiff && !bEventStarted)
                {
                    uiObjectToClick = urand(1,3);
                    SayEventText(uiObjectToClick);
                    m_uiEventStartTimer = 0;
                    bEventStarted = true;
                    uiPhase++;
                }
                else m_uiEventStartTimer -= uiDiff;

                if(bEventStarted)
                {
                    if (m_uiStepTimer <=uiDiff)
                    {
                        switch(uiPhase)
                        {
                            case 1:
                                if(CheckClickedObject(uiObjectToClick))
                                {
                                    DoScriptText(GoogJobText[urand(0,3)] , m_creature);
                                    uiObjectToClick = urand(1,3);
                                    SayEventText(uiObjectToClick);

                                    uiPhaseCounter++;
                                    if(uiPhaseCounter == 3){ uiPhase++; uiPhaseCounter=0;}
                                }
                                else {Reset(); DoScriptText(SAY_FAIL, m_creature);}
                                break;

                             case 2:
                                if(CheckClickedObject(uiObjectToClick))
                                {
                                    DoScriptText(GoogJobText[urand(0,3)] , m_creature);
                                    uiObjectToClick = urand(4,5);
                                    SayEventText(uiObjectToClick);
                                    uiPhase++;
                                }
                                else {Reset(); DoScriptText(SAY_FAIL, m_creature);}
                                break;
                             case 3:
                                 if(CheckClickedObject(uiObjectToClick))
                                 {
                                    DoScriptText(GoogJobText[urand(0,3)] , m_creature);
                                    uiObjectToClick = urand(1,5);
                                    SayEventText(uiObjectToClick);
                                    uiPhaseCounter++;
                                    if(uiPhaseCounter == 6)uiPhase++;
                                 }
                                 else {Reset(); DoScriptText(SAY_FAIL, m_creature);}
                                 break;
                             case 4:
                                 DoScriptText(SAY_EVENT_END , m_creature);
                                 if(GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_JUNGLE_PUNCH, 15.0f))
                                 {
                                     pGo->SetRespawnTime(3*MINUTE);        //despawn object in ? seconds
                                     pGo->Respawn();

                                     Reset();
                                 }
                                 break;

                             default: break;

                        }
                        m_uiStepTimer=5000;
                    }
                    else m_uiStepTimer -=uiDiff;

                }
        }
    }
};

CreatureAI* GetAI_npc_mastro_ermes(Creature* pCreature)
{
    return new npc_mastro_ermesAI(pCreature);
};

bool GossipHello_npc_mastro_ermes(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_ZABAIONE_SPECIALE) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MASTRO_ERMES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_mastro_ermes(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        if (npc_mastro_ermesAI* pErmesAI = dynamic_cast<npc_mastro_ermesAI*>(pCreature->AI()))
        {
            pErmesAI->bEventCanBeStarted=true;
            pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoScriptText(SAY_EVENT_STARTED, pCreature);
        }
    }
    return true;
}

/*################################
#Quest Zabaione Speciale - gameobjects #
#################################*/
enum
{
    GO_BARREL_OF_ORANGES           =  490637,
    GO_BARREL_OF_PAPAYAS           =  490639,
    GO_BARREL_OF_BANANAS           =  490638,
    GO_PRESSURE_VALVE              =  490635,
    GO_BRAZIER                     =  490636,
    NPC_MASTRO_ERMES              =  428566,
};

bool GOUse_go_zabaione(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_ZABAIONE_SPECIALE) == QUEST_STATUS_INCOMPLETE)
    {
        if(Creature* pErmes = GetClosestCreatureWithEntry(pPlayer, NPC_MASTRO_ERMES, 30.0f))
        {
            if (npc_mastro_ermesAI* pErmesAI = dynamic_cast<npc_mastro_ermesAI*>(pErmes->AI()))
            {
                 switch (pGo->GetEntry())
                {
                    case GO_BARREL_OF_ORANGES: pErmesAI->uiCurrentClickedObject = 2; break;
                    case GO_BARREL_OF_PAPAYAS: pErmesAI->uiCurrentClickedObject = 3; break;
                    case GO_BARREL_OF_BANANAS: pErmesAI->uiCurrentClickedObject = 1; break;
                    case GO_PRESSURE_VALVE:    pErmesAI->uiCurrentClickedObject = 4; break;
                    case GO_BRAZIER:           pErmesAI->uiCurrentClickedObject = 5; break;

                        break;
                }
            }
        }
    }
    return false;
};

}

#define GOSSIP_ITEM_1 "Heal me.. please.. "
#define GOSSIP_ITEM_2 "BRRRAINS! "
// Argent Healer Gossip Menu
//This function is called when the player opens the gossip menubool
bool GossipHello_npc_argent_healer(Player* pPlayer, Creature* pCreature)
{
    if(pPlayer->HasAura(43958))
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        pPlayer->PlayerTalkClass->SendGossipMenu(907, pCreature->GetObjectGuid());
    }
    return true;
}

bool GossipSelect_npc_argent_healer(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiSender == GOSSIP_SENDER_MAIN)
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                //pPlayer->CastSpell(pPlayer, SPELL_SHIELD_OF_BLUE, false);
                pCreature->MonsterSay("May the Light purge your infection", LANG_UNIVERSAL, pPlayer);
                pCreature->CastSpell(pPlayer,528,true);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->CLOSE_GOSSIP_MENU();
        }
    }
    return true;
}

/*######
## npc_winter_reveler
######*/

enum
{
    AURA_MISTLETOE          = 26004,
    ITEM_SNOWFLAKES         = 34191,
    ITEM_MISTLETOE          = 21519,
    ITEM_FRESH_HOLLY        = 21212,
};

struct MANGOS_DLL_DECL npc_winter_revelerAI : public ScriptedAI
{
    npc_winter_revelerAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void ReceiveEmote(Player* pPlayer, uint32 uiEmote)
    {
        if (uiEmote == TEXTEMOTE_KISS)
        {
            if (!pPlayer->HasAura(AURA_MISTLETOE))
            {
                switch (rand()%3)
                {
                    case 0:
                    {
                        ItemPosCountVec dest;
                        InventoryResult msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_SNOWFLAKES, 1, NULL);
                        if (msg == EQUIP_ERR_OK)
                            pPlayer->StoreNewItem(dest, ITEM_SNOWFLAKES, true);
                        else
                            pPlayer->SendEquipError(msg, NULL, NULL);
                        break;
                    }
                    case 1:
                    {
                        ItemPosCountVec dest;
                        InventoryResult msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_MISTLETOE, 1, NULL);
                        if (msg == EQUIP_ERR_OK)
                            pPlayer->StoreNewItem(dest, ITEM_MISTLETOE, true);
                        else
                            pPlayer->SendEquipError(msg, NULL, NULL);
                        break;
                    }
                    case 2:
                    {
                        ItemPosCountVec dest;
                        InventoryResult msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_FRESH_HOLLY, 1, NULL);
                        if (msg == EQUIP_ERR_OK)
                            pPlayer->StoreNewItem(dest, ITEM_FRESH_HOLLY, true);
                        else
                            pPlayer->SendEquipError(msg, NULL, NULL);
                        break;
                    }
                }
                m_creature->CastSpell(pPlayer, AURA_MISTLETOE, false);
            }
        }
    }


    void Reset() {}
};

CreatureAI* GetAI_npc_winter_reveler(Creature* pCreature)
{
    return new npc_winter_revelerAI(pCreature);
}

/*######
## npc_metzen
######*/

#define GOSSIP_TEXT_METZEN          "Berieselt Metzten mit etwas Rentierstaub"

enum
{
    QUEST_METZEN_ALLIANCE           = 8762,
    QUEST_METZEN_HORDE              = 8746,
    NPC_METZEN_KILL_CREDIT          = 15664,
    SPELL_REINDEER_DUST             = 25952
};

bool GossipHello_npc_metzen(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT_METZEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    pPlayer->SEND_GOSSIP_MENU(8076, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_metzen(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if(uiSender != GOSSIP_SENDER_MAIN) return true;

    if(uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        if((pPlayer->GetQuestStatus(QUEST_METZEN_ALLIANCE) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(QUEST_METZEN_HORDE) == QUEST_STATUS_INCOMPLETE))
        {
            pPlayer->KilledMonsterCredit(NPC_METZEN_KILL_CREDIT);
            pCreature->CastSpell(pCreature, SPELL_REINDEER_DUST, true);
        }
    }
    return true;
}

/*######
# npc_wormhole
######*/

enum eWormhole
{
    SPELL_BOREAN_TUNDRA         = 67834,
    SPELL_HOWLING_FJORD         = 67838,
    SPELL_SHOLAZAR_BASIN        = 67835,
    SPELL_ICECROWN              = 67836,
    SPELL_STORM_PEAKS           = 67837,

    TEXT_WORMHOLE               = 907,

    MAP_NORTHREND               = 571
};

static const char* WormholeGossip[6] =
{
    "Borean Tundra.",
    "Howling Fjord.",
    "Sholazar Basin.",
    "Icecrown.",
    "Storm Peaks.",
    "Underground...",
};

static float WormholeTele[7][4] =
{
    {3172.33f, 5608.70f, 595.01f, 1.18f},                   // Borean Tundra 1
    {1180.55f, -4876.56f, 408.80f, 0.43f},                  // Howling Fjord
    {6234.36f, 4766.68f, 224.71f, 4.23f},                   // Sholazar Basin
    {8113.86f, 1352.32f, 848.87f, 0.26f},                   // Icecrown
    {8992.79f, -1222.29f, 1058.40f, 2.87f},                 // Storm Peaks
    {5859.29f, 516.25f, 599.81f, 3.15f},                    // Underground (under Horde section of Dalaran)
    {4297.19f, 5464.91f, 52.60f, 3.38f},                    // Borean Tundra 2
};

bool GossipHello_npc_wormhole(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetSkillValue(SKILL_ENGINEERING) >= 415)
    {
        for (uint8 i=0; i<=5; ++i)
        {
            if (i == 5 && urand(0, 19))
                continue;
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, WormholeGossip[i], GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+i+1);
        }

        pPlayer->PlayerTalkClass->SendGossipMenu(TEXT_WORMHOLE, pCreature->GetObjectGuid());
    }
    return true;
}

bool GossipSelect_npc_wormhole(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    pPlayer->PlayerTalkClass->ClearMenus();

    uint8 uiI = uiAction - GOSSIP_ACTION_INFO_DEF - 1;
    uint8 uiH;

    if (uiI != 5 && !urand(0, 9))
        uiH = urand(10, 50);
    else
        uiH = 0;

    pPlayer->CLOSE_GOSSIP_MENU();
    // borean tundra has two locations and we have to pick one randomly
    if(uiI == 0 && urand(0,1))
        uiI = 6;
    pPlayer->TeleportTo(MAP_NORTHREND, WormholeTele[uiI][0], WormholeTele[uiI][1], WormholeTele[uiI][2]+uiH, WormholeTele[uiI][3]);
    pCreature->ForcedDespawn();
    return true;
};

/*#####
# npc_bunny_bark
#####*/
enum
{
    NPC_BUNNY_1             = 24202,
    NPC_BUNNY_2             = 24203,
    NPC_BUNNY_3             = 24204,
    NPC_BUNNY_4             = 24205,

    QUEST_BARK_FOR_BARLEY   = 11293,
    QUEST_BARK_FOR_THUNDER  = 11294,
    QUEST_BARK_FOR_DROHN    = 11407,
    QUEST_BARK_FOR_TCHALI   = 11408,

    SAY_DROHN_1             = -2000001,
    SAY_DROHN_2             = -2000002,
    SAY_DROHN_3             = -2000003,
    SAY_DROHN_4             = -2000004,
    SAY_TCHALI_1            = -2000005,
    SAY_TCHALI_2            = -2000006,
    SAY_TCHALI_3            = -2000007,
    SAY_TCHALI_4            = -2000008,
    SAY_BARLEY_1            = -2000009,
    SAY_BARLEY_2            = -2000010,
    SAY_BARLEY_3            = -2000011,
    SAY_BARLEY_4            = -2000012,
    SAY_THUNDER_1           = -2000013,
    SAY_THUNDER_2           = -2000014,
    SAY_THUNDER_3           = -2000015,
    SAY_THUNDER_4           = -2000016
};

struct MANGOS_DLL_DECL npc_bunny_bark : public ScriptedAI
{
    npc_bunny_bark (Creature* pCreature) : ScriptedAI (pCreature)
    {
        m_pMap = pCreature->GetMap();
        Reset();
    }

    Map* m_pMap;

    void Reset () {}

    void MoveInLineOfSight (Unit* pWho)
    {
        // player should be near the npc
        if (m_creature->GetDistance(pWho) > 10.0f)
            return;

        if (pWho->GetTypeId() != TYPEID_PLAYER)
            return;

        Player* pPlayer = m_pMap->GetPlayer(pWho->GetObjectGuid());
        if (!pPlayer)
            return;

        if (pPlayer->GetQuestStatus(QUEST_BARK_FOR_BARLEY) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(QUEST_BARK_FOR_THUNDER) == QUEST_STATUS_INCOMPLETE ||
            pPlayer->GetQuestStatus(QUEST_BARK_FOR_DROHN) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(QUEST_BARK_FOR_TCHALI) == QUEST_STATUS_INCOMPLETE )
        {
            int32 iText = 0;
            switch (m_creature->GetEntry())
            {
            case NPC_BUNNY_1:
                if (pPlayer->HasQuest(QUEST_BARK_FOR_BARLEY))
                    iText = SAY_BARLEY_1;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_THUNDER))
                    iText = SAY_THUNDER_1;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_DROHN))
                    iText = SAY_DROHN_1;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_TCHALI))
                    iText = SAY_TCHALI_1;
                break;
            case NPC_BUNNY_2:
                if (pPlayer->HasQuest(QUEST_BARK_FOR_BARLEY))
                    iText = SAY_BARLEY_2;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_THUNDER))
                    iText = SAY_THUNDER_2;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_DROHN))
                    iText = SAY_DROHN_2;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_TCHALI))
                    iText = SAY_TCHALI_2;
                break;
            case NPC_BUNNY_3:
                if (pPlayer->HasQuest(QUEST_BARK_FOR_BARLEY))
                    iText = SAY_BARLEY_3;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_THUNDER))
                    iText = SAY_THUNDER_3;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_DROHN))
                    iText = SAY_DROHN_3;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_TCHALI))
                    iText = SAY_TCHALI_3;
                break;
            case NPC_BUNNY_4:
                if (pPlayer->HasQuest(QUEST_BARK_FOR_BARLEY))
                    iText = SAY_BARLEY_4;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_THUNDER))
                    iText = SAY_THUNDER_4;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_DROHN))
                    iText = SAY_DROHN_4;
                else if (pPlayer->HasQuest(QUEST_BARK_FOR_TCHALI))
                    iText = SAY_TCHALI_4;
                break;
            }
            if (iText)
                DoScriptText(iText, pPlayer, NULL);
            pPlayer->KilledMonsterCredit(m_creature->GetEntry());
        }
    }
};

CreatureAI* GetAI_npc_bunny_bark(Creature* pCreature)
{
    return new npc_bunny_bark(pCreature);
}

/*#####
# npc_bunny_fire_training
#####*/
enum
{
    NPC_BUNNY_FIRE_CREDIT   = 23537,
    SPELL_BUCKET_LANDS      = 42339
};

struct MANGOS_DLL_DECL npc_bunny_fire_trainingAI : public ScriptedAI
{
    npc_bunny_fire_trainingAI (Creature* pCreature) : ScriptedAI (pCreature)
    {
        Reset();
    }

    void Reset () {}

    void SpellHit(Unit* pWho, const SpellEntry* pSpell)
    {
        if (pWho->GetTypeId() != TYPEID_PLAYER)
            return;

        if (pSpell->Id == SPELL_BUCKET_LANDS)
        {
            ((Player*)pWho)->KilledMonsterCredit(NPC_BUNNY_FIRE_CREDIT);
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->ForcedDespawn();
        }
    }
};

CreatureAI* GetAI_npc_bunny_fire_training(Creature* pCreature)
{
    return new npc_bunny_fire_trainingAI(pCreature);
}

/*#####
# npc_bunny_fire_town
#####*/
enum
{
    NPC_BUNNY_HORSEMAN   = 23543,
};

struct sLocation
{
    float x, y, z, o;
    uint32 map;
    uint32 zone;
    uint32 area;
};

static sLocation asLocation [6] =
{
//   X            Y             Z           O           MapId   ZoneId  AreaId
    {-9458.885f,   62.351f,       55.785f,     6.264f,      0,      12,     87  },      // Elwyn's Forest, Goldshire
    {-5602.197f,   -483.211f,     396.981f,    3.000f,      0,      1,      131 },      // Dun Murogh, Kharanos
    {-4179.134f,   -12483.937f,   44.348f,     6.275f,      530,    3524,   3576},      // Azuremyst Isle, Azure Watch
    {2259.250f,    290.430f,      34.114f,     0.987f,      0,      85,     159 },      // Trisifal Glades, Brill
    {322.1362f,    -4742.212f,    9.656f,      3.180f,      1,       14,     362 },      // Durotar, Razor Hill
    {9525.682f,    -6830.646f,    16.493f,     2.969f,      530,    3430,   3665}       // Eversong Woods, Falconing Square
};

struct MANGOS_DLL_DECL npc_bunny_fire_townAI : public ScriptedAI
{
    npc_bunny_fire_townAI (Creature* pCreature) : ScriptedAI (pCreature)
    {
        m_uiMapId = pCreature->GetMapId();
        pCreature->GetZoneAndAreaId(m_uiZoneId, m_uiAreaId);
        Reset();
    }

    uint32 m_uiMapId;
    uint32 m_uiAreaId;
    uint32 m_uiZoneId;
    bool m_bBucketReceived;

    void Reset ()
    {

        m_bBucketReceived = false;
    }
    void SpellHit(Unit* pWho, const SpellEntry* pSpell)
    {
        if (pWho->GetTypeId() != TYPEID_PLAYER)
            return;

        if (pSpell->Id == SPELL_BUCKET_LANDS)
            m_bBucketReceived = true;
    }

    void UpdateAI (const uint32 uiDiff)
    {
        if (m_bBucketReceived)
        {
            uint8 uiIndex;
            bool bFind = false;
            for (uiIndex = 0; uiIndex < 6; uiIndex++)
            {
                if (m_uiMapId == asLocation[uiIndex].map && m_uiZoneId == asLocation[uiIndex].zone && m_uiAreaId == asLocation[uiIndex].area)
                {
                    bFind = true;
                    break;
                }
            }

            if (bFind)
            {
                Creature* pHorseman = m_creature->SummonCreature(NPC_BUNNY_HORSEMAN, asLocation[uiIndex].x, asLocation[uiIndex].y, asLocation[uiIndex].z, asLocation[uiIndex].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                pHorseman->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
                pHorseman->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                pHorseman->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pHorseman->setFaction(14);
                pHorseman->SetPhaseMask(2,true);
            }
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->ForcedDespawn();
        }
    }
};

CreatureAI* GetAI_npc_bunny_fire_town(Creature* pCreature)
{
    return new npc_bunny_fire_townAI(pCreature);
}

/*#####
# npc_bunny_horseman
#####*/
enum
{
    SPELL_SUMMON_LANTERN_MISSILE        = 44255,
    SPELL_SUMMON_LANTERN                = 44231,
    QUEST_LET_THE_FIRE_COMES_A          = 12135,
    QUEST_LET_THE_FIRE_COMES_H          = 12139,
    SAY_HORSEMAN_SHADE_SUMMONED         = -2760020,
    SAY_HORSEMAN_SHADE_DEATH            = -2760021
};

struct MANGOS_DLL_DECL npc_bunny_horsemanAI : ScriptedAI
{
    npc_bunny_horsemanAI (Creature* pCreature) : ScriptedAI (pCreature)
    {
        Reset();
    }

    void Reset()
    {
        DoScriptText(SAY_HORSEMAN_SHADE_SUMMONED, m_creature);
    }

    void JustDied (Unit* pKiller)
    {
        DoCast(m_creature, SPELL_SUMMON_LANTERN);
        DoScriptText(SAY_HORSEMAN_SHADE_DEATH, m_creature);

        // to give the quest completed to all player near the Horseman
        Map* pMap = m_creature->GetMap();
        Map::PlayerList const &lPlayers = pMap->GetPlayers();
        for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
        {
            Player* pPlayer = itr->getSource();
            if(pPlayer)
            {
                // consider only player near the corpse
                if (!pPlayer->IsWithinDist2d(m_creature->GetPositionX(), m_creature->GetPositionY(), 20.0f))
                    continue;

                if (pPlayer->GetQuestStatus(QUEST_LET_THE_FIRE_COMES_A) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->SetQuestStatus(QUEST_LET_THE_FIRE_COMES_A, QUEST_STATUS_COMPLETE);

                if (pPlayer->GetQuestStatus(QUEST_LET_THE_FIRE_COMES_H) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->SetQuestStatus(QUEST_LET_THE_FIRE_COMES_H, QUEST_STATUS_COMPLETE);
            }
        }
    }
};

CreatureAI* GetAI_npc_bunny_horseman(Creature* pCreature)
{
    return new npc_bunny_horsemanAI(pCreature);
}

void AddSC_npcs_special()
{    
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_air_force_bots";
    newscript->GetAI = &GetAI_npc_air_force_bots;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_chicken_cluck";
    newscript->GetAI = &GetAI_npc_chicken_cluck;
    newscript->pQuestAcceptNPC =   &QuestAccept_npc_chicken_cluck;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_chicken_cluck;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dancing_flames";
    newscript->GetAI = &GetAI_npc_dancing_flames;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_injured_patient";
    newscript->GetAI = &GetAI_npc_injured_patient;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_doctor";
    newscript->GetAI = &GetAI_npc_doctor;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_doctor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_garments_of_quests";
    newscript->GetAI = &GetAI_npc_garments_of_quests;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_guardian";
    newscript->GetAI = &GetAI_npc_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_innkeeper";
    newscript->pGossipHello = &GossipHello_npc_innkeeper;
    newscript->pGossipSelect = &GossipSelect_npc_innkeeper;
    newscript->RegisterSelf(false);                         // script and error report disabled, but script can be used for custom needs, adding ScriptName
    
    newscript = new Script;
    newscript->Name = "npc_kingdom_of_dalaran_quests";
    newscript->pGossipHello =  &GossipHello_npc_kingdom_of_dalaran_quests;
    newscript->pGossipSelect = &GossipSelect_npc_kingdom_of_dalaran_quests;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lunaclaw_spirit";
    newscript->pGossipHello =  &GossipHello_npc_lunaclaw_spirit;
    newscript->pGossipSelect = &GossipSelect_npc_lunaclaw_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mount_vendor";
    newscript->pGossipHello =  &GossipHello_npc_mount_vendor;
    newscript->pGossipSelect = &GossipSelect_npc_mount_vendor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_rogue_trainer";
    newscript->pGossipHello =  &GossipHello_npc_rogue_trainer;
    newscript->pGossipSelect = &GossipSelect_npc_rogue_trainer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_sayge";
    newscript->pGossipHello = &GossipHello_npc_sayge;
    newscript->pGossipSelect = &GossipSelect_npc_sayge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tabard_vendor";
    newscript->pGossipHello =  &GossipHello_npc_tabard_vendor;
    newscript->pGossipSelect = &GossipSelect_npc_tabard_vendor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_locksmith";
    newscript->pGossipHello =  &GossipHello_npc_locksmith;
    newscript->pGossipSelect = &GossipSelect_npc_locksmith;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_experience_eliminator";
    newscript->pGossipHello = &GossipHello_npc_experience_eliminator;
    newscript->pGossipSelect = &GossipSelect_npc_experience_eliminator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mirror_image";
    newscript->GetAI = &GetAI_npc_mirror_image;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_snake_trap_serpents";
    newscript->GetAI = &GetAI_npc_snake_trap_serpents;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_runeblade";
    newscript->GetAI = &GetAI_npc_rune_blade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_death_knight_gargoyle";
    newscript->GetAI = &GetAI_npc_death_knight_gargoyle;
    newscript->RegisterSelf();  
    
    newscript = new Script;
    newscript->Name = "npc_risen_ally";
    newscript->GetAI = &GetAI_npc_risen_ally;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_explosive_decoy";
    newscript->GetAI = &GetAI_npc_explosive_decoy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_eye_of_kilrogg";
    newscript->GetAI = &GetAI_npc_eye_of_kilrogg;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_battle_standard";
    newscript->GetAI = &GetAI_npc_battle_standard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_zabaione";
    newscript->pGOUse = &Pasqua2011::GOUse_go_zabaione;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mastro_ermes";
    newscript->GetAI = &Pasqua2011::GetAI_npc_mastro_ermes;
    newscript->pGossipHello = &Pasqua2011::GossipHello_npc_mastro_ermes;
    newscript->pGossipSelect = &Pasqua2011::GossipSelect_npc_mastro_ermes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_argent_healer";
    newscript->pGossipHello = &GossipHello_npc_argent_healer;
    newscript->pGossipSelect = &GossipSelect_npc_argent_healer;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_training_dummy";
    newscript->GetAI = &GetAI_npc_training_dummy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_winter_reveler";
    newscript->GetAI = &GetAI_npc_winter_reveler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_metzen";
    newscript->pGossipHello = &GossipHello_npc_metzen;
    newscript->pGossipSelect = &GossipSelect_npc_metzen;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_wormhole";
    newscript->pGossipHello =  &GossipHello_npc_wormhole;
    newscript->pGossipSelect = &GossipSelect_npc_wormhole;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bunny_bark";
    newscript->GetAI = &GetAI_npc_bunny_bark;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bunny_fire_training";
    newscript->GetAI = &GetAI_npc_bunny_fire_training;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bunny_fire_town";
    newscript->GetAI = &GetAI_npc_bunny_fire_town;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bunny_horseman";
    newscript->GetAI = &GetAI_npc_bunny_horseman;
    newscript->RegisterSelf();
}
