#include <ChainableLED.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include "DS1307.h"
#include <SD.h>
#include <EEPROM.h>
#define NUM_LEDS 1
#define BOUTON_VERT 2
#define BOUTON_ROUGE 3
#define adresseI2CduBME280
Adafruit_BME280 bme;
DS1307 clock;
int interval= 5000;
//SD
const int sd_model = 4;
int FILE_MAX_SIZE = 2048;
int revision = 0;
int num_copie = 0;
String Fichier = "";

//GPS
SoftwareSerial SoftSerial(4,5);
String DonneesGPS;
bool cond;

enum Mode {
    STANDARD,
    MAINTENANCE,
    CONFIGURATION,
    ECONOMIQUE
};
Mode modeActuel;
Mode modePrecedent;

ChainableLED leds (7, 8, NUM_LEDS);
volatile bool flag = false;
volatile bool flag2 =false;
unsigned long debut = 0;
unsigned long debut2 = 0;
unsigned long temp = 0;
const int light_sensor = A3;

const char standard[] PROGMEM = "Mode Standard";//0
const char configuration[] PROGMEM = "Mode Configuration";//1
const char maintenance[] PROGMEM = "Mode Maintenance";//2
const char economique[] PROGMEM = "Mode Economie";//3
const char retourStandard[] PROGMEM = "Passage automatique en mode Standard après 30 minutes d'inactivité";//4

// BME & Lumin
const char luminosite[] PROGMEM = "Luminosité = ";//22
const char temperature[] PROGMEM = "Temperature = ";//23
const char celcius[] PROGMEM = " °C ";//24
const char humidity[] PROGMEM = "Humidity = ";//25
const char pourcent[] PROGMEM = " % ";//26
const char pressure[] PROGMEM = "Pressure = ";//27
const char hpa[] PROGMEM = " hPa ";//28
const char erreurLuminosite[] PROGMEM = "Erreur de communication avec le capteur luminosite !";//6
const char erreurBME[] PROGMEM = "Erreur de communication avec le capteur BME280 !";//7

// config
const char log_Interval[] PROGMEM = "LOG_INTERVAL mis à jour";//29
const char fileMaxSize[] PROGMEM = "FileMaxSize mis à jour";//30
const char reset[] PROGMEM = "Paramètres réinitialisés.";
const char version[] PROGMEM = "Version : 1.0.0, Lot : 001";
const char timeout[] PROGMEM = "Timeout mis à jour";
const char lumin[] PROGMEM = "Lumin Sensor mis à jour";
const char luminHigh[] PROGMEM = "LUMIN_HIGH mis à jour";
const char luminLow[] PROGMEM = "LUMIN_HIGH mis à jour";
const char tempAir[] PROGMEM = "Temperature Sensor mis à jour";
const char minTempAir[] PROGMEM = "LUMIN_HIGH mis à jour";
const char maxTempAir[] PROGMEM = "LUMIN_HIGH mis à jour";
const char hygr[] PROGMEM = "HYGR Sensor mis à jour: ";
const char hygrMinT[] PROGMEM = "HYGR_MINT mis à jour: ";
const char hygrMaxT[] PROGMEM = "HYGR_MAXT mis à jour: ";
const char pressureMaj[] PROGMEM = "PRESSURE Sensor mis à jour: ";
const char pressureMin[] PROGMEM = "PRESSURE_MIN mis à jour: ";
const char pressureMax[] PROGMEM = "PRESSURE_MAX mis à jour: ";
const char dateMaj[] PROGMEM = "date mis à jour: ";
const char clockMaj[] PROGMEM = "clock mis à jour: ";
const char dayMaj[] PROGMEM = "jour mis à jour: ";

