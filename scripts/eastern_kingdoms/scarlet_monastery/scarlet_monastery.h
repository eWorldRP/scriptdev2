/* Copyright (C) 2006 - 2011 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SCARLETM_H
#define DEF_SCARLETM_H

enum
{
    MAX_ENCOUNTER                   = 1,

    TYPE_MOGRAINE_AND_WHITE_EVENT   = 1,

    DATA_MOGRAINE                   = 2,
    DATA_WHITEMANE                  = 3,
    DATA_DOOR_WHITEMANE             = 4,
    DATA_VORREL                     = 5,
    DATA_HORSEMAN_EVENT             = 6,
    DATA_PUMPKIN_SHRINE             = 7,

    NPC_HH_MOUNTED                  = 23682,
    NPC_HH_UNHORSED                 = 23800,
    NPC_HEAD                        = 23775,
    NPC_PULSING_PUMPKIN             = 23694,
    NPC_PUMPKIN_FIEND               = 23545,
    NPC_HELPER                      = 23686,
    NPC_WISP_INVIS                  = 24034,
    NPC_SIR_THOMAS                  = 23904,
    NPC_MOGRAINE                    = 3976,
    NPC_WHITEMANE                   = 3977,
    NPC_VORREL                      = 3981,

    GO_WHITEMANE_DOOR               = 104600,
    GO_PUMPKIN_SHRINE               = 186267,
};

class MANGOS_DLL_DECL instance_scarlet_monastery : public ScriptedInstance
{
    public:
        instance_scarlet_monastery(Map* pMap);

        void Initialize();

        void OnCreatureCreate(Creature* pCreature);
        void OnObjectCreate(GameObject* pGo);
        void HandleGameObject(ObjectGuid uiGuid, bool bOpen);

        void SetData(uint32 uiType, uint32 uiData);
        uint32 GetData(uint32 uiType);

    private:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
};

#endif
