/* Copyright (C) 2006 - 2011 ScriptDev2 <http://www.scriptdev2.com/>
 * Copyright (C) 2011 MangosR2_ScriptDev2
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
SDName: boss_kiljaeden
SD%Complete:
SDComment: Firebloom needs implented // 
SDCategory: Sunwell Plateau
EndScriptData */

#include "precompiled.h"
#include "sunwell_plateau.h"

enum UsedSpells
{
    /*** Spells used during the encounter ***/
    /* Hand of the Deceiver's spells and cosmetics */
    SPELL_SHADOW_BOLT_VOLLEY               = 45770, // ~30 yard range Shadow Bolt Volley for ~2k(?) damage
    SPELL_SHADOW_INFUSION                  = 45772, // They gain this at 20% - Immunity to Stun/Silence and makes them look angry!
    SPELL_FELFIRE_PORTAL                   = 46875, // Creates a portal that spawns Felfire Fiends (LIVE FOR THE SWARM!1 FOR THE OVERMIND!)
    SPELL_SHADOW_CHANNELING                = 46757, // Channeling animation out of combat
    SPELL_SUMMON_FELFIRE_FIEND             = 46464,

    /* Volatile Felfire Fiend's spells */
    SPELL_FELFIRE_FISSION                  = 45779, // Felfire Fiends explode when they die or get close to target.

    /* Kil'Jaeden's spells and cosmetics */
    SPELL_TRANS                            = 23188, // Surprisingly, this seems to be the right spell.. (Where is it used?)
    SPELL_REBIRTH                          = 44200, // Emerge from the Sunwell
    SPELL_SOUL_FLAY_DAMAGE                 = 45442, // 9k Shadow damage over 3 seconds. Spammed throughout all the fight.
    SPELL_SOUL_FLAY_MOVEMENT               = 47106, // -50% Movement component of the above. Players cast it on selves unless this spell is fixed.
    SPELL_LEGION_LIGHTNING                 = 45664, // Chain Lightning, 4 targets, ~3k Shadow damage, 1.5k mana burn
    SPELL_FIRE_BLOOM                       = 45641, // Places a debuff on 5 raid members, which causes them to deal 2k Fire damage to nearby allies and selves. MIGHT NOT WORK
    SPELL_SINISTER_REFLECTION              = 45785, // Summon shadow copies of 5 raid members that fight against KJ's enemies
    SPELL_COPY_WEAPON                      = 41055, // }
    SPELL_COPY_WEAPON2                     = 41054, // }
    SPELL_COPY_OFFHAND                     = 45206, // }- Spells used in Sinister Reflection creation
    SPELL_COPY_OFFHAND_WEAPON              = 45205, // }
    SPELL_SINISTER_REFLECTION_CLASS        = 45893,
    SPELL_SINISTER_REFLECTION_CLONE        = 45785, // Cause the target to become a clone of the caster.
    SPELL_SINISTER_REFLECTION_SUMMON       = 45891,
    SPELL_SHADOW_SPIKE                     = 46680, // Bombard random raid members with Shadow Spikes (Very similar to Void Reaver orbs)
    SPELL_SHADOW_SPIKE_VISUAL              = 46589,
    SPELL_SHADOW_SPIKE_EFFECT              = 45885, // Inflicts 5100 to 6900% Shadow damage to an enemy and leaves it wounded, reducing the effectiveness of any healing by 50% for 10 sec.
    SPELL_FLAME_DART                       = 45740, // Bombards the raid with flames every 3(?) seconds
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS     = 46605, // Begins a 8-second channeling, after which he will deal 50'000 damage to the raid
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS_EFFECT = 45657, // Deals 47500 to 52500 Shadow damage to all enemies within reach.
    SPELL_ARMAGEDDON                       = 45909, // Meteor spell
    SPELL_ARMAGEDDON_VISUAL                = 45911, // Does the hellfire visual to indicate where the meteor missle lands
    SPELL_SACRIFICE_OF_ANVEENA             = 46474, // Anveena sacrifices herself, causing Kil'jaeden to take 25% increased Holy damage.

    /* Anveena's spells and cosmetics (Or, generally, everything that has "Anveena" in name) */
    SPELL_ANVEENA_PRISON                   = 46367, // She hovers locked within a bubble
    SPELL_ANVEENA_ENERGY_DRAIN             = 46410, // Sunwell energy glow animation (Control mob uses this)

    /* Shield Orb spells */
    SPELL_SHADOW_BOLT                      = 45680, // constantly shooting Shadow Bolts at the raid (up to 3 bolts within 1 second)
    SPELL_SHADOW_ORB_BOLT_TRIGG            = 45679,

    /* Orb of the Blue Dragonflight */
    SPELL_BLINK                            = 45862,
    SPELL_BREATH_REVITALIZE                = 45860,
    SPELL_BREATH_HASTE                     = 45856,
    SPELL_SHIELD_OF_THE_BLUE               = 45848,

    /*** Other Spells (used by players, etc) ***/
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT     = 45839, // Possess the blue dragon from the orb to help the raid.
    SPELL_POWER_OF_THE_BLUE_FLIGHT         = 45833, // Empowers a humanoid with the essence of the Blue Flight.
    SPELL_SUMMON_BLUE_DRAKE                = 45836, // Summon (25653)
    SPELL_ENTROPIUS_BODY                   = 46819, // Visual for Entropius at the Epilogue
    SPELL_RING_OF_BLUE_FLAME               = 45825, // Blue Flame Ring on actived orb

    //Orbs of DragonFligth
    SPELL_REVITALIZE            = 45027, // 100%
    SPELL_SHIELD_OF_BLUE        = 45848, // 100%
    AURA_BLUESHIELD             = 26013, // :D
    AURA_NODRAGON               = 44032, // workaround to dont summon more than 1 dragon per minute
    SPELL_HASTE                 = 45856, // NOT WORKING
};

enum Text
{
    /*** Speech and sounds***/
    // These are used throughout Sunwell and Magisters(?). Players can hear this while running through the instances.
    SAY_KJ_OFFCOMBAT1                       = -1580056,
    SAY_KJ_OFFCOMBAT2                       = -1580057,
    SAY_KJ_OFFCOMBAT3                       = -1580058,
    SAY_KJ_OFFCOMBAT4                       = -1580059,
    SAY_KJ_OFFCOMBAT5                       = -1580060,

    // Encounter speech and sounds
    SAY_KJ_EMERGE                           = -1580061,
    SAY_KJ_SLAY1                            = -1580062,
    SAY_KJ_SLAY2                            = -1580063,
    SAY_KJ_REFLECTION1                      = -1580064,
    SAY_KJ_REFLECTION2                      = -1580065,
    SAY_KJ_DARKNESS1                        = -1580066,
    SAY_KJ_DARKNESS2                        = -1580067,
    SAY_KJ_DARKNESS3                        = -1580068,
    SAY_KJ_CANNOT_WIN                       = -1580070,
    SAY_KJ_DENINE                           = -1580069,
    SAY_KJ_LOST_POWER                       = -1580071,

    /*** Kalecgos - Anveena speech at the beginning of Phase 5; Anveena's sacrifice ***/
    SAY_KALECGOS_INTRO                      = -1580072,
    SAY_KALECGOS_AWAKEN                     = -1580073,
    SAY_ANVEENA_IMPRISONED                  = -1580074,
    SAY_KALECGOS_LETGO                      = -1580075,
    SAY_ANVEENA_LOST                        = -1580076,
    SAY_KALECGOS_FOCUS                      = -1580077,
    SAY_ANVEENA_KALEC                       = -1580078,
    SAY_KALECGOS_FATE                       = -1580079,
    SAY_ANVEENA_GOODBYE                     = -1580080,
    SAY_KALECGOS_GOODBYE                    = -1580081,
    SAY_KALECGOS_ENCOURAGE                  = -1580082,
    SAY_KALECGOS_ORB1                       = -1580083,
    SAY_KALECGOS_ORB2                       = -1580084,
    SAY_KALECGOS_ORB3                       = -1580085,
    SAY_KALECGOS_ORB4                       = -1580086,

