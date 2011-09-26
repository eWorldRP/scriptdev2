#include "precompiled.h"

/*######
## nnpc_vanndar_stormpike
######*/
#define    SAY_AGGRO_A                   "Soldiers of Stormpike, your General is under attack! I require aid! Come! Come! Slay these mangy Frostwolf dogs."
#define    SAY_KILL_1_A                  "Take no prisoners! Drive these heathens from our lands!"
#define    SAY_KILL_2_A                  "We, the Alliance, will prevail!"
#define    SAY_KILL_3_A                  "The Stormpike clan bows to no one, especially the horde!"
#define    SAY_KILL_4_A                  "Is that the best you can do?"
#define    SAY_KILL_5_A                  "Your attacks are weak! Go practice on some rabbits and come back when you're stronger."
#define    SAY_EVADE_1_A                 "You'll never get me out of me bunker, heathens!"
#define    SAY_EVADE_2_A                 "Why don't ya try again without yer cheap tactics, pansies! Or are you too chicken?"

enum
{
    SPELL_AVATAR                = 19135,
    SPELL_THUNDERCLAP           = 15588,
    SPELL_STORMBOLT             = 20685
};

struct MANGOS_DLL_DECL npc_vanndar_stormpikeAI : public ScriptedAI
{
    npc_vanndar_stormpikeAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiEvadeCheckCooldown;
    uint32 m_uiAvatar_Timer;
    uint32 m_uiStormBolt_Timer;
    uint32 m_uiThunderClap_Timer;

    void Reset()
    {
        m_uiEvadeCheckCooldown = 2500;
        m_uiAvatar_Timer = 2000+rand()%17000;
        m_uiThunderClap_Timer = 4000+rand()%12000;
        m_uiStormBolt_Timer = 6000+rand()%20000;
    }

    void Aggro(Unit *pWho)
    {
        if (pWho)
            m_creature->MonsterYell(SAY_AGGRO_A, LANG_UNIVERSAL, pWho);
            
        m_creature->CallForHelp(20.0f);            
    }

    void JustReachedHome()
    {

    }

    void JustDied(Unit* pKiller)
    {
    }

    void KilledUnit(Unit* victim)
    {
        switch(rand()%5)
        {
            case 0: m_creature->MonsterYell(SAY_KILL_1_A, LANG_UNIVERSAL, 0); break;
            case 1: m_creature->MonsterYell(SAY_KILL_2_A, LANG_UNIVERSAL, 0); break;
            case 2: m_creature->MonsterYell(SAY_KILL_3_A, LANG_UNIVERSAL, 0); break;
            case 3: m_creature->MonsterYell(SAY_KILL_4_A, LANG_UNIVERSAL, 0); break;
            case 4: m_creature->MonsterYell(SAY_KILL_5_A, LANG_UNIVERSAL, 0); break;            
        }
    }
    
    void UpdateAI(const uint32 uiDiff)
    {        
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiAvatar_Timer < uiDiff)
        {
            DoCast(m_creature, SPELL_AVATAR);
            m_uiAvatar_Timer = 2000+rand()%17000;
        }
        else m_uiAvatar_Timer -= uiDiff;
        
        if (m_uiThunderClap_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_THUNDERCLAP);
            
