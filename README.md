# Estudos-CMSIS

Repositório dedicado ao estudo de desenvolvimento de firmware utilizando o padrão **CMSIS (Cortex Microcontroller Software Interface Standard)** com acesso direto aos registradores dos microcontroladores, sem utilização de bibliotecas de alto nível como a HAL.

Os projetos têm como objetivo explorar a arquitetura dos periféricos, implementação de drivers reutilizáveis e boas práticas de organização de software para sistemas embarcados.

## Plataformas utilizadas

Atualmente os estudos são desenvolvidos nas seguintes placas:

- **STMicroelectronics NUCLEO-F446RE**
  - Microcontrolador STM32F446RE (ARM Cortex-M4)
- **Texas Instruments EK-TM4C1294XL**
  - Microcontrolador TM4C1294NCPDT (ARM Cortex-M4F)

---

## I2C-Protocol

Este projeto implementa uma arquitetura de firmware para aquisição de dados de sensores conectados ao barramento I²C. A aplicação é organizada em módulos independentes, onde a `main()` é responsável apenas pela inicialização do hardware e pelo fluxo principal do programa. Um módulo de gerenciamento centraliza a aquisição dos dados, enquanto cada sensor possui seu próprio driver contendo toda a lógica de configuração, leitura dos registradores e conversão dos dados para grandezas físicas.

A comunicação com os dispositivos é realizada por meio de um driver genérico de I²C, que fornece funções de leitura e escrita reutilizáveis para qualquer periférico compatível com o protocolo. O projeto utiliza um sensor BME280 para leitura de temperatura, umidade e pressão atmosférica, e um sensor BH1750 para medição de luminosidade. Após cada ciclo de aquisição, as medições são armazenadas em uma estrutura compartilhada e transmitidas pela UART para visualização em um monitor serial.

---

## Direct-Memory-Access

Este projeto implementa um driver genérico para o controlador DMA, permitindo configurar transferências entre memória e periféricos por meio de uma interface única. O driver abstrai a configuração dos controladores, streams, canais e interrupções, podendo ser reutilizado por diferentes periféricos suportados pelo microcontrolador.

Como aplicação, o ADC realiza conversões periódicas e o DMA transfere automaticamente cada amostra para um buffer em memória, eliminando a necessidade de a CPU copiar os dados manualmente. Enquanto as conversões e transferências ocorrem, o processador permanece livre para executar outras rotinas do programa. Ao final da transferência, a interrupção do DMA informa que o buffer está pronto para processamento, e os resultados são enviados pela UART para um computador. Dessa forma, o fluxo da aplicação é composto pela aquisição dos dados via ADC, armazenamento em memória através do DMA, processamento realizado pela CPU e transmissão das informações pela interface serial.

---

## PWM-Control

**TODO**

---

## CAN-Communication

**TODO**

---

## Ethernet-Connection

**TODO**

---

# Operações de bits

```c
REG |=  (1 << n);   // seta bit n
REG &= ~(1 << n);   // limpa bit n
REG ^=  (1 << n);   // inverte bit n
REG &   (1 << n);   // testa bit n

REG |= MASK;        // seta bits
REG &= ~MASK;       // limpa bits
REG ^= MASK;        // alterna bits
REG & MASK;         // testa bits

REG &= ~MASK;
REG |= VALUE;       // escreve campo

(REG >> POS) & MASK;              // lê campo
REG = (REG & ~MASK) | VALUE;      // substitui campo
```