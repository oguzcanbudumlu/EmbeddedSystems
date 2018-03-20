#include "p18f8722.inc"
; CONFIG1H
  CONFIG  OSC = HSPLL, FCMEN = OFF, IESO = OFF
; CONFIG2L
  CONFIG  PWRT = OFF, BOREN = OFF, BORV = 3
; CONFIG2H
  CONFIG  WDT = OFF, WDTPS = 32768
; CONFIG3L
  CONFIG  MODE = MC, ADDRBW = ADDR20BIT, DATABW = DATA16BIT, WAIT = OFF
; CONFIG3H
  CONFIG  CCP2MX = PORTC, ECCPMX = PORTE, LPT1OSC = OFF, MCLRE = ON
; CONFIG4L
  CONFIG  STVREN = ON, LVP = OFF, BBSIZ = BB2K, XINST = OFF
; CONFIG5L
  CONFIG  CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF, CP4 = OFF, CP5 = OFF
  CONFIG  CP6 = OFF, CP7 = OFF
; CONFIG5H
  CONFIG  CPB = OFF, CPD = OFF
; CONFIG6L
  CONFIG  WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF, WRT4 = OFF
  CONFIG  WRT5 = OFF, WRT6 = OFF, WRT7 = OFF
; CONFIG6H
  CONFIG  WRTC = OFF, WRTB = OFF, WRTD = OFF
; CONFIG7L
  CONFIG  EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF, EBTR4 = OFF
  CONFIG  EBTR5 = OFF, EBTR6 = OFF, EBTR7 = OFF
; CONFIG7H
  CONFIG  EBTRB = OFF

;*******************************************************************************
; Variables & Constants
;*******************************************************************************
UDATA_ACS
  t1            res 1	; used in delay
  t2            res 1	; used in delay
  t3            res 1	; used in delay
  control_reg   res 1   ; 1'se bir defa RA4'e basilmistir
  turn_reg      res 1   ; 1'se don(bitirici)
  control_reg2  res 1   ; 1'se bir defa RB5'e basilmistir
  corner_reg    res 1   ; 1'se koseyi don(bitirici)

  DCounter1     EQU 0X0C
  DCounter2     EQU 0X0D
  DCounter3     EQU 0X0E


;*******************************************************************************
; Reset Vector
;*******************************************************************************

RES_VECT  CODE    0x0000            ; processor reset vector
    GOTO    START                   ; go to beginning of program

;*******************************************************************************
; MAIN PROGRAM
;*******************************************************************************

MAIN_PROG CODE	; let linker place main program

;-------------------------------------------------
;-------------------------------------------------

START
    call INIT	; initialize variables and ports
    call DELAY	; wait a second

    CLRF LATA   ;ledleri söndür
    CLRF LATB
    CLRF LATC
    CLRF LATD

;-------------------------------------------------
;-------------------------------------------------
STATE1 ;saat yönünde hareket
    _upper_edge00:
    CLRF turn_reg

    MOVLW 0x01 ; RA0 ve RB0 yansin
    MOVWF LATA
    MOVWF LATB

    call DELAY750 

    CLRF LATA
    CLRF LATB

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _left_edge10
;-------------------------------------------------
    _upper_edge01:
    CLRF turn_reg

    MOVLW 0x01 ; RB0 ve RC0 yansin
    MOVWF LATB
    MOVWF LATC

    call DELAY750  

    CLRF LATB
    CLRF LATC

    CLRF WREG
    CPFSEQ turn_reg
    goto _upper_edge12
;-------------------------------------------------
    _upper_edge02:
    CLRF turn_reg

    MOVLW 0x01
    MOVWF LATC
    MOVWF LATD

    _upper_edge02_control:
    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_TURN ;geri dönüs için kontrol
    CLRF WREG
    CPFSEQ turn_reg
    goto _upper_edge02_exten

    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_CORNER ;köseyi dönüs için kontrol
    CLRF WREG
    CPFSEQ corner_reg
    goto _right_edge00 ;köseyi dönecek
    goto _upper_edge02_control ;loop

    _upper_edge02_exten:
    CLRF LATC ;???klar?
    CLRF LATD ;sönüdr
    goto _upper_edge11
