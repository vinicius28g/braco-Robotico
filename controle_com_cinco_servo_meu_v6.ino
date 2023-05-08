#include <EEPROM.h>

#include "PS2X_lib.h"  //for v1.6
#include <Servo.h>

PS2X ps2x; // create PS2 Controller Class
Servo servo1; //create servo class
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original M
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use 
 ****************************************************************/
#define PS2_DAT         7 //marrom
#define PS2_CMD         5 //laranja
#define PS2_SEL         4 //amarelo
#define PS2_CLK         6 //azul

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false


int error = 0;
byte type = 0;
byte vibrate = 0;

uint8_t velocidade =  3;

int position_servo_1 = 90;
int position_servo_2 = 60;
int position_servo_3 = 93;
int position_servo_4 = 30;
int position_servo_5 = 30;


#define TAM_VET_POSICOES           200
#define TAM_VET_VELOCIDADE         TAM_VET_POSICOES/5
#define TAM_EEPROM                 TAM_VET_POSICOES + TAM_VET_VELOCIDADE
#define TEMPO_MOVIMENTO           60 // testado! NÃO MODIFICAR
#define TEMPO_ENTRE_POSICAO       300

#define POSICAO_INICIAL_SERVO_1   90
#define POSICAO_INICIAL_SERVO_2   60
#define POSICAO_INICIAL_SERVO_3   93
#define POSICAO_INICIAL_SERVO_4   30
#define POSICAO_INICIAL_SERVO_5   48

uint8_t velocidade_posicao[TAM_VET_VELOCIDADE]; 
uint8_t num_vet_velocidade                       = 0;
uint8_t posicoes[TAM_VET_POSICOES]; 
uint8_t num_vet_posicao                          = 0;


unsigned long ultima_modificacao_ocorrida        = 0;
unsigned long tempo_modificacao_solicitado       = 0;
double tempo_q_passou                            = 0;

bool flag_gravar_posicao                         = 0;
bool flag_movimento_autonomo                     = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
    //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
    if (pressures)
      Serial.println("true ");
    else
      Serial.println("false");
    Serial.print("rumble = ");
    if (rumble)
      Serial.println("true)");
    else
      Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
//  Serial.print(ps2x.Analog(1), HEX);
  
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
  case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
   }

   servo1.attach(9);
   servo2.attach(10);
   servo3.attach(11);
   servo4.attach(12);
   servo5.attach(13);
   
   for(int i = 0; i < TAM_VET_POSICOES; i ++){
     posicoes[i]= 200;
   }
   for(int i = 0; i < TAM_VET_VELOCIDADE; i ++){
     velocidade_posicao[i]=200;
   }
  
   /*for (int i = 0 ; i < TAM_VET_POSICOES ; i++){
    Serial.println(posicoes[i]);
   }*/
   
     servo1.write(POSICAO_INICIAL_SERVO_1);
     servo2.write(POSICAO_INICIAL_SERVO_2);
     servo3.write(POSICAO_INICIAL_SERVO_3);
     servo4.write(POSICAO_INICIAL_SERVO_4);
     servo5.write(POSICAO_INICIAL_SERVO_5);
}

