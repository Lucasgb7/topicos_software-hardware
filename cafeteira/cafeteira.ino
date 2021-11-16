#include <Servo.h>

// Sinais de controle do motor e sensor de agua
#define SERVO_PIN 7
#define WATER_PIN A0

// Sinais para outros grupos
#define FILL 5      // Grupo da bomba de água 
#define LID 3       // Grupo que abre e fecha a tampa
#define FILLDONE 8  // Avisar que terminou de encher a água
const unsigned long waterDelay = 25*1000; // 25 segundos

Servo s;
int pos;
int idealThreshold = 50;
int counterWater = 0;

int readWaterLevel() {
  int level = analogRead(WATER_PIN);
  
  // Etapa para verificar o nivel conforme seu threshold
  if (level <= idealThreshold) {
    Serial.print("Nivel de Agua: Baixo ("); Serial.print(level); Serial.println(")");
  } else {
    Serial.print("Nivel de Agua: OK ("); Serial.print(level); Serial.println(")");
  }
  
  return level;
}

// Despeja agua na cafeteira
void throwWater(bool maxWater, bool lid) {
  // Se a tampa da cafeteira estiver aberta e a agua estiver no nivel maximo, despeja a agua.
  if(lid && maxWater){
      Serial.println("Liberando fluxo de agua");
      // Libera o fluxo de agua
      s.write(90);
      Serial.println("Evacoando...");
      // Tempo para vasao de agua
      counterWater++;
      delay(waterDelay);
      Serial.println("Bloqueando fluxo de agua");
      // Bloqueia o fluxo de agua
      s.write(0);
      //delay(waterDelay);
      
  }
}

// Verifica se a tampa esta aberta
bool openLid() {
    return digitalRead(LID) == HIGH ? true : false;
}

// Avisa para a outra equipe se precisa encher ou não
void fillBottle(bool maxWater){
    if(maxWater){
      digitalWrite(FILL, LOW); // Avisa que não precisa encher
    }else{
      digitalWrite(FILL, HIGH);  // Avisa que precisa encher
    }
}

// Verifica o nivel de agua
bool checkWater(){
    return readWaterLevel() > idealThreshold ? true : false; 
}

// Configuração inicial
void setup() {
    Serial.begin(9600);
    s.attach(SERVO_PIN);
    s.write(0); // Começa o programa com o fluxo de agua bloqueado
    pinMode(FILL, OUTPUT);
    pinMode(LID, INPUT);
    pinMode(FILLDONE, OUTPUT);
}

void loop() {
  fillBottle(false);
  Serial.println("Abrir porta");
  delay(10000);
  fillBottle(true);
  Serial.println("Fechar porta");
  delay(10000);
  /*
    // Lê informação do nivel de agua
    bool maxWater = checkWater();    
    // Verifica se pode encher a garrafa ou não
    fillBottle(maxWater);
    // Verifica se a tampa esta aberta
    bool lid = openLid();
    lid = true;
    // Verifica se pode despejar a agua
    throwWater(lid, maxWater);
    
    if (counterWater >= 2){
      digitalWrite(FILLDONE, HIGH);
      counterWater = 0;
      fillBottle(maxWater);
    }else{
      digitalWrite(FILLDONE, LOW);
    }
    delay(100);
    */
}