    /*** Outro Sounds***/
    SAY_VELEN_01 = 12515,
    SAY_VELEN_02 = 12516,
    SAY_VELEN_03 = 12517,
    SAY_VELEN_04 = 12518,
    SAY_VELEN_05 = 12519,
    SAY_VELEN_06 = 12520,
    SAY_VELEN_07 = 12521,
    SAY_VELEN_08 = 12522,
    SAY_VELEN_09 = 12523,
    SAY_LIADRIN_01 = 12526,
    SAY_LIADRIN_02 = 12525,
    SAY_LIADRIN_03 = 12524,
};

enum Npcs_Go
{
    NPC_BLUE_DRAGON     = 25653, // controlled by players
    NPC_VELEN_PORTAL    = 24925,
    NPC_CORE_ENTROPIUS  = 26262, // Used in the ending cinematic?

    // encounter
    NPC_DECIVER         = 25588,
    NPC_FELFIRE_PORTAL  = 25603,
    NPC_SHIELD_ORB      = 25502,
    NPC_SINISTER        = 25708, // Sinister Reflection
    NPC_ARMAGEDDON      = 25735,
    NPC_FELFIRE_FIEND   = 25598,
    NPC_SHADOWSPIKE     = 30598,

    // objects
    NPC_BLUE_ORB_TARGET = 25640,
    GO_ORB_OF_THE_BLUE_DRAGONFLIGHT = 188415,

    // misc
    MAX_DECEIVERS       = 3,
};

enum Phase
{
    PHASE_IDLE       = 0,
    PHASE_DECEIVERS  = 1, // Fight with the 3 Deceivers
    PHASE_NORMAL     = 2, // Kil'Jaeden emerges from the sunwell
    PHASE_DARKNESS   = 3, // At 85%, he gains few abilities; Kalecgos joins the fight
    PHASE_ARMAGEDDON = 4, // At 55%, he gains even more abilities
    PHASE_SACRIFICE  = 5, // At 25%, Anveena sacrifices herself into the Sunwell; at this point he becomes enraged and has *significally* shorter cooldowns.
    PHASE_OUTRO      = 6,
};

uint8 m_uiDecieverDead;

/*######
## mob_kalecgos
######*/

struct MANGOS_DLL_DECL mob_kalecgosAI : public ScriptedAI
{
    mob_kalecgosAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiOrbsEmpowered;
    uint32 m_uiEmpowerTimer;
    ObjectGuid m_auiOrb[4];

    void Reset()
    {
        m_uiOrbsEmpowered = 0;
        m_uiEmpowerTimer = 0;

        for(uint8 i = 0; i < 4; ++i)
            m_auiOrb[i] = 0;

        FindOrbs();

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetLevitate(true);
    }

    void UpdateAI(const uint32 uiDiff) 
    {
        // Empower a orb
        if (m_uiEmpowerTimer)
        {
            if(m_uiEmpowerTimer < uiDiff)
            {
                switch(m_uiOrbsEmpowered)
                {
                    case 0: DoScriptText(SAY_KALECGOS_ORB1, m_creature);break;
                    case 1: DoScriptText(SAY_KALECGOS_ORB2, m_creature);break;
                    case 2: DoScriptText(SAY_KALECGOS_ORB3, m_creature);break;
                    case 3: DoScriptText(SAY_KALECGOS_ORB4, m_creature);break;
                }

                EmpowerOrb(m_uiOrbsEmpowered);
                m_uiOrbsEmpowered++;

                if (m_uiOrbsEmpowered == 3)
                    m_uiEmpowerTimer = 20000;
                else
                    m_uiEmpowerTimer = 0;

            }
            else m_uiEmpowerTimer -= uiDiff;
        }
    }

    void EmpowerOrb(uint32 uiActiveOrb)
    {
        if (uiActiveOrb < 4)
        {
            GameObject* pOrb = m_creature->GetMap()->GetGameObject(m_auiOrb[uiActiveOrb]);

            if(!pOrb)
                return;

            // workaround effect
            if (Unit* pSummon = m_creature->SummonCreature(NPC_SINISTER, pOrb->GetPositionX(), pOrb->GetPositionY(), pOrb->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 300000))
            {
                pSummon->SetDisplayId(11686);
                pSummon->setFaction(35);
                pSummon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                pSummon->CastSpell(pSummon, SPELL_RING_OF_BLUE_FLAME, true);
            }

            //m_creature->CastSpell(orb, SPELL_RING_OF_BLUE_FLAME, true);
            pOrb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
        }
    }

    void FindOrbs()
    {
        std::list<GameObject*> orbList;
        GetGameObjectListWithEntryInGrid(orbList, m_creature, GO_ORB_OF_THE_BLUE_DRAGONFLIGHT, 200.0f);

        if(orbList.empty())
            return;

        uint8 i = 0;
        for(std::list<GameObject*>::iterator itr = orbList.begin(); itr != orbList.end(); ++itr)
        {
            m_auiOrb[i] = (*itr)->GetObjectGuid();
            (*itr)->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            i++;
        }
    }

};

CreatureAI* GetAI_mob_kalecgos(Creature *_Creature)
{
    return new mob_kalecgosAI (_Creature);
}

/*######
## mob_kiljaeden_controller
######*/

struct MANGOS_DLL_DECL mob_kiljaeden_controllerAI : public Scripted_NoMovementAI
{
    mob_kiljaeden_controllerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();

