Route117_PokemonDayCare_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route117_PokemonDayCare_OnTransition
	.byte 0

Route117_PokemonDayCare_OnTransition:
	setflag FLAG_LANDMARK_POKEMON_DAYCARE
	end


// ==========================
// SCRIPT PRINCIPAL
// ==========================
IvTrainer_EventScript::
	lock
	faceplayer

	specialvar VAR_RESULT, GetIvTrainerState
	goto_if_eq VAR_RESULT, 1, IvTrainer_HasMon

// ==========================
// NÃO TEM MON DEPOSITADO
// ==========================
IvTrainer_Empty::
	msgbox IvTrainer_Text_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IvTrainer_ComeAgain

	// Mesma proteção do Daycare (não deixar sem mon)
	specialvar VAR_RESULT, CountPartyAliveNonEggMons
	goto_if_eq VAR_RESULT, 1, IvTrainer_OnlyOneAliveMon
	specialvar VAR_RESULT, CountPartyAliveNonEggMons
	goto_if_eq VAR_RESULT, 2, IvTrainer_OnlyTwoAliveMons

	msgbox IvTrainer_Text_WhichMon, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	special ChooseSendIvTrainerMon
	waitstate

	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, IvTrainer_ComeAgain

	// Proteção extra igual Daycare (se escolheu o único vivo)
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, IvTrainer_OnlyOneAliveMon

	// (Opcional) toca o cry e mostra mensagem “vou treinar”
	specialvar VAR_0x8005, GetSelectedMonNicknameAndSpecies
	waitse
	playmoncry VAR_0x8005, CRY_MODE_NORMAL

IvTrainer_SelectStat_DepositLoop::
	msgbox IvTrainer_Text_SelectStat, MSGBOX_DEFAULT
	multichoice 0, 0, MULTI_IV_TRAINER_STATS, FALSE

	copyvar VAR_0x8005, VAR_RESULT
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, IvTrainer_ComeAgain
	goto_if_eq VAR_RESULT, 6, IvTrainer_ComeAgain

	// Checa “já está no máximo (31)?”
	specialvar VAR_RESULT, IvTrainer_IsChosenStatMaxed
	goto_if_eq VAR_RESULT, 1, IvTrainer_StatAlreadyMaxed_Deposit

	// Deposita (usa VAR_0x8004 = party idx, VAR_0x8005 = stat)
	special StoreSelectedPokemonInIvTrainer
	msgbox IvTrainer_Text_WellTrain, MSGBOX_DEFAULT
	waitmoncry

	// Se você usa follower system e tiver essa função, descomente:
	// callnative UpdateFollowingPokemon

	release
	end

IvTrainer_StatAlreadyMaxed_Deposit::
	msgbox IvTrainer_Text_StatAlreadyMaxed, MSGBOX_DEFAULT
	goto IvTrainer_SelectStat_DepositLoop

// ==========================
// TEM MON DEPOSITADO
// ==========================
IvTrainer_HasMon::
	msgbox IvTrainer_Text_GoodToSeeYou, MSGBOX_DEFAULT

	// STR_VAR_1 = nome, STR_VAR_2 = iv gained, STR_VAR_3 = stat name
	special BufferIvTrainerSummary
	msgbox IvTrainer_Text_Summary, MSGBOX_DEFAULT

	msgbox IvTrainer_Text_TakeBackQ, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, IvTrainer_TryRetrieve

	msgbox IvTrainer_Text_ChangeStatQ, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IvTrainer_ComeAgain

IvTrainer_SelectStat_ChangeLoop::
	multichoice 0, 0, MULTI_IV_TRAINER_STATS, FALSE
	copyvar VAR_0x8005, VAR_RESULT
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, IvTrainer_ComeAgain
	goto_if_eq VAR_RESULT, 6, IvTrainer_ComeAgain

	specialvar VAR_RESULT, IvTrainer_IsChosenStatMaxed
	goto_if_eq VAR_RESULT, 1, IvTrainer_StatAlreadyMaxed_Change

	special IvTrainer_SetTrainingStat
	msgbox IvTrainer_Text_Changed, MSGBOX_DEFAULT
	goto IvTrainer_ComeAgain

IvTrainer_StatAlreadyMaxed_Change::
	msgbox IvTrainer_Text_StatAlreadyMaxed, MSGBOX_DEFAULT
	goto IvTrainer_SelectStat_ChangeLoop

// ==========================
// RETIRAR (igual Daycare)
// ==========================
IvTrainer_TryRetrieve::
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, PARTY_SIZE, IvTrainer_NoRoom

	// Preenche STR_VAR_1 (nome) e STR_VAR_2 (custo), custo fica em VAR_0x8005
	special GetIvTrainerCost

	msgbox IvTrainer_Text_ItWillCostX, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, IvTrainer_CheckEnoughMoney
	goto IvTrainer_ComeAgain

