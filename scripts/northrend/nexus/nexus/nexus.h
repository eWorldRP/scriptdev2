/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_NEXUS_H
#define DEF_NEXUS_H

enum
{
    MAX_ENCOUNTER                  = 4,

    TYPE_TELESTRA                  = 0,
    TYPE_ANOMALUS                  = 1,
    TYPE_ORMOROK                   = 2,
    TYPE_KERISTRASZA               = 3,

    NPC_TELESTRA                   = 26731,
    NPC_ANOMALUS                   = 26763,
    NPC_ORMOROK                    = 26794,
    NPC_KERISTRASZA                = 26723,

    NPC_BREATH_CASTER              = 27048,

    MOB_ALLY_1                     = 26800,
    MOB_ALLY_2                     = 26805,
    MOB_ALLY_3                     = 26802,
    BOSS_STOUTBEARD_ALLY           = 26796,

    MOB_HORDE_1                    = 26799,
    MOB_HORDE_2                    = 26803,
    MOB_HORDE_3                    = 26801,
    BOSS_KOLURG_HORDE              = 26798,

    COMMANDER_ALLY                 = 27949,
    COMMANDER_HORDE                = 27947,

    GO_CONTAINMENT_SPHERE_TELESTRA = 188526,
    GO_CONTAINMENT_SPHERE_ANOMALUS = 188527,
    GO_CONTAINMENT_SPHERE_ORMOROK  = 188528,

    SPELL_FROZEN_PRISON            = 47854                 // may not be correct spell
};
#endif