        // hack spell 45885
        SpellEntry* pTempSpell = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_SHADOW_SPIKE_EFFECT);
        if (pTempSpell && pTempSpell->EffectImplicitTargetB[0] != 16)
        {
               pTempSpell->EffectImplicitTargetA[0] = 53;
               pTempSpell->EffectImplicitTargetA[1] = 0;
               pTempSpell->EffectImplicitTargetB[0] = 16;
               pTempSpell->EffectImplicitTargetB[1] = 0;
        }
        pTempSpell = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_SHIELD_OF_THE_BLUE);
        if (pTempSpell && pTempSpell->EffectImplicitTargetB[0] != 22)
        {
               pTempSpell->EffectImplicitTargetB[0] = 22;
        }
    }

    ScriptedInstance* m_pInstance;
    std::list<ObjectGuid> m_uiHandDeceivers;

    bool m_bIsOutroEnd;
    bool m_bIsProphetSpawned;
    bool m_bIsOnce;

    ObjectGuid m_auiOrb[4];

    uint32 m_uiOrbsEmpowered;
    uint32 m_uiEmpowerTimer;
    uint32 m_uiEventTimer;
    uint32 m_uiAnveenaSpeechTimer;
    uint32 m_uiAnveenaSpeechCounter;
    uint32 m_uiProphetsTimer;
    uint32 m_uiProphetsCount;
    uint32 m_uiOrdersTimer;


    void Reset()
    {
        m_bIsOnce                   = true;
        m_bIsProphetSpawned         = true;
        m_bIsOutroEnd               = false;
        m_uiEventTimer              = 1000;
        m_uiOrdersTimer             = 30000;
        m_uiAnveenaSpeechTimer      = 0;
        m_uiAnveenaSpeechCounter    = 0;
        m_uiProphetsCount           = 10;
        m_uiOrbsEmpowered           = 0;
        m_uiEmpowerTimer            = 0;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        std::list<GameObject*> orbList;
        GetGameObjectListWithEntryInGrid(orbList, m_creature, GO_ORB_OF_THE_BLUE_DRAGONFLIGHT, 50.0f);

        if(!orbList.empty())
        {
            uint8 i = 0;
            for(std::list<GameObject*>::iterator itr = orbList.begin(); itr != orbList.end(); ++itr)
            {
                m_auiOrb[i] = (*itr)->GetObjectGuid();
                (*itr)->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                i++;
            }
        }

        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_IDLE);
            m_pInstance->SetData(TYPE_KILJAEDEN, NOT_STARTED);

            if (Creature* pKalecgos = m_pInstance->GetSingleCreatureFromStorage(NPC_KALECGOS))
            {
                if(mob_kalecgosAI* pKalecgosAI = dynamic_cast<mob_kalecgosAI*>(pKalecgos->AI()))
                    pKalecgosAI->Reset();
            }
        }
    }

    void EmpowerOrb(uint32 uiActiveOrb)
    {
        if (uiActiveOrb < 4)
        {
            GameObject* pOrb = m_creature->GetMap()->GetGameObject(m_auiOrb[uiActiveOrb]);

            if(!pOrb)
                return;

            // workaround effect
            if (Unit* pSummon = m_creature->SummonCreature(NPC_SINISTER, pOrb->GetPositionX(), pOrb->GetPositionY(), pOrb->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 300000))
            {
                pSummon->SetDisplayId(11686);
                pSummon->setFaction(35);
                pSummon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                pSummon->CastSpell(pSummon, SPELL_RING_OF_BLUE_FLAME, true);
            }

            //m_creature->CastSpell(orb, SPELL_RING_OF_BLUE_FLAME, true);
            pOrb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiEventTimer < uiDiff)
        {
            if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_IDLE)
            {
                // for reset in case of crash during the encounter
                std::list<Creature*> lDeceiver;
                GetCreatureListWithEntryInGrid(lDeceiver, m_creature, NPC_DECIVER, 50.0f);

                if (!lDeceiver.empty())
                {
                    for(std::list<Creature*>::iterator itr = lDeceiver.begin(); itr != lDeceiver.end(); ++itr)
                    {
                        if ((*itr) && !(*itr)->isAlive())
                            (*itr)->Respawn();
                    }
                }
            }

            if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_DECEIVERS)
            {

                if (m_uiOrdersTimer < uiDiff)
                {
                    switch (rand()%5)
                    {
                        case 0:
                            DoScriptText(SAY_KJ_OFFCOMBAT1, m_creature);
                            break;
                        case 1:
                            DoScriptText(SAY_KJ_OFFCOMBAT2, m_creature);
                            break;
                        case 2:
                            DoScriptText(SAY_KJ_OFFCOMBAT3, m_creature);
                            break;
                        case 3:
                            DoScriptText(SAY_KJ_OFFCOMBAT4, m_creature);
                            break;
                        case 4:
                            DoScriptText(SAY_KJ_OFFCOMBAT5, m_creature);
                            break;
                    }
                    m_uiOrdersTimer = 30000;
                }
                else m_uiOrdersTimer -= uiDiff;

                // if all dead, KILJAEDEN
                if (m_uiDecieverDead >= MAX_DECEIVERS)
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_ANVEENA_ENERGY_DRAIN);
                    m_uiAnveenaSpeechTimer = 25000; // first speech after 25 secs

                    if(Creature* pKiljaeden = DoSpawnCreature(NPC_KILJAEDEN, 0, 0,0, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, HOUR*2*IN_MILLISECONDS))
                    {
                        pKiljaeden->CastSpell(pKiljaeden, SPELL_REBIRTH, false);
                        DoScriptText(SAY_KJ_EMERGE, pKiljaeden);
                        pKiljaeden->SetInCombatWithZone();
                    }
                }
            }

            // Anveena and Kil'jaeden spawn check
            if (m_pInstance)
            {
                Creature* pKiljaeden;
                if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) >= PHASE_NORMAL)
                    pKiljaeden = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN);
                else
                    pKiljaeden = NULL;

                Creature* pAnveena = m_pInstance->GetSingleCreatureFromStorage(NPC_ANVEENA);
                Creature* pKalecgos = m_pInstance->GetSingleCreatureFromStorage(NPC_KALECGOS);

                if (m_pInstance->GetData(TYPE_KILJAEDEN) == IN_PROGRESS)
                {
                    if(m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) < PHASE_SACRIFICE)
                    {
                        m_uiAnveenaSpeechCounter = 0;
                        m_uiAnveenaSpeechTimer = 5000;
                    }
                    else if(m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) < PHASE_ARMAGEDDON)
                    {
                        m_uiAnveenaSpeechCounter = 0;
                        m_uiAnveenaSpeechTimer = 5000;
                    }
                    else if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) < PHASE_DARKNESS)
                    {
                        m_uiAnveenaSpeechCounter = 0;
                        m_uiAnveenaSpeechTimer = 5000;
                    }
                }
                
                if (m_pInstance->GetData(TYPE_KILJAEDEN) == DONE && m_bIsOutroEnd)
                {
                    m_creature->ForcedDespawn();
                    return;
                }

                // Empower a orb
                if (m_uiEmpowerTimer && pKalecgos)
                {
                    if(m_uiEmpowerTimer < uiDiff)
                    {
                        switch(m_uiOrbsEmpowered)
                        {
                            case 0:
                                DoScriptText(SAY_KALECGOS_ORB1, pKalecgos);
                                break;
                            case 1:
                                DoScriptText(SAY_KALECGOS_ORB2, pKalecgos);
                                break;
                            case 2:
                                DoScriptText(SAY_KALECGOS_ORB3, pKalecgos);
                                break;
                            case 3:
                                DoScriptText(SAY_KALECGOS_ORB4, pKalecgos);
                                break;
                        }

                        EmpowerOrb(m_uiOrbsEmpowered);
                        m_uiOrbsEmpowered++;

                        if (m_uiOrbsEmpowered == 3)
                            m_uiEmpowerTimer = 20000;
                        else
                            m_uiEmpowerTimer = 0;

                    }
                    else m_uiEmpowerTimer -= uiDiff;
                }

                // Phase 2 speech
                if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_NORMAL && m_uiAnveenaSpeechTimer)
                {
                    if (m_uiAnveenaSpeechTimer < uiDiff)
                    {
                        if (pKalecgos && pKalecgos->isAlive())
                            DoScriptText(SAY_KALECGOS_INTRO, pKalecgos);
                        m_uiAnveenaSpeechTimer = 0;
                    }
                    else
                        m_uiAnveenaSpeechTimer -= uiDiff;
                }

                // Phase 3 speech
                else if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_DARKNESS && m_uiAnveenaSpeechTimer)
                {
                    if(m_uiAnveenaSpeechTimer < uiDiff)
                    {
                        switch(m_uiAnveenaSpeechCounter)
                        {
                            case 0:
                                    if (pKalecgos)
                                        DoScriptText(SAY_KALECGOS_AWAKEN, pKalecgos);
                                        m_uiAnveenaSpeechTimer = 5000;
                                    break;
                            case 1:
                                    if (pAnveena)
                                        DoScriptText(SAY_ANVEENA_IMPRISONED, pAnveena);
                                        m_uiAnveenaSpeechTimer = 5000;
                                    break;
                            case 2:
                                    if (pKiljaeden)
                                        DoScriptText(SAY_KJ_DENINE, pKiljaeden);
                                        m_uiAnveenaSpeechTimer = 0;
                                    break;
                        }
                        
                        m_uiAnveenaSpeechCounter++;
                    }
                    else
                        m_uiAnveenaSpeechTimer -= uiDiff;
                }
                // Phase 4 speech
                else if ( m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_ARMAGEDDON && m_uiAnveenaSpeechTimer)
                {
                    if (m_uiAnveenaSpeechTimer < uiDiff)
                    {
                        switch(m_uiAnveenaSpeechCounter)
                        {
                            case 0:
                                    if (pKalecgos)
                                        DoScriptText(SAY_KALECGOS_LETGO, pKalecgos);
                                        m_uiAnveenaSpeechTimer = 7000;
                                    break;
                            case 1:
                                    if (pAnveena)
                                        DoScriptText(SAY_ANVEENA_LOST, pAnveena);
                                        m_uiAnveenaSpeechTimer = 5000;
                                    break;
                            case 2:
                                    if (pKiljaeden)
                                        DoScriptText(SAY_KJ_CANNOT_WIN, pKiljaeden);
                                        m_uiAnveenaSpeechTimer = 0;
                                    break;
                        }
                        
                        m_uiAnveenaSpeechCounter++;
                    }
                    else
                        m_uiAnveenaSpeechTimer -= uiDiff;
                }
                // Phase 5 speech
                else if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_SACRIFICE && m_uiAnveenaSpeechTimer)
                {
                    if (m_uiAnveenaSpeechTimer < uiDiff)
                    {
                        switch(m_uiAnveenaSpeechCounter)
                        {
                            case 0:
                                if (pKalecgos)
                                    DoScriptText(SAY_KALECGOS_FOCUS, pKalecgos);
                                m_uiAnveenaSpeechTimer = 10000;
                                break;
                            case 1:
                                if (pAnveena)
                                    DoScriptText(SAY_ANVEENA_KALEC, pAnveena);
                                m_uiAnveenaSpeechTimer = 3000;
                                break;
                            case 2:
                                if (pKalecgos)
                                    DoScriptText(SAY_KALECGOS_FATE, pKalecgos);
                                m_uiAnveenaSpeechTimer = 7000;
                                break;
                            case 3:
                                if (pAnveena)
                                    DoScriptText(SAY_ANVEENA_GOODBYE, pAnveena);
                                m_uiAnveenaSpeechTimer = 6000;
                                break;
                            case 4:
                                if (pKiljaeden && pKiljaeden->isAlive() && pAnveena && pAnveena->isAlive())
                                {
                                    pAnveena->CastSpell(pKiljaeden, SPELL_SACRIFICE_OF_ANVEENA, true);

                                    pAnveena->ForcedDespawn();
                                    DoScriptText(SAY_KJ_LOST_POWER, pKiljaeden);
                                }
                                m_uiAnveenaSpeechTimer = 13000;
                                break;
                            case 5:
                                if (pKalecgos)
                                    DoScriptText(SAY_KALECGOS_GOODBYE, pKalecgos);
                                m_uiAnveenaSpeechTimer = 14000;
                                break;
                            case 6:
                                if (pKalecgos)
                                    DoScriptText(SAY_KALECGOS_ENCOURAGE, pKalecgos);
                                m_uiAnveenaSpeechTimer = 0;
                                break;
                        }
                        m_uiAnveenaSpeechCounter++;
                    }
                    else
                        m_uiAnveenaSpeechTimer -= uiDiff;
                }
            }
        }
        else
            m_uiEventTimer -=uiDiff;
        
        // OUTRO
        if (m_pInstance && m_pInstance->GetData(TYPE_KILJAEDEN) == DONE && m_bIsOnce)
        {
            m_bIsProphetSpawned = false;
            m_uiProphetsCount = 0;
            m_uiProphetsTimer = 20000;
            m_bIsOnce = false;
        }
        
        if (!m_bIsProphetSpawned)
        {
            if (Creature* pVelen = m_creature->SummonCreature(NPC_VELEN, m_creature->GetPositionX()+15, m_creature->GetPositionY()+15, m_creature->GetPositionZ(), 3.874f, TEMPSUMMON_TIMED_DESPAWN, 360000))
            {
                pVelen->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pVelen->setFaction(35);
            }

            if (Creature* pLiadrin = m_creature->SummonCreature(NPC_LIADRIN, m_creature->GetPositionX()+20, m_creature->GetPositionY()+17, m_creature->GetPositionZ(), 3.874f, TEMPSUMMON_TIMED_DESPAWN, 360000))
            {
            pLiadrin->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            pLiadrin->setFaction(35);
            }

            m_bIsProphetSpawned = true;
        }

        if (!m_bIsOnce)
        {
            if ((m_uiProphetsTimer < uiDiff) && !m_bIsOutroEnd)
            {
                switch(m_uiProphetsCount)
                {
                    case 0:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_01);
                        m_uiProphetsTimer = 25000;
                        break;
                    case 1:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_02);
                        m_uiProphetsTimer = 15000;
                        break;
                    case 2:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_03);
                        m_uiProphetsTimer = 10000;
                        break;
                    case 3:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_04);
                        m_uiProphetsTimer = 26000;
                        break;
                    case 9:
                        DoPlaySoundToSet(m_creature, SAY_LIADRIN_01);
                        m_uiProphetsTimer = 20000;
                        break;
                    case 5:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_05);
                        m_uiProphetsTimer = 16000;
                        break;
                    case 6:
                        DoPlaySoundToSet(m_creature, SAY_LIADRIN_02);
                        m_uiProphetsTimer = 3000;
                        break;
                    case 7:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_06);
                        m_uiProphetsTimer = 17000;
                        break;
                    case 8:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_07);
                        m_uiProphetsTimer = 16000;
                        break;
                    case 4:
                        DoPlaySoundToSet(m_creature, SAY_LIADRIN_03);
                        m_uiProphetsTimer = 11000;
                        break;
                    case 10:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_08);
                        m_uiProphetsTimer = 7000;
                        break;
                    case 11:
                        DoPlaySoundToSet(m_creature, SAY_VELEN_09);
                        m_bIsOutroEnd = true;
                        m_creature->setFaction(35);
                        break;
                }
                ++m_uiProphetsCount;
            }
            else
                m_uiProphetsTimer -= uiDiff;
        }
    }

    void FindHandDeceivers()
    {
        std::list<Creature*> DeceiverList;
        GetCreatureListWithEntryInGrid(DeceiverList, m_creature, NPC_DECIVER, 50.0f);

        if (!DeceiverList.empty())
        {
            m_uiHandDeceivers.clear();
            for(std::list<Creature*>::iterator itr = DeceiverList.begin(); itr != DeceiverList.end(); ++itr)
                m_uiHandDeceivers.push_back((*itr)->GetObjectGuid());
        }
    }
};

