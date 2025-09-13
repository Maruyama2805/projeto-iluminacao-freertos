#include "main.h"
#include "cmsis_os2.h"
#include "threads_app.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define VELOCIDADE_PADRAO 500
#define NUM_LEDS 7

osMutexId_t g_FuncMutexHandle;
volatile uint8_t g_func_ativa = 0;
volatile uint8_t g_leds_acesos = 0;
volatile uint8_t g_modo_sonoro = 1;  //1=Sonoro, 0=Mudo (inicia ligado)
//suavizaçao
volatile uint32_t g_velocidade_delay_ms = VELOCIDADE_PADRAO;
volatile uint32_t g_velocidade_alvo_ms = VELOCIDADE_PADRAO;

volatile uint32_t g_timestamp_ultima_atividade = 0; // registra ultima atividade exercida
volatile uint8_t g_evento_troca_funcao = 0;
volatile bool g_sistema_em_erro = false;
volatile uint32_t g_heartbeat_timestamps[MONITORED_THREADS_COUNT] = { 0 };

volatile uint32_t g_adc_valor_pot = 0;

volatile bool g_suavizacao_ativa = true;

void ThreadADCReader(void *argument) {
    for(;;) {
    uint32_t adc_val;

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
    	adc_val = HAL_ADC_GetValue(&hadc1);
    	// Escreve diretamente na variável global.
    	g_adc_valor_pot = adc_val;
    }
    HAL_ADC_Stop(&hadc1);
    osDelay(50);
    }
}

void watchdog_check_in(Watchdog_ThreadID_t thread_id) {
	if (thread_id < MONITORED_THREADS_COUNT) {
		osMutexAcquire(g_FuncMutexHandle, osWaitForever);
		g_heartbeat_timestamps[thread_id] = osKernelGetTickCount();
		osMutexRelease(g_FuncMutexHandle);
	}
}

void ThreadLedParesImpares(void *argument) {
	uint8_t func_local = 0;
	uint32_t delay_local = VELOCIDADE_PADRAO;
	for (;;) {
		watchdog_check_in(WD_THREAD_PARES_IMPARES);
		// 1. Pega o valor do handle em segurança
		if (osMutexAcquire(g_FuncMutexHandle, 10) == osOK) {
			func_local = g_func_ativa; // 2. Faz uma cópia RÁPIDA dos valores partilhado
			delay_local = g_velocidade_delay_ms;
			osMutexRelease(g_FuncMutexHandle); // 3.  libera o mutex IMEDIATAMENTE

			if (func_local == 1) {
				//acende leds pares
				HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_6_GPIO_Port, LED_6_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_7_GPIO_Port, LED_7_Pin, GPIO_PIN_SET);
				//HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_RESET);

				osDelay(delay_local);

				//acende impares
				HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_6_GPIO_Port, LED_6_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_7_GPIO_Port, LED_7_Pin, GPIO_PIN_RESET);
				//HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_SET);

				osDelay(delay_local);
			} else {
				osDelay(100);
			}
		} else {
			// Se o mutex não foi adquirido, esperar um pouco antes de tentar de novo.
			osDelay(10);
		}
	}
}

