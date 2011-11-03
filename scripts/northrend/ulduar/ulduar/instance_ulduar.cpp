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
SDName: Instance_Ulduar
SD%Complete:
SDComment:
SDCategory: Ulduar
EndScriptData */

#include "precompiled.h"
#include "ulduar.h"

struct MANGOS_DLL_DECL instance_ulduar : public ScriptedInstance
{
    instance_ulduar(Map* pMap) : ScriptedInstance(pMap)
    {
        m_bRegular = pMap->IsRegularDifficulty();
        Initialize();
    }

    bool m_bRegular;

    // initialize the encouter variables
    std::string m_strInstData;
    uint32 m_auiEncounter[MAX_ENCOUNTER];
    uint32 m_auiHardBoss[HARD_ENCOUNTER];
    uint32 m_auiUlduarKeepers[KEEPER_ENCOUNTER];
    uint32 m_auiUlduarTeleporters[3];
    uint32 m_auiMiniBoss[6];

    // boss phases which need to be used inside the instance script
    uint32 m_uiMimironPhase;
    uint32 m_uiYoggPhase;
    uint32 m_uiVisionPhase;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        memset(&m_auiHardBoss, 0, sizeof(m_auiHardBoss));
        memset(&m_auiUlduarKeepers, 0, sizeof(m_auiUlduarKeepers));
        memset(&m_auiUlduarTeleporters, 0, sizeof(m_auiUlduarTeleporters));

        for(uint8 i = 0; i < 6; i++)
            m_auiMiniBoss[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        {
            if (m_auiEncounter[i] == IN_PROGRESS)
                return true;
        }

        return false;
    }

    void OnCreatureCreate(Creature* pCreature)
    {
        switch(pCreature->GetEntry())
        {
            case NPC_LEVIATHAN:
            case NPC_IGNIS:
            case NPC_RAZORSCALE:
            case NPC_COMMANDER:
            case NPC_XT002:
            case NPC_STEELBREAKER:
            case NPC_MOLGEIM:
            case NPC_BRUNDIR:
            case NPC_KOLOGARN:
            case NPC_RIGHT_ARM:
            case NPC_LEFT_ARM:
            case NPC_AURIAYA:
            case NPC_FERAL_DEFENDER:
            case NPC_MIMIRON:
            case NPC_LEVIATHAN_MK:
            case NPC_HODIR:
            case NPC_THORIM:
            case NPC_RUNIC_COLOSSUS:
            case NPC_RUNE_GIANT:
            case NPC_JORMUNGAR_BEHEMOTH:
            case NPC_FREYA:
            case NPC_BRIGHTLEAF:
            case NPC_IRONBRACH:
            case NPC_STONEBARK:
            case NPC_VEZAX:
            case NPC_ANIMUS:
            case NPC_YOGGSARON:
            case NPC_SARA:
            case NPC_YOGG_BRAIN:
            case NPC_ALGALON:
                break;
            // used to handle the keepers images
            // set to invisible by default and only made visible if the encounter is done
            case HODIR_IMAGE:
                pCreature->SetVisibility(VISIBILITY_OFF);
                if(m_auiEncounter[8] == DONE)
                    pCreature->SetVisibility(VISIBILITY_ON);
                break;
            case FREYA_IMAGE:
                pCreature->SetVisibility(VISIBILITY_OFF);
                if(m_auiEncounter[10] == DONE)
                    pCreature->SetVisibility(VISIBILITY_ON);
                break;
            case THORIM_IMAGE:
                pCreature->SetVisibility(VISIBILITY_OFF);
                if(m_auiEncounter[9] == DONE)
                    pCreature->SetVisibility(VISIBILITY_ON);
                break;
            case MIMIRON_IMAGE:
                pCreature->SetVisibility(VISIBILITY_OFF);
                if(m_auiEncounter[7] == DONE)
                    pCreature->SetVisibility(VISIBILITY_ON);
                break;
        }
        m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
    }

