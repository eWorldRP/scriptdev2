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
SDName: Boss_Ymiron
SD%Complete: 90%
SDComment: TODO: soul fount is not moving/working 100% right, spirit channel animation/event not working perfekt
SDCategory: Utgarde Pinnacle
EndScriptData */

#include "precompiled.h"
#include "utgarde_pinnacle.h"

enum
{
    SAY_AGGRO                   = -1575031,
    SAY_SUMMON_BJORN            = -1575032,
    SAY_SUMMON_HALDOR           = -1575033,
    SAY_SUMMON_RANULF           = -1575034,
    SAY_SUMMON_TORGYN           = -1575035,
    SAY_SLAY_1                  = -1575036,
    SAY_SLAY_2                  = -1575037,
    SAY_SLAY_3                  = -1575038,
    SAY_SLAY_4                  = -1575039,
    SAY_DEATH                   = -1575040,

    ACHIEV_KINGS_BANE           = 2157,
    ACHIEV_HAIL_TO_THE_KING     = 1790,

    SPELL_BANE                  = 48294,
    SPELL_BANE_H                = 59301,
    SPELL_DARK_SLASH            = 48292,
    SPELL_FETID_ROT             = 48291,
    SPELL_FETID_ROT_H           = 59300,
    SPELL_SCREAMS_OF_THE_DEAD   = 51750,
    SPELL_SPIRIT_BURST          = 48529, // when Ranulf
    SPELL_SPIRIT_BURST_H        = 59305, // when Ranulf
    SPELL_SPIRIT_STRIKE         = 48423, // when Haldor
    SPELL_SPIRIT_STRIKE_H       = 59304, // when Haldor
    SPELL_ANCESTORS_VENGEANCE   = 16939, // 2 normal, 4 heroic

    SPELL_SUMMON_AVENGING_SPIRIT    = 48592,
    SPELL_SUMMON_SPIRIT_FOUNT       = 48386,

    SPELL_CHANNEL_SPIRIT_TO_YMIRON  = 48316,
    SPELL_CHANNEL_YMIRON_TO_SPIRIT  = 48307,

    SPELL_SPIRIT_FOUNT              = 48380,
    SPELL_SPIRIT_FOUNT_H            = 59320,

    NPC_BJORN           = 27303, // Near Right Boat, summon Spirit Fount
    NPC_BJORN_VISUAL    = 27304,
    NPC_HALDOR          = 27307, // Near Left Boat, debuff Spirit Strike on player
    NPC_HALDOR_VISUAL   = 27310,
    NPC_RANULF          = 27308, // Far Left Boat, ability to cast spirit burst
    NPC_RANULF_VISUAL   = 27311,
    NPC_TORGYN          = 27309, // Far Right Boat, summon 4 Avenging Spirit
    NPC_TORGYN_VISUAL   = 27312,

    NPC_SPIRIT_FOUNT    = 27339,
    NPC_AVENGING_SPIRIT = 27386
};

struct ActiveBoatStruct
{
    uint32 npc;
    uint32 say;
    float MoveX,MoveY,MoveZ,SpawnX,SpawnY,SpawnZ,SpawnO;
};

static ActiveBoatStruct ActiveBoat[4] =
{
    {NPC_BJORN_VISUAL, SAY_SUMMON_BJORN, 381.546f, -314.362f, 104.756f, 370.841f, -314.426f, 107.995f, 6.232f},
    {NPC_HALDOR_VISUAL, SAY_SUMMON_HALDOR, 404.310f, -314.761f, 104.756f, 413.992f, -314.703f, 107.995f, 3.157f},
    {NPC_RANULF_VISUAL, SAY_SUMMON_RANULF, 404.379f, -335.335f, 104.756f, 413.594f, -335.408f, 107.995f, 3.157f},
    {NPC_TORGYN_VISUAL, SAY_SUMMON_TORGYN, 380.813f, -335.069f, 104.756f, 369.994f, -334.771f, 107.995f, 6.232f}
};

/*######
## boss_ymiron
######*/

struct MANGOS_DLL_DECL boss_ymironAI : public ScriptedAI
{
    boss_ymironAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    std::vector<ActiveBoatStruct> boatList;
    uint8 m_uiBoatActiveCount;
    ActiveBoatStruct currentActiveBoat;
    uint32 m_uiChannelSpiritTimer;
    uint32 m_bIsChanneling;
    uint32 m_uiTempSpellTimer;
    uint64 m_uiSpiritFountGUID;
    std::vector<uint64> mobList;
    uint32 m_uiBaneTimer;
    uint32 m_uiDarkSlashFetidRotTimer;
    bool m_bBaneAchievement;

