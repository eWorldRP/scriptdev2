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
SDName: instance_trial_of_the_crusader
SD%Complete: 80%
SDComment: by /dev/rsa
SDCategory: Trial of the Crusader
EndScriptData */

#include "precompiled.h"
#include "trial_of_the_crusader.h"

struct MANGOS_DLL_DECL instance_trial_of_the_crusader : public BSWScriptedInstance
{
    instance_trial_of_the_crusader(Map* pMap) : BSWScriptedInstance(pMap) 
    { 
        m_uiDifficulty = pMap->GetDifficulty();
        Initialize(); 
    }

    uint32 m_auiEncounter[MAX_ENCOUNTERS+1];
    uint32 m_auiEventTimer;
    uint32 m_auiEventNPCId;
    uint32 m_auiNorthrendBeasts[3];
    uint8 m_uiDifficulty;
    std::string m_strInstData;
    bool needsave;

    uint32 m_uiDataDamageFjola;
    uint32 m_uiDataDamageEydis;
    uint32 m_uiValkyrsCasting;

    uint32 m_uiTributeChest;

    uint32 m_auiCrusadersCount;

    void Initialize()
    {
        for (uint8 i = 0; i < MAX_ENCOUNTERS; ++i)
                m_auiEncounter[i] = NOT_STARTED;

        m_auiEncounter[0] = 0;
        m_auiEncounter[7] = 50;
        m_auiEncounter[8] = 0;

        m_auiNorthrendBeasts[0] = NOT_STARTED;
        m_auiNorthrendBeasts[1] = NOT_STARTED;
        m_auiNorthrendBeasts[2] = NOT_STARTED;

        m_auiEventTimer = 1000;
        m_auiCrusadersCount = 6;
        m_uiTributeChest = 0;
        needsave = false;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 1; i < MAX_ENCOUNTERS-2 ; ++i)
            if (m_auiEncounter[i] == IN_PROGRESS)
                return true;