;-------------------------------------------------
;-------------------------------------------------
    _right_edge00:
    call CLEAN

    MOVLW 0x03
    MOVWF LATD

    call DELAY750  

    CLRF LATD

    CLRF WREG
    CPFSEQ turn_reg
    goto _upper_edge10
;-------------------------------------------------

    _right_edge01:
    call CLEAN

    MOVLW 0x06
    MOVWF LATD

    call DELAY750  

    CLRF LATD

    CLRF WREG
    CPFSEQ turn_reg
    goto _right_edge12
;-------------------------------------------------
    _right_edge02:
    call CLEAN

    MOVLW 0x0C
    MOVWF LATD


    _right_edge02_control:
    CLRF WREG 
    call BUTTON_TASK_TURN       ;geri donus icin kontrol
    CPFSEQ turn_reg
    goto _right_edge11

    call BUTTON_TASK_CORNER     ;koseyi donus için kontrol
    CPFSEQ corner_reg
    goto _right_edge02_exten    ;köseyi dönecek
    goto _right_edge02_control  ;loop


    _right_edge02_exten:
    CLRF LATD ;isiklar söndür
    CLRF corner_reg

;-------------------------------------------------
;-------------------------------------------------
    _lower_edge00:
    call CLEAN


    MOVLW 0x08
    MOVWF LATD
    MOVWF LATC

    call DELAY750

    CLRF LATD
    CLRF LATC

    CLRF WREG
    CPFSEQ turn_reg
    goto _right_edge10

;-------------------------------------------------
    _lower_edge01:
    call CLEAN

    MOVLW 0x08
    MOVWF LATB
    MOVWF LATC

    call DELAY750

    CLRF LATB
    CLRF LATC

    CLRF WREG
    CPFSEQ turn_reg
    goto _lower_edge12

;-------------------------------------------------
    _lower_edge02:
    call CLEAN

    MOVLW 0x08
    MOVWF LATB
    MOVWF LATA


    _lower_edge02_control:
    CLRF WREG
    call BUTTON_TASK_TURN       ;geri donus icin kontrol
    CPFSEQ turn_reg
    goto _lower_edge11

    call BUTTON_TASK_CORNER     ;koseyi donus için kontrol
    CPFSEQ corner_reg
    goto _lower_edge02_exten     ;köseyi dönecek
    goto _lower_edge02_control   ;loop


    _lower_edge02_exten:
    CLRF LATB ;isiklar söndür
    CLRF LATA ;isiklar söndür

    CLRF corner_reg
;-------------------------------------------------
;-------------------------------------------------
    _left_edge00:
    call CLEAN

    MOVLW 0x0C
    MOVWF LATA

    call DELAY400

    CLRF LATA

    CLRF WREG
    CPFSEQ turn_reg
    goto _lower_edge10

;-------------------------------------------------

    _left_edge01:
    call CLEAN

    MOVLW 0x06
    MOVWF LATA

    call DELAY400

    CLRF LATA

    CLRF WREG
    CPFSEQ turn_reg
    goto _left_edge12
;-------------------------------------------------
    _left_edge02:
    call CLEAN

    MOVLW 0x03
    MOVWF LATA


    _left_edge02_control:
    CLRF WREG
    call BUTTON_TASK_TURN       ;geri donus icin kontrol
    CPFSEQ turn_reg
    goto _left_edge12

    call BUTTON_TASK_CORNER     ;koseyi donus için kontrol
    CPFSEQ corner_reg
    goto _left_edge00_exten     ;köseyi dönecek
    goto _left_edge02_control   ;loop


    _left_edge00_exten:
    CLRF LATA ;isiklar söndür
    CLRF corner_reg
    goto _upper_edge00

;-------------------------------------------------
;-------------------------------------------------
STATE2 ;saatin tersi yönünde hareket
    _upper_edge10:
    call CLEAN

    MOVLW 0x01
    MOVWF LATC
    MOVWF LATD

    call DELAY400 

    CLRF LATC
    CLRF LATD

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _right_edge00

