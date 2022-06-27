#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
#include "driverlib/timer.h"
#include "utils/ustdlib.h"
#include "driverlib/hibernate.h"

#define GPIO_O_LOCK                 0x520
#define GPIO_O_CR                   0x524
#define GPIO_LOCK_KEY               0x4C4F434B
#define GPIO_O_PUR                  0x510
#define ESC_REG(x)                  (*((volatile uint32_t *)(x)))

#define SYSCTL_RCGC2_GPIOF          0x00000020
#define GPIOHBCTL                   0x400FE06C
//clock
#define SYSCTL_RCGC2_R              0x400FE108 // leitura dummy
#define SYSCTL_RCGCGPIO             0x400FE608 // usada na habilita clockGPIO




//defines Funcoes
//HabilitaPortal
#define HabPortalA                  0x01
#define HabPortalB                  0x02
#define HabPortalC                  0x04
#define HabPortalD                  0x08
#define HabPortalE                  0x10
#define HabPortalF                  0x20

//Registradores
#define SYSCTL_RCGCGPIO             0x400FE608
#define GPIO_OS_DIR                 0x400
#define GPIO_OS_DEN                 0x51C
#define GPIO_OS_DR2R                0x500
#define GPIO_OS_DR4R                0x504
#define GPIO_OS_DR8R                0x508
#define GPIO_OS_SLR                 0x518
#define GPIO_OS_PULLUP              0x510
#define GPIO_OS_DATA                0x000

#define GPIOPortA_base              0x40004000
#define GPIOPortB_base              0x40005000
#define GPIOPortC_base              0x40006000
#define GPIOPortD_base              0x40007000
#define GPIOPortE_base              0x40024000
#define GPIOPortF_base              0x40025000

//ConfiguraPinoSaida
#define Pino_0                      0x01
#define Pino_1                      0x02
#define Pino_2                      0x04
#define Pino_3                      0x08
#define Pino_4                      0x10
#define Pino_5                      0x20
#define Pino_6                      0x40
#define Pino_7                      0x80

// configuracoes entrada/saida
#define GPIO_O_DIR                  0x400
#define GPIO_O_DR2R                 0x500
#define GPIO_O_DEN                  0x51C
#define GPIO_O_PUR                  0x510
#define GPIO_O_DATA                 0x000

//argumentos para habilitar o clock do portalX
#define portalGPIO_a                0x01
#define portalGPIO_b                0x02
#define portalGPIO_c                0x04
#define portalGPIO_d                0x08
#define portalGPIO_e                0x10
#define portalGPIO_f                0x20

//base dos portais
#define portalA_base                0x40004000
#define portalB_base                0x40005000
#define portalC_base                0x40006000
#define portalD_base                0x40007000
#define portalE_base                0x40024000
#define portalF_base                0x40025000

//pinos dos portais
#define pino0                       0x01
#define pino1                       0x02
#define pino2                       0x04
#define pino3                       0x08
#define pino4                       0x10
#define pino5                       0x20
#define pino6                       0x40
#define pino7                       0x80

//serve para da permissao de uso do pino0 do portalF
#define GPIO_O_LOCK                 0x520
#define GPIO_O_CR                   0x524
#define GPIO_LOCK_KEY               0x4C4F434B

//Valores Display 7seg
#define sseg_0                      0b00111111
#define sseg_1                      0b00000110
#define sseg_2                      0b01011011
#define sseg_3                      0b01001111
#define sseg_4                      0b01100110
#define sseg_5                      0b01101101
#define sseg_6                      0b01111101
#define sseg_7                      0b00000111
#define sseg_8                      0b01111111
#define sseg_9                      0b01101111
#define sseg_A                      0b01110111
#define sseg_B                      0b01111100
#define sseg_C                      0b00111001
#define sseg_D                      0b01011110
#define sseg_E                      0b01111001
#define sseg_F                      0b01110001
#define sseg_V                      0b00111110
#define sseg_R                      0b01010000
#define sseg_VAZIO                  0b00000000
int tempo;
int botao=0;

//---------------------------------- variaves de controle estados atuais display 7 seg -------------------------//
int como_ta[]= {0,0,0,0};
int ql_vez = 0 ;
int jafoi1 = 0 ;
int jafoi2 = 0 ;
int jafoi3 = 0 ;
int jafoi4 = 0 ;
//----------------------------------  fim variaveis controle estados atuais display 7 seg -------------------------//