// error config
const char errorNoCommand[] PROGMEM = "Erreur : aucune commande fournie.";//8
const char errorInvalidDate[] PROGMEM = "Erreur : Valeurs de date invalides.";//9
const char errorInvalidCommand[] PROGMEM = "Commande non reconnue.";//10
const char erreurDate[] PROGMEM = "Erreur : Format de date invalide. Utilisez JJ/MM/AAAA.";//12
const char errorStockage[] PROGMEM = "mémoire pleine, archivage";//14
const char errorOuverture[] PROGMEM = "Erreur ouverture du fichier de données";//15
const char initCarteSd[] PROGMEM = "Initialisation carte SD\n";//16
const char errorSdInit[] PROGMEM = "Mauvaise ou aucune carte détéctée";//17
const char carteSdInit[] PROGMEM = "Carte initialisée";//18
const char erreurHeure[] PROGMEM = "Erreur : Format d'heure invalide. Utilisez HH:MM:SS.";//19
const char erreurFormat[] PROGMEM = "Erreur : Heure, minute ou seconde invalide.";//20
const char erreurJour[] PROGMEM = "Erreur : Jour invalide. Utilisez MON, TUE, WED, THU, FRI, SAT, SUN.";//21


void afficherMessage(const char* message) {
  char buffer[50];
  strcpy_P(buffer, message);
  Serial.println(buffer);
}
void command() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // read the line until the last character
    
    if (input.startsWith("LOG_INTERVAL=")) {
      EEPROM.put(1, input.substring(13).toInt());
      afficherMessage(log_Interval);

    } else if (input.startsWith("FILE_MAX_SIZE=")) {
      EEPROM.put(2, input.substring(14).toInt());
      afficherMessage(fileMaxSize);

    } else if (input == "RESET") {
      EEPROM.put(1, 60000);EEPROM.put(2, 4096); EEPROM.put(3, 30000);EEPROM.write(4, 1);EEPROM.put(5, 255);EEPROM.put(6, 768);EEPROM.write(7, 1);EEPROM.write(8, -10);
      EEPROM.write(9, 60);EEPROM.write(10, 1);EEPROM.write(11, 0); EEPROM.write(12, 50);EEPROM.write(13, 1);EEPROM.put(14, 850);EEPROM.put(15, 1080);
      afficherMessage(reset);

    } else if (input == "VERSION") {
      afficherMessage(version);

    } else if (input.startsWith("TIMEOUT=")) {
      EEPROM.put(3, input.substring(8).toInt());
      afficherMessage(timeout);

    }else if (input.startsWith("LUMIN=")) {
    if (input.substring(6).toInt() == 1 || input.substring(6).toInt() == 0) {
        EEPROM.put(4, input.substring(6).toInt());
        afficherMessage(lumin);
    } else {
        afficherMessage(errorInvalidCommand);
    }
    } else if (input.startsWith("LUMIN_HIGH=")) {
      if (input.substring(11).toInt() >= 0 && input.substring(11).toInt() <= 1023) {
      EEPROM.put(5, input.substring(11).toInt());
      afficherMessage(luminHigh);
    } else {
      afficherMessage(errorInvalidCommand);
    }

    } else if (input.startsWith("LUMIN_LOW=")) {
      if (input.substring(10).toInt() >= 0 && input.substring(10).toInt() <= 1023) {
      EEPROM.put(6, input.substring(10).toInt());
      afficherMessage(luminLow);
    } else {
      afficherMessage(errorInvalidCommand);
    }
    
    } else if (input.startsWith("TEMP_AIR=")) {
      if (input.substring(9).toInt() == 1 || input.substring(9).toInt() == 0){
        EEPROM.put(7, input.substring(9).toInt());
        afficherMessage(tempAir);
      }else {
        afficherMessage(errorInvalidCommand);
      }

    } else if (input.startsWith("MIN_TEMP_AIR=")) {
      if (input.substring(13).toInt() >= -40 && input.substring(13).toInt() <= 85) {
      EEPROM.put(8, input.substring(13).toInt());
      afficherMessage(minTempAir);
    } else {
      afficherMessage(errorInvalidCommand);
    }

    } else if (input.startsWith("MAX_TEMP_AIR=")) {
      if (input.substring(13).toInt() >= -40 && input.substring(13).toInt() <= 85) {
      EEPROM.put(9, input.substring(13).toInt());
      afficherMessage(maxTempAir);
    } else {
      afficherMessage(errorInvalidCommand);
    }

    } else if (input.startsWith("HYGR=")) {
      if (input.substring(5).toInt() == 1 || input.substring(5).toInt() == 0) {
        EEPROM.put(10, input.substring(5).toInt());
        afficherMessage(hygr);
      }else {
        afficherMessage(errorInvalidCommand);
      }

    } else if (input.startsWith("HYGR_MINT=")) {
      if (input.substring(10).toInt() >= -40 && input.substring(10).toInt() <= 85) {
      EEPROM.put(11, input.substring(10).toInt());
      afficherMessage(hygrMinT);
    } else {
      afficherMessage(errorInvalidCommand);
    }

    } else if (input.startsWith("HYGR_MAXT=")) {
      if (input.substring(10).toInt() >= -40 && input.substring(10).toInt() <= 85) {
      EEPROM.put(12, input.substring(10).toInt());
      afficherMessage(hygrMaxT);
    } else {
      afficherMessage(errorInvalidCommand);
    }

     } else if (input.startsWith("PRESSURE=")) {
      if (input.substring(9).toInt() == 1 || input.substring(9).toInt() == 0) {
        EEPROM.put(13, input.substring(9).toInt());
        afficherMessage(pressureMaj);
      } else {
        afficherMessage(errorInvalidCommand);
      }

    } else if (input.startsWith("PRESSURE_MIN=")) {
      if (input.substring(9).toInt() >= 300 && input.substring(9).toInt() <= 1100) {
      EEPROM.put(14, input.substring(9).toInt());
      afficherMessage(pressureMin);
    } else {
      afficherMessage(errorInvalidCommand);
    }

    } else if (input.startsWith("PRESSURE_MAX=")) {
      if (input.substring(9).toInt() >= 300 && input.substring(9).toInt() <= 1100) {
      EEPROM.put(15, input.substring(9).toInt());
      afficherMessage(pressureMax);
    } else {
      afficherMessage(errorInvalidCommand);
    }
    
    }else if (input.startsWith("DATE=")) {
    uint8_t day, month, year;
    if (sscanf(input.c_str(), "%d/%d/%d", &day, &month, &year) == 3) {
      if (month >= 1 && month <= 12 && day >= 1 && day <= 31 && year >= 2000 && year <= 2099) {
        EEPROM.put(18, day);
        EEPROM.put(17, month);
        EEPROM.put(16, year);
        afficherMessage(dateMaj);
        } else {
          afficherMessage(errorInvalidDate);
        }
    } else {
        afficherMessage(erreurDate);
      }
    }else if (input.startsWith("DAY=")) {
    const char* validDays[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    for (int i = 0; i < 7; i++) {
      if (input.equalsIgnoreCase(validDays[i])) {
        EEPROM.put(19, validDays[i]);
        afficherMessage(dayMaj);
        return;
      }else{
        afficherMessage(erreurJour);
      }
    }
    }else if (input.startsWith("CLOCK=")) {
    int hour, minute, second;
    if (sscanf(input.c_str(), "%d:%d:%d", &hour, &minute, &second) == 3) {
        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60 && second >= 0 && second < 60) {
          EEPROM.put(20, hour);
          EEPROM.put(21, minute);
          EEPROM.put(22, second);
          afficherMessage(clockMaj);
        } else {
            afficherMessage(erreurFormat);
        }
    } else {
        afficherMessage(erreurDate);
    }
    }else {
      afficherMessage(errorInvalidCommand);
    }
  }
}
void parametre(){
  EEPROM.put(1, 5000);//Log interval
  EEPROM.put(2, 4096);// File max size
  EEPROM.put(3, 30000);//timeout
  EEPROM.write(4, 1);  //lumin
  EEPROM.write(5, 255);
  EEPROM.put(6, 768);
  EEPROM.write(7, 1);  //temp
  EEPROM.write(8, -10);
  EEPROM.write(9, 60);
  EEPROM.write(10, 1);  //humidity
  EEPROM.write(11, 0);
  EEPROM.write(12, 50);
  EEPROM.write(13, 1);  //press
  EEPROM.put(14, 850);
  EEPROM.put(15, 1080);
  EEPROM.put(16, 2024);//year
  EEPROM.put(17, 1);
  EEPROM.put(18, 1);
  EEPROM.put(19, "Lundi");
  EEPROM.put(20, 12);//hour
  EEPROM.put(21, 0);
  EEPROM.put(22, 0);
}