void ThreadLedGrayCode(void *argument) {
	uint8_t func_local;
	uint32_t delay_local;
	const uint8_t gray_code_sequence[] = { 0b0000, 0b0001, 0b0011, 0b0010,
			0b0110, 0b0111, 0b0101, 0b0100, 0b1100, 0b1101, 0b1111, 0b1110,
			0b1010, 0b1011, 0b1001, 0b1000 };
	uint8_t i = 0;

	for (;;) {
		watchdog_check_in(WD_THREAD_GRAY_CODE);
		if (osMutexAcquire(g_FuncMutexHandle, 10) == osOK) {
			func_local = g_func_ativa;
			delay_local = g_velocidade_delay_ms;
			osMutexRelease(g_FuncMutexHandle);

			if (func_local == 2) {
				uint8_t pattern = gray_code_sequence[i];

				HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin,
						(pattern & 0b0001) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin,
						(pattern & 0b0010) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin,
						(pattern & 0b0100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin,
						(pattern & 0b1000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				i = (i + 1) % 16;
				osDelay(delay_local);
			} else
				osDelay(100);
		}
	}
}

void ThreadPotenciometro(void *argument) {

	uint8_t func_local;
	uint32_t adc_valor = 0;

	for (;;) {
		watchdog_check_in(WD_THREAD_POTENCIOMETRO);
		if (osMutexAcquire(g_FuncMutexHandle, 10) == osOK) {
			func_local = g_func_ativa;
			adc_valor = g_adc_valor_pot;
			osMutexRelease(g_FuncMutexHandle);
			if (func_local == 3) {
				//valor que vai adquirir é de 0 a 4095, entao 4096/8 = 512
				g_leds_acesos = adc_valor / 512;
				if (g_leds_acesos > 8) {
					g_leds_acesos = 8;
				}

				DesligaAllLeds();
				if (g_leds_acesos >= 1)
					HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET);
				if (g_leds_acesos >= 2)
					HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET);
				if (g_leds_acesos >= 3)
					HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_SET);
				if (g_leds_acesos >= 4)
					HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, GPIO_PIN_SET);
				if (g_leds_acesos >= 5)
					HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, GPIO_PIN_SET);
				if (g_leds_acesos >= 6)
					HAL_GPIO_WritePin(LED_6_GPIO_Port, LED_6_Pin, GPIO_PIN_SET);
				if (g_leds_acesos >= 7)
					HAL_GPIO_WritePin(LED_7_GPIO_Port, LED_7_Pin, GPIO_PIN_SET);
				/*			if (g_leds_acesos >= 8)
				 HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_SET);
				 */
				osDelay(50);
			} else {
				// Se a função 3 não está ativa, zera os leds para o buzzer parar
				if (g_leds_acesos != 0)
					g_leds_acesos = 0;
				osDelay(100);
			}
		} else {
			osDelay(10);
		}
	}
}

void ThreadKnightRider(void *argument) {
	uint8_t func_local;
	uint32_t delay_local;

	// Arrays para facilitar o controle dos LEDs em sequência
	// A ordem aqui deve corresponder à disposição física dos LEDs (1 a 7)
	static GPIO_TypeDef *ports[NUM_LEDS] = { LED_1_GPIO_Port,
			LED_2_GPIO_Port,
			LED_3_GPIO_Port, LED_4_GPIO_Port,
			LED_5_GPIO_Port, LED_6_GPIO_Port, LED_7_GPIO_Port };
	static uint16_t pins[NUM_LEDS] = {
	LED_1_Pin, LED_2_Pin, LED_3_Pin, LED_4_Pin,
	LED_5_Pin, LED_6_Pin, LED_7_Pin };

	for (;;) {
		osMutexAcquire(g_FuncMutexHandle, osWaitForever);
		func_local = g_func_ativa;
		delay_local = g_velocidade_delay_ms / 4; // Dividido por 4 para um efeito mais rápido e fluido
		osMutexRelease(g_FuncMutexHandle);

		watchdog_check_in(WD_THREAD_KNIGHT_RIDER);

		if (func_local == 4) {
			// Movimento da esquerda para a direita (LED 1 ao 7)
			for (int i = 0; i < NUM_LEDS; i++) {
				HAL_GPIO_WritePin(ports[i], pins[i], GPIO_PIN_SET);
				osDelay(delay_local);
				// Não apaga o LED imediatamente para criar um pequeno rasto
				if (i > 0)
					HAL_GPIO_WritePin(ports[i - 1], pins[i - 1],
							GPIO_PIN_RESET);
				else
					HAL_GPIO_WritePin(ports[NUM_LEDS - 1], pins[NUM_LEDS - 1],
							GPIO_PIN_RESET);
			}

			// Movimento contrario
			for (int i = NUM_LEDS - 1; i >= 0; i--) {
				HAL_GPIO_WritePin(ports[i], pins[i], GPIO_PIN_SET);
				osDelay(delay_local);

				if (i < NUM_LEDS - 1)
					HAL_GPIO_WritePin(ports[i + 1], pins[i + 1],
							GPIO_PIN_RESET);
				else
					HAL_GPIO_WritePin(ports[0], pins[0], GPIO_PIN_RESET);
			}

		} else {
			osDelay(200);
		}
	}
}