void destrava_pino(uint32_t portal_base){
    ESC_REG(portal_base + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    ESC_REG(portal_base + GPIO_O_CR) = 0x01;
}

void bloqueia_GPIO(uint32_t portal){
    ESC_REG(portal + GPIO_O_CR) = 0;
}

//---------------------------------- habilita botoes e interupção portal f E B -------------------------//
void habilita_portas_matriz_botoes(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    destrava_pino(GPIO_PORTF_BASE);
    // Envio de informações ( COLUNAS )
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    // Recebe informações ( LINHAS )
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5);
    // Configurando as portas como pulldown
    ESC_REG(GPIO_PORTF_BASE + GPIO_O_PUR)|= GPIO_PIN_4 ;
    ESC_REG(GPIO_PORTB_BASE + GPIO_O_PUR)|= GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5 ;
   bloqueia_GPIO(GPIO_PORTF_BASE); // se der problema pode ser aqui
 // INTERUP PORTAL F
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_RISING_EDGE);
    IntEnable(INT_GPIOF);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
    IntMasterEnable();
    // INTERUP PORTAL B
       GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5,GPIO_RISING_EDGE);
       IntEnable(INT_GPIOB);
       GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5);
       IntMasterEnable();
}
//---------------------------------- fim habilita botoes e interupção portal f E B -------------------------//

void EscritaPinosPortal (uint32_t PortalBase, uint8_t Pino, uint8_t Valor)
{
    ESC_REG(PortalBase + (GPIO_OS_DATA + (Pino << 2))) = Valor;
}

//----------------------------------escreve em uma posição "digito" um valor "valor" -------------------------//
void EscreveDisplay (uint8_t Digito, uint8_t valor)
{
    switch (Digito)
    {
        case 1:
            EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0b10111111);
            EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0xFF);
            break;
        case 2:
            EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0b01111111);
            EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0xFF);
            break;
        case 3:
            EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0xFF);
            EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0b11111011);
            break;
        case 4:
            EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0xFF);
            EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0b11110111);
            break;
        default:
            break;
    }

    switch (valor)
    {
        case -1:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_VAZIO);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_VAZIO);
            break;
         case 0:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_0);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_0);
            break;
        case 1:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_1);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_1);
            break;
        case 2:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_2);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_2);
            break;
        case 3:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_3);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_3);
            break;
        case 4:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_4);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_4);
            break;
        case 5:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_5);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_5);
            break;
        case 6:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_6);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_6);
            break;
        case 7:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_7);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_7);
            break;
        case 8:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_8);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_8);
            break;
        case 9:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_9);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_9);
            break;
        case 10:
            EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_A);
            EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_A);
            break;
        case 11:
                    EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_B);
                    EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_B);
                    break;
        case 12:
                    EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_C);
                    EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_C);
                    break;
        case 13:
                    EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_D);
                    EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_D);
                    break;
        case 14:
                    EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_E);
                    EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_E);
                    break;

        case 15:
                   EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_F);
                   EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_F);
                   break;


        default:

            break;
    }

}
//----------------------------------fim escreve em uma posição "digito" um valor "valor" -------------------------//
void HabilitaPortal (uint8_t HabPortalX)
{
    ESC_REG(SYSCTL_RCGCGPIO) |= HabPortalX;

}

void ConfiguraPinoSaida (uint32_t PortalBase, uint8_t Pino)
{
    ESC_REG(PortalBase + GPIO_OS_DIR) |= Pino;
    ESC_REG(PortalBase + GPIO_OS_DEN) |= Pino;
    ESC_REG(PortalBase + GPIO_OS_DR2R) |= Pino;
    ESC_REG(PortalBase + GPIO_OS_DR4R) &= ~(Pino);
    ESC_REG(PortalBase + GPIO_OS_DR8R) &= ~(Pino);
    ESC_REG(PortalBase + GPIO_OS_SLR) &= ~(Pino);

}

void ConfiguraPinoEntrada (uint32_t PortalBase, uint8_t Pino)
{
    ESC_REG(PortalBase + GPIO_OS_DIR) &= ~(Pino);
    ESC_REG(PortalBase + GPIO_OS_DEN) |= Pino;
    ESC_REG(PortalBase + GPIO_OS_DR2R) |= Pino;
    ESC_REG(PortalBase + GPIO_OS_DR4R) &= ~(Pino);
    ESC_REG(PortalBase + GPIO_OS_DR8R) &= ~(Pino);
    ESC_REG(PortalBase + GPIO_OS_SLR) &= ~(Pino);

}

//----------------------------------escreve over no display de 7 seg e n aceita mais nenhuma entrada desabilitando a interupção-------------------------//
void faz_over(void){
    IntDisable(INT_GPIOF);
    IntDisable(INT_GPIOB);
    while(1){
        volatile uint32_t ui32Loop;
        for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0b10111111);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0xFF);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_0);
        EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_0);
        for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0b01111111);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0xFF);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_V);
         EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_V);
         for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0xFF);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0b11111011);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_E);
        EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_E);
        for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0xFF);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0b11110111);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_R);
         EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_R);
    }
}
//----------------------------------fim escreve over no display de 7 seg e n aceita mais nenhuma entrada desabilitando a interupção-------------------------//


