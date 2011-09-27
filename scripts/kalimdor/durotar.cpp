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
    QUEST_BARK_FOR_DORN     = 11407,
    QUEST_BARK_FOR_TCHALI   = 11408,
    QUEST_NOW_RAM_RACING_H  = 11409,
    QUEST_BACK_AGAIN_H      = 11412,

    SPELL_RENTAL_RAM              = 43883,
    SPELL_RAM_FATIGUE             = 43052,
    SPELL_SPEED_RAM_GALLOP        = 42994,
    SPELL_SPEED_RAM_CANTER        = 42993,
    SPELL_SPEED_RAM_TROT          = 42992,
    SPELL_SPEED_RAM_NORMAL        = 43310,
    SPELL_SPEED_RAM_EXHAUSED      = 43332,
    SPELL_RENTAL_RAM_DND          = 42146
};

bool QuestAccept_npc_ram_master_ray(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    switch(pQuest->GetQuestId())
    {
    case QUEST_BARK_FOR_DORN:
    case QUEST_BARK_FOR_TCHALI:
    case QUEST_NOW_RAM_RACING_H:
    case QUEST_BACK_AGAIN_H:
		pPlayer->CastSpell(pPlayer, SPELL_RENTAL_RAM, false);
        break;
    }
    return true;
}

bool QuestRewarded_npc_ram_master_ray(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pPlayer->HasAura(SPELL_RENTAL_RAM))
    {
        pPlayer->RemoveAurasDueToSpell(SPELL_RENTAL_RAM);
        pPlayer->RemoveAurasDueToSpell(SPELL_RAM_FATIGUE);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_GALLOP);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_CANTER);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_TROT);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_NORMAL);
        pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_EXHAUSED);
        pPlayer->RemoveAurasDueToSpell(SPELL_RENTAL_RAM_DND);
    }
    return true;
}

void AddSC_durotar()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_ram_master_ray";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ram_master_ray;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_ram_master_ray;
    newscript->RegisterSelf();
}
