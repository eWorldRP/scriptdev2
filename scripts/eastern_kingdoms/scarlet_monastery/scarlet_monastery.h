/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
   Copyright (C) 2011 MangosR2
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
