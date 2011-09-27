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
SDName: Item_Scripts
SD%Complete: 100
SDComment: Items for a range of different items. See content below (in script)
SDCategory: Items
EndScriptData */

/* ContentData
item_arcane_charges                 Prevent use if player is not flying (cannot cast while on ground)
item_flying_machine(i34060,i34061)  Engineering crafted flying machines
item_gor_dreks_ointment(i30175)     Protecting Our Own(q10488)
Item_jungle_punch_offer
item_complimentary_brewfest_sampler Brewfest event
item_ram_racing_reins               Brewfest event
EndContentData */

#include "precompiled.h"
#include "Spell.h"

/*#####
# item_arcane_charges
#####*/

enum
{
    SPELL_ARCANE_CHARGES    = 45072
};

bool ItemUse_item_arcane_charges(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
    if (pPlayer->IsTaxiFlying())
        return false;

    pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, NULL);

    if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry(SPELL_ARCANE_CHARGES))
        Spell::SendCastResult(pPlayer, pSpellInfo, 1, SPELL_FAILED_NOT_ON_GROUND);

    return true;
}

/*#####
# item_flying_machine
#####*/

bool ItemUse_item_flying_machine(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
    uint32 itemId = pItem->GetEntry();

    if (itemId == 34060)
        if (pPlayer->GetBaseSkillValue(SKILL_RIDING) >= 225)
            return false;

    if (itemId == 34061)
        if (pPlayer->GetBaseSkillValue(SKILL_RIDING) == 300)
            return false;

    debug_log("SD2: Player attempt to use item %u, but did not meet riding requirement",itemId);
    pPlayer->SendEquipError(EQUIP_ERR_CANT_EQUIP_SKILL, pItem, NULL);
    return true;
}

/*#####
# item_gor_dreks_ointment
#####*/

enum
{
    NPC_TH_DIRE_WOLF        = 20748,
    SPELL_GORDREKS_OINTMENT = 32578
};

bool ItemUse_item_gor_dreks_ointment(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
    if (pTargets.getUnitTarget() && pTargets.getUnitTarget()->GetTypeId() == TYPEID_UNIT && pTargets.getUnitTarget()->HasAura(SPELL_GORDREKS_OINTMENT))
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, NULL);

        if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry(SPELL_GORDREKS_OINTMENT))
            Spell::SendCastResult(pPlayer, pSpellInfo, 1, SPELL_FAILED_TARGET_AURASTATE);

        return true;
    }

    return false;
}

/*#####
# item_petrov_cluster_bombs
#####*/

enum
{
    SPELL_PETROV_BOMB           = 42406,
    AREA_ID_SHATTERED_STRAITS   = 4064,
    ZONE_ID_HOWLING             = 495
};

bool ItemUse_item_petrov_cluster_bombs(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
    if (pPlayer->GetZoneId() != ZONE_ID_HOWLING)
        return false;

    if (!pPlayer->GetTransport() || pPlayer->GetAreaId() != AREA_ID_SHATTERED_STRAITS)
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, NULL);

        if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry(SPELL_PETROV_BOMB))
            Spell::SendCastResult(pPlayer, pSpellInfo, 1, SPELL_FAILED_NOT_HERE);

        return true;
    }

    return false;
}

/*####
# jungle_punch_
####*/

enum
{
    SPELL_OFFER_JUNGLE_PUNCH = 51962
};

bool ItemUse_item_jungle_punch_sample(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
    Unit* pTarget = pPlayer->GetMap()->GetUnit(pPlayer->GetTargetGuid());
    if (pTarget && pTarget->GetTypeId() == TYPEID_UNIT)
    {
        pPlayer->CastSpell(pTarget, SPELL_OFFER_JUNGLE_PUNCH, false);
        return true;
    }
    else
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, NULL);
        return true;
    }
}

/*#####
# item_complimentary_brewfest_sampler
#####*/

enum
{
    NPC_SCOUT  = 24108,
    QUEST_CHUG_AND_CHUCK_A = 12022,
    QUEST_CHUG_AND_CHUCK_H = 12191
};

