#include "global.h"
#include "ev_trainer.h"

#include "daycare.h"                 // ChooseMonForDaycare, GetBoxMonNickname
#include "event_data.h"              // gSpecialVar_0x8004/0x8005
#include "main.h"
#include "party_menu.h"  			// ChooseMonForDaycare
#include "overworld.h"               // CB2_ReturnToField
#include "pokemon.h"
#include "pokemon_storage_system.h"  // CompactPartySlots
#include "string_util.h"
#include "strings.h"

#include "config/ev_trainer.h"


#define EV_STAT_CAP 252
#define EV_TOTAL_CAP 510

static bool8 EvTrainer_HasPokemon(struct EvTrainerFacility *fac)
{
    return GetBoxMonData(&fac->mon, MON_DATA_SPECIES) != SPECIES_NONE;
}

static void ClearEvTrainerFacility(struct EvTrainerFacility *fac)
{
    ZeroBoxMonData(&fac->mon);
    fac->evStepCounter = 0;
    fac->evPoints = 0;
    fac->stat = EV_TRAIN_STAT_NONE;
}

static u8 EvTrainer_GetStatEvDataId(u8 stat)
{
    static const u8 sEvIds[EV_TRAIN_STAT_COUNT] =
    {
        [EV_TRAIN_STAT_HP]     = MON_DATA_HP_EV,
        [EV_TRAIN_STAT_ATK]    = MON_DATA_ATK_EV,
        [EV_TRAIN_STAT_DEF]    = MON_DATA_DEF_EV,
        [EV_TRAIN_STAT_SPATK]  = MON_DATA_SPATK_EV,
        [EV_TRAIN_STAT_SPDEF]  = MON_DATA_SPDEF_EV,
        [EV_TRAIN_STAT_SPEED]  = MON_DATA_SPEED_EV,
    };

    if (stat >= EV_TRAIN_STAT_COUNT)
        return MON_DATA_HP_EV;

    return sEvIds[stat];
}

static const u8 *EvTrainer_GetStatName(u8 stat)
{
    static const u8 *const sNames[EV_TRAIN_STAT_COUNT] =
    {
        [EV_TRAIN_STAT_HP]     = gText_HP3,
        [EV_TRAIN_STAT_ATK]    = gText_Attack3,
        [EV_TRAIN_STAT_DEF]    = gText_Defense3,
        [EV_TRAIN_STAT_SPATK]  = gText_SpAtk3,
        [EV_TRAIN_STAT_SPDEF]  = gText_SpDef3,
        [EV_TRAIN_STAT_SPEED]  = gText_Speed2,
    };

    if (stat >= EV_TRAIN_STAT_COUNT)
        return gText_EmptyString2;

    return sNames[stat];
}

static u16 GetBoxMonTotalEVs(struct BoxPokemon *boxMon)
{
    u16 total = 0;
    total += (u8)GetBoxMonData(boxMon, MON_DATA_HP_EV);
    total += (u8)GetBoxMonData(boxMon, MON_DATA_ATK_EV);
    total += (u8)GetBoxMonData(boxMon, MON_DATA_DEF_EV);
    total += (u8)GetBoxMonData(boxMon, MON_DATA_SPATK_EV);
    total += (u8)GetBoxMonData(boxMon, MON_DATA_SPDEF_EV);
    total += (u8)GetBoxMonData(boxMon, MON_DATA_SPEED_EV);
    return total;
}

// -------------------------------------------------
// Step: só EV (respeitando 252 e 510)
// -------------------------------------------------
void EvTrainer_OnStep(void)
{
    struct EvTrainerFacility *fac = &gSaveBlock1Ptr->evTrainer;

    if (!EvTrainer_HasPokemon(fac))
        return;

    if (fac->stat >= EV_TRAIN_STAT_COUNT)
        return;

    {
        u8 evId = EvTrainer_GetStatEvDataId(fac->stat);
        u8 statEv = (u8)GetBoxMonData(&fac->mon, evId);
        u16 totalEv = GetBoxMonTotalEVs(&fac->mon);

        // já no teto (considerando pontos pendentes)
        if ((u16)statEv + fac->evPoints >= EV_STAT_CAP)
            return;

        if ((u16)totalEv + fac->evPoints >= EV_TOTAL_CAP)
            return;
    }

    fac->evStepCounter++;
    if (fac->evStepCounter >= EV_TRAINER_STEPS_PER_EV)
    {
        fac->evStepCounter -= EV_TRAINER_STEPS_PER_EV;

        // incrementa 1 EV pendente se ainda houver espaço
        {
            u8 evId = EvTrainer_GetStatEvDataId(fac->stat);
            u8 statEv = (u8)GetBoxMonData(&fac->mon, evId);
            u16 totalEv = GetBoxMonTotalEVs(&fac->mon);

            if ((u16)statEv + fac->evPoints < EV_STAT_CAP
             && (u16)totalEv + fac->evPoints < EV_TOTAL_CAP)
            {
                fac->evPoints++;
            }
        }
    }
}