CreatureAI* GetAI_mob_kiljaeden_controller(Creature *pCreature)
{
    return new mob_kiljaeden_controllerAI(pCreature);
}

/*######
## boss_kiljaeden
######*/

struct MANGOS_DLL_DECL boss_kiljaedenAI : public Scripted_NoMovementAI
{
    boss_kiljaedenAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        Reset();
    }

    /* General */
    ScriptedInstance* m_pInstance;
    uint32 m_uiCancelShieldTimer;

    /* Phase 2+ spells */
    uint32 m_uiSoulFlayTimer;
    uint32 m_uiLegionLightningTimer;
    uint32 m_uiFireBloomTimer;
    uint32 m_uiSummonShieldOrbTimer;

    /* Phase 3+ spells */
    uint32 m_uiFlameDartTimer;
    uint32 m_uiDarknessTimer;
    uint32 m_uiDarknessBombTimer;
    uint32 m_uiShadowSpikeTimer;
    uint32 m_uiShadowSpikeCount;

    /* Phase 4+ spells */
    uint32 m_uiArmageddonTimer;

    void Reset()
    {
        // Special Timers and Stuff Reset
        m_uiSoulFlayTimer         = 15000;
        m_uiLegionLightningTimer  = 30000;
        m_uiFireBloomTimer        = 30000;
        m_uiSummonShieldOrbTimer  = 45000;

        m_uiFlameDartTimer        = 3000;
        m_uiDarknessTimer         = 60000;
        m_uiDarknessBombTimer     = 0;
        m_uiShadowSpikeTimer      = 5000;
        m_uiShadowSpikeCount       = 30000;

        m_uiArmageddonTimer       = 10000;
        m_uiCancelShieldTimer     = 3000000;
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_KJ_EMERGE, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_NORMAL);
    }

    void JustReachedHome()
    {
        m_uiDecieverDead = 0;

        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_KILJAEDEN, NOT_STARTED);
            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_IDLE);

            if (Creature* pAnveena = m_pInstance->GetSingleCreatureFromStorage(NPC_ANVEENA))
            {
                if (!pAnveena->isAlive())
                    pAnveena->Respawn();
            }

            if (Creature* pController = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN_CONTROLLER))
            {
                if (pController->isAlive())
                    pController->AI()->EnterEvadeMode();
            }
        }

        std::list<Creature*> lDecievers;
        GetCreatureListWithEntryInGrid(lDecievers, m_creature, NPC_DECIVER, 40.0f);
        if (!lDecievers.empty())
        {
            for(std::list<Creature*>::iterator iter = lDecievers.begin(); iter != lDecievers.end(); ++iter)
            {
                if ((*iter) && !(*iter)->isAlive())
                   (*iter)->Respawn();
            }
        }

        m_creature->ForcedDespawn();
    }

    void CastSinisterReflection()
    {
//        for (uint8 i = 0; i < 4; ++i) // disadvantage is it may be duplicated target
//            DoCast(m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0), SPELL_SINISTER_REFLECTION);
        // workaround
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (Player* pPlayer = pTarget->GetCharmerOrOwnerPlayerOrPlayerItself())
                {
                    if (Creature* pSummon = pPlayer->SummonCreature(NPC_SINISTER, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000))
                    {
                        pSummon->SetDisplayId(pPlayer->GetDisplayId());
                        pSummon->setFaction(m_creature->getFaction());
                        pSummon->SetMaxHealth(100000);
                        pSummon->SetHealth(100000);
                        
                        pSummon->setPowerType(pPlayer->getPowerType());
                        pSummon->SetMaxPower(pPlayer->getPowerType(),pPlayer->GetMaxPower(pPlayer->getPowerType()));
                        pSummon->SetPower(pPlayer->getPowerType(),pPlayer->GetMaxPower(pPlayer->getPowerType()));

                     // pSummon->CastSpell(pPlayer, SPELL_SINISTER_REFLECTION_CLASS, true);
                     // pSummon->CastSpell(pPlayer, SPELL_SINISTER_REFLECTION_CLONE, true);
                        pSummon->CastSpell(pPlayer, SPELL_COPY_WEAPON, true);
                        pSummon->CastSpell(pPlayer, SPELL_COPY_WEAPON2, true);
                        pSummon->CastSpell(pPlayer, SPELL_COPY_OFFHAND, true);
                        pSummon->CastSpell(pPlayer, SPELL_COPY_OFFHAND_WEAPON, true);
                        pSummon->AI()->AttackStart(pPlayer);
                    }
                }
            }
        }

        switch(urand(0,1))
        {
            case 0: DoScriptText(SAY_KJ_REFLECTION1, m_creature);break;
            case 1: DoScriptText(SAY_KJ_REFLECTION2, m_creature);break;
        }
    }

    void DamageDeal(Unit* pDoneTo, uint32& uiDamage) 
    {
        if(pDoneTo->HasAura(AURA_BLUESHIELD))
            uiDamage = uiDamage * 0.05;
    }

    void KilledUnit(Unit* pVictim)
    {
        switch (rand()%2)
        {
            case 0: DoScriptText(SAY_KJ_SLAY1, m_creature); break;
            case 1: DoScriptText(SAY_KJ_SLAY2, m_creature); break;
        }
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_KILJAEDEN, DONE);
            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_OUTRO);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || !m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if(m_uiCancelShieldTimer < uiDiff)
        {
            std::list<HostileReference *> t_list = m_creature->getThreatManager().getThreatList();
            for(std::list<HostileReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
            {
                Player* pPlayer = m_creature->GetMap()->GetPlayer((*itr)->getUnitGuid());
                if (pPlayer && pPlayer->HasAura(AURA_BLUESHIELD))
                {
                    if(pPlayer->HasAura(AURA_BLUESHIELD))
                        pPlayer->RemoveAurasDueToSpell(AURA_BLUESHIELD);
                } 
            }
            m_uiCancelShieldTimer = 3000000;
        }
        else
            m_uiCancelShieldTimer -= uiDiff;


        // *****************************************
        // ********* Check for Blue Aura ***********
        // *****************************************
        if(Unit* pDragon = GetClosestCreatureWithEntry(m_creature, NPC_BLUE_DRAGON, 80.0f))
        {
            if(pDragon && pDragon->HasAura(SPELL_SHIELD_OF_BLUE))
            {
                std::list<HostileReference *> t_list = m_creature->getThreatManager().getThreatList();
                for(std::list<HostileReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
                {
                    Player *pPlayer = m_creature->GetMap()->GetPlayer((*itr)->getUnitGuid());
                    if (pPlayer && pPlayer->IsWithinDistInMap(pDragon, 15.0f))
                    {
                        pPlayer->RemoveAurasDueToSpell(AURA_BLUESHIELD);
                        pPlayer->CastSpell(pPlayer, AURA_BLUESHIELD, true);
                    }
                }
            }
        }

        // *****************************************
        // *********** Phase 2 spells **************
        // *****************************************

        // Soul Flay : 9k damage over 3 seconds on highest aggro
        if (m_uiSoulFlayTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_SOUL_FLAY_DAMAGE);
            m_creature->getVictim()->CastSpell(m_creature->getVictim(), SPELL_SOUL_FLAY_MOVEMENT, true);
            m_uiSoulFlayTimer = 3500;
        }
        else
            m_uiSoulFlayTimer -= uiDiff;

        // Legion Lightning : Lightning on random target, jumps to 3 additional players, burns 1500 mana and deals 3k damage
        if (m_uiLegionLightningTimer < uiDiff) 
        {
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0), SPELL_LEGION_LIGHTNING);
            m_uiLegionLightningTimer = (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_SACRIFICE) ? 18000 : 30000; // 18 seconds in PHASE_SACRIFICE
        }
        else
            m_uiLegionLightningTimer -= uiDiff;

        // Fire Bloom : Places a debuff on 5 random targets; targets and all nearby allies suffer ~1.7k fire damage every 2 seconds
        if (m_uiFireBloomTimer < uiDiff)
        {
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature, SPELL_FIRE_BLOOM);
            m_uiFireBloomTimer = (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_SACRIFICE) ? 25000 : 40000; // 25 seconds in PHASE_SACRIFICE
        }
        else
            m_uiFireBloomTimer -= uiDiff;

        // Summon Shield Orb : Spawns a Shield Orb add and constantly shooting Shadow Bolts at the raid
        if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) != PHASE_SACRIFICE)
        {
            if (m_uiSummonShieldOrbTimer < uiDiff)
            {
                // northeast of Kil'jaeden
                for(uint8 i = 1; i < m_pInstance->GetData(TYPE_KILJAEDEN_PHASE); ++i)
                {
                    if (Creature* pOrb = m_creature->SummonCreature(NPC_SHIELD_ORB, 1709.000f, 615.000f, 40.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000))
                        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                            pOrb->AI()->AttackStart(pTarget);
                }
                m_uiSummonShieldOrbTimer = (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_SACRIFICE) ? 30000 : 60000; // 30 seconds in PHASE_SACRIFICE
            }
            else
                m_uiSummonShieldOrbTimer -= uiDiff;
        }

        // *****************************************
        // *********** Phase 3 spells **************
        // *****************************************
        if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_NORMAL && (m_creature->GetHealthPercent() < 85.0f))
        {
            m_creature->InterruptNonMeleeSpells(true);

            // Sinister Reflection
            CastSinisterReflection();

            // Shadow Spike
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(pTarget, SPELL_SHADOW_SPIKE);

            m_uiSoulFlayTimer         = 30000; // Don't let other spells
            m_uiLegionLightningTimer += 30000; // interrupt Shadow Spikes
            m_uiFireBloomTimer       += 30000;
            m_uiSummonShieldOrbTimer += 30000;
            m_uiFlameDartTimer       += 30000;
            //m_uiDarknessTimer        += 30000;
            m_uiShadowSpikeCount      = 30000; // shadow spike visual timer

            // Active orb after 35 seconds
            if (Creature* pController = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN_CONTROLLER))
            {
                if(mob_kiljaeden_controllerAI* pControllerAI = dynamic_cast<mob_kiljaeden_controllerAI*>(pController->AI()))
                    pControllerAI->m_uiEmpowerTimer = 35000;
            }

            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_DARKNESS);
        }
        else if(m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) <= PHASE_NORMAL)
            return;

        // Shadow Spike Effect Count
        if (m_uiShadowSpikeCount)
        {
            if (m_uiShadowSpikeTimer < uiDiff)
            {
                // workaround
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    m_creature->CastSpell(pTarget, SPELL_SHADOW_SPIKE_VISUAL, true);
                    // visual effect to ground (because missile is glitched)
                    if(Creature *pSpikeAlert = m_creature->SummonCreature(NPC_SINISTER, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ()+1, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 4000))
                    {
                        pSpikeAlert->SetDisplayId(15882);
                        pSpikeAlert->SetName("Shadow Spike");
                        pSpikeAlert->setFaction(35);
                        pSpikeAlert->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    }
                }

                m_uiShadowSpikeTimer = 5000;
                m_uiShadowSpikeCount -= m_uiShadowSpikeTimer;
            }
            else
                m_uiShadowSpikeTimer -= uiDiff;
        }

        // Flame Dart Explosion
        if(m_uiFlameDartTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(pTarget, SPELL_FLAME_DART);

            m_uiFlameDartTimer = 3000;
        }
        else
            m_uiFlameDartTimer -= uiDiff;

        // Darkness of a Thousand Souls : Begins to channel for 8 seconds, then deals 50'000 damage to all raid members.
        if (m_uiDarknessTimer < uiDiff)
        {
            m_creature->InterruptNonMeleeSpells(true);
            //DoCast(m_creature, SPELL_DARKNESS_OF_A_THOUSAND_SOULS);
            m_creature->CastSpell(m_creature, SPELL_DARKNESS_OF_A_THOUSAND_SOULS, true);

            m_uiSummonShieldOrbTimer += 9000; // Don't let other spells
            m_uiFireBloomTimer       += 9000; // interrupt Darkness of a Thousand Souls
            m_uiLegionLightningTimer += 9000;
            m_uiFlameDartTimer       += 9000;
            
            if(m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) >= PHASE_ARMAGEDDON)
                m_uiArmageddonTimer  += 9000; // Armageddon on the other hand, can be casted now, if Anveena has already sacrificed
            
            m_uiSoulFlayTimer         = 9000;
            m_uiDarknessBombTimer     = 8000; // Darkness of a Thousand Souls effect timer

            switch(rand()%3)
            {
                case 0: DoScriptText(SAY_KJ_DARKNESS1, m_creature);break;
                case 1: DoScriptText(SAY_KJ_DARKNESS2, m_creature);break;
                case 2: DoScriptText(SAY_KJ_DARKNESS3, m_creature);break;
            }

            m_uiDarknessTimer = (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_SACRIFICE) ? 30000 + rand()%10000 : 40000 + rand()%30000;
        }
        else
            m_uiDarknessTimer -= uiDiff;

        // Darkness of a Thousand Souls Effect
        if (m_uiDarknessBombTimer)
        {
            if(m_uiDarknessBombTimer < uiDiff)
            {
                DoCast(m_creature, SPELL_DARKNESS_OF_A_THOUSAND_SOULS_EFFECT);
                m_uiDarknessBombTimer = 0;
            }
            else
                m_uiDarknessBombTimer -= uiDiff;
        }

        // *****************************************
        // *********** Phase 4 spells **************
        // *****************************************
        if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_DARKNESS && (m_creature->GetHealthPercent() < 55.0f))
        {
            m_creature->InterruptNonMeleeSpells(true);

            // Sinister Reflection
            CastSinisterReflection();

            // Shadow Spike
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(pTarget, SPELL_SHADOW_SPIKE);

            m_uiSoulFlayTimer         = 30000; // Don't let other spells
            m_uiLegionLightningTimer += 30000; // interrupt Shadow Spikes
            m_uiFireBloomTimer       += 30000;
            m_uiSummonShieldOrbTimer += 30000;
            m_uiFlameDartTimer       += 30000;
            m_uiDarknessTimer        += 30000;
            m_uiArmageddonTimer      += 30000;
            m_uiShadowSpikeCount      = 30000; // shadow spike visual timer

            // Active orb after 35 seconds
            if (Creature* pController = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN_CONTROLLER))
            {
                if(mob_kiljaeden_controllerAI* pControllerAI = dynamic_cast<mob_kiljaeden_controllerAI*>(pController->AI()))
                    pControllerAI->m_uiEmpowerTimer = 35000;
            }

            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_ARMAGEDDON);
        }
        else if(m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) <= PHASE_DARKNESS)
            return;

        // Armageddon
        if (m_uiArmageddonTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                float fX, fY, fZ;
                pTarget->GetPosition(fX, fY, fZ);
                if(Creature* pArmageddon = m_creature->SummonCreature(NPC_ARMAGEDDON, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 5000))
                {
                    pArmageddon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    DoCast(pArmageddon, SPELL_ARMAGEDDON, true);
                    pArmageddon->CastSpell(pArmageddon, SPELL_ARMAGEDDON_VISUAL, true);
                }
            }
            m_uiArmageddonTimer = 4000; // No, I'm not kidding
        }
        else
            m_uiArmageddonTimer -= uiDiff;


        // *****************************************
        // *********** Phase 5 spells **************
        // *****************************************
        if (m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) == PHASE_ARMAGEDDON && (m_creature->GetHealthPercent() < 25.0f))
        {
            m_creature->InterruptNonMeleeSpells(true);

            // Sinister Reflection
            CastSinisterReflection();

            // Shadow Spike
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(pTarget, SPELL_SHADOW_SPIKE);

            m_uiSoulFlayTimer         = 30000; // Don't let other spells
            m_uiLegionLightningTimer += 30000; // interrupt Shadow Spikes
            m_uiFireBloomTimer       += 30000;
            m_uiSummonShieldOrbTimer += 30000;
            m_uiFlameDartTimer       += 30000;
            m_uiDarknessTimer        += 30000;
            m_uiArmageddonTimer      += 30000;
            m_uiShadowSpikeCount      = 30000; // shadow spike visual timer

            // Active orb after 35 seconds
            if (Creature* pController = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN_CONTROLLER))
            {
                if(mob_kiljaeden_controllerAI* pControllerAI = dynamic_cast<mob_kiljaeden_controllerAI*>(pController->AI()))
                    pControllerAI->m_uiEmpowerTimer = 35000;
            }

            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_SACRIFICE);
        }
        else if(m_pInstance->GetData(TYPE_KILJAEDEN_PHASE) <= PHASE_ARMAGEDDON)
            return;

    }
};

