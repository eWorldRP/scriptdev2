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
SDName: Boss_Urom
SD%Complete: 70%
SDComment:
SDAuthor: originally from TC, reworked by MaxXx2021 Aka Mioka, corrected by /dev/rsa
SDCategory: Oculus
EndScriptData */

#include "precompiled.h"
#include "oculus.h"

enum
{
    SAY_AGGRO                                     = -1578012,
    SAY_KILL_1                                    = -1578013,
    SAY_KILL_2                                    = -1578014,
    SAY_KILL_3                                    = -1578015,
    SAY_DEATH                                     = -1578016,
    SAY_EXPLOSION_1                               = -1578017,
    SAY_EXPLOSION_2                               = -1578018,
    SAY_SUMMON_1                                  = -1578019,
    SAY_SUMMON_2                                  = -1578020,
    SAY_SUMMON_3                                  = -1578021,

    SPELL_ARCANE_BARRIER                          = 53813, //Dummy --> Channeled, shields the caster from damage.
    SPELL_EMPOWERED_ARCANE_EXPLOSION              = 51110,
    SPELL_EMPOWERED_ARCANE_EXPLOSION_2            = 59377,
    SPELL_FROSTBOMB                               = 51103, //Urom throws a bomb, hitting its target with the highest aggro which inflict directly 650 frost damage and drops a frost zone on the ground. This zone deals 650 frost damage per second and reduce the movement speed by 35%. Lasts 1 minute.
    SPELL_SUMMON_MENAGERIE                        = 50476, //Summons an assortment of creatures and teleports the caster to safety.
    SPELL_SUMMON_MENAGERIE_2                      = 50495,
    SPELL_SUMMON_MENAGERIE_3                      = 50496,
    SPELL_TELEPORT                                = 51112, //Teleports to the center of Oculus
    SPELL_TIME_BOMB                               = 51121, //Deals arcane damage to a random player, and after 6 seconds, deals zone damage to nearby equal to the health missing of the target afflicted by the debuff.
    SPELL_TIME_BOMB_2                             = 59376,

    //NPCs
    NPC_PHANTASMAL_MAMMOTH                 = 27642,
    NPC_PHANTASMAL_WOLF                    = 27644,
    NPC_PHANTASMAL_CLOUDSCRAPER            = 27645,

    NPC_PHANTASMAL_OGRE                    = 27647,
    NPC_PHANTASMAL_NAGA                    = 27648,
    NPC_PHANTASMAL_MURLOC                  = 27649,

    NPC_PHANTASMAL_AIR                     = 27650,
    NPC_PHANTASMAL_FIRE                    = 27651,
    NPC_PHANTASMAL_WATER                   = 27653
};

/*struct Locations
{
    float x, y, z, o;
    uint32 id;
};*/

struct Locations Teleport[]=
{
    {1177.469f, 937.721f, 527.405f, 2.21f}, //first platform
    {968.5880f, 1042.58f, 527.321f, 0.13f}, //second platform
    {1163.671f, 1171.02f, 527.321f, 4.19f}, //third platform
    {1116.765f, 1075.93f, 508.361f, 4.26f}, //middle platform
    {1103.659f, 1049.88f, 518.148f, 5.80f}  //oculus center
};

struct MANGOS_DLL_DECL boss_uromAI : public ScriptedAI
{
    boss_uromAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiFrostBombTimer;
    uint32 m_uiLiveBombTimer;
    uint32 m_uiTeleportTimer;
    uint32 m_uiArcaneExplodeTimer;
    uint32 m_uiRelocateTimer;
    uint32 m_uiBackTimer;
    uint32 m_uiCheckTimer;

    bool m_bIsTeleported;
    bool m_bIsTalk;

    void Reset()
    {
       m_uiFrostBombTimer = 9000;
       m_uiLiveBombTimer = 14000;
       m_uiTeleportTimer = 30000;
       m_uiRelocateTimer = 31500;
       m_uiBackTimer = 9000;
       m_uiCheckTimer = 2000;
       m_uiArcaneExplodeTimer = 1000;

       m_bIsTeleported = false;
       m_bIsTalk = false;

       if (m_pInstance && m_pInstance->GetData(TYPE_UROM) != IN_PROGRESS)
       {
           m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
           m_pInstance->SetData(TYPE_UROM, NOT_STARTED);
           if(m_pInstance->GetData(TYPE_VAROS) == DONE)
           {
              m_creature->RemoveAurasDueToSpell(SPELL_ARCANE_BARRIER);
              m_creature->InterruptNonMeleeSpells(false);
//              m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
           }
           else
           {
              DoCast(m_creature, SPELL_ARCANE_BARRIER);
           }
        }
    }

