/*

  -------------Pin Descritions:------------------
        Relay               :
        Display             :
        Current Sensor      :
        Voltage Sensor      :
        IoT                 :


  ----------------Daily Reports--------------
  08/12/19:
  Current Measurement is done.
  Calculate Watt and Tarriff

  Login Cred:
  mail: energy.meter.iiuc@yandex.com
  pass: energymeter123456



*/




#include  <Wire.h>
#include  <LiquidCrystal_I2C.h>
#define   BLYNK_PRINT Serial          // Comment this out to disable prints and save space
#include  <WiFi.h>
#include  <BlynkSimpleEsp32.h>
#include  <EEPROM.h>


char auth[] = "Zc9JhaT2Nciww4MwXUM9TFsEooaOsXH1";
char ssid[] = "Arkan";
char pass[] = "#include<mma.ijs>";


const int current_sensor_1 = 34;


const int relay_4 = 33;
const int relay_3 = 25;
const int relay_2 = 26;
const int relay_1 = 27;

int relay_1_status = 1;
int relay_2_status = 1;
int relay_3_status = 1;
int relay_4_status = 1;


int button_1_state = 0;
int button_2_state = 0;
int button_3_state = 0;
int button_4_state = 0;

int notification_time = 0;



float voltage = 250.22;

float watt = 0.0;
float show_watt = 0.0;
float killo_watt = 0.0;
float watt_per_hour = 0.0;
float killo_watt_per_hour = 0.0;
float cost_per_unit = 200.0;


float balance ; //700.00; // read from eeprom




LiquidCrystal_I2C lcd(0x27, 16, 2);


BLYNK_WRITE(V4) {
  balance += param.asInt();
  Serial.print("balance: "); Serial.println(balance);

  String message = "Meter credit updated.\n";
  //  send_message(message);

  relay_1_status = 1;
  relay_2_status = 1;
  relay_3_status = 1;
  relay_4_status = 1;


  //  eeprom_balance_write();
}



BLYNK_WRITE(V5) {
  button_1_state = param.asInt();
  Serial.print("button_1_state: "); Serial.println(button_1_state);
}

BLYNK_WRITE(V6) {
  button_2_state = param.asInt();
  Serial.print("button_2_state: "); Serial.println(button_2_state);
}


BLYNK_WRITE(V7) {
  button_3_state = param.asInt();
  Serial.print("button_3_state: "); Serial.println(button_3_state);
}


BLYNK_WRITE(V8) {
  button_4_state = param.asInt();
  Serial.print("button_4_state: "); Serial.println(button_4_state);
}





void setup() {
  Serial.begin(115200); // BAUD rate
  Blynk.begin   (auth, ssid, pass);


  pinMode(current_sensor_1, INPUT);

  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(relay_4, OUTPUT);


  lcd.init();
  lcd.backlight();


  intro();

}


void loop() {


  watt_calculation();
  watt_per_hour_calculation();

  show_data_on_lcd();
  load_usage_notification(); //to lcd

  relay_control();

  balance_notification();



  Serial.println("______________________________________________");

}

void balance_notification() {
  if (notification_time % 10 == 0) {
    if (balance < 10 && balance > 0) {
      Blynk.notify("Balance will expire within 2 days");
      Blynk.run();
    }

    else if (balance < 70 && balance > 60) {
      Blynk.notify("Balance will expire within 17 days");
      Blynk.run();
    }

    else if (balance == 0) {
      Blynk.notify("Meter has no credit!");
      Blynk.run();
    }

  }

  notification_time++;
}



void watt_calculation() {
  float current_value = current_read(current_sensor_1);


  watt = voltage * current_value;


  //  watt_stablizer();

  killo_watt = watt / 1000;

  Blynk.virtualWrite(V1, killo_watt);
  delay(200);
  Blynk.run();


  Blynk.virtualWrite(V3, watt);
  delay(200);
  Blynk.run();

  //  show_watt_lcd();

}





void watt_per_hour_calculation() {
  watt_per_hour += watt * (2.05 / 60 / 60);
  killo_watt_per_hour = watt_per_hour / 1000;
  Serial.print("watt_per_hour: "); Serial.println(watt_per_hour);

  Blynk.virtualWrite(V2, watt_per_hour);
  delay(200);
  Blynk.run();

}




void tarrif_calculation() {
  balance = balance - (killo_watt_per_hour * cost_per_unit);

  if (balance < 0) {
    balance = 0;
  }

  Blynk.virtualWrite(V0, balance);
  delay(200);
  Blynk.run();

}



