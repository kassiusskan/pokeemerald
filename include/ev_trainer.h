#ifndef GUARD_EV_TRAINER_H
#define GUARD_EV_TRAINER_H

#include "global.h"

#define EV_TRAINER_EMPTY   0
#define EV_TRAINER_HAS_MON 1

enum EvTrainerStat
{
    EV_TRAIN_STAT_HP = 0,
    EV_TRAIN_STAT_ATK,
    EV_TRAIN_STAT_DEF,
    EV_TRAIN_STAT_SPATK,
    EV_TRAIN_STAT_SPDEF,
    EV_TRAIN_STAT_SPEED,
    EV_TRAIN_STAT_COUNT,

    EV_TRAIN_STAT_NONE = 0xFF,
};

void EvTrainer_OnStep(void);

u8 GetEvTrainerState(void);
void ChooseSendEvTrainerMon(void);
void StoreSelectedPokemonInEvTrainer(void);   // VAR_0x8004 (party idx), VAR_0x8005 (stat)
void EvTrainer_SetTrainingStat(void);         // VAR_0x8005 (stat)
void BufferEvTrainerSummary(void);            // STR_VAR_1..3
void GetEvTrainerCost(void);                  // VAR_0x8005 (cost), STR_VAR_1/2
u16 TakePokemonFromEvTrainer(void);           // retorna species (cry)

void GetEvTrainerResetCost(void);             // VAR_0x8005 (cost), STR_VAR_1/2 (nick/cost)
void EvTrainer_ResetSelectedMonEVs(void);     // usa VAR_0x8004
u16 EvTrainer_GetChosenTrainState(void);      // 0 ok, 1 stat max, 2 total max

#endif