    void Reset()
    {
        boatList.clear();
        boatList.push_back(ActiveBoat[0]);
        boatList.push_back(ActiveBoat[1]);
        boatList.push_back(ActiveBoat[2]);
        boatList.push_back(ActiveBoat[3]);

        mobList.clear();
        m_uiChannelSpiritTimer = 0;
        m_bIsChanneling = false;
        m_uiTempSpellTimer = 15000;
        m_uiBaneTimer = 5000;
        m_uiDarkSlashFetidRotTimer = 1000;
        m_uiBoatActiveCount = 0;
        m_bBaneAchievement = true;
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_YMIRON,IN_PROGRESS);
    }

    void KilledUnit(Unit* pVictim)
    {
        switch(urand(0, 3))
        {
            case 0: DoScriptText(SAY_SLAY_1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY_2, m_creature); break;
            case 2: DoScriptText(SAY_SLAY_3, m_creature); break;
            case 3: DoScriptText(SAY_SLAY_4, m_creature); break;
        }
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_YMIRON, FAIL);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_YMIRON,DONE);

        if (!m_bIsRegularMode && m_bBaneAchievement)
            m_pInstance->DoCompleteAchievement(ACHIEV_KINGS_BANE);


        Map::PlayerList const &pPlayers = m_creature->GetMap()->GetPlayers();

        for (Map::PlayerList::const_iterator i = pPlayers.begin(); i != pPlayers.end(); ++i)
        {
            if (Player* pPlayer = i->getSource())
            {
                if (pPlayer->GetMiniPet() && pPlayer->isAlive())
                {
                    uint32 pet = pPlayer->GetMiniPet()->GetEntry();
                    if (pet == 14305 || pet == 22817 || pet == 22818 || pet == 33533 || pet == 14444 || pet == 33532)
                        pPlayer->CompletedAchievement(ACHIEV_HAIL_TO_THE_KING);
                }
            }
        }
    }

    void SpellHitTarget (Unit* pUnit, const SpellEntry* pSpellEntry)
    {
        if (pSpellEntry->Id == 59302 && pUnit->GetTypeId() == TYPEID_PLAYER)
            m_bBaneAchievement = false;
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if(uiType != POINT_MOTION_TYPE)
                return;

        if (uiPointId == 1)
        {
            if (Creature* pTemp = m_creature->SummonCreature(currentActiveBoat.npc, currentActiveBoat.SpawnX, currentActiveBoat.SpawnY, currentActiveBoat.SpawnZ, currentActiveBoat.SpawnO, TEMPSUMMON_TIMED_DESPAWN, 7000))
            {
                m_uiChannelSpiritTimer = 7000;
                m_bIsChanneling = true;
                pTemp->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pTemp->SetLevitate(true);
            }
        }
     }

    void JustSummoned(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_BJORN_VISUAL:
            case NPC_HALDOR_VISUAL:
            case NPC_RANULF_VISUAL:
            case NPC_TORGYN_VISUAL:
                pSummoned->CastSpell(m_creature, SPELL_CHANNEL_SPIRIT_TO_YMIRON, true);
            break;
            default: break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_bIsChanneling)
        {
            if (m_uiChannelSpiritTimer < uiDiff) // channel complete, apply new skills
            {
                if (!mobList.empty())
                {
                    for (std::vector<uint64>::iterator itr = mobList.begin(); itr!=mobList.end(); ++itr)
                        if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                            pTemp->ForcedDespawn();
                    mobList.clear();
                }

                if (currentActiveBoat.npc == NPC_BJORN_VISUAL)
                {
                    if(Creature* pFount = m_creature->SummonCreature(NPC_SPIRIT_FOUNT, m_creature->GetPositionX()+rand()%10, m_creature->GetPositionY()+rand()%10, m_creature->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0))
                    {   
                        pFount->GetMotionMaster()->MoveConfused();
                        mobList.push_back(pFount->GetGUID());
                    }
                }
                else if (currentActiveBoat.npc == NPC_TORGYN_VISUAL)
                {
                    for (int i=0;i<4;++i)
                        if (Creature* pTemp = m_creature->SummonCreature(NPC_AVENGING_SPIRIT, m_creature->GetPositionX()+rand()%10, m_creature->GetPositionY()+rand()%10, m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 10000))
                        {
                            pTemp->SetInCombatWithZone();
                            mobList.push_back(pTemp->GetGUID());
                        }
                }

                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->Attack(m_creature->getVictim(),true);
                m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                m_bIsChanneling = false;
            }
            else
                m_uiChannelSpiritTimer -= uiDiff;
            
            return;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if ((m_bIsRegularMode && ((m_uiBoatActiveCount == 0 && m_creature->GetHealthPercent() < 67.f) || 
            (m_uiBoatActiveCount == 1 && m_creature->GetHealthPercent() < 34.f))) ||
            (!m_bIsRegularMode && (m_uiBoatActiveCount == 0 && m_creature->GetHealthPercent() < 80.f) ||
            (m_uiBoatActiveCount == 1 && m_creature->GetHealthPercent() < 60.f) ||
            (m_uiBoatActiveCount == 2 && m_creature->GetHealthPercent() < 40.f) ||
            (m_uiBoatActiveCount == 3 && m_creature->GetHealthPercent() < 20.f)))
            {
                ++m_uiBoatActiveCount;
                // get random boat
                std::vector<ActiveBoatStruct>::iterator itr = boatList.begin();
                std::advance(itr,rand()%boatList.size());
                currentActiveBoat = *itr;
                boatList.erase(itr);
                // start boat event
                m_creature->InterruptNonMeleeSpells(false);
                m_creature->CastSpell(m_creature, SPELL_SCREAMS_OF_THE_DEAD, true);
                DoScriptText(currentActiveBoat.say,m_creature);
                m_creature->SetWalk(false);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->GetMotionMaster()->MovePoint(1, currentActiveBoat.MoveX, currentActiveBoat.MoveY, currentActiveBoat.MoveZ);
            }

        if (m_uiBoatActiveCount > 0 )
            if (currentActiveBoat.npc == NPC_HALDOR_VISUAL)
            {
                if (m_uiTempSpellTimer < uiDiff)
                {
                    DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_SPIRIT_STRIKE : SPELL_SPIRIT_STRIKE_H);
                    m_uiTempSpellTimer = urand(6000,10000);
                }
                else
                    m_uiTempSpellTimer -= uiDiff;
            } 
            else if (currentActiveBoat.npc == NPC_RANULF_VISUAL)
            {
                if (m_uiTempSpellTimer < uiDiff)
                {
                    DoCastSpellIfCan(m_creature, m_bIsRegularMode ? SPELL_SPIRIT_BURST : SPELL_SPIRIT_BURST_H);
                    m_uiTempSpellTimer = urand(10000,18000);
                }
                else
                    m_uiTempSpellTimer -= uiDiff;
            }

            if (m_uiDarkSlashFetidRotTimer < uiDiff)
            {
                if(rand()%2)
                    DoCastSpellIfCan(m_creature->getVictim(),SPELL_DARK_SLASH);
                else
                    DoCastSpellIfCan(m_creature->getVictim(), m_bIsRegularMode ? SPELL_FETID_ROT : SPELL_FETID_ROT_H);
                m_uiDarkSlashFetidRotTimer = urand(1000,8000);
            }
            else
                m_uiDarkSlashFetidRotTimer -= uiDiff;

            if (m_uiBaneTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, m_bIsRegularMode ? SPELL_BANE : SPELL_BANE_H);
                m_uiBaneTimer = urand(10000,25000);
            }
            else
                m_uiBaneTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ymiron(Creature* pCreature)
{
    return new boss_ymironAI(pCreature);
}

