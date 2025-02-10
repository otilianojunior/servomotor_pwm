#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Define os pinos utilizados:
#define GPIO_SERVOMOTOR 22   // Pino para controle do servomotor via PWM
#define GPIO_LED        12   // Pino para o LED RGB

// Configurações do PWM para o servomotor:
#define FREQ_PWM_HZ       50           // Frequência do PWM (50 Hz -> período de 20 ms)
#define PERIODO_PWM_US    20000        // Período em microsegundos (20 ms)
#define DIVISOR_PWM       125.0f       // Divisor para que 1 tick = 1 µs (125 MHz / 125 = 1 MHz)

// Pulsos para as posições (em µs)
#define PULSO_180_US 2400  // Aproximadamente 180° (12% de 20 ms)
#define PULSO_90_US  1470  // Aproximadamente 90° (7,35% de 20 ms)
#define PULSO_0_US   500   // Aproximadamente 0° (2,5% de 20 ms)

// Parâmetros para varredura suave
#define PULSO_PASSO_US 5     // Incremento/decremento de 5 µs por passo
#define ATRASO_PASSO_MS 10   // Atraso de 10 ms entre os passos

int main() {
    // Inicializa as stdio (útil para debug via USB)
    stdio_init_all();

    // 1. Configuração do PWM na GPIO_SERVOMOTOR:
    gpio_set_function(GPIO_SERVOMOTOR, GPIO_FUNC_PWM);
    uint slice_pwm = pwm_gpio_to_slice_num(GPIO_SERVOMOTOR);
    uint canal_pwm  = pwm_gpio_to_channel(GPIO_SERVOMOTOR);

    // Configura o divisor para que cada tick valha 1 µs
    pwm_set_clkdiv(slice_pwm, DIVISOR_PWM);
    // Define o valor de wrap para um período de 20.000 µs (20 ms)
    pwm_set_wrap(slice_pwm, PERIODO_PWM_US - 1);
    // Inicializa com duty cycle 0
    pwm_set_chan_level(slice_pwm, canal_pwm, 0);
    pwm_set_enabled(slice_pwm, true);

    // Configura o LED na GPIO_LED como saída digital
    gpio_init(GPIO_LED);
    gpio_set_dir(GPIO_LED, GPIO_OUT);

    // 2. Posição 180°: pulso de 2400 µs
    printf("Servo para 180° (2400 µs)\n");
    pwm_set_chan_level(slice_pwm, canal_pwm, PULSO_180_US);
    sleep_ms(5000);

    // 3. Posição 90°: pulso de 1470 µs
    printf("Servo para 90° (1470 µs)\n");
    pwm_set_chan_level(slice_pwm, canal_pwm, PULSO_90_US);
    sleep_ms(5000);

    // 4. Posição 0°: pulso de 500 µs
    printf("Servo para 0° (500 µs)\n");
    pwm_set_chan_level(slice_pwm, canal_pwm, PULSO_0_US);
    sleep_ms(5000);

    // 5. Varredura suave entre 0° e 180°
    int pulso = PULSO_0_US;       // Inicia com 0° (500 µs)
    int pulso_min = PULSO_0_US;
    int pulso_max = PULSO_180_US;
    bool incremento = true;

    // Variável auxiliar para controle do LED (pisca a cada 100 ms aproximadamente)
    int contador_led = 0;

    printf("Iniciando varredura suave entre 0° e 180°...\n");
    while (true) {
        // Atualiza o duty cycle para o servomotor
        pwm_set_chan_level(slice_pwm, canal_pwm, pulso);

        // Atualiza o LED: a cada 10 iterações
        contador_led++;
        if (contador_led >= 10) {
            gpio_xor_mask(1u << GPIO_LED);  // Inverte o estado do LED
            contador_led = 0;
        }

        sleep_ms(ATRASO_PASSO_MS);

        // Atualiza o valor do pulso de forma suave:
        if (incremento) {
            pulso += PULSO_PASSO_US;
            if (pulso >= pulso_max) {
                pulso = pulso_max;
                incremento = false;
            }
        } else {
            pulso -= PULSO_PASSO_US;
            if (pulso <= pulso_min) {
                pulso = pulso_min;
                incremento = true;
            }
        }
    }

    return 0;
}
