#ifndef GUARD_CONFIG_IV_TRAINER_H
#define GUARD_CONFIG_IV_TRAINER_H

// Quantos passos para ganhar +1 IV no stat escolhido
#define IV_TRAINER_STEPS_PER_IV  1

// Custo (EM GOLD BOTTLE CAPS) ao retirar o Pokémon após treinar IVs:
// custo = IV_TRAINER_BASE_COST + soma do custo por cada +1 IV, dependendo do IV alvo
//
// Exemplo de “faixas” (bem parecido com o que você descreveu):
#define IV_TRAINER_BASE_COST     1

// Custo por +1 IV quando o IV FINAL (após o incremento) cair nessa faixa:
#define IV_TRAINER_COST_IV_1_10   0
#define IV_TRAINER_COST_IV_11_20  1
#define IV_TRAINER_COST_IV_21_31  2

#endif // GUARD_CONFIG_IV_TRAINER_H