;-------------------------------------------------

    _upper_edge11:
    call CLEAN

    MOVLW 0x01
    MOVWF LATC
    MOVWF LATB

    call DELAY400 

    CLRF LATC
    CLRF LATB

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _upper_edge02
;-------------------------------------------------

    _upper_edge12:
    call CLEAN

    MOVLW 0x01
    MOVWF LATA
    MOVWF LATB


    _upper_edge12_control:
    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_TURN ;geri dönüs için kontrol
    CLRF WREG
    CPFSEQ turn_reg
    goto _upper_edge12_exten

    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_CORNER ;köseyi dönüs için kontrol
    CLRF WREG
    CPFSEQ corner_reg
    goto _left_edge10 ;köseyi dönecek
    goto _upper_edge12_control ;loop


   _upper_edge12_exten:
    CLRF LATA ;???klar?
    CLRF LATB ;sönüdr
    goto _upper_edge01

;-------------------------------------------------
    _left_edge10:
    call CLEAN

    MOVLW 0x03
    MOVWF LATA

    call DELAY400 

    CLRF LATA

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _upper_edge00

;--------------------------------------------------
    _left_edge11:
    call CLEAN

    MOVLW 0x06
    MOVWF LATA

    call DELAY400 

    CLRF LATA

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _left_edge02

;-------------------------------------------------
    _left_edge12:
    call CLEAN

    MOVLW 0x0C
    MOVWF LATA

    _left_edge12_control:
    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_TURN ;geri dönüs için kontrol
    CLRF WREG
    CPFSEQ turn_reg
    goto _left_edge12_exten

    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_CORNER ;köseyi dönüs için kontrol
    CLRF WREG
    CPFSEQ corner_reg
    goto _lower_edge10 ;köseyi dönecek
    goto _left_edge12_control ;loop


   _left_edge12_exten:
    CLRF LATA ;???klar?
    goto _left_edge01
;-------------------------------------------------
    _lower_edge10:
    call CLEAN

    MOVLW 0x08
    MOVWF LATB
    MOVWF LATA

    call DELAY400 

    CLRF LATB
    CLRF LATA

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _left_edge00
;-------------------------------------------------
    _lower_edge11:
    call CLEAN

    MOVLW 0x08
    MOVWF LATB
    MOVWF LATC

    call DELAY400 

    CLRF LATB
    CLRF LATC

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _lower_edge02

;-------------------------------------------------
    _lower_edge12:
    call CLEAN

    MOVLW 0x08
    MOVWF LATD
    MOVWF LATC

    _lower_edge12_control:
    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_TURN ;geri dönüs için kontrol
    CLRF WREG
    CPFSEQ turn_reg
    goto _lower_edge12_exten

    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_CORNER ;köseyi dönüs için kontrol
    CLRF WREG
    CPFSEQ corner_reg
    goto _right_edge10 ;köseyi dönecek
    goto _lower_edge12_control ;loop


    _lower_edge12_exten:
    CLRF LATD ;???klar?
    CLRF LATC ;???klar?
    goto _lower_edge01

;-------------------------------------------------
    _right_edge10:
    call CLEAN

    MOVLW 0x0C
    MOVWF LATD

    call DELAY400 
    CLRF LATD

    CLRF WREG ;kontrol
    CPFSEQ turn_reg
    goto _lower_edge00

;-------------------------------------------------
    _right_edge11:
    call CLEAN

    MOVLW 0x06
    MOVWF LATD

    call DELAY400 

    CLRF LATD

   CLRF WREG ;kontrol
   CPFSEQ turn_reg
   goto _right_edge02

;-------------------------------------------------
    _right_edge12:
    call CLEAN

    MOVLW 0x03
    MOVWF LATD

    _right_edge12_control:
    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_TURN ;geri dönüs için kontrol
    CLRF WREG
    CPFSEQ turn_reg
    goto _right_edge12_exten

    CLRF WREG
    CLRF turn_reg
    CLRF corner_reg
    call BUTTON_TASK_CORNER ;köseyi dönüs için kontrol
    CLRF WREG
    CPFSEQ corner_reg
    goto _upper_edge10 ;köseyi dönecek
    goto _right_edge12_control ;loop

    _right_edge12_exten:
    CLRF LATD ;ledleri sondur
    goto _right_edge01
