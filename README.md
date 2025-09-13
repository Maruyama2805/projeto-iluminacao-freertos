# 💡 Sistema de Iluminação Inteligente com STM32 e FreeRTOS

![Status](https://img.shields.io/badge/status-concluído-brightgreen)
![Plataforma](https://img.shields.io/badge/plataforma-STM32F103-blue)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS%20(CMSIS--OS)-orange)

Um sistema embarcado multifuncional para controlo de iluminação, construído com FreeRTOS. Demonstra a gestão de múltiplas threads, comunicação inter-thread com Fila de Mensagens, e controlo de periféricos em tempo real. Este projeto foi desenvolvido com base na proposta de trabalho final para a disciplina de **ECOS03 - Sistemas Operacionais Embarcados**.

---

### 🎬 Demonstração em Ação

![GIF do Projeto a Funcionar]("ProjetoSOE2025-ezgif.com-optimize.gif")

---

### ✨ Funcionalidades Principais

* 🌀 **5 Padrões de Animação:** Incluindo Pares/Ímpares, Código Gray, Knight Rider e Contador Binário.
* 🚀 **Controlo de Velocidade Avançado:** Com ajuste dinâmico, presets e um algoritmo de **suavização de transição** para mudanças de velocidade graduais.
* 🔊 **Feedback Audiovisual Completo:** Um buzzer com tons variáveis e um **LED de status dedicado** para feedback de modo, atividade e erros críticos do sistema.
* 🏗️ **Arquitetura RTOS Robusta:** Utiliza **Fila de Mensagens** para gestão de estado, uma thread "Guardiã" para acesso seguro ao ADC, e um **Watchdog por Software** para monitorizar a saúde do sistema.

---

### 🛠️ Tecnologias e Periféricos

* **Hardware:**
    * Microcontrolador: STM32F103C8T6 (Placa BluePill/RedPill)
    * Periféricos: 8x LEDs, 14x Botões, Potenciómetro, Buzzer
* **Software:**
    * Linguagem: C
    * IDE: STM32CubeIDE
    * RTOS: FreeRTOS (com a camada de abstração CMSIS-OS v2)
    * Drivers: STM32 HAL

---

### 📂 Detalhes Técnicos e Diagrama de Pinos

<details>
<summary><strong>Clique para expandir e ver a arquitetura do software e a pinagem completa...</strong></summary>

### Arquitetura de Software
O sistema foi construído seguindo práticas de design robustas para sistemas de tempo real:
* **Fila de Mensagens:** As threads de input (`ThreadControleBot`, `ThreadPollingBotoes`) não modificam o estado diretamente. Elas produzem "eventos" e os enviam para uma fila central.
* **Gestor de Estado:** Uma única thread (`ThreadGestoraDeEstado`) consome os eventos da fila e é a única responsável por alterar as variáveis de estado globais, prevenindo condições de corrida.
* **Thread Guardiã:** A `ThreadADCReader`, com alta prioridade, é a única thread que acede ao hardware do ADC, lendo o valor do potenciómetro continuamente e disponibilizando-o de forma segura para o resto do sistema.
* **Sincronização:** Mutexes (`g_FuncMutexHandle`) são usados pelas threads "leitoras" para garantir que acedem a um estado consistente.
* **Modularidade:** Cada funcionalidade principal (animações, suavização, watchdog, status) é encapsulada na sua própria thread, facilitando a manutenção e expansão.

### Pinagem do Hardware
A tabela abaixo detalha a ligação física dos periféricos aos pinos do microcontrolador.

| Periférico | Pino Físico | Definição no Código |
| :--- | :--- | :--- |
| **LED 1** | PA0 | `LED_1_Pin` |
| **LED 2** | PA1 | `LED_2_Pin` |
| **LED 3** | PA2 | `LED_3_Pin` |
| **LED 4** | PA15 | `LED_4_Pin` |
| **LED 5** | PA8 | `LED_5_Pin` |
| **LED 6** | PA6 | `LED_6_Pin` |
| **LED 7** | PA5 | `LED_7_Pin` |
| **LED 8 (Status)** | PA11 | `LED_8_Pin` |
| **Potenciómetro** | PB1 | `POT_Pin` |
| **Buzzer** | PB0 | `BUZZ_Pin` (TIM1_CH2N) |
| **Botão C** | PB5 | `BOT_C_Pin` |
| **Botão D** | PB4 | `BOT_D_Pin` |
| **Botão E** | PB3 | `BOT_E_Pin` |
| **Botão F** | PA3 | `BOT_F_Pin` |
| **Botão G** | PA4 | `BOT_G_Pin` |
| **Botão H** | PB8 | `BOT_H_Pin` |
| **Botão Y** | PB15 | `BOT_Y_Pin` |
| **Botão A** | PB13 | `BOT_A_Pin` |
| **Botão X** | PB14 | `BOT_X_Pin` |
| **Botão B** | PB12 | `BOT_B_Pin` |
| **Botão L** | PC15 | `BOT_L_Pin` |
| **Botão M** | PC14 | `BOT_M_Pin` |
| **Botão N** | PC13 | `BOT_N_Pin` |

</details>

---

### 🚀 Como Executar o Projeto

1.  Clone este repositório: `git clone https://github.com/Maruyama2005/projeto-iluminacao-freertos.git`
2.  Abra o **STM32CubeIDE** e importe o projeto (`File > Import > General > Existing Projects into Workspace`).
3.  Compile o projeto clicando no ícone do martelo (Build).
4.  Grave o firmware na sua placa STM32 utilizando um programador ST-Link.

---

### 📄 Licença

Distribuído sob a licença MIT. Veja o ficheiro `LICENSE` para mais detalhes.
