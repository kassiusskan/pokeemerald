#include "global.h"
#include "iv_trainer.h"

#include "daycare.h"                 // GetBoxMonNickname, ChooseMonForDaycare
#include "event_data.h"              // gSpecialVar_0x8004/0x8005
#include "main.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"  // CompactPartySlots
#include "string_util.h"
#include "strings.h"
#include "event_object_movement.h"

#include "config/iv_trainer.h"

// ---------------- helpers ----------------

static bool8 IvTrainer_HasPokemon(struct IvTrainerFacility *fac)
{
    return GetBoxMonData(&fac->mon, MON_DATA_SPECIES) != SPECIES_NONE;
}

static void ClearIvTrainerFacility(struct IvTrainerFacility *fac)
{
    ZeroBoxMonData(&fac->mon);
    fac->ivStepCounter = 0;
    fac->ivPoints = 0;
    fac->stat = IV_TRAIN_STAT_NONE;
}

static u8 IvTrainer_GetStatMonDataId(u8 stat)
{
    static const u8 sIvMonDataIds[IV_TRAIN_STAT_COUNT] =
    {
        [IV_TRAIN_STAT_HP]     = MON_DATA_HP_IV,
        [IV_TRAIN_STAT_ATK]    = MON_DATA_ATK_IV,
        [IV_TRAIN_STAT_DEF]    = MON_DATA_DEF_IV,
        [IV_TRAIN_STAT_SPATK]  = MON_DATA_SPATK_IV,
        [IV_TRAIN_STAT_SPDEF]  = MON_DATA_SPDEF_IV,
        [IV_TRAIN_STAT_SPEED]  = MON_DATA_SPEED_IV,
    };

    if (stat >= IV_TRAIN_STAT_COUNT)
        return MON_DATA_HP_IV;

    return sIvMonDataIds[stat];
}

static const u8 *IvTrainer_GetStatName(u8 stat)
{
    static const u8 *const sNames[IV_TRAIN_STAT_COUNT] =
    {
        [IV_TRAIN_STAT_HP]     = gText_HP3,
        [IV_TRAIN_STAT_ATK]    = gText_Attack3,
        [IV_TRAIN_STAT_DEF]    = gText_Defense3,
        [IV_TRAIN_STAT_SPATK]  = gText_SpAtk3,
        [IV_TRAIN_STAT_SPDEF]  = gText_SpDef3,
        [IV_TRAIN_STAT_SPEED]  = gText_Speed2, // <- NÃO existe Speed3 nessa base
    };

    if (stat >= IV_TRAIN_STAT_COUNT)
        return gText_EmptyString2;

    return sNames[stat];
}

// -------------------------------------------------
// Per-step: só IV (sem EXP/Level)
// -------------------------------------------------
void IvTrainer_OnStep(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;

    if (!IvTrainer_HasPokemon(fac))
        return;

    if (fac->stat >= IV_TRAIN_STAT_COUNT)
        return;

    // Se já está (ou vai ficar) no teto, não conta mais
    {
        u8 monDataId = IvTrainer_GetStatMonDataId(fac->stat);
        u8 currentIv = (u8)GetBoxMonData(&fac->mon, monDataId);

        if ((u16)currentIv + fac->ivPoints >= 31)
            return;
    }

    fac->ivStepCounter++;
    if (fac->ivStepCounter >= IV_TRAINER_STEPS_PER_IV)
    {
        fac->ivStepCounter -= IV_TRAINER_STEPS_PER_IV;

        // clamp p/ não passar de 31
        u8 monDataId = IvTrainer_GetStatMonDataId(fac->stat);
        u8 currentIv = (u8)GetBoxMonData(&fac->mon, monDataId);

        if ((u16)currentIv + fac->ivPoints < 31)
            fac->ivPoints++;
    }
}

// -------------------------------------------------
// Specials (scripts)
// -------------------------------------------------
u8 GetIvTrainerState(void)
{
    return IvTrainer_HasPokemon(&gSaveBlock1Ptr->ivTrainer) ? IV_TRAINER_HAS_MON : IV_TRAINER_EMPTY;
}

void ChooseSendIvTrainerMon(void)
{
    ChooseMonForDaycare();
    gMain.savedCallback = CB2_ReturnToField;
}

void StoreSelectedPokemonInIvTrainer(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;
    u8 monIdx = (u8)gSpecialVar_0x8004;
    u8 stat = (u8)gSpecialVar_0x8005;

    if (IvTrainer_HasPokemon(fac))
        return;

    if (monIdx >= PARTY_SIZE)
        return;

    if (GetMonData(&gPlayerParty[monIdx], MON_DATA_IS_EGG))
        return;

    fac->mon = gPlayerParty[monIdx].box;
    fac->ivStepCounter = 0;
    fac->ivPoints = 0;
    fac->stat = (stat < IV_TRAIN_STAT_COUNT) ? stat : IV_TRAIN_STAT_NONE;

    // remove da party
    ZeroMonData(&gPlayerParty[monIdx]);
    CompactPartySlots();
    CalculatePlayerPartyCount();
	UpdateFollowingPokemon();
}

