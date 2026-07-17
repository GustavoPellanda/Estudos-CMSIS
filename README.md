# Estudos-CMSIS
Estudos do padrão CMSIS com STM32 e Tiva Launchpad

## Operações de bits

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