void basculer() {
  if(digitalRead(BOUTON_ROUGE) == LOW){
  debut = millis();
  flag = !flag;
  }
}
void basculer2() {
  if(digitalRead(BOUTON_VERT) == LOW){
  debut2 = millis();
  flag2 = !flag2;
  }
}
bool AppuiLong(volatile unsigned long &dernierAppui, volatile bool &appui) {
     while (digitalRead(BOUTON_ROUGE) == LOW || digitalRead(BOUTON_VERT) == LOW){
        if (appui && millis() - dernierAppui >= 5000) {
            delay(50);
            appui = false;
            return true;
        }
    }
    return false;
}
void erreur_capt() {
    leds.setColorRGB(0, 255, 0, 0);
    delay(1000);
    leds.setColorRGB(0, 0, 255, 0);
    delay(1000);
}
void gps()
{
  String DonneesGPS = "";
  if (SoftSerial.available()){
    cond = true;
    while(cond){
      DonneesGPS += SoftSerial.readStringUntil('\n');
      if (DonneesGPS.startsWith("$GPGGA",0)){
        cond  = false;
      }
    }
    Serial.println(DonneesGPS);
  }
  else {
    while (SoftSerial.available() !=0){
    }
  }
}
void capteurLuminosite(){
  int light = analogRead(light_sensor);
  afficherMessage(luminosite); 
  Serial.println(light);
  Serial.println();
}