IvTrainer_CheckEnoughMoney::
	checkitem ITEM_GOLD_BOTTLE_CAP, VAR_0x8005
	goto_if_eq VAR_RESULT, 1, IvTrainer_RetrieveMon
	msgbox IvTrainer_Text_NotEnoughMoney, MSGBOX_DEFAULT
	release
	end

IvTrainer_RetrieveMon::
	applymovement LOCALID_DAYCARE_OLD_MAN, IvTrainer_Movement_RetrieveMon
	waitmovement 0

	specialvar VAR_RESULT, TakePokemonFromIvTrainer
	removeitem ITEM_GOLD_BOTTLE_CAP, VAR_0x8005
	playse SE_SHOP

	msgbox IvTrainer_Text_HeresYourMon, MSGBOX_DEFAULT
	waitse
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	msgbox IvTrainer_Text_TookBackMon, MSGBOX_DEFAULT
	waitmoncry

	// Se você usa follower system e tiver essa função, descomente:
	// callnative UpdateFollowingPokemon

	release
	end

// ==========================
// SAÍDAS / ERROS
// ==========================
IvTrainer_NoRoom::
	msgbox IvTrainer_Text_TeamFull, MSGBOX_DEFAULT
	release
	end

IvTrainer_ComeAgain::
	msgbox IvTrainer_Text_ComeAgain, MSGBOX_DEFAULT
	release
	end

IvTrainer_OnlyOneAliveMon::
	msgbox IvTrainer_Text_WhatWillYouBattleWith, MSGBOX_DEFAULT
	release
	end

IvTrainer_OnlyTwoAliveMons::
	msgbox IvTrainer_Text_YoullBeLeftWithJustOne, MSGBOX_DEFAULT
	release
	end

// ==========================
// MOVEMENT (igual Daycare)
// Ajuste se seu NPC não estiver atrás do balcão.
// ==========================
IvTrainer_Movement_RetrieveMon:
	delay_16
	delay_16
	face_left
	delay_16
	delay_16
	face_right
	delay_16
	delay_16
	face_up
	walk_slow_up
	set_invisible
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	face_down
	set_visible
	walk_slow_down
	step_end

// ==========================
// TEXTOS
// ==========================
IvTrainer_Text_Intro:
	.string "Eu posso treinar 1 IV do seu POKéMON\n"
	.string "enquanto você anda por aí.\p"
	.string "Quer deixar um comigo?$"

IvTrainer_Text_WhichMon:
	.string "Qual POKéMON você quer deixar?$"

IvTrainer_Text_SelectStat:
	.string "Em qual atributo você quer treinar o IV?$"

IvTrainer_Text_StatAlreadyMaxed:
	.string "Esse IV já está no máximo (31)!\n"
	.string "Escolha outro atributo.$"

IvTrainer_Text_WellTrain:
	.string "Certo.\n"
	.string "Vou treinar esse IV.\p"
	.string "Volte depois para buscar.$"

IvTrainer_Text_GoodToSeeYou:
	.string "Ah, é você! Que bom ver você.$"

IvTrainer_Text_Summary:
	.string "Seu {STR_VAR_1} ganhou +{STR_VAR_2} IV\n"
	.string "em {STR_VAR_3}.$"

IvTrainer_Text_TakeBackQ:
	.string "Você quer seu POKéMON de volta?$"

IvTrainer_Text_ChangeStatQ:
	.string "Quer trocar qual IV está sendo treinado?$"

IvTrainer_Text_Changed:
	.string "Beleza! Vou focar nesse IV a partir de agora.$"

IvTrainer_Text_ItWillCostX:
    .string "Se você quer seu {STR_VAR_1} de volta,\n"
    .string "vai custar {STR_VAR_2} Gold Bottle Cap(s).$"

IvTrainer_Text_NotEnoughMoney:
    .string "Você nao tem Gold Bottle Cap(s) suficiente(s).$"

IvTrainer_Text_HeresYourMon:
	.string "Perfeito!\n"
	.string "Aqui está seu POKéMON.$"

IvTrainer_Text_TookBackMon:
	.string "{PLAYER} pegou {STR_VAR_1}\n"
	.string "de volta.$"

IvTrainer_Text_TeamFull:
	.string "Seu time está cheio.\n"
	.string "Abra espaço e volte.$"

IvTrainer_Text_ComeAgain:
	.string "Certo.\n"
	.string "Volte depois.$"