//----------------------------------converte decimal p hexadecimal -------------------------//
void conversao_decimal_hexa(void) // decimal --> hexa
{
    int numero;
    if(como_ta[0] >9 || (como_ta[1] >9) || (como_ta[0] >9) || como_ta[3] >9)

        {
           return;
        }

    numero = (como_ta[0] * (10 * 10 * 10) + como_ta[1] * (10 * 10) + como_ta[2] * 10 + como_ta[3]);
    como_ta[3] =  numero & 0x0F;//numero % 16;
    como_ta[2] = (numero>>4)&0x0F; //(numero%(16*16)-como_ta[0])/16;
    como_ta[1] = (numero>>8)&0x0F;//(numero %(16*16*16)-como_ta[0]-como_ta[1])/(16*16); //
    como_ta[0] = (numero>>12)&0x0F;//(numero-como_ta[0]-como_ta[1]-como_ta[2])/(16*16*16); //

    if (como_ta[0]==0){
            ql_vez=100;
            jafoi4=0;
        }
        if (como_ta[1]==0 && como_ta[0]==0){
               ql_vez=71;
               jafoi4=0;
               jafoi3=0;
           }
        if (como_ta[2]==0 && como_ta[1]==0 && como_ta[0]==0){
               ql_vez=40;
               jafoi4=0;
               jafoi3=0;
               jafoi2=0;
           }
        if (como_ta[3]==0 && como_ta[2]==0 && como_ta[1]==0 && como_ta[0]==0){
                   ql_vez=0;
                   jafoi4=0;
                   jafoi3=0;
                   jafoi2=0;
                   jafoi1=0;
               }


}
//---------------------------------- fim converte decimal p hexadecimal -------------------------//


//----------------------------------escreve erro no display de 7 seg e n aceita mais nenhuma entrada desabilitando a interupção-------------------------//
void faz_erro(void){
    IntDisable(INT_GPIOF);
    IntDisable(INT_GPIOB);
    while(1){
        volatile uint32_t ui32Loop;
        for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0b10111111);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0xFF);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_E);
        EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_E);
        for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0b01111111);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0xFF);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_R);
         EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_R);
         for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0xFF);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0b11111011);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_R);
        EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_R);
        for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
        EscritaPinosPortal(GPIOPortB_base, Pino_6|Pino_7, 0xFF);
        EscritaPinosPortal(GPIOPortD_base, Pino_2|Pino_3, 0b11110111);
        EscritaPinosPortal(GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3, sseg_0);
         EscritaPinosPortal(GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7, sseg_0);
    }
}
//----------------------------------fim escreve erro no display de 7 seg e n aceita mais nenhuma entrada desabilitando a interupção-------------------------//


//----------------------------------converte hexadecimal para decimal-------------------------//
void conversao_hexa_decimal(void) // hexa --> decimal numero max 270F ~ 9999
{
    int numero;
    if(como_ta[0] >2 || (como_ta[0] ==2 && como_ta[1] >7) || (como_ta[0] ==2 && como_ta[1] ==7 && como_ta[2] >0))

    {
       faz_over();
    }

    numero = como_ta[0] *(16*16*16)+ como_ta[1] *(16*16)+ como_ta[2] *(16)+ como_ta[3] ;

    como_ta[0] =numero/1000;
    numero = numero%1000;

    como_ta[1] =numero/100;
    numero = numero%100;

    como_ta[2] =numero/10;
    numero = numero%10;

    como_ta[3] =numero;


}
//----------------------------------fim converte hexadecimal para decimal-------------------------//


//----------------------------------arruma o vetor pelas variaveis de estados dele--------------------------//
void arruma_o_certo(int novo_val){
    ql_vez++;
    if (ql_vez<=10 && !jafoi1){
           como_ta[3]= novo_val;
           jafoi1++;
       }
    if (ql_vez>=40&& ql_vez<=70 && !jafoi2){
        como_ta[2]=como_ta[3];
        como_ta[3]= novo_val;
        jafoi2++;
    }
    if (ql_vez>=71 && ql_vez<=100 && !jafoi3){
           como_ta[1]=como_ta[2];
           como_ta[2]=como_ta[3];
           como_ta[3]= novo_val;
           jafoi3++;
       }
    if (ql_vez>=100 && ql_vez<=130 && !jafoi4){
               como_ta[0]=como_ta[1];
               como_ta[1]=como_ta[2];
               como_ta[2]=como_ta[3];
               como_ta[3]= novo_val;
               jafoi4++;
           }
    if ( ql_vez>130){
        faz_erro();
    }
    }

//----------------------------------fim arruma o vetor pelas variaveis de estados dele-------------------------//

