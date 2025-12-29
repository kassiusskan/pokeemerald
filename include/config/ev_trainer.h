#ifndef GUARD_CONFIG_EV_TRAINER_H
#define GUARD_CONFIG_EV_TRAINER_H

// Quantos passos para ganhar +1 EV no stat escolhido
#define EV_TRAINER_STEPS_PER_EV  1

// Custo (EM BOTTLE CAPS) ao retirar o Pokémon após treinar EVs:
// custo = EV_TRAINER_BASE_COST + (EV_TRAINER_COST_PER_EV * evGanhos)
#define EV_TRAINER_BASE_COST     1
#define EV_TRAINER_COST_PER_EV   1   // Recomendo 0 se você não quer 1 cap por EV (fica absurdo)

// Custo (EM BOTTLE CAPS) para RESETAR os EVs do Pokémon
#define EV_TRAINER_RESET_COST    200

#endif // GUARD_CONFIG_EV_TRAINER_H