CreatureAI* GetAI_boss_kiljaeden(Creature *pCreature)
{
    return new boss_kiljaedenAI(pCreature);
}

/*######
## mob_deciever
######*/

struct MANGOS_DLL_DECL mob_deceiverAI : public ScriptedAI
{
    mob_deceiverAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    bool m_bHasInfusion;
    uint32 m_uiShadowBoltTimer;
    uint32 m_uiPortalTimer;
    bool m_bHasPortal;
    ObjectGuid m_uiPortalGUID;

    void Reset()
    {
        DoCast(m_creature, SPELL_SHADOW_CHANNELING);
        m_bHasInfusion      = false;
        m_uiPortalTimer     = urand(20000, 30000);
        m_uiShadowBoltTimer = 10000;
        m_bHasPortal        = false;
        m_uiPortalGUID.Clear();
        m_uiDecieverDead    = 0;

        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_IDLE);
            m_pInstance->SetData(TYPE_KILJAEDEN, NOT_STARTED);
        }
    }

    void Aggro(Unit* pWho)
    {
        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_KILJAEDEN, IN_PROGRESS);
            m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_DECEIVERS);

            if (Creature* pAnveena = m_pInstance->GetSingleCreatureFromStorage(NPC_ANVEENA))
            {
                pAnveena->GetMap()->CreatureRelocation(pAnveena, 1698.45f, 628.03f, 70.1989f, 0.0f);
                pAnveena->GetMotionMaster()->MovePoint(1, 1698.45f, 628.03f, 70.1989f);
            }
        }

        m_creature->CastStop();
        std::list<Creature*> lDecievers;
        GetCreatureListWithEntryInGrid(lDecievers, m_creature, NPC_DECIVER, 40.0f);

        if (!lDecievers.empty())
        {
            for(std::list<Creature*>::iterator iter = lDecievers.begin(); iter != lDecievers.end(); ++iter)
            {
                if ((*iter) && (*iter)->isAlive())
                   (*iter)->AI()->AttackStart(pWho);
            }
        }
    }

    void JustDied(Unit* pKiller)
    {
        ++m_uiDecieverDead;

        if (Creature* pPortal = m_creature->GetMap()->GetCreature(m_uiPortalGUID))
            pPortal->ForcedDespawn();
    }

    void JustReachedHome()
    {
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_KILJAEDEN) != NOT_STARTED)
            {
                m_pInstance->SetData(TYPE_KILJAEDEN, NOT_STARTED);
                m_pInstance->SetData(TYPE_KILJAEDEN_PHASE, PHASE_IDLE);
            }
        }

        std::list<Creature*> lDecievers;
        GetCreatureListWithEntryInGrid(lDecievers, m_creature, NPC_DECIVER, 40.0f);
        if (!lDecievers.empty())
        {
            for(std::list<Creature*>::iterator iter = lDecievers.begin(); iter != lDecievers.end(); ++iter)
            {
                 if ((*iter) && !(*iter)->isAlive())
                    (*iter)->Respawn();
            }
        }
    }

    void JustSummoned(Creature* pSummon)
    {
        if (pSummon->GetEntry() == NPC_FELFIRE_PORTAL)
            m_uiPortalGUID = pSummon->GetObjectGuid();
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealthPercent() < 20.0f && !m_bHasInfusion)
        {
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature, SPELL_SHADOW_INFUSION);
            m_bHasInfusion = true;
        }

        if (m_uiShadowBoltTimer < uiDiff)
        {
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature, SPELL_SHADOW_BOLT_VOLLEY);
            m_uiShadowBoltTimer = urand(6000, 10000);
        }
        else m_uiShadowBoltTimer -= uiDiff;

        if (m_uiPortalTimer < uiDiff && !m_bHasPortal)
        {
            //DoCast(m_creature, SPELL_FELLFIRE_PORTAL);
            m_creature->SummonCreature(NPC_FELFIRE_PORTAL, 0, 0, 0, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            m_uiPortalTimer = 30000;
            m_bHasPortal = true;
        }
        else m_uiPortalTimer -= uiDiff;

       DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_deceiver(Creature *pCreature)
{
    return new mob_deceiverAI(pCreature);
}

/*######
## mob_shield_orb
######*/

struct MANGOS_DLL_DECL mob_shield_orbAI : public ScriptedAI
{
    mob_shield_orbAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        SetCombatMovement(false);
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMovementTimer;
    uint32 m_uiMovingSteps;

    void Reset()
    {
        DoCast(m_creature, SPELL_SHADOW_ORB_BOLT_TRIGG);
        m_creature->SetRespawnDelay(DAY);
        m_uiMovementTimer = 1000 + rand()%2000;
        m_uiMovingSteps = 0;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiMovementTimer < uiDiff)
        {
            if (m_pInstance)
            {
                if (Creature* pKiljaeden = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN))
                {
                    float angle = (M_PI / 10) * m_uiMovingSteps;
                    float X = pKiljaeden->GetPositionX() + 22 * cos(angle);
                    float Y = pKiljaeden->GetPositionY() + 22 * sin(angle);

                    m_creature->GetMotionMaster()->MovePoint(0, X, Y, 39.0f);

                    m_uiMovingSteps++;
                    if (m_uiMovingSteps > 19)
                        m_uiMovingSteps = 0;
                }
            }
            m_uiMovementTimer = 500;
        }
        else m_uiMovementTimer -= uiDiff;
    }
};