//---------------------------------- arruma o display de 7 seg de acordo com o vetor de estados dele-------------------------//
void escreve_certo(){
    volatile uint32_t ui32Loop;
    for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
    EscreveDisplay (1, como_ta[0]);
    for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
    EscreveDisplay (2, como_ta[1]);
    for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
    EscreveDisplay (3, como_ta[2]);
    for(ui32Loop = 0; ui32Loop < 2000; ui32Loop++) {  }
    EscreveDisplay (4,como_ta[3]);

}
//----------------------------------fim arruma o display de 7 seg de acordo com o vetor de estados dele-------------------------//


//----------------------------------função que entende o que fazer de acordo com a leitura de botão-------------------------//
void controla_tudo(int botao){
    volatile uint32_t ui32Loop;

            if(botao==-5){
                conversao_hexa_decimal();
                        escreve_certo();
                   for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++) { }
            }
            if(botao==-3){
                conversao_decimal_hexa();
                    escreve_certo();
               for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++) { }
                        }
            if(botao>-1){
                arruma_o_certo(botao);

            }
            escreve_certo();
           // Atraso



}
//----------------------------------fim função que entende o que fazer de acordo com a leitura de botão-------------------------//

//----------------------------------função de interção para ambos os portais, como foi colocado na tm4c-------------------------//
void IntPortalB (void){
    uint32_t ui32Loop;
    for(ui32Loop = 0; ui32Loop < 200; ui32Loop++) { }
    int botao = -1;
    uint32_t portal_linhas[4] = {GPIO_PORTF_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE};
      uint32_t portal_colunas[4] = {GPIO_PORTF_BASE, GPIO_PORTF_BASE, GPIO_PORTF_BASE, GPIO_PORTF_BASE};
      uint32_t pinos_linhas[4] = {GPIO_PIN_4, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_5};
      uint32_t pinos_colunas[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
      int i=0,j=0;
      //for(ui32Loop = 0; ui32Loop < 200; ui32Loop++) {  }
      GPIOPinWrite(portal_colunas[i], pinos_colunas[0]|pinos_colunas[1]|pinos_colunas[2]|pinos_colunas[3], ~(pinos_colunas[i]));
      if(GPIOPinRead(portal_linhas[j], pinos_linhas[j]) == 0x00 && GPIOPinRead(portal_linhas[j+1], pinos_linhas[j+1]) == 0x00){
          controla_tudo(-5);
          for(ui32Loop = 0; ui32Loop < 200; ui32Loop++) { }
      }
      i=3;
      j=2;
      GPIOPinWrite(portal_colunas[i], pinos_colunas[0]|pinos_colunas[1]|pinos_colunas[2]|pinos_colunas[3], ~(pinos_colunas[i]));
      if(GPIOPinRead(portal_linhas[j], pinos_linhas[j]) == 0x00 && GPIOPinRead(portal_linhas[j+1], pinos_linhas[j+1]) == 0x00){
          controla_tudo(-3);
          for(ui32Loop = 0; ui32Loop < 200; ui32Loop++) { }
      }
      for ( i = 0; i < 4; i++){
        GPIOPinWrite(portal_colunas[i], pinos_colunas[0]|pinos_colunas[1]|pinos_colunas[2]|pinos_colunas[3], ~(pinos_colunas[i]));
        for ( j = 0; j < 4; j++){
           if(GPIOPinRead(portal_linhas[j], pinos_linhas[j]) == 0x00 ){
               botao = j*4 + i;
               controla_tudo(botao);
               for(ui32Loop = 0; ui32Loop < 200; ui32Loop++) { }
           }

        }
      }
     controla_tudo(botao);


}
//----------------------------------fim função de interção para ambos os portais, como foi colocado na tm4c-------------------------//


  int main(void){
    volatile uint32_t ui32Loop;

    HabilitaPortal(HabPortalB);
    HabilitaPortal(HabPortalC);
    HabilitaPortal(HabPortalD);
    HabilitaPortal(HabPortalE);
    HabilitaPortal(HabPortalF);

    // Faz leitura dummy para efeito de atraso
    for(ui32Loop = 0; ui32Loop < 200; ui32Loop++) { }

    // Habilita o pino 3 do portal F, configura como saída digital
    ConfiguraPinoSaida (GPIOPortF_base, Pino_0|Pino_3);
    ConfiguraPinoSaida (GPIOPortE_base, Pino_0|Pino_1|Pino_2|Pino_3);
    ConfiguraPinoSaida (GPIOPortD_base, Pino_2|Pino_3);
    ConfiguraPinoSaida (GPIOPortC_base, Pino_4|Pino_5|Pino_6|Pino_7);
    ConfiguraPinoSaida (GPIOPortB_base, Pino_6|Pino_7);

    habilita_portas_matriz_botoes();

    while (1){
      //  controla_tudo();
        escreve_certo();
    }
    // Loop principal

}
