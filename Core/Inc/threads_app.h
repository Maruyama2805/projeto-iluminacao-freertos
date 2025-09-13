/*
 * threads_app.h
 *
 *  Created on: Aug 14, 2025
 *      Author: Gabri
 */

#ifndef INC_THREADS_APP_H_
#define INC_THREADS_APP_H_

// --- Sinais para os Botões ---
#define SINAL_BOTAO_C (1UL << 0)
#define SINAL_BOTAO_D (1UL << 1)
#define SINAL_BOTAO_E (1UL << 2)
#define SINAL_BOTAO_F (1UL << 3)
#define SINAL_BOTAO_G (1UL << 4)
#define SINAL_BOTAO_H (1UL << 12)

#define SINAL_BOTAO_A (1UL << 5)
#define SINAL_BOTAO_B (1UL << 6)
#define SINAL_BOTAO_X (1UL << 7)
#define SINAL_BOTAO_Y (1UL << 8)

#define SINAL_BOTAO_L (1UL << 9)
#define SINAL_BOTAO_M (1UL << 10)
#define SINAL_BOTAO_N (1UL << 11)

#include "cmsis_os2.h"
#include <stdbool.h>

// Enumeração das threads que estao sendo verificadas pelo watchdog
typedef enum {
    WD_THREAD_PARES_IMPARES = 0,
    WD_THREAD_GRAY_CODE,
	WD_THREAD_POTENCIOMETRO,
	WD_THREAD_KNIGHT_RIDER,
	WD_THREAD_BINARY_COUNTER,
    // Adicione aqui o ID de qualquer outra thread
    MONITORED_THREADS_COUNT
} Watchdog_ThreadID_t;

// Enumeração com todos os comandos possíveis no sistema
typedef enum {
    CMD_SET_MODE,
    CMD_TOGGLE_SOUND,
    CMD_SPEED_UP,       // Dobrar velocidade
    CMD_SPEED_DOWN,     // Metade da velocidade
    CMD_SPEED_RESET,
    CMD_SPEED_FROM_POT,
    CMD_SPEED_PRESET_L,
    CMD_SPEED_PRESET_M,
    CMD_SPEED_PRESET_N
} CommandType_t;

// estrutura da mensagem que será enviada para a fila
typedef struct {
    CommandType_t command;
    uint32_t value;
} SystemEvent_t;


// 'extern' para diz que estas variáveis existem, mas são definidas em outro lugar
extern osMutexId_t g_FuncMutexHandle;
extern volatile uint8_t g_func_ativa;
extern volatile uint8_t g_leds_acesos;      // Variável para sincronização
extern volatile uint8_t g_modo_sonoro;      // Controle Mudo/Sonoro
extern volatile uint32_t g_velocidade_alvo_ms;
extern volatile uint32_t g_velocidade_delay_ms;
extern volatile uint32_t g_timestamp_ultima_atividade;
extern volatile uint8_t g_evento_troca_funcao;
extern volatile bool g_sistema_em_erro;
extern volatile uint32_t g_heartbeat_timestamps[MONITORED_THREADS_COUNT];
extern osMessageQueueId_t g_eventQueueHandle;
extern volatile uint32_t g_adc_valor_pot;
extern volatile bool g_suavizacao_ativa;




void ThreadControleBot(void *argument);
void ThreadPollingBotoes(void *argument);
void ThreadLedParesImpares(void *argument);
void ThreadLedGrayCode(void *argument);
void DesligaAllLeds(void);
void ThreadPotenciometro (void * argument);
void ThreadBuzzer(void *argument);
void ThreadSuavizacao (void *argument);
void ThreadStatusLed(void *argument);
void ThreadWatchdog(void *argument);
void ThreadKnightRider(void *argument);
void ThreadBinaryCounter(void *argument);
void ThreadGestoraDeEstado(void *argument);
void ThreadADCReader(void *argument);
void watchdog_check_in(Watchdog_ThreadID_t thread_id);


#endif /* INC_THREADS_APP_H_ */
