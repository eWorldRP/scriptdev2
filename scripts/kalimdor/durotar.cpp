/* ScriptData
SDName: Durotar
SD%Complete: 
SDComment: Support for quest 11409
SDCategory: Durotar
EndScriptData */

/* ContentData
npc_ram_master_ray        Brewfest event
npc_bok_ropcertain        Brewfest event
npc_driz_tumblequick      Brewfest event
EndContentData */

#include "precompiled.h"

/*#####
# Ram Master Ray
#####*/

enum
{
    QUEST_BARK_FOR_DROHN     = 11407,
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
        case QUEST_BARK_FOR_DROHN:
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

/*#####
# Bok Dropcertain
#####*/
enum
{
    ITEM_PORTABLE_KEG = 33797
};

struct MANGOS_DLL_DECL npc_bok_dropcertain : public ScriptedAI
{
    npc_bok_dropcertain (Creature* pCreature) : ScriptedAI (pCreature)
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
        if (pPlayer->GetQuestStatus(QUEST_BACK_AGAIN_H) == QUEST_STATUS_INCOMPLETE)
        {
            // player can only have 1 keg
            if (pPlayer->HasItemCount(ITEM_PORTABLE_KEG, 1))
                return;

            ItemPosCountVec dest;
            uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_PORTABLE_KEG, 1, false);
            if (msg == EQUIP_ERR_OK)
            {
                pPlayer->StoreNewItem(dest, ITEM_PORTABLE_KEG, 1, true);
                pPlayer->GetItemByEntry(ITEM_PORTABLE_KEG)->SendCreateUpdateToPlayer(pPlayer);
            }
        }
    }
};

CreatureAI* GetAI_npc_bok_dropcertain(Creature* pCreature)
{
    return new npc_bok_dropcertain(pCreature);
}

/*#####
# Driz Tumberquick
#####*/
enum
{
    NPC_CREDIT_CREATURE     = 24337,
};

struct MANGOS_DLL_DECL npc_driz_tumblequick : public ScriptedAI
{
    npc_driz_tumblequick (Creature* pCreature) : ScriptedAI (pCreature)
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
        if (pPlayer->GetQuestStatus(QUEST_BACK_AGAIN_H) == QUEST_STATUS_INCOMPLETE)
        {
            // player can only have 1 keg
            if (pPlayer->HasItemCount(ITEM_PORTABLE_KEG, 1))
            {
                pPlayer->DestroyItemCount(ITEM_PORTABLE_KEG, 1, true);
                pPlayer->KilledMonsterCredit(NPC_CREDIT_CREATURE,0);
            }
        }
    }
};

CreatureAI* GetAI_npc_driz_tumblequick(Creature* pCreature)
{
    return new npc_driz_tumblequick(pCreature);
}
void AddSC_durotar()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_ram_master_ray";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ram_master_ray;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_ram_master_ray;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bok_dropcertain";
    newscript->GetAI = &GetAI_npc_bok_dropcertain;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_driz_tumblequick";
    newscript->GetAI = &GetAI_npc_driz_tumblequick;
    newscript->RegisterSelf();
}