void loop() {
   ps2x.read_gamepad();

  //*************************** Modo Autônomo ******************************
   if(flag_movimento_autonomo == 1){ 
      for(int i = 0; i < TAM_VET_POSICOES; i ++){
          posicoes[i]= EEPROM.read(i);    
      }
      for(int i = 0; i < TAM_VET_VELOCIDADE ; i ++){
          velocidade_posicao[i] = EEPROM.read ( i+ TAM_VET_POSICOES);
      }
      num_vet_posicao       = 0;
      num_vet_velocidade    = 0;
      
      position_servo_1 = servo1.read();
      position_servo_2 = servo2.read();
      position_servo_3 = servo3.read();
      position_servo_4 = servo4.read();
      position_servo_5 = servo5.read();
      
      while(flag_movimento_autonomo == 1){
        
        while( abs(posicoes[num_vet_posicao] - position_servo_1) > velocidade_posicao[num_vet_velocidade]){
          position_servo_1 = position_servo_1 + calcula_sentido( posicoes[num_vet_posicao],position_servo_1) * velocidade_posicao[num_vet_velocidade];
          servo1.write(position_servo_1);
          delay(TEMPO_MOVIMENTO);
        }
      num_vet_posicao++;
      
        while( abs(posicoes[num_vet_posicao] - position_servo_2) > velocidade_posicao[num_vet_velocidade]){
          position_servo_2 = position_servo_2 + calcula_sentido( posicoes[num_vet_posicao],position_servo_2) * velocidade_posicao[num_vet_velocidade];
          servo2.write(position_servo_2);
          delay(TEMPO_MOVIMENTO);
        }
      num_vet_posicao++;

        while( abs(posicoes[num_vet_posicao] - position_servo_3) > velocidade_posicao[num_vet_velocidade]){
          position_servo_3 = position_servo_3 + calcula_sentido( posicoes[num_vet_posicao],position_servo_3) * velocidade_posicao[num_vet_velocidade];
          servo3.write(position_servo_3);
          delay(TEMPO_MOVIMENTO);
        }
      num_vet_posicao++;
      
        while( abs(posicoes[num_vet_posicao] - position_servo_4) > velocidade_posicao[num_vet_velocidade]){
          position_servo_4 = position_servo_4 + calcula_sentido( posicoes[num_vet_posicao],position_servo_4) * velocidade_posicao[num_vet_velocidade];
          servo4.write(position_servo_4);
          delay(TEMPO_MOVIMENTO);
        }
      num_vet_posicao++;
     
        while( abs(posicoes[num_vet_posicao] - position_servo_5) > velocidade_posicao[num_vet_velocidade]){
          position_servo_5 = position_servo_5 + calcula_sentido( posicoes[num_vet_posicao],position_servo_5) * velocidade_posicao[num_vet_velocidade];
          servo5.write(position_servo_5);
          delay(TEMPO_MOVIMENTO);
        }
        
        num_vet_posicao++;
        num_vet_velocidade++;
        delay(TEMPO_ENTRE_POSICAO);
        if(posicoes[num_vet_posicao]==200){          // Quando posição for 200, significa que acabou as posições. Então volta para a posição inicial.
          num_vet_posicao= 0;
          num_vet_velocidade =0;
        }
//*********************** Desativar o Modo Autônomo ********************************
        ps2x.read_gamepad();
        if(ps2x.Button(PSB_TRIANGLE)==1){
          tempo_modificacao_solicitado= millis();
          
          if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 2000){
            flag_movimento_autonomo = 0;
            Serial.println("Modo Autônomo desativado");
            ultima_modificacao_ocorrida= millis();
          }
        }
      }
   }
   
//******************* Aumento da Velocidade****************************    
   if(ps2x.Button(PSB_PAD_RIGHT)){ 
      
     tempo_modificacao_solicitado= millis();
     if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 1000){
          velocidade = aumento_velocidade(velocidade); 
          Serial.print("Velocidade: ");
          Serial.println(velocidade);
          ultima_modificacao_ocorrida= millis();     
     }  
   }
//******************* Diminuição da Velocidade ****************************    
   if(ps2x.Button(PSB_PAD_LEFT)){      
      tempo_modificacao_solicitado= millis();
      
     if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 500){
         velocidade = diminuicao_velocidade(velocidade);
         Serial.print("Velocidade: ");
         Serial.println(velocidade);
         ultima_modificacao_ocorrida= millis(); 
     }
   }
//*********************** Ativar o Modo de Gravação ********************************
    
   if(ps2x.Button(PSB_L2)){
     tempo_modificacao_solicitado= millis();    
     if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 1000){ 
         
       if(flag_gravar_posicao == 0){
         flag_gravar_posicao = 1;
         Serial.println("modo gravação ativado");
         // Reseta os index  para gravar no vetor desde o inicio.
         num_vet_posicao         = 0;            
         num_vet_velocidade      = 0;

         //Reseta o vetor para iniciar gravação.
         for(int i = 0; i < TAM_VET_POSICOES; i ++){
            posicoes[i]= 200;
         }
         for(int i = 0; i < TAM_VET_VELOCIDADE; i ++){
            velocidade_posicao[i]=200;
         }            
         ultima_modificacao_ocorrida= millis();  
       }
     }
   tempo_modificacao_solicitado= millis(); 
   }
     
        