void ThreadBinaryCounter(void *argument) {
	uint8_t func_local;
	uint32_t delay_local;
	uint8_t contador = 0;

	GPIO_TypeDef *ports[NUM_LEDS] = {
	LED_1_GPIO_Port, LED_2_GPIO_Port, LED_3_GPIO_Port, LED_4_GPIO_Port,
	LED_5_GPIO_Port, LED_6_GPIO_Port, LED_7_GPIO_Port };
	uint16_t pins[NUM_LEDS] = {
	LED_1_Pin, LED_2_Pin, LED_3_Pin, LED_4_Pin,
	LED_5_Pin, LED_6_Pin, LED_7_Pin };

	for (;;) {
		watchdog_check_in(WD_THREAD_BINARY_COUNTER);

		osMutexAcquire(g_FuncMutexHandle, osWaitForever);
		func_local = g_func_ativa;
		delay_local = g_velocidade_delay_ms;
		osMutexRelease(g_FuncMutexHandle);

		if (func_local == 5) {
			for (int i = 0; i < NUM_LEDS; i++) {
				// Usa uma máscara de bits para verificar se o bit 'i' está ligado
				if ((contador >> i) & 0x01) {
					// Se o bit for 1, acende o LED correspondente
					HAL_GPIO_WritePin(ports[i], pins[i], GPIO_PIN_SET);
				} else {
					// Se o bit for 0, apaga o LED correspondente
					HAL_GPIO_WritePin(ports[i], pins[i], GPIO_PIN_RESET);
				}
			}

			contador++;
			osDelay(delay_local);

		} else {
			contador = 0;
			osDelay(200);
		}
	}
}

void ThreadBuzzer(void *argument) {
	uint8_t leds_local = 0;
	uint8_t modo_sonoro_local = 0;
	uint32_t pulse_value = 0;
	// Frequências base (Hz)
	const uint16_t frequencias[] = { 0,    // 0 LEDs
			261,  // C4 (Dó)
			293,  // D4 (Ré)
			329,  // E4 (Mi)
			349,  // F4 (Fá)
			392,  // G4 (Sol)
			440,  // A4 (Lá)
			493,  // B4 (Si)
			523   // C5 (Dó)
			};

	for (;;) {
		// 1. PRIORIDADE MÁXIMA: VERIFICAÇÃO DE ERRO NO SISTEMA
		// =================================================================
		if (g_sistema_em_erro) {
			const uint32_t TOM_SIRENE_1 = 988; // Nota B5
			const uint32_t TOM_SIRENE_2 = 880; // Nota A5

			while (1) {
				pulse_value = (72000000 / TOM_SIRENE_1) / 2;
				__HAL_TIM_SET_AUTORELOAD(&htim1, (72000000 / TOM_SIRENE_1));
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_value);
				osDelay(150);

				pulse_value = (72000000 / TOM_SIRENE_2) / 2;
				__HAL_TIM_SET_AUTORELOAD(&htim1, (72000000 / TOM_SIRENE_2));
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_value);
				osDelay(150);
			}
		}

		// =================================================================
		// 2. LÓGICA DE FUNCIONAMENTO NORMAL (se não houver erro)
		if (osMutexAcquire(g_FuncMutexHandle, 10) == osOK) {
			leds_local = g_leds_acesos;
			modo_sonoro_local = g_modo_sonoro;
			osMutexRelease(g_FuncMutexHandle);

			if (g_func_ativa == 3 && modo_sonoro_local == 1) {
				if (leds_local > 0) {
					// SystemCoreClock é 72MHz. A fórmula é (Clock / Frequencia_Desejada)
					// prescaler para caber nos 16 bits do registrador
					pulse_value = (72000000 / frequencias[leds_local]) / 2;
					__HAL_TIM_SET_AUTORELOAD(&htim1, pulse_value * 2); //Esta macro ajusta o período do timer
					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_value); // Esta macro ajusta  a "largura" do pulso
				} else {
					// Silencia o buzzer se nenhum LED estiver aceso
					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
				}
			} else {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
			}
		}
		osDelay(50); // Sincroniza com a ThreadPotenciometro
	}
}