    void OnObjectCreate(GameObject *pGo)
    {
        switch(pGo->GetEntry())
        {
            // doors & other
            // The siege
            case GO_SHIELD_WALL:
                break;
            case GO_LEVIATHAN_GATE:
                if(m_auiEncounter[0] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_XT002_GATE:
                pGo->SetGoState(GO_STATE_READY);
                if(m_auiEncounter[TYPE_XT002] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                if(m_auiEncounter[TYPE_IGNIS] == DONE && m_auiEncounter[TYPE_RAZORSCALE] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_BROKEN_HARPOON:
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                break;
            // Archivum
            case GO_IRON_ENTRANCE_DOOR:
                break;
            case GO_ARCHIVUM_DOOR:
                if(m_auiEncounter[TYPE_ASSEMBLY])
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_ARCHIVUM_CONSOLE:
                break;
            case GO_UNIVERSE_FLOOR_ARCHIVUM:
                break;
            // Celestial Planetarium
            case GO_CELESTIAL_ACCES:
            case GO_CELESTIAL_DOOR:
            case GO_UNIVERSE_FLOOR_CELESTIAL:
            case GO_AZEROTH_GLOBE:
                break;
            // Shattered Hallway
            case GO_KOLOGARN_BRIDGE:
                pGo->SetGoState(GO_STATE_ACTIVE);
                if(m_auiEncounter[TYPE_KOLOGARN] == DONE)
                {
                    pGo->SetUInt32Value(GAMEOBJECT_LEVEL, 0);
                    pGo->SetGoState(GO_STATE_READY);
                }
                break;
            case GO_SHATTERED_DOOR:
                break;
            // The keepers
            // Hodir
            case GO_HODIR_EXIT:
                if(m_auiEncounter[TYPE_HODIR])
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_HODIR_ICE_WALL:
                if(m_auiEncounter[TYPE_HODIR])
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_HODIR_ENTER:
                break;
            // Mimiron
            case GO_MIMIRON_TRAM:
                if (m_auiEncounter[TYPE_AURIAYA] == DONE)
                {
                    pGo->SetUInt32Value(GAMEOBJECT_LEVEL, 0);
                    pGo->SetGoState(GO_STATE_READY);
                }
                break;
            case GO_MIMIRON_BUTTON:
                if (m_auiEncounter[TYPE_MIMIRON] == NOT_STARTED)
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                break;
            case GO_MIMIRON_DOOR_1:
            case GO_MIMIRON_DOOR_2:
            case GO_MIMIRON_DOOR_3:
            case GO_MIMIRON_ELEVATOR:
            case GO_MIMIRON_TEL1:
            case GO_MIMIRON_TEL2:
            case GO_MIMIRON_TEL3:
            case GO_MIMIRON_TEL4:
            case GO_MIMIRON_TEL5:
            case GO_MIMIRON_TEL6:
            case GO_MIMIRON_TEL7:
            case GO_MIMIRON_TEL8:
            case GO_MIMIRON_TEL9:
                break;
            // Thorim
            case GO_DARK_IRON_PORTCULIS:
            case GO_RUNED_STONE_DOOR:
            case GO_THORIM_STONE_DOOR:
            case GO_LIGHTNING_FIELD:
                break;
            case GO_DOOR_LEVER:
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                break;
            // Prison
            case GO_ANCIENT_GATE:
                DoOpenMadnessDoorIfCan();
                break;
            case GO_VEZAX_GATE:
                pGo->SetGoState(GO_STATE_READY);
                if(m_auiEncounter[TYPE_VEZAX])
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_YOGG_GATE:
            case GO_BRAIN_DOOR1:
            case GO_BRAIN_DOOR2:
            case GO_BRAIN_DOOR3:
                break;
            // loot
            // Kologarn
            case GO_CACHE_OF_LIVING_STONE:
            case GO_CACHE_OF_LIVING_STONE_H:
            // Hodir
            case GO_CACHE_OF_WINTER:
            case GO_CACHE_OF_WINTER_H:
            // Hodir rare
            case GO_CACHE_OF_RARE_WINTER:
            case GO_CACHE_OF_RARE_WINTER_H:
            // Freya
            case GO_FREYA_GIFT:
            case GO_FREYA_GIFT_H:
            // Freya rare
            case GO_FREYA_GIFT_1:
            case GO_FREYA_GIFT_H_1:
            case GO_FREYA_GIFT_2:
            case GO_FREYA_GIFT_H_2:
            case GO_FREYA_GIFT_3:
            case GO_FREYA_GIFT_H_3:
            // Thorim
            case GO_CACHE_OF_STORMS:
            case GO_CACHE_OF_STORMS_H:
            // Thorim rare
            case GO_CACHE_OF_RARE_STORMS:
            case GO_CACHE_OF_RARE_STORMS_H:
            // Mimiron
            case GO_CACHE_OF_INOV:
            case GO_CACHE_OF_INOV_H:
            case GO_CACHE_OF_INOV_HARD:
            case GO_CACHE_OF_INOV_HARD_H:
            // Alagon
            case GO_GIFT_OF_OBSERVER:
            case GO_GIFT_OF_OBSERVER_H:
                break;
        }
        m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
    }

    // used in order to unlock the door to Vezax and make vezax attackable
    void DoOpenMadnessDoorIfCan()
    {
        if (m_auiEncounter[TYPE_MIMIRON] == DONE && m_auiEncounter[TYPE_HODIR] == DONE && m_auiEncounter[TYPE_THORIM] == DONE && m_auiEncounter[TYPE_FREYA] == DONE)
        {
            if (GameObject* pDoor = GetSingleGameObjectFromStorage(GO_ANCIENT_GATE))
                pDoor->SetGoState(GO_STATE_ACTIVE);
        }
    }

    // used to open the door to XT (custom script because Leviathan is disabled)
    // this will be removed when the Leviathan will be implemented
    void OpenXtDoor()
    {
        if(m_auiEncounter[TYPE_IGNIS] == DONE && m_auiEncounter[TYPE_RAZORSCALE] == DONE)
            DoUseDoorOrButton(GO_XT002_GATE);
    }

    void SetData(uint32 uiType, uint32 uiData)
    {
        switch(uiType)
        {
        case TYPE_LEVIATHAN:
            m_auiEncounter[TYPE_LEVIATHAN] = uiData;
            DoUseDoorOrButton(GO_SHIELD_WALL);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_XT002_GATE);
                DoUseDoorOrButton(GO_LEVIATHAN_GATE);
            }
            break;
        case TYPE_IGNIS:
            m_auiEncounter[TYPE_IGNIS] = uiData;
            OpenXtDoor();       // remove when leviathan implemented
            break;
        case TYPE_RAZORSCALE:
            m_auiEncounter[TYPE_RAZORSCALE] = uiData;
            OpenXtDoor();       // remove when leviathan implemented
            break;
        case TYPE_XT002:
            m_auiEncounter[TYPE_XT002] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_XT002_GATE);
            else if (uiData == IN_PROGRESS)
                DoUseDoorOrButton(GO_XT002_GATE);
            break;
        case TYPE_ASSEMBLY:
            m_auiEncounter[TYPE_ASSEMBLY] = uiData;
            DoUseDoorOrButton(GO_IRON_ENTRANCE_DOOR);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_ARCHIVUM_DOOR);
                CheckIronCouncil();        // used for a hacky achiev, remove for revision!
            }
            break;
        case TYPE_KOLOGARN:
            m_auiEncounter[TYPE_KOLOGARN] = uiData;
            DoUseDoorOrButton(GO_SHATTERED_DOOR);
            if (uiData == DONE)
            {
                DoRespawnGameObject(m_bRegular ? GO_CACHE_OF_LIVING_STONE : GO_CACHE_OF_LIVING_STONE_H, 30*MINUTE);
                if (GameObject* pBridge = GetSingleGameObjectFromStorage(GO_KOLOGARN_BRIDGE))
                    pBridge->SetGoState(GO_STATE_READY);
            }
            break;
        case TYPE_AURIAYA:
            m_auiEncounter[TYPE_AURIAYA] = uiData;
            if (uiData == DONE)
            {
                if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_MIMIRON_TRAM))
                {
                    pGO->SetUInt32Value(GAMEOBJECT_LEVEL, 0);
                    pGO->SetGoState(GO_STATE_READY);
                }
            }
            break;
            // Keepers
        case TYPE_MIMIRON:
            m_auiEncounter[TYPE_MIMIRON] = uiData;
            DoUseDoorOrButton(GO_MIMIRON_DOOR_1);
            DoUseDoorOrButton(GO_MIMIRON_DOOR_2);
            DoUseDoorOrButton(GO_MIMIRON_DOOR_3);
            if (uiData == DONE)
            {
                if(m_auiHardBoss[3] != DONE)
                    DoRespawnGameObject(m_bRegular ? GO_CACHE_OF_INOV : GO_CACHE_OF_INOV_H, 30*MINUTE);
                // used to make the friendly keeper visible
                if(Creature* pImage = GetSingleCreatureFromStorage(MIMIRON_IMAGE))
                    pImage->SetVisibility(VISIBILITY_ON);
                DoOpenMadnessDoorIfCan();
                CheckKeepers();        // used for a hacky achiev, remove for revision!
            }
            break;
        case TYPE_HODIR:
            m_auiEncounter[TYPE_HODIR] = uiData;
            DoUseDoorOrButton(GO_HODIR_ENTER);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_HODIR_ICE_WALL);
                DoUseDoorOrButton(GO_HODIR_EXIT);
                DoRespawnGameObject(m_bRegular ? GO_CACHE_OF_WINTER : GO_CACHE_OF_WINTER_H, 30*MINUTE);
                // used to make the friendly keeper visible
                if(Creature* pImage = GetSingleCreatureFromStorage(HODIR_IMAGE))
                    pImage->SetVisibility(VISIBILITY_ON);
                DoOpenMadnessDoorIfCan();
                CheckKeepers();        // used for a hacky achiev, remove for revision!
            }
            break;
        case TYPE_THORIM:
            m_auiEncounter[TYPE_THORIM] = uiData;
            DoUseDoorOrButton(GO_LIGHTNING_FIELD);
            if (uiData == IN_PROGRESS || uiData == NOT_STARTED)
                DoUseDoorOrButton(GO_DARK_IRON_PORTCULIS);

            if (uiData == DONE)
            {
                if(m_auiHardBoss[5] != DONE)
                    DoRespawnGameObject(m_bRegular ?  GO_CACHE_OF_STORMS : GO_CACHE_OF_STORMS_H, 30*MINUTE);
                // used to make the friendly keeper visible
                if(Creature* pImage = GetSingleCreatureFromStorage(THORIM_IMAGE))
                    pImage->SetVisibility(VISIBILITY_ON);
                DoOpenMadnessDoorIfCan();
                CheckKeepers();        // used for a hacky achiev, remove for revision!
            }
            break;
        case TYPE_FREYA:
            m_auiEncounter[TYPE_FREYA] = uiData;
            if (uiData == DONE)
            {
                // do this in order to see how many elders were alive and spawn the correct chest
                if(m_auiHardBoss[6] == 0)
                    DoRespawnGameObject(m_bRegular ? GO_FREYA_GIFT : GO_FREYA_GIFT_H, 30*MINUTE);
                else if(m_auiHardBoss[6] == 1)
                    DoRespawnGameObject(m_bRegular ? GO_FREYA_GIFT_1 : GO_FREYA_GIFT_H_1, 30*MINUTE);
                else if(m_auiHardBoss[6] == 2)
                    DoRespawnGameObject(m_bRegular ? GO_FREYA_GIFT_2 : GO_FREYA_GIFT_H_2, 30*MINUTE);
                else if(m_auiHardBoss[6] == 3)
                    DoRespawnGameObject(m_bRegular ? GO_FREYA_GIFT_3 : GO_FREYA_GIFT_H_3, 30*MINUTE);
                // used to make the friendly keeper visible
                if(Creature* pImage = GetSingleCreatureFromStorage(FREYA_IMAGE))
                    pImage->SetVisibility(VISIBILITY_ON);
                DoOpenMadnessDoorIfCan();
                CheckKeepers();        // used for a hacky achiev, remove for revision!
            }
            break;

            // Prison
        case TYPE_VEZAX:
            m_auiEncounter[TYPE_VEZAX] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_VEZAX_GATE);
            break;
        case TYPE_YOGGSARON:
            m_auiEncounter[TYPE_YOGGSARON] = uiData;
            DoUseDoorOrButton(GO_YOGG_GATE);
            break;

            // Celestial Planetarium
        case TYPE_ALGALON:
            m_auiEncounter[TYPE_ALGALON] = uiData;
            DoUseDoorOrButton(GO_CELESTIAL_DOOR);
            DoUseDoorOrButton(GO_UNIVERSE_FLOOR_CELESTIAL);
            if (uiData == DONE)
                DoRespawnGameObject(m_bRegular ? GO_GIFT_OF_OBSERVER : GO_GIFT_OF_OBSERVER_H, 30*MINUTE);
            break;

            // Hard modes
        case TYPE_LEVIATHAN_HARD:
            m_auiHardBoss[0] = uiData;  // todo: add extra loot
            break;
        case TYPE_XT002_HARD:
            m_auiHardBoss[1] = uiData;  // hard mode loot in sql -> hacky way
            break;
        case TYPE_HODIR_HARD:
            m_auiHardBoss[4] = uiData;
            if(uiData == DONE)
                DoRespawnGameObject(m_bRegular ? GO_CACHE_OF_RARE_WINTER : GO_CACHE_OF_RARE_WINTER_H, 30*MINUTE);
            break;
        case TYPE_ASSEMBLY_HARD:
            m_auiHardBoss[2] = uiData;  // hard mode loot in sql
            break;
        case TYPE_FREYA_HARD:
            m_auiHardBoss[6] = uiData;  // hard mode loot in the script above
            break;
        case TYPE_THORIM_HARD:
            m_auiHardBoss[5] = uiData;
            if(uiData == DONE)
                DoRespawnGameObject(m_bRegular ? GO_CACHE_OF_RARE_STORMS : GO_CACHE_OF_RARE_STORMS_H, 30*MINUTE);
            break;
        case TYPE_MIMIRON_HARD:
            m_auiHardBoss[3] = uiData;
            if(uiData == DONE)
                DoRespawnGameObject(m_bRegular ? GO_CACHE_OF_INOV_HARD : GO_CACHE_OF_INOV_HARD_H, 30*MINUTE);
            break;
        case TYPE_VEZAX_HARD:
            m_auiHardBoss[7] = uiData;  // hard mode loot in sql -> hacky way
            break;
        case TYPE_YOGGSARON_HARD:
            m_auiHardBoss[8] = uiData;  // todo: add extra loot
            break;

            // Ulduar keepers
        case TYPE_KEEPER_HODIR:
            m_auiUlduarKeepers[0] = uiData;
            break;
        case TYPE_KEEPER_THORIM:
            m_auiUlduarKeepers[1] = uiData;
            break;
        case TYPE_KEEPER_FREYA:
            m_auiUlduarKeepers[2] = uiData;
            break;
        case TYPE_KEEPER_MIMIRON:
            m_auiUlduarKeepers[3] = uiData;
            break;

            // teleporters
        case TYPE_LEVIATHAN_TP:
            m_auiUlduarTeleporters[0] = uiData;
            break;
        case TYPE_XT002_TP:
            m_auiUlduarTeleporters[1] = uiData;
            break;
        case TYPE_MIMIRON_TP:
            m_auiUlduarTeleporters[2] = uiData;
            break;

            // mini boss
        case TYPE_RUNIC_COLOSSUS:
            m_auiMiniBoss[0] = uiData;
            DoUseDoorOrButton(GO_RUNED_STONE_DOOR);
            break;
        case TYPE_RUNE_GIANT:
            m_auiMiniBoss[1] = uiData;
            DoUseDoorOrButton(GO_THORIM_STONE_DOOR);
            break;
        case TYPE_LEVIATHAN_MK:
            m_auiMiniBoss[2] = uiData;
            break;
        case TYPE_VX001:
            m_auiMiniBoss[3] = uiData;
            if (uiData == DONE)     // just for animation :)
            {
                DoUseDoorOrButton(GO_MIMIRON_TEL1);
                DoUseDoorOrButton(GO_MIMIRON_TEL2);
                DoUseDoorOrButton(GO_MIMIRON_TEL3);
                DoUseDoorOrButton(GO_MIMIRON_TEL4);
                DoUseDoorOrButton(GO_MIMIRON_TEL5);
                DoUseDoorOrButton(GO_MIMIRON_TEL6);
                DoUseDoorOrButton(GO_MIMIRON_TEL7);
                DoUseDoorOrButton(GO_MIMIRON_TEL8);
                DoUseDoorOrButton(GO_MIMIRON_TEL9);
            }
            break;
        case TYPE_AERIAL_UNIT:
            m_auiMiniBoss[4] = uiData;
            break;
        case TYPE_YOGG_BRAIN:
            m_auiMiniBoss[5] = uiData;
            break;

            //phases
        case TYPE_MIMIRON_PHASE:
            m_uiMimironPhase = uiData;
            break;
        case TYPE_YOGG_PHASE:
            m_uiYoggPhase = uiData;
            break;
        case TYPE_VISION_PHASE:
            m_uiVisionPhase = uiData;
            break;
        }

        if (uiData == DONE || uiData == FAIL)
        {
            OUT_SAVE_INST_DATA;

            // save all encounters, hard bosses and keepers
            std::ostringstream saveStream;
            saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                << m_auiEncounter[12] << " " << m_auiEncounter[13] << " " << m_auiHardBoss[0] << " "
                << m_auiHardBoss[1] << " " << m_auiHardBoss[2] << " " << m_auiHardBoss[2] << " "
                << m_auiHardBoss[4] << " " << m_auiHardBoss[5] << " " << m_auiHardBoss[6] << " "
                << m_auiHardBoss[7] << " " << m_auiHardBoss[8] << " " << m_auiUlduarKeepers[0] << " "
                << m_auiUlduarKeepers[1] << " " << m_auiUlduarKeepers[2] << " " << m_auiUlduarKeepers[3] << " "
                << m_auiUlduarTeleporters[0] << " " << m_auiUlduarTeleporters[1] << " " << m_auiUlduarTeleporters[2];

            m_strInstData = saveStream.str();

            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
        }
    }

    // TODO: implement all achievs here!
    bool CheckAchievementCriteriaMeet(uint32 criteria_id, const Player *source)
    {
        switch(criteria_id)
        {
            case ACHIEV_CRIT_SARONITE_N:
            case ACHIEV_CRIT_SARONITE_H:
                return GetData(TYPE_VEZAX_HARD) == DONE;

            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
                break;
        }
        return false;
    }

    // TODO: implement all hard mode loot here!
    bool CheckConditionCriteriaMeet(Player const* source, uint32 map_id, uint32 instance_condition_id)
    {
        if (map_id != instance->GetId())
            return false;
        switch (instance_condition_id)
        {
           case TYPE_XT002_HARD:
               break;
        }
        return true;
    }

    uint32 GetData(uint32 uiType)
    {
        switch(uiType)
        {
        case TYPE_LEVIATHAN:
            return m_auiEncounter[0];
        case TYPE_IGNIS:
            return m_auiEncounter[1];
        case TYPE_RAZORSCALE:
            return m_auiEncounter[2];
        case TYPE_XT002:
            return m_auiEncounter[3];
        case TYPE_ASSEMBLY:
            return m_auiEncounter[4];
        case TYPE_KOLOGARN:
            return m_auiEncounter[5];
        case TYPE_AURIAYA:
            return m_auiEncounter[6];
        case TYPE_MIMIRON:
            return m_auiEncounter[7];
        case TYPE_HODIR:
            return m_auiEncounter[8];
        case TYPE_THORIM:
            return m_auiEncounter[9];
        case TYPE_FREYA:
            return m_auiEncounter[10];
        case TYPE_VEZAX:
            return m_auiEncounter[11];
        case TYPE_YOGGSARON:
            return m_auiEncounter[12];
        case TYPE_ALGALON:
            return m_auiEncounter[13];

        // hard modes
        case TYPE_LEVIATHAN_HARD:
            return m_auiHardBoss[0];
        case TYPE_XT002_HARD:
            return m_auiHardBoss[1];
        case TYPE_ASSEMBLY_HARD:
            return m_auiHardBoss[2];
        case TYPE_MIMIRON_HARD:
            return m_auiHardBoss[3];
        case TYPE_HODIR_HARD:
            return m_auiHardBoss[4];
        case TYPE_THORIM_HARD:
            return m_auiHardBoss[5];
        case TYPE_FREYA_HARD:
            return m_auiHardBoss[6];
        case TYPE_VEZAX_HARD:
            return m_auiHardBoss[7];
        case TYPE_YOGGSARON_HARD:
            return m_auiHardBoss[8];

            // ulduar keepers
        case TYPE_KEEPER_HODIR:
            return m_auiUlduarKeepers[0];
        case TYPE_KEEPER_THORIM:
            return m_auiUlduarKeepers[1];
        case TYPE_KEEPER_FREYA:
            return m_auiUlduarKeepers[2];
        case TYPE_KEEPER_MIMIRON:
            return m_auiUlduarKeepers[3];

            // teleporters
        case TYPE_LEVIATHAN_TP:
            return m_auiUlduarTeleporters[0];
        case TYPE_XT002_TP:
            return m_auiUlduarTeleporters[1];
        case TYPE_MIMIRON_TP:
            return m_auiUlduarTeleporters[2];

            // mini boss
        case TYPE_RUNE_GIANT:
            return m_auiMiniBoss[1];
        case TYPE_RUNIC_COLOSSUS:
            return m_auiMiniBoss[0];
        case TYPE_LEVIATHAN_MK:
            return m_auiMiniBoss[2];
        case TYPE_VX001:
            return m_auiMiniBoss[3];
        case TYPE_AERIAL_UNIT:
            return m_auiMiniBoss[4];
        case TYPE_YOGG_BRAIN:
            return m_auiMiniBoss[5];

        case TYPE_MIMIRON_PHASE:
            return m_uiMimironPhase;
        case TYPE_YOGG_PHASE:
            return m_uiYoggPhase;
        case TYPE_VISION_PHASE:
            return m_uiVisionPhase;
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
        loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
        >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
        >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
        >> m_auiEncounter[12] >> m_auiEncounter[13] >> m_auiHardBoss[0] >> m_auiHardBoss[1]
        >> m_auiHardBoss[2] >> m_auiHardBoss[3] >> m_auiHardBoss[4] >> m_auiHardBoss[5]
        >> m_auiHardBoss[6] >> m_auiHardBoss[7] >> m_auiHardBoss[8] >> m_auiUlduarKeepers[0]
        >> m_auiUlduarKeepers[1] >> m_auiUlduarKeepers[2] >> m_auiUlduarKeepers[3] >> m_auiUlduarTeleporters[0]
        >> m_auiUlduarTeleporters[1] >> m_auiUlduarTeleporters[2];

        for(uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        {
            if (m_auiEncounter[i] == IN_PROGRESS)
                m_auiEncounter[i] = NOT_STARTED;
        }

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    // Hacky way of completing some achievs
    // PLEASE REMOVE FOR REVISION!
    void CheckIronCouncil()
    {
        // check if the other bosses in the antechamber are dead
        // hacky way to complete achievements; use only if you have this function
        if(m_auiEncounter[4] == DONE && m_auiEncounter[5] == DONE && m_auiEncounter[6] == DONE)
            DoCompleteAchievement(instance->IsRegularDifficulty() ? ACHIEV_IRON_COUNCIL : ACHIEV_IRON_COUNCIL_H);
    }

    void CheckKeepers()
    {
        // check if the other bosses in the antechamber are dead
        // hacky way to complete achievements; use only if you have this function
        if(m_auiEncounter[7] == DONE && m_auiEncounter[8] == DONE && m_auiEncounter[9] == DONE && m_auiEncounter[10] == DONE)
            DoCompleteAchievement(instance->IsRegularDifficulty() ? ACHIEV_KEEPERS : ACHIEV_KEEPERS_H);
    }
};

InstanceData* GetInstanceData_instance_ulduar(Map* pMap)
{
    return new instance_ulduar(pMap);
}

void AddSC_instance_ulduar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_ulduar";
    newscript->GetInstanceData = &GetInstanceData_instance_ulduar;
    newscript->RegisterSelf();
}