//*********************** Ativar o Modo Autônomo ********************************        
   if(ps2x.Button(PSB_START)==1){
     tempo_modificacao_solicitado= millis();
     if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 2000){
       flag_movimento_autonomo = 1;
       Serial.println("Modo Autônomo ativado");
       ultima_modificacao_ocorrida= millis();
     }
   }

//**************** Gravar posiçoes e velocidade na EEPROM *************************
   if(ps2x.Button(PSB_SQUARE)){
     tempo_modificacao_solicitado= millis();  
     if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 1000){
       
       
       Serial.println("Limpando EEPROM.");
       for(int i = 0; i < TAM_EEPROM; i ++){
         EEPROM.write (i,200);
        }
         
       Serial.println("Gravando posição na EEPROM.");
       for(int i = 0; i < TAM_VET_POSICOES; i ++){
           EEPROM.write (i,posicoes[i]);
       }
       
       for(int i = 0; i < TAM_VET_VELOCIDADE ; i ++){
          EEPROM.write ( i+ TAM_VET_POSICOES,velocidade_posicao[i]);
       }
   Serial.println("Posições gravadas na EEPROM."); 
   flag_gravar_posicao = 0;
//########################## Dedug EEPROM #########################################
    /*  Serial.println("exibindo vetor posições: ");
      
      for(int i=0;i<TAM_VET_POSICOES; i++){
        Serial.println(EEPROM.read(i),DEC);
      } 
      Serial.println("\n");
      
      Serial.println("exibindo vetor velocidade: ");
      for(int i=0;i<TAM_VET_VELOCIDADE; i++){
        Serial.println(EEPROM.read(i + TAM_VET_POSICOES),DEC);
      }*/
//##################### Fim dedug EEPROM #########################################   
   ultima_modificacao_ocorrida = millis();
     }  
   }
      
//**************** Gravar posiçoes e velocidade no Vetor *************************     
    if(ps2x.Button(PSB_CROSS)){
      tempo_modificacao_solicitado= millis();    
      if(tempo_modificacao_solicitado- ultima_modificacao_ocorrida> 1000){   
         
        if(flag_gravar_posicao == 1){
          
            posicoes[num_vet_posicao] = position_servo_1;
            num_vet_posicao++;
                      
            posicoes[num_vet_posicao] = position_servo_2;
            num_vet_posicao++;
            
            posicoes[num_vet_posicao] = position_servo_3;
            num_vet_posicao++;
            
            posicoes[num_vet_posicao] = position_servo_4;
            num_vet_posicao++;
            
            posicoes[num_vet_posicao] = position_servo_5;
            num_vet_posicao++;
            velocidade_posicao[num_vet_velocidade] = velocidade;
            num_vet_velocidade++;

          Serial.println("escolha a próxima posição");
          
          if( num_vet_posicao >= TAM_VET_POSICOES -1){
              Serial.println("ERRO!! Limite de posições atingido.");
              num_vet_posicao= num_vet_posicao - 5;
              num_vet_velocidade--;
          }
          ultima_modificacao_ocorrida= millis();    
        }
      }      
    }   
      
//########################## Dedug Analógico #########################################
  /* if(ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) { //print stick values if either is TRUE
      Serial.print("Stick Values:");
      Serial.print(ps2x.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX  
      Serial.print(",");
      Serial.print(ps2x.Analog(PSS_LX), DEC); 
      Serial.print(",");
      Serial.print(ps2x.Analog(PSS_RY), DEC); 
      Serial.print(",");
      Serial.println(ps2x.Analog(PSS_RX), DEC); 
    }*/
//########################## Fim Dedug Analógico #########################################    
 