void DesligaAllLeds(void) {
	HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_6_GPIO_Port, LED_6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_7_GPIO_Port, LED_7_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_RESET);
}

void ThreadStatusLed(void *argument) {
	uint32_t timestamp_local;
	uint8_t modo_sonoro_local;
	uint8_t evento_local;
	const uint32_t PERIODO_FEEDBACK_MS = 1000; // Duração do feedback de atividade

	for (;;) {

		// PRIORIDADE MÁXIMA: VERIFICAÇÃO DE ERRO NO SISTEMA
		if (g_sistema_em_erro) {
			while (1) {
				HAL_GPIO_TogglePin(LED_8_GPIO_Port, LED_8_Pin);
				osDelay(100);
			}
		}
		//Caso nao haja erro:
		osMutexAcquire(g_FuncMutexHandle, osWaitForever);
		timestamp_local = g_timestamp_ultima_atividade;
		modo_sonoro_local = g_modo_sonoro;
		evento_local = g_evento_troca_funcao;
		osMutexRelease(g_FuncMutexHandle);

		if (evento_local > 0) {
			HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_RESET);
			osDelay(100);

			if (evento_local == 1) { // Função 1: Um piscar longo
				HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_SET);
				osDelay(750);
				HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_RESET);
			} else { // Funções 2 e 3: Piscares rápidos
				for (int i = 0; i < evento_local; i++) {
					HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_SET);
					osDelay(150);
					HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin,
							GPIO_PIN_RESET);
					osDelay(150);
				}
			}

			osMutexAcquire(g_FuncMutexHandle, osWaitForever);
			g_evento_troca_funcao = 0;
			osMutexRelease(g_FuncMutexHandle);
		}
		// 2. Se não houve evento, verifica a atividade recente
		else if ((osKernelGetTickCount() - timestamp_local)
				< PERIODO_FEEDBACK_MS) {
			HAL_GPIO_TogglePin(LED_8_GPIO_Port, LED_8_Pin);
			osDelay(50);
		}
		// 3. Se não, verifica o modo mudo
		else if (modo_sonoro_local == 0) {
			HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_SET);
			osDelay(400);
			HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_RESET);
			osDelay(4000);
		}
		// 4. Se não, executa o "heartbeat" normal
		else {
			HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_SET);
			osDelay(200);
			HAL_GPIO_WritePin(LED_8_GPIO_Port, LED_8_Pin, GPIO_PIN_RESET);
			osDelay(2000);
		}
	}
}

void ThreadGestoraDeEstado(void *argument) {
	SystemEvent_t event;
	osStatus_t status;

	for (;;) {
		status = osMessageQueueGet(g_eventQueueHandle, &event, NULL,
				osWaitForever);

		if (status == osOK) {
			//osMutexAcquire(g_FuncMutexHandle, osWaitForever);

			switch (event.command) {
			case CMD_SET_MODE:
                g_func_ativa = event.value;
                g_evento_troca_funcao = event.value; // Para o LED de status
                if (event.value != 3) { // Desliga LEDs se não for o modo pote
                   DesligaAllLeds();
                }
                break;
			    break;

			case CMD_TOGGLE_SOUND:
				g_modo_sonoro = !g_modo_sonoro;
				break;

			case CMD_SPEED_UP:
				g_velocidade_alvo_ms /= 2;
				if (g_velocidade_alvo_ms < 50)
					g_velocidade_alvo_ms = 50;
				g_suavizacao_ativa = true;
				break;

			case CMD_SPEED_DOWN:
				g_velocidade_alvo_ms *= 2;
				if (g_velocidade_alvo_ms > 4000)
					g_velocidade_alvo_ms = 4000;
				g_suavizacao_ativa = true;
				break;

			case CMD_SPEED_RESET:
				g_velocidade_alvo_ms = VELOCIDADE_PADRAO;
				g_suavizacao_ativa = true;
				break;

			case CMD_SPEED_FROM_POT:
			{
			    uint32_t adc_valor_lido;
			    uint32_t nova_velocidade;

			    adc_valor_lido = g_adc_valor_pot;

			    nova_velocidade = 100 + (adc_valor_lido * 1900) / 4095;

			    g_velocidade_delay_ms = nova_velocidade;
			    g_velocidade_alvo_ms = nova_velocidade;
			    g_suavizacao_ativa = false;
			}
			break;


			case CMD_SPEED_PRESET_L:
				g_velocidade_alvo_ms = 250;
				g_suavizacao_ativa = true;
				break;

			case CMD_SPEED_PRESET_M:
				g_velocidade_alvo_ms = 2000;
				g_suavizacao_ativa = true;
				break;

			case CMD_SPEED_PRESET_N:
				g_velocidade_alvo_ms = 10000;
				g_suavizacao_ativa = true;
				break;

			}

			//osMutexRelease(g_FuncMutexHandle);
		}
	}
}