void IvTrainer_SetTrainingStat(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;
    u8 stat = (u8)gSpecialVar_0x8005;

    if (!IvTrainer_HasPokemon(fac))
        return;

    if (stat >= IV_TRAIN_STAT_COUNT)
        stat = IV_TRAIN_STAT_NONE;

    fac->stat = stat;
    fac->ivStepCounter = 0;
}

u8 GetNumIvPointsGainedFromIvTrainer(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;
    if (!IvTrainer_HasPokemon(fac))
        return 0;

    return fac->ivPoints;
}

void BufferIvTrainerSummary(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;

    if (!IvTrainer_HasPokemon(fac))
    {
        StringCopy(gStringVar1, gText_EmptyString2);
        StringCopy(gStringVar2, gText_EmptyString2);
        StringCopy(gStringVar3, gText_EmptyString2);
        StringCopy(gStringVar4, gText_EmptyString2);
        return;
    }

    GetBoxMonNickname(&fac->mon, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, fac->ivPoints, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(gStringVar3, IvTrainer_GetStatName(fac->stat));
    StringCopy(gStringVar4, gText_EmptyString2);
}

void GetIvTrainerCost(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;
    u32 cost = 0;

    if (!IvTrainer_HasPokemon(fac))
    {
        gSpecialVar_0x8005 = 0;
        return;
    }

    // Nome em STR_VAR_1 (igual Daycare)
    GetBoxMonNickname(&fac->mon, gStringVar1);

    // Se ainda não escolheu stat, cobra só base (ou 0, como preferir)
    if (fac->stat >= IV_TRAIN_STAT_COUNT || fac->ivPoints == 0)
    {
        cost = IV_TRAINER_BASE_COST;
        if (cost > 99999) cost = 99999;
        gSpecialVar_0x8005 = (u16)cost;
        ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 5);
        return;
    }

    // IV atual do stat treinado
    {
        u8 monDataId = IvTrainer_GetStatMonDataId(fac->stat);
        u8 currentIv = (u8)GetBoxMonData(&fac->mon, monDataId);

        // Base + soma por cada +1 IV, usando a faixa do IV FINAL
        cost = IV_TRAINER_BASE_COST;

        for (u8 i = 1; i <= fac->ivPoints; i++)
        {
            u8 finalIv = currentIv + i;
            if (finalIv > 31)
                finalIv = 31;

            if (finalIv <= 10)
                cost += IV_TRAINER_COST_IV_1_10;
            else if (finalIv <= 20)
                cost += IV_TRAINER_COST_IV_11_20;
            else
                cost += IV_TRAINER_COST_IV_21_31;

            if (finalIv == 31)
                break; // já bateu o teto
        }
    }

    if (cost > 99999)
        cost = 99999;

    gSpecialVar_0x8005 = (u16)cost;
    ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 5);
}


u16 TakePokemonFromIvTrainer(void)
{
    struct IvTrainerFacility *fac = &gSaveBlock1Ptr->ivTrainer;
    struct Pokemon pokemon;
    u32 species;

    if (!IvTrainer_HasPokemon(fac))
        return SPECIES_NONE;

    species = GetBoxMonData(&fac->mon, MON_DATA_SPECIES);
    BoxMonToMon(&fac->mon, &pokemon);

    // aplica IV points no stat escolhido
    if (fac->stat < IV_TRAIN_STAT_COUNT && fac->ivPoints != 0)
    {
        u8 monDataId = IvTrainer_GetStatMonDataId(fac->stat);
        u8 iv = (u8)GetMonData(&pokemon, monDataId);
        u16 newIv = iv + fac->ivPoints;

        if (newIv > 31)
            newIv = 31;

        iv = (u8)newIv;
        SetMonData(&pokemon, monDataId, &iv);
    }

    CalculateMonStats(&pokemon);

    // devolve pra party (script garante espaço)
    gPlayerParty[PARTY_SIZE - 1] = pokemon;
    CompactPartySlots();
    CalculatePlayerPartyCount();
	UpdateFollowingPokemon();

    ClearIvTrainerFacility(fac);
    return (u16)species;
	
}

// -------------------------------------------------
// Special: checa se o stat escolhido já está IV 31
// VAR_0x8005 = stat
// Se já tiver depositado: checa o BoxMon depositado
// Se ainda não depositou: checa o mon selecionado (VAR_0x8004)
// Retorna 1 se IV>=31, senão 0
// -------------------------------------------------
u16 IvTrainer_IsChosenStatMaxed(void)
{
    u8 stat = (u8)gSpecialVar_0x8005;

    if (stat >= IV_TRAIN_STAT_COUNT)
        return 1;

    {
        u8 monDataId = IvTrainer_GetStatMonDataId(stat);

        if (GetIvTrainerState() == IV_TRAINER_HAS_MON)
        {
            u16 iv = (u16)GetBoxMonData(&gSaveBlock1Ptr->ivTrainer.mon, monDataId);
            return (iv >= 31);
        }
        else
        {
            u8 monIdx = (u8)gSpecialVar_0x8004;
            if (monIdx >= PARTY_SIZE)
                return 1;

            {
                u16 iv = (u16)GetMonData(&gPlayerParty[monIdx], monDataId);
                return (iv >= 31);
            }
        }
    }
}