void bmesensor(){
  while (!bme.begin(0x76)) {
    afficherMessage(erreurBME);
    erreur_capt();
  }
  if(EEPROM[7] == 1){
    float temperature_bme = bme.readTemperature();
    afficherMessage(temperature);
    Serial.print(temperature_bme);
    afficherMessage(celcius);
  }
  if(EEPROM[10] == 1){
    float humidity_bme = bme.readHumidity();
    afficherMessage(humidity);
    Serial.print(humidity_bme);
    afficherMessage(pourcent);
  }

  if(EEPROM[13] == 1){
    float pressure_bme = bme.readPressure() / 100.0F;
    afficherMessage(pressure);
    Serial.print(pressure_bme);
    afficherMessage(hpa);
  }
}
void ClockDS1307()
{
    clock.getTime();
    Serial.print(clock.hour, DEC);
    Serial.print(F(":"));
    Serial.print(clock.minute, DEC);
    Serial.print(F(":"));
    Serial.print(clock.second, DEC);
    Serial.print(F("  "));
    Serial.print(clock.month, DEC);
    Serial.print(F("/"));
    Serial.print(clock.dayOfMonth, DEC);
    Serial.print(F("/"));
    Serial.print(clock.year+2000, DEC);
    Serial.println(F(" "));
}