            m_uiThunderClap_Timer = 4000+rand()%12000;
            m_creature->CallForHelp(40.0f);
        }
        else m_uiThunderClap_Timer -= uiDiff;
        
        if (m_uiStormBolt_Timer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))                
                DoCast(m_creature->getVictim(),SPELL_STORMBOLT);
            m_uiStormBolt_Timer = 6000+rand()%20000;
        }
        else m_uiStormBolt_Timer -= uiDiff;
        
        DoMeleeAttackIfReady();
        
        EnterEvadeIfOutOfCombatArea(uiDiff);
    }
    
    bool EnterEvadeIfOutOfCombatArea(const uint32 uiDiff)
    {
        if (m_uiEvadeCheckCooldown < uiDiff)
            m_uiEvadeCheckCooldown = 2500;
        else
        {
            m_uiEvadeCheckCooldown -= uiDiff;
            return false;
        }

        if (m_creature->IsInEvadeMode() || !m_creature->getVictim())
            return false;

        float fX,fY,fZ;
        m_creature->GetRespawnCoord(fX,fY,fZ);

        if (m_creature->GetDistance2d(fX, fY) < 40.0f)
            return false;
            
        if (m_creature->getVictim())
        {
            switch(rand()%2)
            {
                case 0: m_creature->MonsterYell(SAY_EVADE_1_A,LANG_UNIVERSAL,m_creature->getVictim());
                    break;
                case 1: m_creature->MonsterYell(SAY_EVADE_2_A,LANG_UNIVERSAL,m_creature->getVictim());
                    break;
            }
            m_creature->DealDamage(m_creature->getVictim(), m_creature->getVictim()->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }                    

        EnterEvadeMode();
        return true;
    }        
};

CreatureAI* GetAI_npc_vanndar_stormpike(Creature* pCreature)
{
    return new npc_vanndar_stormpikeAI(pCreature);
}


/*######
## npc_drekthar
######*/
#define    SAY_AGGRO_H                   "Stormpike filth! In my keep?! Slay them all!"
#define    SAY_KILL_1_H                  "Your attacks are slowed by the cold, I think!"
#define    SAY_KILL_2_H                  "Today, you will meet your ancestors!"
#define    SAY_KILL_3_H                  "If you will not leave Alterac Valley on your own, then the Frostwolves will force you out!"
#define    SAY_KILL_4_H                  "You cannot defeat the Frostwolf clan!"
#define    SAY_KILL_5_H                  "Your attacks are weak! Go practice on some rabbits and come back when you're stronger."
#define    SAY_EVADE_1_H                 "You seek to draw the General of the Frostwolf legion out from his fortress? PREPOSTEROUS!"
#define    SAY_EVADE_2_H                 "You are no match for the strength of the Horde!"

enum
{
    SPELL_FRENZY                = 8269,
    SPELL_KNOCKDOWN             = 19128,
    SPELL_WHIRLWIND             = 13736   
};

struct MANGOS_DLL_DECL npc_drektharAI : public ScriptedAI
{
    npc_drektharAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiEvadeCheckCooldown;
    uint32 m_uiFrenzy_Timer;
    uint32 m_uiKnockback_Timer;
    uint32 m_uiWhirlwind_Timer;

    void Reset()
    {
        m_uiEvadeCheckCooldown = 2500;
        m_uiFrenzy_Timer = urand(2000, 19000);
        m_uiKnockback_Timer = urand(4000, 15000);
        m_uiWhirlwind_Timer = urand(6000, 20000);
    }

    void Aggro(Unit *pWho)
    {
        if (pWho)
            m_creature->MonsterYell(SAY_AGGRO_H, LANG_UNIVERSAL, pWho);
            
        m_creature->CallForHelp(20.0f);
    }

    void KilledUnit(Unit* victim)
    {
        switch(urand(0,5))
        {
            case 0: m_creature->MonsterYell(SAY_KILL_1_H, LANG_UNIVERSAL, 0); break;
            case 1: m_creature->MonsterYell(SAY_KILL_2_H, LANG_UNIVERSAL, 0); break;
            case 2: m_creature->MonsterYell(SAY_KILL_3_H, LANG_UNIVERSAL, 0); break;
            case 3: m_creature->MonsterYell(SAY_KILL_4_H, LANG_UNIVERSAL, 0); break;
            case 4: m_creature->MonsterYell(SAY_KILL_5_H, LANG_UNIVERSAL, 0); break;            
        }
    }
    