IvTrainer_Text_WhatWillYouBattleWith:
	.string "Se você deixar esse POKéMON comigo,\n"
	.string "com o que vai batalhar?\p"
	.string "Volte outra hora.$"

IvTrainer_Text_YoullBeLeftWithJustOne:
	.string "Se você deixar esse POKéMON,\n"
	.string "vai ficar só com um.\p"
	.string "Volte outra hora.$"
	
// EV TRAINER

// EV TRAINER states returned by GetEvTrainerState
#define EV_TRAINER_NO_MON   0
#define EV_TRAINER_HAS_MON  1

EvTrainer_EventScript::
    lock
    faceplayer

    specialvar VAR_RESULT, GetEvTrainerState
    goto_if_eq VAR_RESULT, EV_TRAINER_HAS_MON, EvTrainer_HasMon

    msgbox EvTrainer_Text_Intro, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, NO, EvTrainer_End

    // escolher pokemon
    specialvar VAR_RESULT, CountPartyNonEggMons
    goto_if_eq VAR_RESULT, 1, EvTrainer_OnlyOneMon

    specialvar VAR_RESULT, CountPartyAliveNonEggMons
    goto_if_eq VAR_RESULT, 2, EvTrainer_OnlyTwoAliveMons

    msgbox EvTrainer_Text_WhichMon, MSGBOX_DEFAULT
    fadescreen FADE_TO_BLACK
    special ChooseSendEvTrainerMon
    waitstate
    goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, EvTrainer_ComeAgain

    specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
    goto_if_eq VAR_RESULT, 0, EvTrainer_OnlyOneAliveMon

    // Resetar ou Treinar (sem multichoice, só yes/no)
    msgbox EvTrainer_Text_ResetQ, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, YES, EvTrainer_DoReset

    msgbox EvTrainer_Text_TrainQ, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, YES, EvTrainer_TrainFlow
    goto EvTrainer_End

EvTrainer_DoReset::
    special GetEvTrainerResetCost
    msgbox EvTrainer_Text_ResetCost, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, NO, EvTrainer_End

    checkitem ITEM_BOTTLE_CAP, VAR_0x8005
    goto_if_eq VAR_RESULT, 0, EvTrainer_NoMoney

    removeitem ITEM_BOTTLE_CAP, VAR_0x8005
    playse SE_SHOP
    waitse

    special EvTrainer_ResetSelectedMonEVs
    msgbox EvTrainer_Text_ResetDone, MSGBOX_DEFAULT

    msgbox EvTrainer_Text_WantTrainNow, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, YES, EvTrainer_TrainFlow
    goto EvTrainer_End

EvTrainer_TrainFlow::
EvTrainer_SelectStatLoop::
    msgbox EvTrainer_Text_SelectStat, MSGBOX_DEFAULT
    multichoice 0, 0, MULTI_IV_TRAINER_STATS, FALSE

    copyvar VAR_0x8005, VAR_RESULT
    goto_if_eq VAR_RESULT, MULTI_B_PRESSED, EvTrainer_End
    goto_if_eq VAR_RESULT, 6, EvTrainer_End

    specialvar VAR_RESULT, EvTrainer_GetChosenTrainState
    goto_if_eq VAR_RESULT, 1, EvTrainer_StatMaxed
    goto_if_eq VAR_RESULT, 2, EvTrainer_TotalMaxed

    special StoreSelectedPokemonInEvTrainer
    callnative UpdateFollowingPokemon
    msgbox EvTrainer_Text_Deposited, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_StatMaxed::
    msgbox EvTrainer_Text_StatMaxed, MSGBOX_DEFAULT
    goto EvTrainer_SelectStatLoop

EvTrainer_TotalMaxed::
    msgbox EvTrainer_Text_TotalMaxed, MSGBOX_DEFAULT
    goto EvTrainer_End

// ---------------------
// Já tem Pokémon guardado
// ---------------------
EvTrainer_HasMon::
    msgbox EvTrainer_Text_GoodToSeeYou, MSGBOX_DEFAULT
    special BufferEvTrainerSummary
    msgbox EvTrainer_Text_Summary, MSGBOX_DEFAULT

    msgbox EvTrainer_Text_TakeBackQ, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, YES, EvTrainer_TryRetrieve
    goto EvTrainer_End