void ThreadControleBot(void *argument) {

	SystemEvent_t event;
	for (;;) {
		// A thread dorme aqui até que uma interrupção de botão envie um sinal
		uint32_t flags = osThreadFlagsWait(0x0000FFFFU, osFlagsWaitAny,
		osWaitForever);

		if (flags & SINAL_BOTAO_C) {
			event.command = CMD_SET_MODE;
			event.value = 1;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		} else if (flags & SINAL_BOTAO_D) {
			event.command = CMD_SET_MODE;
			event.value = 2;
				osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		} else if (flags & SINAL_BOTAO_E) {
			event.command = CMD_SET_MODE;
			event.value = 3;
				osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		} else if (flags & SINAL_BOTAO_Y) {
			event.command = CMD_SPEED_UP;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		} else if (flags & SINAL_BOTAO_A) {
			event.command = CMD_SPEED_DOWN;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		} else if (flags & SINAL_BOTAO_X) {
			event.command = CMD_SPEED_RESET;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		} else if (flags & SINAL_BOTAO_B) {
			event.command = CMD_SPEED_FROM_POT;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
	}
}

void ThreadPollingBotoes(void *argument) {
	SystemEvent_t event;

	// Variáveis para guardar o estado anterior de cada botão (para debounce)
	GPIO_PinState estadoAnterior_BOT_F = GPIO_PIN_SET;
	GPIO_PinState estadoAnterior_BOT_G = GPIO_PIN_SET;
	GPIO_PinState estadoAnterior_BOT_H = GPIO_PIN_SET;
	GPIO_PinState estadoAnterior_BOT_L = GPIO_PIN_SET;
	GPIO_PinState estadoAnterior_BOT_M = GPIO_PIN_SET;
	GPIO_PinState estadoAnterior_BOT_N = GPIO_PIN_SET;

	for (;;) {
		osDelay(40);

		//verifica os botões conflituosos.
		GPIO_PinState estadoAtual_BOT_F = HAL_GPIO_ReadPin(BOT_F_GPIO_Port,
		BOT_F_Pin);
		GPIO_PinState estadoAtual_BOT_G = HAL_GPIO_ReadPin(BOT_G_GPIO_Port,
		BOT_G_Pin);
		GPIO_PinState estadoAtual_BOT_H = HAL_GPIO_ReadPin(BOT_H_GPIO_Port,
		BOT_H_Pin);
		GPIO_PinState estadoAtual_BOT_L = HAL_GPIO_ReadPin(BOT_N_GPIO_Port,
		BOT_L_Pin);
		GPIO_PinState estadoAtual_BOT_M = HAL_GPIO_ReadPin(BOT_N_GPIO_Port,
		BOT_M_Pin);
		GPIO_PinState estadoAtual_BOT_N = HAL_GPIO_ReadPin(BOT_N_GPIO_Port,
		BOT_N_Pin);

		// Lógica de Debounce e Envio de Sinal
		if (estadoAnterior_BOT_F == GPIO_PIN_SET
				&& estadoAtual_BOT_F == GPIO_PIN_RESET) {
			event.command = CMD_TOGGLE_SOUND;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
		estadoAnterior_BOT_F = estadoAtual_BOT_F;

		if (estadoAnterior_BOT_G == GPIO_PIN_SET
				&& estadoAtual_BOT_G == GPIO_PIN_RESET) {
			event.command = CMD_SET_MODE;
			event.value = 4;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
		estadoAnterior_BOT_G = estadoAtual_BOT_G;

		if (estadoAnterior_BOT_H == GPIO_PIN_SET
				&& estadoAtual_BOT_H == GPIO_PIN_RESET) {
			event.command = CMD_SET_MODE;
			event.value = 5;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
		estadoAnterior_BOT_H = estadoAtual_BOT_H;

		if (estadoAnterior_BOT_L == GPIO_PIN_SET
				&& estadoAtual_BOT_L == GPIO_PIN_RESET) {
			event.command = CMD_SPEED_PRESET_L;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
		estadoAnterior_BOT_L = estadoAtual_BOT_L;

		if (estadoAnterior_BOT_M == GPIO_PIN_SET
				&& estadoAtual_BOT_M == GPIO_PIN_RESET) {
			event.command = CMD_SPEED_PRESET_M;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
		estadoAnterior_BOT_M = estadoAtual_BOT_M;

		if (estadoAnterior_BOT_N == GPIO_PIN_SET
				&& estadoAtual_BOT_N == GPIO_PIN_RESET) {
			event.command = CMD_SPEED_PRESET_N;
			osMessageQueuePut(g_eventQueueHandle, &event, 0, 0);
		}
		estadoAnterior_BOT_N = estadoAtual_BOT_N;
	}
}

// --- THREAD DE SUAVIZAÇÃO CORRIGIDA E SIMPLIFICADA ---
void ThreadSuavizacao(void *argument) {
	const uint32_t PASSO_SUAVIZACAO = 50; // Aumente este valor para transições mais rápidas

	for (;;) {
		osMutexAcquire(g_FuncMutexHandle, osWaitForever);

		if (g_suavizacao_ativa) {
			if (g_velocidade_delay_ms < g_velocidade_alvo_ms) {
				g_velocidade_delay_ms += PASSO_SUAVIZACAO;

				if (g_velocidade_delay_ms > g_velocidade_alvo_ms) {
					g_velocidade_delay_ms = g_velocidade_alvo_ms;
				}
			} else if (g_velocidade_delay_ms > g_velocidade_alvo_ms) {
				g_velocidade_delay_ms -= PASSO_SUAVIZACAO;

				if (g_velocidade_delay_ms < g_velocidade_alvo_ms) {
					g_velocidade_delay_ms = g_velocidade_alvo_ms;
				}
			}
		}
		osMutexRelease(g_FuncMutexHandle);
		osDelay(50);
	}
}

void ThreadWatchdog(void *argument) {
	const uint32_t TIMEOUT_MS = 25000; // 25 segundos de tolerância

	// Inicializa os timestamps para evitar falsos positivos no arranque
	for (int i = 0; i < MONITORED_THREADS_COUNT; i++) {
		watchdog_check_in(i);
	}

	for (;;) {
		osDelay(2000);

		for (int i = 0; i < MONITORED_THREADS_COUNT; i++) {
			uint32_t ultimo_heartbeat;

			osMutexAcquire(g_FuncMutexHandle, osWaitForever);
			ultimo_heartbeat = g_heartbeat_timestamps[i];
			osMutexRelease(g_FuncMutexHandle);

			// Verifica há quanto tempo a thread 'i' não faz "check-in"
			if ((osKernelGetTickCount() - ultimo_heartbeat) > TIMEOUT_MS) {
				g_sistema_em_erro = true;

				// Adicione um log aqui se tiver UART/CLI para saber qual thread falhou
				// printf("ERRO: Watchdog timeout na thread ID %d\r\n", i);

				break;
			}
		}

		if (g_sistema_em_erro) {
			// Se o erro foi detectado, a thread watchdog pode parar de verificar
			osThreadTerminate(NULL);
		}
	}
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {

	g_sistema_em_erro = true;
	/* Este código é executado se a stack de uma tarefa estourar. */
	__asm("BKPT #0");
	/* Coloque um breakpoint */
	while (1)
		;
}
