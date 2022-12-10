// Library
  #include <DS3231.h>             //RTC Library

  DS3231 rtc(A1, A0);//Ponto onde é possível ajustar a porta do arduino (ex.: rtc(A1, A2))
  Time t; //RTC

  float TmpWatering = 1;  // Minutes - For RTC Scneario
  int WateringInterval = 4; // Hours
  int water = 0;
  int nexthour;
  int lasthour;
  int lastminute;
  int lastsec;
  bool flagWatering;

void setup() {
  // inicia o módulo relógio
  rtc.begin(); 
  
  //as linhas abaixo devem ser descomentadas para configurar o relógio interno. Descomente, carregue o código para o arduino, comente novamente e suba o código mais uma vez.
   //rtc.setDate(21,10,2021);    // determina a data (dia, mes, ano)
   //rtc.setDOW(THURSDAY);     // determina o dia da semana
   //rtc.setTime(18, 36, 0);     // determina o horário (hora, minuto, segundo)

  // Pin Mapping
  pinMode(11, OUTPUT); //Pump Relay
  digitalWrite(11, LOW);

  Serial.begin(9600);
}

void loop() {

    // Define current time
    t = rtc.getTime();
    int h = t.hour;
    int m = t.min;
    int s = t.sec;
    
    Serial.print("Time (RTC): ");
    Serial.print (h);
    Serial.print (":");
    Serial.print (m);
    Serial.print (":");
    Serial.print (s);
    Serial.print(" -/- Water Flag: ");
    Serial.println(water);
    
    // Pump Activation
    if (water == 0) {
        digitalWrite(11, LOW); // Watering Pump On
        flagWatering = 1;
        Serial.print("Pump: On");
        water = 1;
        lasthour = h;
        lastminute = m;
        
        nexthour = lasthour + WateringInterval;

        if (nexthour > 23) {  // Avoid time bigger than 24 hours
          nexthour = nexthour - 24;
        }
    }

    // Pump Deactivation
    if (m >= (lastminute + TmpWatering)) {
        digitalWrite(11, HIGH); // Watering Pump Off
        flagWatering = 0;
        Serial.println("Pump Off ");
    }

    if (nexthour == h) {
        water = 0;
    }    
    Serial.println("**************************");  

}