    void UpdateAI(const uint32 uiDiff)
    {        
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiFrenzy_Timer < uiDiff)
        {
            DoCast(m_creature, SPELL_FRENZY);
            m_uiFrenzy_Timer = urand(2000, 18000);
        }
        else
            m_uiFrenzy_Timer -= uiDiff;
        
        if (m_uiKnockback_Timer < uiDiff)
        {
            if (m_creature->getVictim())
                DoCast(m_creature->getVictim(), SPELL_KNOCKDOWN);
            
            m_uiKnockback_Timer = urand(4000, 15000);
            m_creature->CallForHelp(40.0f);
        }
        else
            m_uiKnockback_Timer -= uiDiff;
        
        if (m_uiWhirlwind_Timer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))                
                DoCast(m_creature->getVictim(), SPELL_WHIRLWIND);
                
            m_uiWhirlwind_Timer = urand(5000, 25000);
        }
        else
            m_uiWhirlwind_Timer -= uiDiff;
        
        DoMeleeAttackIfReady();
        
        EnterEvadeIfOutOfCombatArea(uiDiff);
    }
    
    bool EnterEvadeIfOutOfCombatArea(const uint32 uiDiff)
    {
        if (m_uiEvadeCheckCooldown < uiDiff)
            m_uiEvadeCheckCooldown = 2500;
        else
        {
            m_uiEvadeCheckCooldown -= uiDiff;
            return false;
        }

        if (m_creature->IsInEvadeMode() || !m_creature->getVictim())
            return false;

        float fX,fY,fZ;
        m_creature->GetRespawnCoord(fX,fY,fZ);

        if (m_creature->GetDistance2d(fX, fY) < 40.0f)
            return false;

        if (m_creature->getVictim())
        {
            switch(urand(0,1))
            {
                case 0: m_creature->MonsterYell(SAY_EVADE_1_H,LANG_UNIVERSAL,m_creature->getVictim());
                    break;
                case 1: m_creature->MonsterYell(SAY_EVADE_2_H,LANG_UNIVERSAL,m_creature->getVictim());
                    break;
            }
            
            m_creature->DealDamage(m_creature->getVictim(), m_creature->getVictim()->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }                    

        EnterEvadeMode();
        return true;
    }        
};

CreatureAI* GetAI_npc_drekthar(Creature* pCreature)
{
    return new npc_drektharAI(pCreature);
}

/*######
## nnpc_marshall
######*/
enum
{
    SPELL_CHARGE                = 22911,
    SPELL_CLEAVE                = 40504,
    SPELL_DEMORALIZINGSHOUT     = 23511
};