CreatureAI* GetAI_mob_shield_orb(Creature *pCreature)
{
    return new mob_shield_orbAI(pCreature);
}

/*######
## mob_felfire_portal
######*/
struct MANGOS_DLL_DECL mob_felfire_portalAI : public Scripted_NoMovementAI
{
    mob_felfire_portalAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        pCreature->setFaction(14);
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset()
    {
        DoCast(m_creature, SPELL_SUMMON_FELFIRE_FIEND);
        m_creature->SetRespawnDelay(DAY);
    }

    void JustSummoned(Creature* pSummon)
    {
        pSummon->SetInCombatWithZone();
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if(m_pInstance->GetData(TYPE_KILJAEDEN) != IN_PROGRESS)
            m_creature->ForcedDespawn();
    }
};

CreatureAI* GetAI_mob_felfire_portal(Creature *pCreature)
{
    return new mob_felfire_portalAI(pCreature);
}

/*######
## mob_felfire_fiend
######*/
struct MANGOS_DLL_DECL mob_felfire_fiendAI : public ScriptedAI
{
    mob_felfire_fiendAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    bool m_bMustDie;
    uint32 m_uiDieTimer;

    void Reset()
    {
        m_bMustDie = false;
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if(uiDamage > m_creature->GetHealth())
        {
            uiDamage = 0;
            if(!m_bMustDie)
            {
                DoCast(m_creature, SPELL_FELFIRE_FISSION);
                m_uiDieTimer = 500;
                m_bMustDie = true;
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if(m_uiDieTimer < uiDiff && m_bMustDie)
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        else m_uiDieTimer -= uiDiff;

        if (m_creature->IsWithinDistInMap(m_creature->getVictim(), ATTACK_DISTANCE) && !m_bMustDie)
        {
            DoCast(m_creature, SPELL_FELFIRE_FISSION);
            m_uiDieTimer = 500;
            m_bMustDie = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_felfire_fiend(Creature* pCreature)
{
    return new mob_felfire_fiendAI(pCreature);
}

/*######
## mob_sinister_reflection
######*/
struct MANGOS_DLL_DECL mob_sinister_reflectionAI : public ScriptedAI
{
    mob_sinister_reflectionAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
        m_uiPlayerClass = 0;
    }

    uint32 m_uiPlayerAbilityTimer;
    uint32 m_uiPlayerClass;

    void Reset()
    {
         m_uiPlayerAbilityTimer = urand(3500, 6000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_uiPlayerClass)
        {
            m_uiPlayerClass = m_creature->getVictim()->getClass();
            
            if (m_uiPlayerClass == CLASS_PRIEST && m_creature->getVictim()->HasSpell(15473))
                m_uiPlayerClass = 10;
        }

        if (m_uiPlayerAbilityTimer < diff)
        {
            UseAbility();
            m_uiPlayerAbilityTimer = urand(3500, 6000);
        }
        else
            m_uiPlayerAbilityTimer -= diff;

        DoMeleeAttackIfReady();
    }
    
    void UseAbility()
    {
        if (!m_uiPlayerClass)
            return;

        uint32 random = urand(0,1);
        uint32 uiSpell = 0;
        Unit* pTarget = NULL;

        switch (m_uiPlayerClass)
        {
            case CLASS_WARRIOR:
                uiSpell = 15576;
                pTarget = m_creature;
                break;
            case CLASS_PALADIN:
                random ? uiSpell = 38921 : uiSpell = 37369;
                pTarget = m_creature->getVictim();
                break;
            case CLASS_HUNTER:
                random ? uiSpell = 48098 : uiSpell = 16496;
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                break;
            case CLASS_ROGUE:
                uiSpell = 45897;
                pTarget = m_creature->getVictim();
                break;
            case CLASS_PRIEST:
                uiSpell = 47079;
                pTarget = DoSelectLowestHpFriendly(50, 0);
                if (pTarget->GetEntry() != m_creature->GetEntry()) // solo altri Sinister Reflection
                    pTarget = m_creature;
                break;
            case 10:
                uiSpell = 47077;
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                break;
            case CLASS_SHAMAN:
                uiSpell = 47077;
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                break;                    
            case CLASS_MAGE:
                uiSpell = 47074;
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                break;  
            case CLASS_WARLOCK:
                random ? uiSpell = 46190 : uiSpell = 47076;
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                break;
            case CLASS_DRUID:
                uiSpell = 47072;
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                break; 
        }

        if (pTarget)
            DoCastSpellIfCan(pTarget, uiSpell);
    }
};


CreatureAI* GetAI_mob_sinister_reflection(Creature *_Creature)
{
    return new mob_sinister_reflectionAI (_Creature);
}

//Dragon Gossip Menu
//This function is called when the player opens the gossip menubool

#define GOSSIP_ITEM_1 "cast on me Shield of the Blue Dragon Flight ! Quikly !"
#define GOSSIP_ITEM_2 "cast on me Dragon Breath: Revitalize !"
#define GOSSIP_ITEM_3 "cast on me Dragon Breath: Haste! (NON FUNZIONA)"
#define GOSSIP_ITEM_4 "cast on me Blink !"
#define GOSSIP_ITEM_5 "Fight with our Enemy !"

bool GossipHello_dragon(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
    pPlayer->PlayerTalkClass->SendGossipMenu(907, pCreature->GetGUID());
    return true;
}

bool GossipSelect_dragon(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    ScriptedInstance *m_pInstance = (ScriptedInstance*)pPlayer->GetInstanceData();

    if (!m_pInstance)
        return false;

    if (uiSender == GOSSIP_SENDER_MAIN)
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                if (Creature* pKJ = m_pInstance->GetSingleCreatureFromStorage(NPC_KILJAEDEN))
                {
                    if (boss_kiljaedenAI* pKJAI = dynamic_cast<boss_kiljaedenAI*>(pKJ->AI()))
                        pKJAI->m_uiCancelShieldTimer = 5500;
                }

                pCreature->CastSpell(pPlayer, SPELL_SHIELD_OF_BLUE, false);
                pPlayer->CastSpell(pPlayer, SPELL_SHIELD_OF_BLUE, false);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+2:
                pCreature->CastSpell(pPlayer, SPELL_REVITALIZE, true);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+3:
                pPlayer->CastSpell(pPlayer, SPELL_HASTE, true);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+4:
                pPlayer->TeleportTo(pPlayer->GetMapId(), pPlayer->GetPositionX()+10, pPlayer->GetPositionY()+10, pPlayer->GetPositionZ(), pPlayer->GetOrientation());
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+5:
                pPlayer->CLOSE_GOSSIP_MENU();
        }
    }

    return true;
}

bool GOUse_orb_of_the_blue_flight(Player* pPlayer, GameObject* pGo)
{
    ScriptedInstance *m_pInstance = (ScriptedInstance*)pPlayer->GetInstanceData();

    outstring_log("[KJ]: using orb of the blue dragon");
    if (m_pInstance && m_pInstance->GetData(TYPE_KILJAEDEN) == IN_PROGRESS)
    {
        if(!pPlayer->HasAura(AURA_NODRAGON))
        {
            outstring_log("[KJ]: start summoning blue dragon...");
            if(Creature* pDragon = pPlayer->SummonCreature(NPC_BLUE_DRAGON, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ()+1, 0, TEMPSUMMON_CORPSE_DESPAWN, 20000))
            {
                outstring_log("[KJ]: summoned blue dragon...");
                pDragon->SetSpeedRate(MOVE_FLIGHT, 1.0f);
                pDragon->SetSpeedRate(MOVE_WALK, 1.0f);
                pDragon->SetSpeedRate(MOVE_RUN, 1.0f);
                pDragon->GetMotionMaster()->MoveFollow(pPlayer, 0.0f, 0.0f);
                pDragon->DealDamage(pDragon, pDragon->GetHealth()/100, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
            pPlayer->CastSpell(pPlayer, SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT, true);
        }
        
        Map *map = pPlayer->GetMap();
        if (map->IsDungeon())
        {
            Map::PlayerList const &PlayerList = map->GetPlayers();

            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {   
                    if (i->getSource() && !i->getSource()->HasAura(AURA_NODRAGON))
                        i->getSource()->CastSpell(i->getSource(), AURA_NODRAGON, true);
                }
            }
        }
    }
    return true;
}

void AddSC_boss_kiljaeden()
{
    Script *pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "mob_kiljaeden_controller";
    pNewScript->GetAI = &GetAI_mob_kiljaeden_controller;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_kiljaeden";
    pNewScript->GetAI = &GetAI_boss_kiljaeden;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_kalecgos";
    pNewScript->GetAI = &GetAI_mob_kalecgos;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_hand_of_the_deceiver";
    pNewScript->GetAI = &GetAI_mob_deceiver;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_shield_orb";
    pNewScript->GetAI = &GetAI_mob_shield_orb;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_felfire_portal";
    pNewScript->GetAI = &GetAI_mob_felfire_portal;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_felfire_fiend";
    pNewScript->GetAI = &GetAI_mob_felfire_fiend;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_sinister_reflection";
    pNewScript->GetAI = &GetAI_mob_sinister_reflection;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "dragon";
    pNewScript->pGossipHello = &GossipHello_dragon;
    pNewScript->pGossipSelect = &GossipSelect_dragon;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_orb_of_the_blue_flight";
    pNewScript->pGOUse = &GOUse_orb_of_the_blue_flight;
    pNewScript->RegisterSelf();
}