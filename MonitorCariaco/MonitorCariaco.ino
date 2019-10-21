/*Monitor de Batimentos Cardíacos
Universidade Tecnológica Federal do Paraná - Câmpus Campo Mourão
Acadêmicos: Alexandre de Oliveira Júnior
            Bruno Henrique de Souza Brolesi
Professor:  André Luis Regis Monteiro
Disciplina: Princípios de Engenharia Biomédica*/

//Inclusão das bibliotecas
#include <Wire.h>                     //Biblioteca de comunicação com o módulo I2C
#include <LiquidCrystal_I2C.h>        //Biblioteca do módulo I2C para utilização do display
#include <Filters.h>                  //Biblioteca para implementação digital de um filtro
#include <math.h>                     //Biblioteca para utilização de funções matemáticas

//Definição das pinagens do arduíno
LiquidCrystal_I2C lcd(0x3F, 20, 4);   //Endereçamento do display

#define SENSOR A1                     //Define o pino do sensor
#define LEDBAT 9                      //Define o pino do led indicador de batimento
#define BATMAXIMO 120                 //Define o número máximo de batimentos cardíacos para não acionar o alarme
#define BATMINIMO 60                  //Define o número mínimo de batimentos cardíacos para não acionar o alarme
#define LEDALARME 10                  //Define o pino do Led do alarme
#define buzzer 11                     //Define o pino do buzzer
#define BOTAO 8                       //Define o pino do botão de mudo

//Declaração de variaveis globais
float sinalAmplif;                    //Variável que recebe o sinal com um fator de amplificação
float sinalFiltrado;                  //Variável que recebe o valor do sinal filtrado
float wL = 1;                         //Frequência de corte inferior (1 Hz)
float wH = 3;                         //Frequência de corte superior (3 Hz)
FilterOnePole filtroPA(HIGHPASS, wL); //Filtro passa-alta
FilterOnePole filtroPB(LOWPASS, wH);  //Filtro passa-baixa
float batCard = 0;                    //Número de batimentos cardíacos (float)
int batimentos = 0;                   //Número de batimentos cardíacos (int)
bool desHeart = true;                 //Variável auxiliar para print do coração
bool som = true;                      //Variável auxiliar para o funcionamento do buzzer devido a tecla mudo

//---------------------------------Funções------------------------------------
//Criando um coração
byte heart[] = {
  B00000,
  B00000,
  B01010,
  B10101,
  B10001,
  B01010,
  B00100,
  B00000
};

//Função de inicialização do display
void lcdStart(){
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Welcome!!!");
  delay(3000);
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Cardiac");
  lcd.createChar(0, heart);
  lcd.home();
  lcd.setCursor(3,1);
  lcd.write(0);
  lcd.print("Monitor");
  lcd.write(0);
  delay(5000);
  lcd.clear();
}

//Função de contagem dos batimentos cardíacos
void contarBatimentos(){
  int picoMin = 500;                    //Tamanho mínimo do pico
  int contPico = 0;                     //Variável para contagem do número de picos
  int tamPico = 0;                      //Variável para armazenar o tamanho do pico do sinal
  int antTamPico = 0;                   //Variável que armazena o tamanho do pico anterior do sinal

  for(int i = 0; i < 100; i++){
    sinalAmplif = 100*analogRead(SENSOR);                         //Variável recebe o valor lido pelo LED Receptor IR com um fator de amplificação
    sinalFiltrado = filtroPA.input(filtroPB.input(sinalAmplif));  //Função de filtragem do sinal
    if(sinalFiltrado > picoMin){                                  //Caso o sinal filtrado seja superior ao valor mínimo de pico
      contPico++;                   //O contador de picos incrementa
      tamPico++;                    //O tamanho do pico incrementa
      digitalWrite(LEDBAT, HIGH);   //Enquanto o pico for superior ao tamanho minimo de pico, o led permanece acesso indicando uma pulsação
    }else{
      if(tamPico > antTamPico){     //Caso o tamanho de pico seja maior que o tamanho de pico anterior
        antTamPico = tamPico;       //O tamanho de pico anterior recebe o tamanho de pico anterior
      }
      digitalWrite(LEDBAT, LOW);    //Como o sinal já é inferior ao tamanho de pico o LED apaga indicando o fim do pulso
      tamPico = 0;                  //O tamanho do pico é resetado
    }
    Serial.println(sinalFiltrado);  //Printa no serial o valor do sinal filtrado
    delay(10);                      //Delay definido para que juntamente ao loop do for totalize um segundo
  }
  //Calculando a frequência cardíaca
  if(antTamPico == 0){
    antTamPico = 1;                 //Função garante que não ocorra uma divisão por zero
  }
  batCard += ((float)contPico/(float)antTamPico)*60.0;
  batCard /= 2;
  batimentos = trunc(batCard);
}

//Função print número de batimentos
void printBat(){
  lcd.clear();
  lcd.home();
  lcd.print(batimentos);
  lcd.setCursor(4,0);
  lcd.print("BPM");
  if (desHeart == true){
    lcd.createChar(0, heart);
    lcd.home();
    lcd.setCursor(8,0);
    lcd.write(0);
    desHeart = false;
  }else{
    desHeart = true;
  }
}

//Função de ativação do alarme
void alarme(){
  digitalWrite(LEDALARME, HIGH);
  if(som == true){
    tone(buzzer, 220);
  }
}

//Função de mudo
void buzzerMute(){
  noTone(buzzer);
  som = false;
  delay(2000);
}

//Função alarme off
void alarmeOFF(){
  digitalWrite(LEDALARME, LOW);
  noTone(buzzer);
  som = true;
}

//Função de Inicialiazação
void setup(){
  Serial.begin(9600);                 //Inicialização do serial
  lcd.init();                         //Inicialiazação do LCD
  lcd.backlight();                    //Liga a luz de fundo do display
  lcdStart();                         //Chama a função de inicialização do sensor
  pinMode(SENSOR, INPUT);             //Inicializa o pino do sensor
  pinMode(LEDALARME, OUTPUT);         //Inicializa o pino do Led do alarme
  pinMode(buzzer, OUTPUT);            //Inicializa o pino do buzzer
  pinMode(BOTAO, INPUT_PULLUP);       //Inicializa o pino do botão
}

//Função Principal
void loop(){
  contarBatimentos();
  printBat();
  if(batimentos >= BATMAXIMO || batimentos <= BATMINIMO){
    alarme();
    if(digitalRead(BOTAO)==LOW && som == true){
      buzzerMute(); 
    }
    if(digitalRead(BOTAO)==LOW && som == false){
      tone(buzzer, 220);
      som = true;
      delay(500); 
    }
  }else{
    alarmeOFF();
  }
}