// -------------------------------------------------
// Specials
// -------------------------------------------------
u8 GetEvTrainerState(void)
{
    return EvTrainer_HasPokemon(&gSaveBlock1Ptr->evTrainer) ? EV_TRAINER_HAS_MON : EV_TRAINER_EMPTY;
}

void ChooseSendEvTrainerMon(void)
{
    ChooseMonForDaycare();
    gMain.savedCallback = CB2_ReturnToField;
}

void StoreSelectedPokemonInEvTrainer(void)
{
    struct EvTrainerFacility *fac = &gSaveBlock1Ptr->evTrainer;
    u8 monIdx = (u8)gSpecialVar_0x8004;
    u8 stat = (u8)gSpecialVar_0x8005;

    if (EvTrainer_HasPokemon(fac))
        return;

    if (monIdx >= PARTY_SIZE)
        return;

    if (GetMonData(&gPlayerParty[monIdx], MON_DATA_IS_EGG))
        return;

    fac->mon = gPlayerParty[monIdx].box;
    fac->evStepCounter = 0;
    fac->evPoints = 0;
    fac->stat = (stat < EV_TRAIN_STAT_COUNT) ? stat : EV_TRAIN_STAT_NONE;

    ZeroMonData(&gPlayerParty[monIdx]);
    CompactPartySlots();
    CalculatePlayerPartyCount();
}

void EvTrainer_SetTrainingStat(void)
{
    struct EvTrainerFacility *fac = &gSaveBlock1Ptr->evTrainer;
    u8 stat = (u8)gSpecialVar_0x8005;

    if (!EvTrainer_HasPokemon(fac))
        return;

    if (stat >= EV_TRAIN_STAT_COUNT)
        stat = EV_TRAIN_STAT_NONE;

    fac->stat = stat;
    fac->evStepCounter = 0;
}