bool ItemUse_item_complimentary_brewfest_sampler(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
	Map* pMap = pPlayer->GetMap();
    if (pMap)
    {
		Unit* pTarget = pMap->GetUnit(pPlayer->GetTargetGuid());
        if (pTarget)
        {
            if ((pTarget->GetEntry() == NPC_SCOUT) && ((pPlayer->GetQuestStatus(QUEST_CHUG_AND_CHUCK_A) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(QUEST_CHUG_AND_CHUCK_H) == QUEST_STATUS_INCOMPLETE)))
                pPlayer->KilledMonsterCredit(NPC_SCOUT,0);
        }
    }
    pPlayer->CastSpell(pPlayer, 42436, false);

    return true;
}

/*#####
# item_ram_racing_reins
#####*/

enum
{
    SPELL_RAMSTEIN_SWIFT_WORK_RAM = 43880,
    SPELL_RENTAL_RAM              = 43883,
    SPELL_RAM_FATIGUE             = 43052,
    SPELL_SPEED_RAM_GALLOP        = 42994,
    SPELL_SPEED_RAM_CANTER        = 42993,
    SPELL_SPEED_RAM_TROT          = 42992,
    SPELL_SPEED_RAM_NORMAL        = 43310,
    SPELL_SPEED_RAM_EXHAUSED      = 43332,
};

bool ItemUse_item_ram_racing_reins(Player* pPlayer, Item* pItem, const SpellCastTargets &pTargets)
{
    if ((pPlayer->HasAura(SPELL_RENTAL_RAM) || pPlayer->HasAura(SPELL_RAMSTEIN_SWIFT_WORK_RAM)) && !pPlayer->HasAura(SPELL_SPEED_RAM_EXHAUSED))
    {
        if (pPlayer->HasAura(SPELL_SPEED_RAM_NORMAL))
        {
            pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_NORMAL);
            pPlayer->CastSpell(pPlayer, SPELL_SPEED_RAM_TROT, false);
            return true;
        }
        if (pPlayer->HasAura(SPELL_SPEED_RAM_TROT))
        {
			if (SpellAuraHolderPtr pAura = pPlayer->GetSpellAuraHolder(SPELL_SPEED_RAM_TROT))
            {
                if (pAura->GetAuraDuration() >= (pAura->GetAuraMaxDuration() - 1000))
                {
                    pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_TROT);
                    pPlayer->CastSpell(pPlayer, SPELL_SPEED_RAM_CANTER, true);
                    return true;
                }
                pAura->SetAuraDuration(pAura->GetAuraMaxDuration());
            }
        }
        if (pPlayer->HasAura(SPELL_SPEED_RAM_CANTER))
        {
            if (SpellAuraHolderPtr pAura = pPlayer->GetSpellAuraHolder(SPELL_SPEED_RAM_CANTER))
            {
                if (pAura->GetAuraDuration() >= (pAura->GetAuraMaxDuration() - 500))
                {
                    pPlayer->RemoveAurasDueToSpell(SPELL_SPEED_RAM_CANTER);
                    pPlayer->CastSpell(pPlayer, SPELL_SPEED_RAM_GALLOP, false);
                    return true;
                }
                pAura->SetAuraDuration(pAura->GetAuraMaxDuration());
            }
        }
        if (pPlayer->HasAura(SPELL_SPEED_RAM_GALLOP))
        {
            if (SpellAuraHolderPtr pAura = pPlayer->GetSpellAuraHolder(SPELL_SPEED_RAM_GALLOP, pPlayer->GetObjectGuid()))
                pAura->SetAuraDuration(pAura->GetAuraMaxDuration());
        }
    }
    else
        pPlayer->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW ,pItem, NULL);


    return true;
}
void AddSC_item_scripts()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "item_arcane_charges";
    newscript->pItemUse = &ItemUse_item_arcane_charges;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_flying_machine";
    newscript->pItemUse = &ItemUse_item_flying_machine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_gor_dreks_ointment";
    newscript->pItemUse = &ItemUse_item_gor_dreks_ointment;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_petrov_cluster_bombs";
    newscript->pItemUse = &ItemUse_item_petrov_cluster_bombs;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_jungle_punch_sample";
    newscript->pItemUse = &ItemUse_item_jungle_punch_sample;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_complimentary_brewfest_sampler";
    newscript->pItemUse = &ItemUse_item_complimentary_brewfest_sampler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_ram_racing_reins";
    newscript->pItemUse = &ItemUse_item_ram_racing_reins;
    newscript->RegisterSelf();
}
