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



---

## Direct-Memory-Access



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