String createFile(int num){
  String Log_Name ="";
  Log_Name += ("%d%d%d%d.LOG", clock.year, clock.month, clock.dayOfMonth, num);
  Serial.println(Log_Name);
  return Log_Name;
} 
void enregistrement_SD(){
  if (Fichier == ""){
    Fichier = createFile(revision);
  }
  String DonneesString = "";  
  File fichierDonnees = SD.open(Fichier, FILE_WRITE);
  if (SD.exists(Fichier)){
    if (fichierDonnees.size() < FILE_MAX_SIZE){
        fichierDonnees.println(DonneesString);
        fichierDonnees.close();
    }
    else {
        afficherMessage(errorStockage);
        num_copie ++;
        String Fichier_archive = createFile(revision+num_copie);
        if (SD.exists(Fichier_archive)){
            File archive = SD.open(Fichier_archive, FILE_WRITE);
            archive.println(fichierDonnees.read());
            archive.close();
        }
        Fichier = createFile(revision+num_copie);
    }
  }
  else {
    afficherMessage(errorOuverture);
  }
}
void setup()
{
  Serial.begin(9600);
  leds.init();
  pinMode(BOUTON_ROUGE,INPUT_PULLUP);
  pinMode(BOUTON_VERT,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOUTON_ROUGE),basculer,CHANGE);
  attachInterrupt(digitalPinToInterrupt(BOUTON_VERT),basculer2,CHANGE);
  SoftSerial.begin(9600);
  clock.begin();
  clock.fillByYMD(2024,10,25);  //Mettre AAAA/MM/JJ
  clock.fillByHMS(10,28,30);    //Mettre HH/MM/SS
  clock.fillDayOfWeek(FRI);
  clock.setTime();

  afficherMessage(initCarteSd);
  if (!SD.begin(sd_model)) {
    afficherMessage(errorSdInit);
    while (true);
  }
  afficherMessage(carteSdInit);

  if (digitalRead(BOUTON_ROUGE) == LOW){
    modeActuel = CONFIGURATION;
    leds.setColorRGB( 0, 125, 125, 0);
    afficherMessage(configuration);
  }else { 
    modeActuel = STANDARD;
    leds.setColorRGB(0, 0, 255, 0);
    afficherMessage(standard);
  }
}

void loop()
{

  switch (modeActuel) {

    case STANDARD:
    ClockDS1307();
    bmesensor();
    capteurLuminosite();
    //gps();
    //enregistrement_SD();
    delay(interval);
      if (AppuiLong(debut, flag)){
        afficherMessage(maintenance);
        leds.setColorRGB(0,255, 125, 0);
        modeActuel = MAINTENANCE;
        modePrecedent = STANDARD;
      }if (AppuiLong(debut2, flag2)) {
        afficherMessage(economique);
        leds.setColorRGB(0,0, 0, 255);
        modeActuel = ECONOMIQUE;
      }break;

    case MAINTENANCE:
    ClockDS1307();
    bmesensor();
    capteurLuminosite();
    //gps();
    delay(interval);
      if (AppuiLong(debut, flag)) {
        if(modePrecedent == STANDARD){
          modeActuel = STANDARD;
          leds.setColorRGB(0, 0, 255, 0);
          afficherMessage(standard);
        }else if(modePrecedent == ECONOMIQUE){
          modeActuel = ECONOMIQUE;
          leds.setColorRGB(0,0, 0, 255);
          afficherMessage(economique);
        }
      }break;
      
    case ECONOMIQUE:
    ClockDS1307();
    bmesensor();
    capteurLuminosite();
    //gps();
    delay(interval*2);
      if (AppuiLong(debut, flag)) {
        modeActuel = STANDARD;
        leds.setColorRGB(0, 0, 255, 0);
        afficherMessage(standard);
      } else if (AppuiLong(debut2, flag2)) {
        modeActuel = MAINTENANCE;
        modePrecedent = ECONOMIQUE;
        leds.setColorRGB(0,255, 125, 0);
        afficherMessage(economique);
      }break;

    case CONFIGURATION:
    command();
    temp = millis();
    if (millis() - temp >= 1800000) { // 30 minutes d'inactivité
      modeActuel = STANDARD;
      leds.setColorRGB(0, 0, 255, 0);
      afficherMessage(retourStandard); // Message de retour automatique au mode Standard
    } break;
  }
}