void load_usage_notification() {
  if (watt > 100 && watt < 250) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Running in:        ");

    lcd.setCursor(0, 1);
    lcd.print("PWR SV Mode         ");
    delay(1000);
    //    lcd.clear();

  }

  else if (watt > 250 && watt < 450) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Couple of divices       ");

    lcd.setCursor(0, 1);
    lcd.print("are conected         ");
    delay(1000);
    //    lcd.clear();
  }

  else if (watt > 450 && watt < 650) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moderate power            ");

    lcd.setCursor(0, 1);
    lcd.print("usage detected           ");
    delay(1000);
    //    lcd.clear();
  }


  else if (watt > 650 && watt < 850) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moderate power           ");

    lcd.setCursor(0, 1);
    lcd.print("usage detected         ");
    delay(1000);
    //    lcd.clear();
  }


  else if (watt > 850 && watt < 1050) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Heavy power           ");

    lcd.setCursor(0, 1);
    lcd.print("usage detected         ");
    delay(1000);
    //    lcd.clear();
  }

  else if (watt > 1050 && watt < 1250) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Heavy power           ");

    lcd.setCursor(0, 1);
    lcd.print("usage detected         ");
    delay(1000);
    //    lcd.clear();
  }


}



void show_data_on_lcd() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(killo_watt); lcd.print("KW "); lcd.print(watt_per_hour); lcd.print("Wh              ");

  lcd.setCursor(0, 1);
  lcd. print(balance); lcd.print(" Taka   ");
  delay(2500);

}



void show_watt_lcd() {
  lcd.setCursor(0, 0);
  lcd.print("Watt Consumption: ");

  lcd.setCursor(0, 1);
  lcd. print(show_watt); lcd.print(" W                               ");
  delay(1000);

}



void relay_control() {
  int thres_balance = 0;

  if (balance <= thres_balance) {
    relay_1_status = 0;
    relay_2_status = 0;
    relay_3_status = 0;
    relay_4_status = 0;

    Serial.println("No Credit.");
  }

  else if (balance > thres_balance) {
    if (button_1_state == 0) {
      relay_1_status = 0;
    }

    else {
      relay_1_status = 1;
    }



    if (button_2_state == 0) {
      relay_2_status = 0;
    }

    else {
      relay_2_status = 1;
    }


    if (button_3_state == 0) {
      relay_3_status = 0;
    }

    else {
      relay_3_status = 1;
    }


    if (button_4_state == 0) {
      relay_4_status = 0;
    }

    else {
      relay_4_status = 1;
    }


  }


  if (balance <= 10 && balance >= 5) {
    //    send_message("Balance is very low.\nPlease Recharge.\n");
  }

  /*

    if (balance < thres_balance - 100) {
    relay_2_status = 0;
    }

    if (balance < thres_balance - 200) {
    relay_3_status = 0;
    }

    if (balance < thres_balance - 300) {
    relay_4_status = 0;
    }

    if (balance < thres_balance - 400) {
    relay_5_status = 0;
    }

    if (balance < thres_balance - 500) {
    relay_6_status = 0;
    }

  */


  digitalWrite(relay_1, relay_1_status);
  digitalWrite(relay_2, relay_2_status);
  digitalWrite(relay_3, relay_3_status);
  digitalWrite(relay_4, relay_4_status);


  delay(1000);
}








float current_read(int sensor_pin) {
  float current_reading = 0.00;
  float result = 0.0;
  float meanResult = 0.0;
  double VRMS = 0;


  int readValue = 0;
  int maxValue = 0;
  int minValue = 4096;

  for (int i = 0; i < 3; i++) { //taking single time for elimininting initial beep
    uint32_t start_time = millis();
    //sample for 1 Sec
    while ((millis() - start_time) < 1000) {
      readValue = analogRead(sensor_pin);
      if (readValue > maxValue) {
        maxValue = readValue;
      }
      if (readValue < minValue) {
        minValue = readValue;
      }
    }

    result = ((maxValue - minValue) * 3.3) / 4096.0; //Peak to Peak //adc value calcilation

    meanResult += result;

  }

  result = meanResult / 3;


  VRMS = (result / 2.0) * 0.707;
  current_reading = ((VRMS * 1000) / 66) - 2.5   ; //Ratiometric value for 30A Model 66 ****** -2.84

  Serial.print("current_reading: ");
  Serial.print(current_reading);
  Serial.println(" A");

  if (current_reading <= 0) {
    current_reading = 0;
  }

  else {
    tarrif_calculation();

  }

  //  Serial.print("current_reading: ");
  //  Serial.print(current_reading);
  //  Serial.println(" A");

  return current_reading;




}




void intro() {
  Serial.println("Rolling Intro");
  lcd.setCursor(4, 0);
  lcd.print("IIUC");
  delay(1000);

  lcd.setCursor(1, 0);
  lcd.print("Department of");
  delay(1000);

  lcd.setCursor(6, 1);
  lcd.print("CSE");
  delay(2000);
  lcd.clear();


  lcd.setCursor(0, 0);
  lcd.print("  Smart Energy   ");

  lcd.setCursor(0, 1);
  lcd.print("Metering System");
  delay(3000);

  lcd.clear();
}
