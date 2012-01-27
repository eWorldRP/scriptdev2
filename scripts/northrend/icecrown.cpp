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
SDName: Icecrown
SD%Complete: 100
SDComment: Vendor support: 34885
Quest Support: 13663, 13665, 13745, 13750, 13756, 13761, 13767, 13772, 13777, 13782, 13787, 14107
SDCategory: Icecrown
EndScriptData */

/* ContentData
npc_dame_evniki_kapsalis
npc_scourge_conventor
npc_fallen_hero_spirit
npc_valiants
npc_champions
npc_black_knights_gryphon
EndContentData */

#include "precompiled.h"
#include "Vehicle.h"
#include "GossipDef.h"
#include "escort_ai.h"
#include "TemporarySummon.h"

/*#####
## npc_black_knights_gryphon
#####*/
 
struct MANGOS_DLL_DECL npc_black_knights_gryphonAI : public npc_escortAI
{
    npc_black_knights_gryphonAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }
 
    void Reset() { }
 
    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(((TemporarySummon*)m_creature)->GetSummonerGuid()))
            pPlayer->KilledMonsterCredit(m_creature->GetEntry(), m_creature->GetObjectGuid());
 
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetSpeedRate(MOVE_RUN, 3.0f);
        Start(true, ((Player*)pCaster));
    }
 
    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 0:
                SetRun();
                break;
            case 10:
                m_creature->SetLevitate(true);
                m_creature->SetSpeedRate(MOVE_RUN, 6.0f);
                break;
            case 15:
                //if (Player* pPlayer = GetPlayerForEscort())
                    //hack to prevent Player's death
                  //  pPlayer->CastSpell(pPlayer, 64505, true);
                    break;
            case 16:
                m_creature->ForcedDespawn(2000);
                return;
            default:
                break;
        }
    }
};
 
CreatureAI* GetAI_npc_black_knights_gryphon(Creature* pCreature)
{
    return new npc_black_knights_gryphonAI(pCreature);
}

/*######
## npc_dame_evniki_kapsalis
######*/

enum
{
    TITLE_CRUSADER    = 123
};

bool GossipHello_npc_dame_evniki_kapsalis(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->HasTitle(TITLE_CRUSADER))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_dame_evniki_kapsalis(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());
    return true;
}

/*######
## npc_argent_tournament_questgiver
######*/

namespace ArgentTournament
{
    Races GetRaceFromNPCId(uint32 id)
    // finds the race of argent tournament NPCs
    {
        switch (id)
        {
            case 33225: return RACE_HUMAN;
            case 33361: return RACE_ORC;
            case 33312: return RACE_DWARF;
            case 33592: return RACE_NIGHTELF;
            case 33373: return RACE_UNDEAD;
            case 33403: return RACE_TAUREN;
            case 33335: return RACE_GNOME;
            case 33372: return RACE_TROLL;
            case 33379: return RACE_BLOODELF;
            case 33593: return RACE_DRAENEI;

        default:
            return Races(0); // invalid
        }
    }

    #define MAX_RACES 12
    // information about valiants given by race-specific questgivers
    static const uint32 InitialValiantsForRace[MAX_RACES] = {0 /*0 = invalid*/,13593 /*1 = human */,13707 /*2 = orc*/,
        13703 /*3 = dwarf*/,13706 /*4 = nightelf*/,13710 /*5 = undead*/,13709 /*6 = tauren*/,13704 /*7 = gnome*/,
        13708 /*8 = troll*/,0 /*invalid: 9 = goblin*/,13711 /*10 = bloodelf*/,13705/*11 = draenei*/};
    static const uint32 FinalValiantsForRace[MAX_RACES] = {0 /*0 = invalid*/,13699 /*1 = human */,13726 /*2 = orc*/,
        13713 /*3 = dwarf*/,13725 /*4 = nightelf*/,13729 /*5 = undead*/,13728 /*6 = tauren*/,13723 /*7 = gnome*/,
        13727 /*8 = troll*/,0 /*invalid: 9 = goblin*/,13731 /*10 = bloodelf*/,13724/*11 = draenei*/};

    bool CompletedValiantForHisRace(Player * pl)
    {
        if (pl->getRace() >= MAX_RACES)
            return false; // ?!?

        return pl->GetQuestRewardStatus(FinalValiantsForRace[pl->getRace()]);
    }

