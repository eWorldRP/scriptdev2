/* ScriptData
SDName: Durotar
SD%Complete: 
SDComment: Support for quest 11409
SDCategory: Durotar
EndScriptData */

/* ContentData
npc_ram_master_ray        Brewfest event
EndContentData */

#include "precompiled.h"

/*#####
# Ram Master Ray
#####*/

enum
{
    QUEST_NOW_RAM_RACING_H  = 11409,
    SPELL_RENTAL_RAM        = 43883
};

bool QuestAccept_npc_ram_master_ray(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_NOW_RAM_RACING_H)
    {
		pPlayer->CastSpell(pPlayer, SPELL_RENTAL_RAM, false);
    }
    return true;
}

void AddSC_durotar()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_ram_master_ray";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ram_master_ray;
    newscript->RegisterSelf();
}
