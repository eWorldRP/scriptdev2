/* Copyright (C) 2009 - 2010 by /dev/rsa for ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_CRUSADER_H
#define DEF_CRUSADER_H
#include "BSW_ai.h"
#include "BSW_instance.h"

enum
{
    TYPE_STAGE                  = 0,
    TYPE_BEASTS                 = 1,
    TYPE_JARAXXUS               = 2,
    TYPE_CRUSADERS              = 3,
    TYPE_VALKIRIES              = 4,
    TYPE_LICH_KING              = 5,
    TYPE_ANUBARAK               = 6,
    TYPE_COUNTER                = 7,
    TYPE_EVENT                  = 8,
    MAX_ENCOUNTERS              = 9,

    TYPE_GORMOK                 = 11,
    TYPE_SNAKES                 = 12,
    TYPE_ICEHOWL                = 13,

    NPC_BARRENT                 = 34816,
    NPC_TIRION                  = 34996,
    NPC_FIZZLEBANG              = 35458,
    NPC_GARROSH                 = 34995,
    NPC_RINN                    = 34990,
    NPC_LICH_KING_0             = 16980,
    NPC_LICH_KING_1             = 35877,

    NPC_THRALL                  = 34994,
    NPC_PROUDMOORE              = 34992,
    NPC_TRIGGER                 = 22517,
    NPC_WILFRED_PORTAL          = 35651,

    NPC_ICEHOWL                 = 34797,
    NPC_GORMOK                  = 34796,
    NPC_DREADSCALE              = 34799,
    NPC_ACIDMAW                 = 35144,

    NPC_JARAXXUS                = 34780,

    NPC_CRUSADER_1_1            = 34460, //Druid
    NPC_CRUSADER_1_2            = 34463, //Shaman
    NPC_CRUSADER_1_3            = 34461, //DK
    NPC_CRUSADER_1_4            = 34472, //Rogue
    NPC_CRUSADER_1_5            = 34475, //Warrior
    NPC_CRUSADER_1_6            = 34471, //Retro pal
    NPC_CRUSADER_1_7            = 34473, //Shadow priest
    NPC_CRUSADER_1_8            = 34468, //Mage
    NPC_CRUSADER_1_9            = 34467, //Hunter
    NPC_CRUSADER_1_10           = 34474, //Warlock
    NPC_CRUSADER_1_11           = 34470, //Enh shaman
    NPC_CRUSADER_1_12           = 34466, //Priest
    NPC_CRUSADER_1_13           = 34465, //Holy paladin
    NPC_CRUSADER_1_14           = 34469, //Moonkin

    NPC_CRUSADER_2_1            = 34451, //Druid
    NPC_CRUSADER_2_2            = 34455, //Shaman
    NPC_CRUSADER_2_3            = 34458, //DK
    NPC_CRUSADER_2_4            = 34454, //Rogue
    NPC_CRUSADER_2_5            = 34453, //Warrior
    NPC_CRUSADER_2_6            = 34456, //Retro pal
    NPC_CRUSADER_2_7            = 34441, //Shadow Priest
    NPC_CRUSADER_2_8            = 34449, //Mage
    NPC_CRUSADER_2_9            = 34448, //Hunter
    NPC_CRUSADER_2_10           = 34450, //Warlock
    NPC_CRUSADER_2_11           = 34444, //Enh shaman
    NPC_CRUSADER_2_12           = 34447, //Priest
    NPC_CRUSADER_2_13           = 34445, //Holy paladin
    NPC_CRUSADER_2_14           = 34459, //Moonkin

    NPC_CRUSADER_0_1            = 35465, //Warlock's pet: Zhaagrym
    NPC_CRUSADER_0_2            = 35610, //Hunter's pet: Cat

    NPC_LIGHTBANE               = 34497,
    NPC_DARKBANE                = 34496,

    NPC_ANUBARAK                = 34564,

    GO_CRUSADERS_CACHE_10       = 195631,
    GO_CRUSADERS_CACHE_25       = 195632,
    GO_CRUSADERS_CACHE_10_H     = 195633,
    GO_CRUSADERS_CACHE_25_H     = 195635,

    GO_TRIBUTE_CHEST_10H_25     = 195665,
    GO_TRIBUTE_CHEST_10H_45     = 195666,
    GO_TRIBUTE_CHEST_10H_50     = 195667,
    GO_TRIBUTE_CHEST_10H_99     = 195668,

    GO_TRIBUTE_CHEST_25H_25     = 195669,
    GO_TRIBUTE_CHEST_25H_45     = 195670,
    GO_TRIBUTE_CHEST_25H_50     = 195671,
    GO_TRIBUTE_CHEST_25H_99     = 195672,

    GO_ARGENT_COLISEUM_FLOOR    = 195527, //20943
    GO_MAIN_GATE_DOOR           = 195647,

    GO_EAST_PORTCULLIS          = 195648,
    GO_SOUTH_PORTCULLIS         = 195649,
    GO_NORTH_PORTCULLIS         = 195650,

    TYPE_DIFFICULTY             = 101,
    TYPE_EVENT_TIMER            = 102,
    TYPE_EVENT_NPC              = 103,
    TYPE_CRUSADERS_COUNT        = 105,

    DATA_HEALTH_EYDIS           = 201,
    DATA_HEALTH_FJOLA           = 202,
    DATA_CASTING_VALKYRS        = 203,

    DESPAWN_TIME                = 300000,

    RAGGIO_ARENA                = 50,
    // Val'kyr Essences
    SPELL_LIGHT_ESSENCE     = 65686,
    SPELL_LIGHT_ESSENCE_N25 = 67222,
    SPELL_LIGHT_ESSENCE_H10 = 67223,
    SPELL_LIGHT_ESSENCE_H25 = 67224,
    SPELL_DARK_ESSENCE      = 65684,
    SPELL_DARK_ESSENCE_N25  = 67176,
    SPELL_DARK_ESSENCE_H10  = 67177,
    SPELL_DARK_ESSENCE_H25  = 67178,
    // Power Up
    SPELL_POWERUP_N10       = 67590,
    SPELL_POWERUP_N25       = 67602,
    SPELL_POWERUP_H10       = 67603,
    SPELL_POWERUP_H25       = 67604,
};

static Locations SpawnLoc[]=
{
    {559.257996f, 90.266197f, 395.122986f},  // 0 Barrent
    {563.672974f, 139.571f, 393.837006f},    // 1 Center
    {563.833008f, 184.244995f, 394.5f},      // 2 Backdoor
    {577.347839f, 184.338888f, 395.14f},     // 3 - Right
    {550.955933f, 184.338888f, 395.14f},     // 4 - Left
    {575.042358f, 184.260727f, 395.137146f}, // 5
    {552.248901f, 184.331955f, 395.132658f}, // 6
    {573.342285f, 184.515823f, 395.135956f}, // 7
    {554.239929f, 184.825577f, 395.137909f}, // 8
    {571.042358f, 184.260727f, 395.137146f}, // 9
    {556.720581f, 184.015472f, 395.132658f}, // 10
    {569.534119f, 184.214478f, 395.139526f}, // 11
    {569.231201f, 184.941071f, 395.139526f}, // 12
    {558.811610f, 184.985779f, 394.671661f}, // 13
    {567.641724f, 184.351501f, 394.659943f}, // 14
    {560.633972f, 184.391708f, 395.137543f}, // 15
    {565.816956f, 184.477921f, 395.136810f}, // 16
    {563.549f, 152.474f, 394.393f},          // 17 - Lich king start
    {563.547f, 141.613f, 393.908f},          // 18 - Lich king end
    {787.932556f, 133.28978f, 142.612152f},  // 19 - Anub'arak start location
    {618.157898f, 132.640869f, 139.559769f}, // 20 - Anub'arak move point location
    {508.104767f, 138.247345f, 395.128052f}, // 21 - Fizzlebang start location
    {586.060242f, 117.514809f, 394.314026f}, // 22 - Dark essence 1
    {541.602112f, 161.879837f, 394.587952f}, // 23 - Dark essence 2
    {541.021118f, 117.262932f, 395.314819f}, // 24 - Light essence 1
    {586.200562f, 162.145523f, 394.626129f}, // 25 - Light essence 2
    {563.833008f, 184.244995f, 394.585561f}, // 26 - outdoor
    {548.610596f, 139.807800f, 394.321838f}, // 27 - fizzlebang end
};

enum uiWorldStates
{
    UPDATE_STATE_UI_SHOW            = 4390,
    UPDATE_STATE_UI_COUNT           = 4389,
};

enum Snakes
{
    DREADSCALE_SUBMERGED     = 5,
    ACIDMAW_SUBMERGED        = 6,
};

enum AnnounserMessages
{
    MSG_BEASTS                 = 724001,
    MSG_JARAXXUS               = 724002,
    MSG_CRUSADERS              = 724003,
    MSG_VALKIRIES              = 724004,
    MSG_LICH_KING              = 724005,
    MSG_ANUBARAK               = 724006,
};

enum FightPhases
{
    FIGHT1_INTRO_1          = 110,
    FIGHT1_INTRO_2          = 120,
    FIGHT1_INTRO_3          = 122,
    FIGHT1_INTRO_4          = 130,
    FIGHT1_INTRO_5          = 132,
    FIGHT1_INTRO_6          = 140,
    FIGHT1_SUMMON           = 150,
    FIGHT1_END              = 160,
    FIGHT2_INTRO_1          = 200,
    FIGHT2_INTRO_2          = 205,
    FIGHT2_SUMMON           = 210,
    FIGHT2_END              = 220,
    FIGHT3_INTRO_1          = 300,
    FIGHT3_INTRO_2          = 305,
    FIGHT3_SUMMON           = 310,
    FIGHT3_END              = 320,
};

#endif