/*######
## mob_spirit_fount
######*/

struct MANGOS_DLL_DECL mob_spirit_fountAI : public ScriptedAI
{
    mob_spirit_fountAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    void Reset()
    {
        m_creature->CastSpell(m_creature, m_bIsRegularMode ? SPELL_SPIRIT_FOUNT : SPELL_SPIRIT_FOUNT_H, true);
        m_creature->SetWalk(false);
    }

    void AttackStart(Unit* pWho)
    {
        return;
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage) // just in case
    {
        uiDamage = 0;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_bIsRegularMode && !m_creature->HasAura(SPELL_SPIRIT_FOUNT))
            m_creature->CastSpell(m_creature, SPELL_SPIRIT_FOUNT, true);
        else if (!m_bIsRegularMode && !m_creature->HasAura(SPELL_SPIRIT_FOUNT_H))
            m_creature->CastSpell(m_creature, SPELL_SPIRIT_FOUNT_H, true);
    }
};

CreatureAI* GetAI_mob_spirit_fount(Creature* pCreature)
{
    return new mob_spirit_fountAI(pCreature);
}

void AddSC_boss_ymiron()
{
    Script *pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_ymiron";
    pNewscript->GetAI = &GetAI_boss_ymiron;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "mob_spirit_fount";
    pNewscript->GetAI = &GetAI_mob_spirit_fount;
    pNewscript->RegisterSelf();
}