;-------------------------------------------------
;-------------------------------------------------
    DELAY ;1s
    MOVLW 0Xbf
    MOVWF DCounter1
    MOVLW 0X76
    MOVWF DCounter2
    MOVLW 0X66
    MOVWF DCounter3
    LOOP3
    DECFSZ DCounter1, 1
    GOTO LOOP3
    DECFSZ DCounter2, 1
    GOTO LOOP3
    DECFSZ DCounter3, 1
    GOTO LOOP3
    NOP
    NOP
    RETURN
;-------------------------------------------------
;-------------------------------------------------
INIT
    CLRF control_reg
    CLRF turn_reg
    CLRF control_reg2
    CLRF corner_reg
    CLRF LATE   ;ledleri söndür
    CLRF LATF
    CLRF LATG


   MOVLW 0Fh ;a portunu analogdan
   MOVWF ADCON1 ;digitale çevir


   CLRF TRISA ;A portunu output yap
   CLRF TRISB ;B portunu output yap
   CLRF TRISC ;C portunu output yap
   CLRF TRISD ;D portunu output yap


   MOVLW 0X10 ;RA4'ü
   MOVWF TRISA ;input yap
   MOVLW 0x20 ;RB5'i
   MOVWF TRISB ;input yap


   MOVLW 0x0F
   MOVWF LATA ;Rx1, Rx2, Rx3, Rx4 ledlerini yak
   MOVWF LATB
   MOVWF LATC
   MOVWF LATD
   return
;-------------------------------------------------
;-------------------------------------------------
CLEAN
    CLRF turn_reg
    CLRF corner_reg
    CLRF LATA
    CLRF LATB
    CLRF LATC
    CLRF LATD
    return

;-------------------------------------------------
;-------------------------------------------------

DELAY750    ;750ms
    MOVLW 0X25
    MOVWF DCounter1
    MOVLW 0X0d
    MOVWF DCounter2
    MOVLW 0X27
    MOVWF DCounter3
    call BUTTON_TASK_TURN       ;geri donus icin kontrol

    LOOP
    DECFSZ DCounter1, 1
    GOTO LOOP
    DECFSZ DCounter2, 1
    GOTO LOOP
    DECFSZ DCounter3, 1
    GOTO LOOP
    RETURN

;-------------------------------------------------
;-------------------------------------------------
DELAY400    ;400ms
    MOVLW 0Xbd
    MOVWF DCounter1
    MOVLW 0X4b
    MOVWF DCounter2
    MOVLW 0X15
    MOVWF DCounter3

LOOP2
    DECFSZ DCounter1, 1
    GOTO LOOP2
    DECFSZ DCounter2, 1
    GOTO LOOP2
    DECFSZ DCounter3, 1
    GOTO LOOP2
    RETURN

;-------------------------------------------------
;-------------------------------------------------
_turn_is_one:
TURN_IS_ONE
    BTFSS PORTA,4
    call MAKE_TURN_REGISTER_ONE
    MOVLW 0xFF
    CPFSEQ turn_reg
    goto _turn_is_one
    return

BUTTON_TASK_TURN
    BTFSC PORTA,4
    call TURN_IS_ONE
    return
;-------------------------------------------------
;-------------------------------------------------

_corner_is_one:
CORNER_IS_ONE
    BTFSS PORTB,5
    call MAKE_CORNER_REGISTER_ONE
    MOVLW 0xFF
    CPFSEQ corner_reg
    goto _corner_is_one
    return

;-------------------------------------------------
BUTTON_TASK_CORNER
    BTFSC PORTB,5
    call CORNER_IS_ONE
    return

;-------------------------------------------------
;-------------------------------------------------
MAKE_TURN_REGISTER_ONE
    MOVLW 0xFF
    MOVWF turn_reg
    MOVLW 0x00
    MOVWF control_reg
    return
;-------------------------------------------------


MAKE_CORNER_REGISTER_ONE
    MOVLW 0xFF
    MOVWF corner_reg
    MOVLW 0x00
    MOVWF control_reg
    return
;-------------------------------------------------
;-------------------------------------------------

END