    void DoSummonAndAttack(Unit* pWho, uint32 Entry01, uint32 Entry02, uint32 Entry03, uint32 Entry04, uint32 Entry05 = 0)
    {
        Creature* pTemp;
        if (pTemp = m_creature->SummonCreature(Entry01, m_creature->GetPositionX() - (10.0f) * cos(M_PI / 2), m_creature->GetPositionY() - (10.0f) * sin(M_PI / 2), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 15000))
        {
            pTemp->SetInCombatWith(pWho);
            pTemp->AddThreat(pWho,100.0f);
        }

        if (pTemp = m_creature->SummonCreature(Entry02, m_creature->GetPositionX() - (10.0f) * cos(M_PI * 2), m_creature->GetPositionY() - (10.0f) * sin(M_PI * 2), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 15000))
        {
            pTemp->SetInCombatWith(pWho);
            pTemp->AddThreat(pWho,100.0f);
        }

        if (pTemp = m_creature->SummonCreature(Entry03, m_creature->GetPositionX() - (10.0f) * cos(M_PI + M_PI / 2), m_creature->GetPositionY() - (10.0f) * sin(M_PI + M_PI / 2), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 15000))
        {
            pTemp->SetInCombatWith(pWho);
            pTemp->AddThreat(pWho,100.0f);
        }

        if (pTemp = m_creature->SummonCreature(Entry04, m_creature->GetPositionX() - (10.0f) * cos(M_PI), m_creature->GetPositionY() - (10.0f) * sin(M_PI), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 15000))
        {
            pTemp->SetInCombatWith(pWho);
            pTemp->AddThreat(pWho,100.0f);
        }

        if(Entry05 != 0)
        {
            if (pTemp = m_creature->SummonCreature(Entry05, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 15000))
            {
                pTemp->SetInCombatWith(pWho);
                pTemp->AddThreat(pWho,100.0f);
            }
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if(pSpell->Id == SPELL_SUMMON_MENAGERIE)
            m_bIsTalk = false;
        if(pSpell->Id == SPELL_SUMMON_MENAGERIE_2)
            m_bIsTalk = false;
        if(pSpell->Id == SPELL_SUMMON_MENAGERIE_3)
            m_bIsTalk = false;
    }

    void TeleportBoss(float fX, float fY, float fZ, float fO)
    {
        m_creature->GetMap()->CreatureRelocation(m_creature, fX, fY, fZ, fO);
        m_creature->MonsterMoveWithSpeed(fX, fY, fZ, 26);
        m_creature->Relocate(fX, fY, fZ, fO);
    }

    void AttackStart(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        if(m_pInstance->GetData(TYPE_UROM_PHASE) < 3)
            return;

        if (m_bIsTeleported)
            return;

        ScriptedAI::AttackStart(pWho);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_pInstance)
            return;

        if(pWho->GetTypeId() == TYPEID_PLAYER && !((Player*)pWho)->isGameMaster()
           && m_creature->IsWithinDistInMap(pWho, 30.0f) && !pWho->GetVehicle()
           && m_pInstance->GetData(TYPE_VAROS) == DONE && !m_bIsTalk)
        {
            switch(m_pInstance->GetData(TYPE_UROM_PHASE))
            {
                case 0:
                    m_bIsTalk = true;
                    SetCombatMovement(false);
                    m_pInstance->SetData(TYPE_UROM, IN_PROGRESS);
                    m_creature->InterruptNonMeleeSpells(false);
                    m_creature->RemoveAurasDueToSpell(SPELL_ARCANE_BARRIER);
                    DoScriptText(SAY_SUMMON_1, m_creature);
                    DoSummonAndAttack(pWho, NPC_PHANTASMAL_FIRE, NPC_PHANTASMAL_FIRE, NPC_PHANTASMAL_AIR, NPC_PHANTASMAL_WATER);
                    DoCast(m_creature, SPELL_SUMMON_MENAGERIE);
                    m_pInstance->SetData(TYPE_UROM_PHASE, 1);
                    break;
                case 1:
                    m_bIsTalk = true;
                    DoScriptText(SAY_SUMMON_2, m_creature);
                    DoSummonAndAttack(pWho, NPC_PHANTASMAL_OGRE, NPC_PHANTASMAL_OGRE, NPC_PHANTASMAL_NAGA, NPC_PHANTASMAL_MURLOC);
                    DoCast(m_creature, SPELL_SUMMON_MENAGERIE_2);
                    m_pInstance->SetData(TYPE_UROM_PHASE, 2);
                    break;
                case 2:
                    m_bIsTalk = true;
                    DoScriptText(SAY_SUMMON_3, m_creature);
                    DoSummonAndAttack(pWho, NPC_PHANTASMAL_MAMMOTH, NPC_PHANTASMAL_WOLF, NPC_PHANTASMAL_WOLF, NPC_PHANTASMAL_CLOUDSCRAPER, NPC_PHANTASMAL_CLOUDSCRAPER);
                    DoCast(m_creature, SPELL_SUMMON_MENAGERIE_3);
                    m_pInstance->SetData(TYPE_UROM_PHASE, 3);
                    break;
                case 3:
                    m_bIsTalk = true;
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    SetCombatMovement(true);
                    AttackStart(pWho);
                    break;
                default:
                    break;
            }
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void Aggro(Unit* pWho)
    {
       if(pWho->GetTypeId() != TYPEID_PLAYER)
          EnterEvadeMode();
       else DoScriptText(SAY_AGGRO, m_creature);
    }

    void EnterEvadeMode()
    {

       if (!m_pInstance)
           return;

       if(m_pInstance->GetData(TYPE_UROM_PHASE) < 3)
       {
           Map::PlayerList const &pList = m_creature->GetMap()->GetPlayers();
           if (!pList.isEmpty())
               return;
       }
       m_pInstance->SetData(TYPE_UROM, FAIL);
       m_pInstance->SetData(TYPE_UROM_PHASE, 0);
       ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_UROM, DONE);
    }

    void KilledUnit(Unit* pVictim)
    {
        switch (urand(0, 2))
        {
           case 0: DoScriptText(SAY_KILL_1, m_creature); break;
           case 1: DoScriptText(SAY_KILL_2, m_creature); break;
           case 2: DoScriptText(SAY_KILL_3, m_creature); break;
        }
    }

    void CheckVehicle()
    {
        Map *map = m_creature->GetMap();
        if(map->IsDungeon())
        {
           Map::PlayerList const &PlayerList = map->GetPlayers();

           if(PlayerList.isEmpty())
              return;

           for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
           {
              if(i->getSource()->isAlive() && i->getSource()->GetVehicle())
                 EnterEvadeMode();
           }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if(!m_bIsTeleported)
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;

           if(m_uiFrostBombTimer < uiDiff)
           {
               m_creature->CastSpell(m_creature->getVictim(), SPELL_FROSTBOMB, false);
               m_uiFrostBombTimer = urand(5000, 7000);
           } else m_uiFrostBombTimer -= uiDiff;

           if(m_uiLiveBombTimer < uiDiff)
           {
               if(Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                  DoCast(pTarget, m_bIsRegularMode ? SPELL_TIME_BOMB : SPELL_TIME_BOMB_2);
               m_uiLiveBombTimer = urand(15000, 25000);
           } else m_uiLiveBombTimer -= uiDiff;

           if(m_uiTeleportTimer < uiDiff)
           {
               DoCast(m_creature, SPELL_TELEPORT);
               m_uiTeleportTimer = 31500;
           } else m_uiTeleportTimer -= uiDiff;

           if(m_uiCheckTimer < uiDiff)
           {
               CheckVehicle();
               m_uiCheckTimer = 2000;
           } else m_uiCheckTimer -= uiDiff;

           if(m_uiRelocateTimer < uiDiff)
           {
               m_bIsTeleported = true;
               m_creature->SetLevitate(true);
               m_creature->GetMotionMaster()->Clear(false);
               m_creature->GetMotionMaster()->MoveIdle();
               m_creature->StopMoving();
               TeleportBoss(Teleport[4].x,Teleport[4].y,Teleport[4].z,Teleport[4].o);
               m_uiRelocateTimer = 31500;
           } else m_uiRelocateTimer -= uiDiff;

           DoMeleeAttackIfReady();
        }
        else
        {
           if(m_uiArcaneExplodeTimer < uiDiff)
           {
               DoScriptText(urand(0,1) ? SAY_EXPLOSION_1 : SAY_EXPLOSION_2, m_creature);
               DoCast(m_creature, SPELL_EMPOWERED_ARCANE_EXPLOSION);
               m_uiArcaneExplodeTimer = 32000;
           } else m_uiArcaneExplodeTimer -= uiDiff;

           if(m_uiBackTimer < uiDiff)
           {
               TeleportBoss(m_creature->getVictim()->GetPositionX(), m_creature->getVictim()->GetPositionY(), m_creature->getVictim()->GetPositionZ(), m_creature->getVictim()->GetOrientation());
               m_creature->SetLevitate(false);
               if(m_creature->getVictim())
                  m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
               m_uiBackTimer = 9000;
               m_bIsTeleported = false;
               m_uiArcaneExplodeTimer = 1000;
           } else m_uiBackTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_boss_urom(Creature* pCreature)
{
    return new boss_uromAI (pCreature);
}

void AddSC_boss_urom()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_urom";
    newscript->GetAI = &GetAI_boss_urom;
    newscript->RegisterSelf();
}