EvTrainer_TryRetrieve::
    specialvar VAR_RESULT, CalculatePlayerPartyCount
    goto_if_eq VAR_RESULT, PARTY_SIZE, EvTrainer_NoRoom

    special GetEvTrainerCost
    msgbox EvTrainer_Text_ItWillCostX, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, NO, EvTrainer_End

    checkitem ITEM_BOTTLE_CAP, VAR_0x8005
    goto_if_eq VAR_RESULT, 0, EvTrainer_NoMoney

    // ajuste o localid "1" pro seu NPC
    applymovement LOCALID_DAYCARE_MAN, EvTrainer_Movement_Retrieve
    waitmovement 0

    specialvar VAR_RESULT, TakePokemonFromEvTrainer
    removeitem ITEM_BOTTLE_CAP, VAR_0x8005
    playse SE_SHOP

    msgbox EvTrainer_Text_HeresYourMon, MSGBOX_DEFAULT
    waitse
    playmoncry VAR_RESULT, CRY_MODE_NORMAL
    msgbox EvTrainer_Text_TookBackMon, MSGBOX_DEFAULT
    waitmoncry

    callnative UpdateFollowingPokemon
    goto EvTrainer_End

EvTrainer_NoRoom::
    msgbox EvTrainer_Text_TeamFull, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_NoMoney::
    msgbox EvTrainer_Text_NotEnoughMoney, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_ComeAgain::
    msgbox EvTrainer_Text_ComeAgain, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_OnlyOneMon::
    msgbox EvTrainer_Text_OnlyOneMon, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_OnlyOneAliveMon::
    msgbox EvTrainer_Text_OnlyOneAliveMon, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_OnlyTwoAliveMons::
    msgbox EvTrainer_Text_OnlyTwoAliveMons, MSGBOX_DEFAULT
    goto EvTrainer_End

EvTrainer_End::
    release
    end

EvTrainer_Movement_Retrieve:
    delay_16
    delay_16
    face_left
    delay_16
    delay_16
    face_right
    delay_16
    delay_16
    face_up
    walk_slow_up
    set_invisible
    delay_16
    delay_16
    delay_16
    delay_16
    delay_16
    face_down
    set_visible
    walk_slow_down
    step_end

// -------- TEXTOS --------
EvTrainer_Text_Intro:
    .string "Eu posso RESETAR ou TREINAR EVs.\n"
    .string "Quer usar o serviço?$"

EvTrainer_Text_WhichMon:
    .string "Qual POKéMON você quer escolher?$"

EvTrainer_Text_ResetQ:
    .string "Você quer RESETAR os EVs desse POKéMON?$"

EvTrainer_Text_ResetCost:
    .string "Resetar os EVs custa\n"
    .string "{STR_VAR_2} Bottle Cap(s).$"

EvTrainer_Text_ResetDone:
    .string "Pronto! Os EVs foram resetados.$"

EvTrainer_Text_WantTrainNow:
    .string "Quer TREINAR EVs agora?$"

EvTrainer_Text_TrainQ:
    .string "Entao quer TREINAR EVs desse POKéMON?$"

EvTrainer_Text_SelectStat:
    .string "Qual atributo você quer treinar?$"

EvTrainer_Text_StatMaxed:
    .string "Esse atributo já está no máximo (252).$"

EvTrainer_Text_TotalMaxed:
    .string "Esse POKéMON já está no limite total (510).$"

EvTrainer_Text_Deposited:
    .string "Certo! Vou treinar esses EVs.\n"
    .string "Volte depois para buscar.$"

EvTrainer_Text_GoodToSeeYou:
    .string "Ah, é você!$"

EvTrainer_Text_Summary:
    .string "{STR_VAR_1} ganhou +{STR_VAR_2} EV\n"
    .string "em {STR_VAR_3}.$"

EvTrainer_Text_TakeBackQ:
    .string "Quer pegar seu POKéMON de volta?$"

EvTrainer_Text_ItWillCostX:
    .string "Se você quer seu {STR_VAR_1} de volta,\n"
    .string "vai custar {STR_VAR_2} Bottle Cap(s).$"

EvTrainer_Text_HeresYourMon:
    .string "Perfeito! Aqui está.$"

EvTrainer_Text_TookBackMon:
    .string "{PLAYER} pegou {STR_VAR_1}\n"
    .string "de volta.$"

EvTrainer_Text_TeamFull:
    .string "Seu time está cheio.\n"
    .string "Abra espaço e volte.$"

EvTrainer_Text_NotEnoughMoney:
    .string "Você nao tem Bottle Cap(s) suficiente(s).$"

EvTrainer_Text_ComeAgain:
    .string "Certo. Volte depois.$"

EvTrainer_Text_OnlyOneMon:
    .string "Você só tem 1 POKéMON.\n"
    .string "Volte depois.$"

EvTrainer_Text_OnlyOneAliveMon:
    .string "Se você deixar esse POKéMON,\n"
    .string "com o que vai batalhar?$"

EvTrainer_Text_OnlyTwoAliveMons:
    .string "Se você deixar esse POKéMON,\n"
    .string "vai ficar só com um.$"