struct MANGOS_DLL_DECL npc_marshallAI : public ScriptedAI
{
    npc_marshallAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiMyAura = GetAuraByEntry();
        Reset();
    }

    uint32 m_uiEvadeCheckCooldown;
    uint32 m_uiCharge_Timer;
    uint32 m_uiCleave_Timer;
    uint32 m_uiWhirlwind_Timer;
    uint32 m_uiDemoralizingShout_Timer;
    uint32 m_uiMyAura;
    uint32 m_uiMyAuraTimer;

    void Reset()
    {
        m_uiEvadeCheckCooldown = 2500;
        m_uiCharge_Timer = 1000;
        m_uiCleave_Timer = urand(3000, 15000);
        m_uiWhirlwind_Timer = urand(5000, 20000);
        m_uiDemoralizingShout_Timer = urand(15000, 20000);
        m_uiMyAuraTimer = 2000;
    }

    void Aggro(Unit* pWho)
    {
        m_creature->CallForHelp(20.0f);            
    }

    void JustDied(Unit* pKiller)
    {
        if (m_uiMyAura)
            m_creature->RemoveAurasDueToSpell(m_uiMyAura);  
    }
    
    void UpdateAI(const uint32 uiDiff)
    {    
        if (m_uiMyAuraTimer < uiDiff)
        {
            if (m_uiMyAura && !m_creature->HasAura(m_uiMyAura))
                m_creature->CastSpell(m_creature, m_uiMyAura, true);         

            m_uiMyAuraTimer = 2000;
        }
        else
            m_uiMyAuraTimer -= uiDiff;
    
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiCleave_Timer < uiDiff)
        {
            if (m_creature->getVictim())
                DoCast(m_creature->getVictim(), SPELL_CLEAVE);            
            
            m_uiCleave_Timer = urand(3000, 15000);
        }
        else
            m_uiCleave_Timer -= uiDiff;
            
        if (m_uiCharge_Timer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))                
                DoCast(target, SPELL_CHARGE);
                
            m_uiCharge_Timer = urand(1000, 15000);
        }
        else
            m_uiCharge_Timer -= uiDiff;        
        
        if (m_uiWhirlwind_Timer < uiDiff)
        {
            DoCast(m_creature,SPELL_WHIRLWIND, true);                
            m_uiWhirlwind_Timer = urand(5000, 20000);
        }
        else
            m_uiWhirlwind_Timer -= uiDiff;
                
        if (m_uiDemoralizingShout_Timer < uiDiff)
        {
            if (m_creature->getVictim())
                DoCast(m_creature->getVictim(), SPELL_DEMORALIZINGSHOUT);            
            
            m_uiDemoralizingShout_Timer = urand(15000, 20000);
            m_creature->CallForHelp(40.0f);
        }
        else
            m_uiDemoralizingShout_Timer -= uiDiff;
        
        DoMeleeAttackIfReady();
        
        EnterEvadeIfOutOfCombatArea(uiDiff);
    }
    
    bool EnterEvadeIfOutOfCombatArea(const uint32 uiDiff)
    {
        if (m_uiEvadeCheckCooldown < uiDiff)
            m_uiEvadeCheckCooldown = 2500;
        else
        {
            m_uiEvadeCheckCooldown -= uiDiff;
            return false;
        }

        if (m_creature->IsInEvadeMode() || !m_creature->getVictim())
            return false;

        float fX,fY,fZ;
        m_creature->GetRespawnCoord(fX,fY,fZ);

        if (m_creature->GetDistance2d(fX, fY) < 40.0f)
            return false;

        if (m_creature->getVictim())
            m_creature->DealDamage(m_creature->getVictim(), m_creature->getVictim()->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
/*
        if (Creature *pBoss = GetClosestCreatureWithEntry(m_creature, m_creature->getFaction() == 730 ? 11948 : 11946, 40.0f))
        {
            if (pBoss->isAlive() && pBoss->isInCombat() && pBoss->getVictim())
                pBoss->AI()->EnterEvadeMode();
        }
*/
        EnterEvadeMode();
        return true;
    }
    
    uint32 GetAuraByEntry()
    {
        switch (m_creature->GetEntry())
        {
            case 14762: // Dun Baldar North Marshal
                return 45828;
            case 14763: // Dun Baldar South Marshal
                return 45829;
            case 14772: // East Frostwolf Warmaster
                return 45826;
            case 14773: // Iceblood Warmaster
                return 45822;
            case 14764: // Icewing Marshal
                return 45831;
            case 14765: // Stonehearth Marshal
                return 45830;
            case 14776: // Tower Point Warmaster
                return 45823;
            case 14777: // West Frostwolf Warmaster
                return 45824;
            default:
                error_log("SD2: Alterac Valley entry %u non prevista per questo script", m_creature->GetEntry());
                return 0;                
        }  
        
    }
            
};

CreatureAI* GetAI_npc_marshall(Creature* pCreature)
{
    return new npc_marshallAI(pCreature);
}

void AddSC_alterac_valley()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_vanndar_stormpike";
    newscript->GetAI = &GetAI_npc_vanndar_stormpike;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_drekthar";
    newscript->GetAI = &GetAI_npc_drekthar;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_marshall";
    newscript->GetAI = &GetAI_npc_marshall;
    newscript->RegisterSelf();    
}