void BufferEvTrainerSummary(void)
{
    struct EvTrainerFacility *fac = &gSaveBlock1Ptr->evTrainer;

    if (!EvTrainer_HasPokemon(fac))
    {
        StringCopy(gStringVar1, gText_EmptyString2);
        StringCopy(gStringVar2, gText_EmptyString2);
        StringCopy(gStringVar3, gText_EmptyString2);
        StringCopy(gStringVar4, gText_EmptyString2);
        return;
    }

    GetBoxMonNickname(&fac->mon, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, fac->evPoints, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(gStringVar3, EvTrainer_GetStatName(fac->stat));
}

void GetEvTrainerCost(void)
{
    struct EvTrainerFacility *fac = &gSaveBlock1Ptr->evTrainer;
    u32 cost;

    if (!EvTrainer_HasPokemon(fac))
    {
        gSpecialVar_0x8005 = 0;
        return;
    }

    cost = (u32)EV_TRAINER_BASE_COST + (u32)fac->evPoints * (u32)EV_TRAINER_COST_PER_EV;
    if (cost > 99999)
        cost = 99999;

    gSpecialVar_0x8005 = (u16)cost;
    GetBoxMonNickname(&fac->mon, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 5);
}

u16 TakePokemonFromEvTrainer(void)
{
    struct EvTrainerFacility *fac = &gSaveBlock1Ptr->evTrainer;
    struct Pokemon pokemon;
    u32 species;

    if (!EvTrainer_HasPokemon(fac))
        return SPECIES_NONE;

    species = GetBoxMonData(&fac->mon, MON_DATA_SPECIES);
    BoxMonToMon(&fac->mon, &pokemon);

    if (fac->stat < EV_TRAIN_STAT_COUNT && fac->evPoints != 0)
    {
        u8 evId = EvTrainer_GetStatEvDataId(fac->stat);

        u8 statEv = (u8)GetMonData(&pokemon, evId);
        u16 totalEv = 0;
        totalEv += (u8)GetMonData(&pokemon, MON_DATA_HP_EV);
        totalEv += (u8)GetMonData(&pokemon, MON_DATA_ATK_EV);
        totalEv += (u8)GetMonData(&pokemon, MON_DATA_DEF_EV);
        totalEv += (u8)GetMonData(&pokemon, MON_DATA_SPATK_EV);
        totalEv += (u8)GetMonData(&pokemon, MON_DATA_SPDEF_EV);
        totalEv += (u8)GetMonData(&pokemon, MON_DATA_SPEED_EV);

        // quanto podemos adicionar sem estourar 252/510
        u16 add = fac->evPoints;
        u16 statRoom = (statEv >= EV_STAT_CAP) ? 0 : (EV_STAT_CAP - statEv);
        u16 totalRoom = (totalEv >= EV_TOTAL_CAP) ? 0 : (EV_TOTAL_CAP - totalEv);

        if (add > statRoom) add = statRoom;
        if (add > totalRoom) add = totalRoom;

        statEv = (u8)(statEv + add);
        SetMonData(&pokemon, evId, &statEv);
    }

    CalculateMonStats(&pokemon);

    gPlayerParty[PARTY_SIZE - 1] = pokemon;
    CompactPartySlots();
    CalculatePlayerPartyCount();

    ClearEvTrainerFacility(fac);
    return (u16)species;
}

// ---------- RESET (imediato na party) ----------
void GetEvTrainerResetCost(void)
{
    u32 cost = EV_TRAINER_RESET_COST;
    u8 monIdx = (u8)gSpecialVar_0x8004;

    if (monIdx >= PARTY_SIZE)
        cost = 0;

    gSpecialVar_0x8005 = (u16)cost;

    if (monIdx < PARTY_SIZE)
        GetMonData(&gPlayerParty[monIdx], MON_DATA_NICKNAME, gStringVar1);
    else
        StringCopy(gStringVar1, gText_EmptyString2);

    ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 5);
}

void EvTrainer_ResetSelectedMonEVs(void)
{
    u8 monIdx = (u8)gSpecialVar_0x8004;
    u8 zero = 0;

    if (monIdx >= PARTY_SIZE)
        return;

    SetMonData(&gPlayerParty[monIdx], MON_DATA_HP_EV, &zero);
    SetMonData(&gPlayerParty[monIdx], MON_DATA_ATK_EV, &zero);
    SetMonData(&gPlayerParty[monIdx], MON_DATA_DEF_EV, &zero);
    SetMonData(&gPlayerParty[monIdx], MON_DATA_SPATK_EV, &zero);
    SetMonData(&gPlayerParty[monIdx], MON_DATA_SPDEF_EV, &zero);
    SetMonData(&gPlayerParty[monIdx], MON_DATA_SPEED_EV, &zero);

    CalculateMonStats(&gPlayerParty[monIdx]);
}

// 0 ok, 1 stat max (252), 2 total max (510)
u16 EvTrainer_GetChosenTrainState(void)
{
    u8 stat = (u8)gSpecialVar_0x8005;
    u8 monIdx = (u8)gSpecialVar_0x8004;

    if (stat >= EV_TRAIN_STAT_COUNT || monIdx >= PARTY_SIZE)
        return 1;

    {
        u8 evId = EvTrainer_GetStatEvDataId(stat);
        u8 statEv = (u8)GetMonData(&gPlayerParty[monIdx], evId);

        u16 totalEv = 0;
        totalEv += (u8)GetMonData(&gPlayerParty[monIdx], MON_DATA_HP_EV);
        totalEv += (u8)GetMonData(&gPlayerParty[monIdx], MON_DATA_ATK_EV);
        totalEv += (u8)GetMonData(&gPlayerParty[monIdx], MON_DATA_DEF_EV);
        totalEv += (u8)GetMonData(&gPlayerParty[monIdx], MON_DATA_SPATK_EV);
        totalEv += (u8)GetMonData(&gPlayerParty[monIdx], MON_DATA_SPDEF_EV);
        totalEv += (u8)GetMonData(&gPlayerParty[monIdx], MON_DATA_SPEED_EV);

        if (totalEv >= EV_TOTAL_CAP)
            return 2;
        if (statEv >= EV_STAT_CAP)
            return 1;
    }

    return 0;
}
