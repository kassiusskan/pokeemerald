#ifndef GUARD_IV_TRAINER_H
#define GUARD_IV_TRAINER_H

#include "global.h"

#define IV_TRAINER_EMPTY   0
#define IV_TRAINER_HAS_MON 1

enum IvTrainerStat
{
    IV_TRAIN_STAT_HP = 0,
    IV_TRAIN_STAT_ATK,
    IV_TRAIN_STAT_DEF,
    IV_TRAIN_STAT_SPATK,
    IV_TRAIN_STAT_SPDEF,
    IV_TRAIN_STAT_SPEED,
    IV_TRAIN_STAT_COUNT,

    IV_TRAIN_STAT_NONE = 0xFF,
};

void IvTrainer_OnStep(void);

u8 GetIvTrainerState(void);
void ChooseSendIvTrainerMon(void);
void StoreSelectedPokemonInIvTrainer(void); // VAR_0x8004 (party idx), VAR_0x8005 (stat)
void IvTrainer_SetTrainingStat(void);       // VAR_0x8005 (stat) opcional
void BufferIvTrainerSummary(void);          // STR_VAR_1..4
void GetIvTrainerCost(void);                // VAR_0x8005 (cost) + STR_VAR_1/2
u8 GetNumIvPointsGainedFromIvTrainer(void);
u16 TakePokemonFromIvTrainer(void);         // retorna species (pra cry)

#endif