    bool HasValiantActiveForRace(Player * pl,uint32 race)
    {
        if (uint32 initial = InitialValiantsForRace[race])
            if (pl->GetQuestStatus(initial)) // has quest chain?
                if (uint32 final = FinalValiantsForRace[race])
                    if (!pl->GetQuestRewardStatus(final)) // is it not complete?
                        return true;

        return false;
    }

    bool HasValiantActive(Player * pl,uint32 exceptRace = 0)
    {
        // search all valiant quest chains on player
        for (uint32 i = 0; i < MAX_RACES; i++)
            if (i != exceptRace && HasValiantActiveForRace(pl,i))
                return true;

        return false; // all complete or none found
    }

    bool IsQuestgiverActiveFor(Creature * qg,Player * pl)
    {
        Races race = GetRaceFromNPCId(qg->GetEntry());
        if (!race)
        {
            error_log("SD2: Argent tournament: unknown race for NPC entry %u, guid %u",qg->GetEntry(),qg->GetGUIDLow());
            return false;
        }

        if (race == pl->getRace())
            return true; // always enabled for player of current race (if quest already complete, none will be shown)

        if (CompletedValiantForHisRace(pl) && !HasValiantActive(pl,race))
            return true; // enable questgiver

        return false;
    }

    bool HasAchievement(Player * player,uint32 id)
    {
        if (!player || !id)
            return false;

        return player->GetAchievementMgr().HasAchievement(id);
    }

    bool IsQuestVisibleFor(Player * player, uint32 questid)
    {
        if (!player || !questid)
            return false;

        if (questid == 13664)
            // must have completed 13663 and (13700 or 13701)
            return (player->GetQuestRewardStatus(13663) && (player->GetQuestRewardStatus(13700) || player->GetQuestRewardStatus(13701)));

        return true;
    }

    enum
    {
        ACHIEVEMENT_EAC_HORDE = 2816,   // Exalted Argent Champion of the Horde
        ACHIEVEMENT_EAC_ALLIANCE = 2817 // Exalted Argent Champion of the Alliance
    };
}

bool GossipHello_npc_argent_tournament_questgiver(Player* pPlayer, Creature* pCreature)
{
    if (!pCreature->isQuestGiver())
        return false; // error

    if (ArgentTournament::IsQuestgiverActiveFor(pCreature,pPlayer))
        return false; // return not handled message: the NPC will behave like a common questgiver

    // else, send empty gossip menu
    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipHello_npc_argent_tournament_requires_EAC(Player* pPlayer, Creature* pCreature)
// all creatures with this script must be enabled only when Exalted Argent Champion of the Alliance or Horde has been achieved
{  
    if (pPlayer->GetTeam() == ALLIANCE && ArgentTournament::HasAchievement(pPlayer,ArgentTournament::ACHIEVEMENT_EAC_ALLIANCE))
        return false; // behave like a common NPC

    if (pPlayer->GetTeam() == HORDE && ArgentTournament::HasAchievement(pPlayer,ArgentTournament::ACHIEVEMENT_EAC_HORDE))
        return false;

    // else, send empty gossip menu
    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

/*######
## npc_rhydalla
######*/

bool GossipHello_npc_rhydalla(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
    {
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

        QuestMenu &qm = pPlayer->PlayerTalkClass->GetQuestMenu();
        
        for (uint32 count = 0; count < qm.MenuItemCount(); ++count )
        {
            QuestMenuItem const& qmi = qm.GetItem(count);
#ifndef WIN32
            if (ArgentTournament::IsQuestVisibleFor(pPlayer, qmi.m_qId))
                qm.RemoveItem(qmi.m_qId);
#endif
        }
    }
    return true;
}

/*######
## npc_squire_david
######*/

enum eSquireDavid
{
    QUEST_THE_ASPIRANT_S_CHALLENGE_H                    = 13680,
    QUEST_THE_ASPIRANT_S_CHALLENGE_A                    = 13679,

    NPC_ARGENT_VALIANT                                  = 33448,

    GOSSIP_TEXTID_READYTOBATTLE                         = 14407,
    GOSSIP_TEXTID_HOWTO                                 = 14476,
    
    SOUND_EVENTSTART                                    = 15852
};

#define GOSSIP_SQUIRE_ITEM_1 "I am ready to fight!"
#define GOSSIP_SQUIRE_ITEM_2 "How do the Argent Crusader raiders fight?"

bool GossipHello_npc_squire_david(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_THE_ASPIRANT_S_CHALLENGE_H) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_ASPIRANT_S_CHALLENGE_A) == QUEST_STATUS_INCOMPLETE )//We need more info about it.
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
    }

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_READYTOBATTLE, pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_squire_david(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->SummonCreature(NPC_ARGENT_VALIANT, 8575.451f, 952.472f, 547.554f, 0.38f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
        pCreature->PlayDirectSound(SOUND_EVENTSTART);
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF+2)
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_HOWTO, pCreature->GetGUID());

    return true;
}