        return false;
    }

    void OnPlayerEnter(Player *m_player)
    {
        if (m_uiDifficulty >= RAID_DIFFICULTY_10MAN_HEROIC)
        {
            m_player->SendUpdateWorldState(UPDATE_STATE_UI_SHOW,1);
            m_player->SendUpdateWorldState(UPDATE_STATE_UI_COUNT, GetData(TYPE_COUNTER));
        }
    }

    bool IsRaidWiped()
    {
        Map::PlayerList const &players = instance->GetPlayers();

        for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
        {
            if(Player* pPlayer = i->getSource())
            {
                if(pPlayer->isAlive())
                    return false;
            }
        }
        return true;
    }

    void OnCreatureCreate(Creature* pCreature)
    {
        if (!pCreature)
            return;
        m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
    }

    void OnObjectCreate(GameObject *pGo)
    {
        if (!pGo)
            return;
        m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
    }

    void SetData(uint32 uiType, uint32 uiData)
    {

        if (uiData == FAIL)
        {
            switch (uiType)
            {
                case TYPE_BEASTS:
                case TYPE_JARAXXUS:
                case TYPE_CRUSADERS:
                case TYPE_VALKIRIES:
                case TYPE_ANUBARAK:
                    if (m_auiEncounter[uiType] != FAIL)
                    {
                        if (m_uiDifficulty >= RAID_DIFFICULTY_10MAN_HEROIC)
                        {
                            --m_auiEncounter[7];
                            needsave = true;
                            DoUpdateWorldState(UPDATE_STATE_UI_SHOW, 1);
                            DoUpdateWorldState(UPDATE_STATE_UI_COUNT, m_auiEncounter[7]);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        switch(uiType)
        {
            case TYPE_STAGE:    m_auiEncounter[0] = uiData; break;
            case TYPE_BEASTS:   m_auiEncounter[1] = uiData; break;
            case TYPE_JARAXXUS: m_auiEncounter[2] = uiData; break;
            case TYPE_CRUSADERS:
                if (uiData == FAIL && (m_auiEncounter[3] == FAIL || m_auiEncounter[3] == NOT_STARTED))
                    m_auiEncounter[3] = NOT_STARTED;
                else
                    m_auiEncounter[3] = uiData;

                if (uiData == DONE)
                {
                    uint32 uiCacheEntry = GO_CRUSADERS_CACHE_10;

                    switch (instance->GetDifficulty())
                    {
                        case RAID_DIFFICULTY_10MAN_HEROIC:
                            uiCacheEntry = GO_CRUSADERS_CACHE_10_H;
                            break;
                        case RAID_DIFFICULTY_25MAN_NORMAL:
                            uiCacheEntry = GO_CRUSADERS_CACHE_25;
                            break;
                        case RAID_DIFFICULTY_25MAN_HEROIC:
                            uiCacheEntry = GO_CRUSADERS_CACHE_25_H;
                            break;
                    }
                    if (GameObject* pChest = GetSingleGameObjectFromStorage(uiCacheEntry))
                        if (!pChest->isSpawned())
                            pChest->SetRespawnTime(7*DAY);
                }
                break;
            case TYPE_CRUSADERS_COUNT:
                if (uiData == 0)
                    --m_auiCrusadersCount;
                else
                    m_auiCrusadersCount = uiData;
                break;
            case TYPE_VALKIRIES:
                if (m_auiEncounter[4] == SPECIAL && uiData == SPECIAL)
                    uiData = DONE;

                // remove light/dark essence and poweringup on success/wipe
                if(uiData == DONE || uiData == FAIL)
                {
                    Map::PlayerList const &players = instance->GetPlayers();

                    for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                    {
                        if(Player* pPlayer = i->getSource())
                        {
                            if(pPlayer && pPlayer->isAlive())
                            {
                                pPlayer->RemoveAurasDueToSpell(SPELL_LIGHT_ESSENCE);
                                pPlayer->RemoveAurasDueToSpell(SPELL_DARK_ESSENCE);
                                // powered up buffs
                                pPlayer->RemoveAurasDueToSpell(67590);
                                pPlayer->RemoveAurasDueToSpell(67602);
                                pPlayer->RemoveAurasDueToSpell(67603);
                                pPlayer->RemoveAurasDueToSpell(67604);
                            }
                        }
                    }
                }
                m_auiEncounter[4] = uiData;
                break;
            case TYPE_LICH_KING: m_auiEncounter[5] = uiData; break;
            case TYPE_ANUBARAK:  m_auiEncounter[6] = uiData;
                if (uiData == DONE)
                {
                    if (m_uiDifficulty == RAID_DIFFICULTY_10MAN_HEROIC)
                    {
                        if (m_auiEncounter[7] == 50)
                            m_uiTributeChest = GO_TRIBUTE_CHEST_10H_50;
                        else if (m_auiEncounter[7] >= 45)
                            m_uiTributeChest = GO_TRIBUTE_CHEST_10H_45;
                        else if (m_auiEncounter[7] >= 25)
                            m_uiTributeChest = GO_TRIBUTE_CHEST_10H_25;
                        else
                            m_uiTributeChest = GO_TRIBUTE_CHEST_10H_99;
                    }
                    if (m_uiDifficulty == RAID_DIFFICULTY_25MAN_HEROIC)
                    {
                        if (m_auiEncounter[7] == 50)
                            m_uiTributeChest = GO_TRIBUTE_CHEST_25H_50;
                        else if (m_auiEncounter[7] >= 45)
                            m_uiTributeChest = GO_TRIBUTE_CHEST_25H_45;
                        else if (m_auiEncounter[7] >= 25)
                            m_uiTributeChest = GO_TRIBUTE_CHEST_25H_25;
                        else
                            m_uiTributeChest = GO_TRIBUTE_CHEST_25H_99;
                    }
                    // reward for clearing TOC instance
                    if (m_uiTributeChest)
                    {
                        DoRespawnGameObject(GetSingleGameObjectFromStorage(m_uiTributeChest)->GetObjectGuid(), DAY);
                    }
                }
                break;
            case TYPE_COUNTER:   m_auiEncounter[7] = uiData; uiData = DONE; break;
            case TYPE_EVENT:     m_auiEncounter[8] = uiData; uiData = NOT_STARTED; break;
            case TYPE_EVENT_TIMER:      m_auiEventTimer = uiData; uiData = NOT_STARTED; break;
            case TYPE_GORMOK:
                m_auiNorthrendBeasts[0] = uiData;
                break;
            case TYPE_SNAKES:
                m_auiNorthrendBeasts[1] = uiData;
                break;
            case TYPE_ICEHOWL:
                m_auiNorthrendBeasts[2] = uiData;
                break;
            case DATA_HEALTH_FJOLA:     m_uiDataDamageFjola = uiData; uiData = NOT_STARTED; break;
            case DATA_HEALTH_EYDIS:     m_uiDataDamageEydis = uiData; uiData = NOT_STARTED; break;
            case DATA_CASTING_VALKYRS:  m_uiValkyrsCasting = uiData; uiData = NOT_STARTED; break;
        }

        if (IsEncounterInProgress())
        {
            DoCloseDoor(GO_EAST_PORTCULLIS);
            DoCloseDoor(GO_NORTH_PORTCULLIS);
            DoCloseDoor(GO_SOUTH_PORTCULLIS);
        }
        else
        {
            DoOpenDoor(GO_EAST_PORTCULLIS);
            DoOpenDoor(GO_NORTH_PORTCULLIS);
            DoOpenDoor(GO_SOUTH_PORTCULLIS);
        }

        if ((uiData == DONE && uiType != TYPE_STAGE && uiType != TYPE_EVENT && uiType != TYPE_EVENT_TIMER) || needsave == true)
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;

            for(uint8 i = 0; i < MAX_ENCOUNTERS; ++i)
                saveStream << m_auiEncounter[i] << " ";

            m_strInstData = saveStream.str();

            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
            needsave = false;
        }
    }

    uint32 GetData(uint32 uiType)
    {
        switch(uiType)
        {
            case TYPE_STAGE:     return m_auiEncounter[0];
            case TYPE_BEASTS:    return m_auiEncounter[1];
            case TYPE_JARAXXUS:  return m_auiEncounter[2];
            case TYPE_CRUSADERS: return m_auiEncounter[3];
            case TYPE_VALKIRIES: return m_auiEncounter[4];
            case TYPE_LICH_KING: return m_auiEncounter[5];
            case TYPE_ANUBARAK:  return m_auiEncounter[6];
            case TYPE_COUNTER:   return m_auiEncounter[7];
            case TYPE_EVENT:     return m_auiEncounter[8];
            case TYPE_DIFFICULTY:   return m_uiDifficulty;
            case TYPE_GORMOK:  return m_auiNorthrendBeasts[0];
            case TYPE_SNAKES:  return m_auiNorthrendBeasts[1];
            case TYPE_ICEHOWL: return m_auiNorthrendBeasts[2];
            case TYPE_EVENT_TIMER:  return m_auiEventTimer;
            case TYPE_CRUSADERS_COUNT:  return m_auiCrusadersCount;
            case TYPE_EVENT_NPC: switch (m_auiEncounter[8]) 
                                 {
                                 case 110:
                                 case 140:
                                 case 150:
                                 case 200:
                                 case 205:
                                 case 210:
                                 case 300:
                                 case 305:
                                 case 310:
                                 case 400:
                                 case 666:
                                 case 1010:
                                 case 1180:
                                 case 2000:
                                 case 2030:
                                 case 3000:
                                 case 3001:
                                 case 3060:
                                 case 3061:
                                 case 3090:
                                 case 3091:
                                 case 3100:
                                 case 3110:
                                 case 4000:
                                 case 4010:
                                 case 4015:
                                 case 4040:
                                 case 4050:
                                 case 5000:
                                 case 5005:
                                 case 5020:
                                 case 6000:
                                 case 6005:
                                 case 6010:
                                 m_auiEventNPCId = NPC_TIRION;
                                 break;

                                 case 5010:
                                 case 5030:
                                 case 5040:
                                 case 5050:
                                 case 5060:
                                 case 5070:
                                 case 5080:
                                 m_auiEventNPCId = NPC_LICH_KING_1;
                                 break;

                                 case 130:
                                 case 132:
                                 case 2020:
                                 case 3080:
                                 case 3051:
                                 case 3071:
                                 case 4020:
                                 m_auiEventNPCId = NPC_RINN;
                                 break;

                                 case 120:
                                 case 122:
                                 case 2010:
                                 case 3050:
                                 case 3070:
                                 case 3081:
                                 case 4030:
                                 m_auiEventNPCId = NPC_GARROSH;
                                 break;

                                 case 1110:
                                 case 1120:
                                 case 1130:
                                 case 1132:
                                 case 1134:
                                 case 1135:
                                 case 1140:
                                 case 1142:
                                 case 1144:
                                 case 1145:
                                 case 1150:
                                 case 1160:
                                 m_auiEventNPCId = NPC_FIZZLEBANG;
                                 break;

                                 default:
                                 m_auiEventNPCId = NPC_TIRION;
                                 break;

                                 };
                                 return m_auiEventNPCId;

        case DATA_HEALTH_FJOLA: return m_uiDataDamageFjola;
        case DATA_HEALTH_EYDIS: return m_uiDataDamageEydis;
        case DATA_CASTING_VALKYRS: return m_uiValkyrsCasting;
        }
        return 0;
    }

    const char* Save()
    {
        return m_strInstData.c_str();
    }

    void Load(const char* strIn)
    {
        if (!strIn)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(strIn);

        std::istringstream loadStream(strIn);

        for(uint8 i = 0; i < MAX_ENCOUNTERS; ++i)
        {
            loadStream >> m_auiEncounter[i];

            if (m_auiEncounter[i] == IN_PROGRESS)
                m_auiEncounter[i] = NOT_STARTED;
        }
        m_auiEncounter[TYPE_EVENT] = 0;
        m_auiEncounter[TYPE_STAGE] = 0;

        OUT_LOAD_INST_DATA_COMPLETE;

    }
};

InstanceData* GetInstanceData_instance_trial_of_the_crusader(Map* pMap)
{
    return new instance_trial_of_the_crusader(pMap);
}

void AddSC_instance_trial_of_the_crusader()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_trial_of_the_crusader";
    newscript->GetInstanceData = &GetInstanceData_instance_trial_of_the_crusader;
    newscript->RegisterSelf();
}