//***************************** Movimentação do motor 04 ***********************************
   
   if(ps2x.Button(PSB_L1)){
     
     position_servo_4 = position_servo_4 + velocidade;
     if(position_servo_4 > 90) // limitar a posição do motor
        position_servo_4 = 90;
     Serial.print("motor 04: ");
     Serial.println(position_servo_4);
     servo4.write(position_servo_4);
     delay(TEMPO_MOVIMENTO);
  }

   if(ps2x.Button(PSB_R1)){
     
     position_servo_4 = position_servo_4 - velocidade;
     if(position_servo_4 < 20) // limitar a posição do motor
        position_servo_4 = 20;
        Serial.print("mootor 04: ");
     Serial.println(position_servo_4);
     servo4.write(position_servo_4);
     delay(TEMPO_MOVIMENTO);
   }

   //***************************** Movimentação da garra***********************************
   
   if(ps2x.Button(PSB_PAD_UP)){
    
     position_servo_5 = position_servo_5 + velocidade;
     if(position_servo_5 > 95) // limitar a posição do motor
        position_servo_5 = 95;
     servo5.write(position_servo_5);
     Serial.print("garra: ");
     Serial.println(position_servo_5);
     delay(TEMPO_MOVIMENTO);
   }

   if(ps2x.Button(PSB_PAD_DOWN)){
     position_servo_5 = position_servo_5 - velocidade;
     if(position_servo_5 < 48) // limitar a posição do motor
        position_servo_5 = 48;
     servo5.write(position_servo_5);
     Serial.print("garra: ");
     Serial.println(position_servo_5);
     delay(TEMPO_MOVIMENTO);
   }
//**************** Movimentação dos Analógicos *************************
   if(ps2x.Analog(PSS_LX)> 245){

    position_servo_1 = position_servo_1 + velocidade;
    if(position_servo_1 > 179)  // limitar a posição do motor
        position_servo_1 =179;
     servo1.write(position_servo_1);
     Serial.print("motor 01: ");
     Serial.println(position_servo_1);
     delay(TEMPO_MOVIMENTO);
   }
   
   if(ps2x.Analog(PSS_LX)<15){
     
     position_servo_1 = position_servo_1 - velocidade;
     if(position_servo_1 < 1)  // limitar a posição do motor
        position_servo_1 =1;
      servo1.write(position_servo_1);
      Serial.print("motor 01: ");
     Serial.println(position_servo_1);
      delay(TEMPO_MOVIMENTO);
   }
   
   if(ps2x.Analog(PSS_LY)> 245){

     position_servo_2 = position_servo_2 + velocidade;
     if(position_servo_2 > 130)  // limitar a posição do motor
        position_servo_2 =130;
     servo2.write(position_servo_2);
     Serial.print("mootor 02: ");
     Serial.println(position_servo_2);
     delay(TEMPO_MOVIMENTO);
   }
   
   if(ps2x.Analog(PSS_LY)< 15){
     
      position_servo_2 = position_servo_2 - velocidade;
      if(position_servo_2 < 60)  // limitar a posição do motor
        position_servo_2 = 60; 
        Serial.print("motor 02: ");
     Serial.println(position_servo_2);
      servo2.write(position_servo_2);
      delay(TEMPO_MOVIMENTO);  
   }
  
   if(ps2x.Analog(PSS_RY)> 245){
      
    position_servo_3 = position_servo_3 + velocidade;
    if(position_servo_3 > 179) // limitar a posição do motor
        position_servo_3 = 179;
     Serial.print("motor 03: ");
     Serial.println(position_servo_3);
     servo3.write(position_servo_3);
     delay(TEMPO_MOVIMENTO);      
   }
   
   if(ps2x.Analog(PSS_RY) < 15){
       
     position_servo_3 = position_servo_3 - velocidade;
     if(position_servo_3 < 95) // limitar a posição do motor
        position_servo_3 = 95;
     Serial.print("mootor 03: ");
     Serial.println(position_servo_3);
     servo3.write(position_servo_3);
     delay(TEMPO_MOVIMENTO); 
   }        
}

int calcula_sentido(int posicao_futura, int posicao_atual){
  return (posicao_futura - posicao_atual)/abs(posicao_futura - posicao_atual);
}

int aumento_velocidade(int Velocidade){
  
  if( velocidade < 5 && velocidade > 1 )
    Velocidade++;
  return(Velocidade); 
}

int diminuicao_velocidade(int Velocidade){
  
  if( velocidade < 6 && velocidade > 2 )
    Velocidade--;
  return(Velocidade);
}          