enum eArgentValiant
{
    SPELL_CHARGE                = 63010,
    SPELL_SHIELD_BREAKER        = 62575,

    NPC_ARGENT_VALIANT_CREDIT   = 24108
};

#define SAY_VALIANT_AGGRO       "The rank of valiant is not easily won. Prepare yourself!"
#define SAY_VALIANT_DEFEAT      "Impressive skills indeed. I believe that you are more than ready for the rank of valiant."

struct npc_argent_valiantAI : public ScriptedAI
{
    npc_argent_valiantAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pCreature->GetMotionMaster()->MovePoint(0.0f,8599.258f,963.951f,547.553f);
        pCreature->setFaction(35); //wrong faction in db?
    }

    uint32 uiRangedActionTimer;
    bool GoingRanged;
    bool CanCastRanged;

    void Reset()
    {
        uiRangedActionTimer = 10000;
        GoingRanged = false;
        CanCastRanged = false;
    }

    void Aggro(Unit* pWho)
    {
        m_creature->MonsterSay(SAY_VALIANT_AGGRO, LANG_UNIVERSAL, NULL);
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
    {
        if (uiDamage > m_creature->GetHealth() 
            && pDoneBy->GetTypeId() == TYPEID_UNIT 
            && pDoneBy->GetVehicleKit()
            && pDoneBy->GetVehicleKit()->GetPassenger(0))
        {
            uiDamage = 0;
            ((Player*)pDoneBy->GetVehicleKit()->GetPassenger(0))->KilledMonsterCredit(NPC_ARGENT_VALIANT_CREDIT);
            m_creature->setFaction(35);
            m_creature->ForcedDespawn(5000);
            m_creature->SetCombatStartPosition(m_creature->GetPositionX(),m_creature->GetPositionY(),m_creature->GetPositionZ());
            CreatureCreatePos pos(m_creature->GetMap(), m_creature->GetPositionX(),m_creature->GetPositionY(),m_creature->GetPositionZ(),m_creature->GetOrientation(), m_creature->GetPhaseMask());
            m_creature->SetSummonPoint(pos);
            m_creature->MonsterSay(SAY_VALIANT_DEFEAT, LANG_UNIVERSAL, NULL);
            pDoneBy->CastSpell(pDoneBy, 64892, true);
            EnterEvadeMode();
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(m_creature->getVictim())
        {
            if(!m_creature->IsWithinDistInMap(m_creature->getVictim(), 5) && m_creature->IsWithinDistInMap(m_creature->getVictim(), 25))
                CanCastRanged = true;
            else 
            {
                float X = m_creature->getVictim()->GetPositionX() + urand(7,11);
                float Y = m_creature->getVictim()->GetPositionY() + urand(7,11);
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(0.0f, X, Y, m_creature->getVictim()->GetPositionZ()); 
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (uiRangedActionTimer <= uiDiff)
        {
            if(!GoingRanged)
            {
                float X = m_creature->getVictim()->GetPositionX() + urand(7,11);
                float Y = m_creature->getVictim()->GetPositionY() + urand(7,11);
                GoingRanged = true;

                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(0.0f, X, Y, m_creature->getVictim()->GetPositionZ()); 
            }

            if(CanCastRanged)
            {
                uint32 radomspell = urand(0,1) ? SPELL_SHIELD_BREAKER : SPELL_CHARGE;

                if (DoCastSpellIfCan(m_creature->getVictim(), radomspell) == CAST_OK)
                {
                    uiRangedActionTimer = urand(7000,12000);
                    GoingRanged = false;
                    CanCastRanged = false;
                }
            }
        } else 
            uiRangedActionTimer -= uiDiff;
                
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_argent_valiant(Creature* pCreature)
{
    return new npc_argent_valiantAI (pCreature);
}


/*######
## npc_scourge_conventor  //quest 14107
######*/

// grip of scourge still needs implented and used

enum QuestFate // shared enum by conventor mob and fallen hero mob
{
    QUEST_THE_FATE_OF_THE_FALLEN        = 14107,
    NPC_FALLEN_HERO_SPIRIT              = 32149,
    NPC_FALLEN_HERO_SPIRIT_PROXY        = 35055,
};

enum
{
    SPELL_CONE_OF_COLD              = 20828,
    SPELL_FORST_NOVA                = 11831,
    SPELL_FROSTBOLT                 = 20822,
    SPELL_GRIP_OF_THE_SCOURGE       = 60212,     // spell casted by mob
};

struct MANGOS_DLL_DECL npc_scourge_conventorAI : public ScriptedAI
{
    npc_scourge_conventorAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiConeofCold_Timer;
    uint32 m_uiFrostNova_Timer;
    uint32 m_uiFrostBolt_Timer;
    //uint32 m_uiGrip_Timer;

    void Reset()
    {
        m_uiConeofCold_Timer  = 10000;
        m_uiFrostNova_Timer    = 11000;
        m_uiFrostBolt_Timer    = 9000;
        //m_uiGrip_Timer = 10000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiConeofCold_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),SPELL_CONE_OF_COLD);
            m_uiConeofCold_Timer = 10000;
        }
        else
            m_uiConeofCold_Timer -= uiDiff;

        if (m_uiFrostNova_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),SPELL_FORST_NOVA);
            m_uiFrostNova_Timer = 11000;
        }
        else
            m_uiFrostNova_Timer -= uiDiff;

        if (m_uiFrostBolt_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),SPELL_FROSTBOLT);
            m_uiFrostBolt_Timer = 9000;
        }
        else
            m_uiFrostBolt_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_scourge_conventor(Creature* pCreature)
{
    return new npc_scourge_conventorAI(pCreature);
}

/*######
## npc_fallen_hero_spirit  //quest 14107
######*/

enum
{
    SAY_BLESS_1                         = -1999819,
    SAY_BLESS_2                         = -1999820,
    SAY_BLESS_3                         = -1999821,
    SAY_BLESS_4                         = -1999822,
    SAY_BLESS_5                         = -1999823,

    SPELL_STRIKE                        = 11976,
    SPELL_BLESSING_OF_PEACE             = 66719,     //spell casted from relic of light
    GRIP_OF_THE_SCOURGE_AURA            = 60231      //might need server side spell script support (when mob has this spell it's immune to fate of light spell)
};


struct MANGOS_DLL_DECL npc_fallen_hero_spiritAI : public ScriptedAI
{
    npc_fallen_hero_spiritAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiStrike_Timer;

    void Reset()
    {
        m_uiStrike_Timer = 10000;
    }

    void SpellHit(Unit *pCaster, const SpellEntry *pSpell)
    {
        // if (m_creature->HasAura(GRIP_OF_THE_SCOURGE_AURA))
        //     return fasle;

        if (pCaster->GetTypeId() == TYPEID_PLAYER && m_creature->isAlive() && ((pSpell->Id == SPELL_BLESSING_OF_PEACE)))
        {
            if (((Player*)pCaster)->GetQuestStatus(QUEST_THE_FATE_OF_THE_FALLEN) == QUEST_STATUS_INCOMPLETE)
            {
                ((Player*)pCaster)->KilledMonsterCredit(NPC_FALLEN_HERO_SPIRIT_PROXY);
                m_creature->ForcedDespawn();
                switch(urand(0, 4))
                {
                    case 0: DoScriptText(SAY_BLESS_1, m_creature); break;
                    case 1: DoScriptText(SAY_BLESS_2, m_creature); break;
                    case 2: DoScriptText(SAY_BLESS_3, m_creature); break;
                    case 3: DoScriptText(SAY_BLESS_4, m_creature); break;
                    case 4: DoScriptText(SAY_BLESS_5, m_creature); break;
                }
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiStrike_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_STRIKE);
            m_uiStrike_Timer = 10000;
        }
        else
            m_uiStrike_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_fallen_hero_spirit(Creature* pCreature)
{
    return new npc_fallen_hero_spiritAI(pCreature);
}

/*#####
## npc_valiants
#####*/

enum
{
    SAY_DEFEATED          = -1999824,
    SPELL_VCHARGE         = 63010,
    SPELL_VSHIELDBREAKER  = 65147,

    SPELL_MOUNTED_MELEE_VICTORY = 62724,
};

struct MANGOS_DLL_DECL npc_valiantsAI : public ScriptedAI
{
   npc_valiantsAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    //uint32 m_uiVCHARGE_Timer;
    //uint32 m_uiVSHIELDBREAKER_Timer;

    void Reset()
    {
       //m_uiVCHARGE_Timer          = 2000;  need correct timers
       //m_uiVSHIELDBREAKER_Timer   = 5000;  need correct timers
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > m_creature->GetHealth())
        {
            uiDamage = 5;

            if (Unit* pPlayer = pDoneBy->GetCharmerOrOwnerPlayerOrPlayerItself())
                pPlayer->CastSpell(pPlayer, SPELL_MOUNTED_MELEE_VICTORY, true);

            DoScriptText(SAY_DEFEATED, m_creature);
            EnterEvadeMode();
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
       /* {    STILL HAVE ATTACK SPELLS TO DO
        }*/

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_valiants(Creature* pCreature)
{
    return new npc_valiantsAI(pCreature);
}

/*#####
## npc_champions
#####*/

enum
{
    // spells are defined above
    SPELL_CHAMP_MOUNTED_MELEE_VICTORY = 63596,
};

struct MANGOS_DLL_DECL npc_championsAI : public ScriptedAI
{
   npc_championsAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    //uint32 m_uiVCHARGE_Timer;
    //uint32 m_uiVSHIELDBREAKER_Timer;

    void Reset()
    {
       //m_uiVCHARGE_Timer          = 2000;  need correct timers
       //m_uiVSHIELDBREAKER_Timer   = 5000;  need correct timers
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > m_creature->GetHealth())
        {
            uiDamage = 5;

            if (Unit* pPlayer = pDoneBy->GetCharmerOrOwnerPlayerOrPlayerItself())
                pPlayer->CastSpell(pPlayer, SPELL_CHAMP_MOUNTED_MELEE_VICTORY, true);

            DoScriptText(SAY_DEFEATED, m_creature);
            EnterEvadeMode();
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
       /* {    STILL HAVE ATTACK SPELLS TO DO
        }*/

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_champions(Creature* pCreature)
{
    return new npc_championsAI(pCreature);
}

void AddSC_icecrown()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_black_knights_gryphon";
    pNewScript->GetAI = &GetAI_npc_black_knights_gryphon;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_dame_evniki_kapsalis";
    pNewScript->pGossipHello = &GossipHello_npc_dame_evniki_kapsalis;
    pNewScript->pGossipSelect = &GossipSelect_npc_dame_evniki_kapsalis;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_scourge_conventor";
    pNewScript->GetAI = &GetAI_npc_scourge_conventor;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_fallen_hero_spirit";
    pNewScript->GetAI = &GetAI_npc_fallen_hero_spirit;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_valiants";
    pNewScript->GetAI = &GetAI_npc_valiants;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_champions";
    pNewScript->GetAI = &GetAI_npc_champions;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_argent_tournament_questgiver";
    pNewScript->pGossipHello = &GossipHello_npc_argent_tournament_questgiver;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_argent_tournament_requires_EAC";
    pNewScript->pGossipHello = &GossipHello_npc_argent_tournament_requires_EAC;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_rhydalla";
    pNewScript->pGossipHello = &GossipHello_npc_rhydalla;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_squire_david";
    pNewScript->pGossipHello = &GossipHello_npc_squire_david;
    pNewScript->pGossipSelect = &GossipSelect_npc_squire_david;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_argent_valiant";
    pNewScript->GetAI = &GetAI_npc_argent_valiant;
    pNewScript->RegisterSelf();
}
