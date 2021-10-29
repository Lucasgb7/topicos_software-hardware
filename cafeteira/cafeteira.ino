#include <Servo.h>
#include <DHT.h>

// Sensor pins
#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define SERVO_PIN 7
#define WATER_PIN A0

// Sinais para outros grupos
#define FILL 5
#define LID 3
const unsigned long waterDelay = 70*1000; // 70 segundos

Servo s;
int pos;
/* Change these values based on your calibration values */
int lowerThreshold = 235;
int upperThreshold = 260;
int idealThreshold = 250;

int readWaterLevel() {
  int level = analogRead(WATER_PIN);
  
  // Etapa para verificar o nivel conforme seu threshold
  if (level == 0) {
    Serial.print("Water Level: Empty ("); Serial.print(level); Serial.println(")");
  }
  else if (level > 0 && level <= lowerThreshold) {
    Serial.print("Water Level: Low ("); Serial.print(level); Serial.println(")");
  }
  else if (level > lowerThreshold && level <= upperThreshold) {
    Serial.print("Water Level: Medium ("); Serial.print(level); Serial.println(")");
  }
  else if (level > upperThreshold) {
    Serial.print("Water Level: High ("); Serial.print(level); Serial.println(")");
  }
  return level;
}

// Despeja agua na cafeteira
void throwWater(bool maxWater, bool lid) {
  // Se a tampa da cafeteira estiver aberta e a agua estiver no nivel maximo, despeja a agua.
  if(lid && maxWater){
      // Libera o fluxo de agua
      for(pos = 90; pos > 0; pos--){
        s.write(pos);
        delay(15);
      }    
      delay(waterDelay);
      
      // Bloqueia o fluxo de agua
      for(pos = 0; pos <90; pos++){
        s.write(pos);
        delay(15);
      }
      
  }
}

// Verifica se a tampa esta aberta
bool openLid() {
    return digitalRead(LID) == HIGH ? true : false;
}

// Avisa para a outra equipe se precisa encher ou não
void fillBottle(bool maxWater){
    if(maxWater){
      digitalWrite(FILL, HIGH); // Nível de água máximo, não precisa encher
    }else{
      digitalWrite(FILL, LOW);  // Abaixo do nível desejado, precisa encher
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
    s.write(90); // Começa o programa com o fluxo de agua bloqueado
    dht.begin();
    pinMode(FILL, OUTPUT);
    pinMode(LID, INPUT);
}

void loop() {

    // Lê informação do nivel de agua
    bool maxWater = checkWater();    
    // Verifica se pode encher a garrafa ou não
    fillBottle(maxWater);
    // Verifica se a tampa esta aberta
    bool lid = openLid();
    // Verifica se pode despejar a agua
    throwWater(maxWater, lid);

    delay(100);
}
