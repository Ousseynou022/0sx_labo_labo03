#include <Wire.h>
#include <OneButton.h>
#include <LiquidCrystal_I2C.h>
#include <LCD_I2C.h>

#define BTN_PIN 2
#define PIN_CAPTEUR_TEMP A0
#define PIN_JOYSTICK_X A1
#define PIN_JOYSTICK_Y A2
#define PIN_LED 8

//LiquidCrystal_I2C lcd(0x27, 16, 2);
LCD_I2C lcd(0x27, 16, 2);
OneButton bouton(BTN_PIN, true, true);
typedef enum {
  TASK_OFF,
  TASK_ON,
} Tasks;
Tasks currentTask = TASK_OFF;

byte chiffre96[8] = {
  0b11100,
  0b11100,
  0b00100,
  0b11111,
  0b00100,
  0b00100,
  0b00111,
  0b00111,
};

int temperature = 0;
int vitesse = 0;
int direction = 0;
int etat = 0;
bool etatClim = false;
unsigned long lastTime = 0;
int Vo;            // Voltage à la sortie
float R1 = 10000;  // Résistance
float logR2, R2, T, Tc, Tf;

// Les coefficients A, B et C.
float c1 = 1.129148e-03, c2 = 2.34125e-04, c3 = 8.76741e-08;

// void afficherMessageDemarrage() {
//   lcd.clear();
//   lcd.createChar(0, chiffre96);
//   lcd.setCursor(0, 0);
//   lcd.print("Nom: Diagne");
//   lcd.setCursor(0, 1);
//   lcd.write(byte(0));
//   lcd.setCursor(14,1);
//   lcd.print("96");
//   delay(3000);
//   lcd.clear();
// }

void clic() {
  if (currentTask == TASK_OFF) {
    lcd.clear();
    currentTask = TASK_ON;
  } else {
    lcd.clear();
    currentTask = TASK_OFF;
  }
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);  // Initialiser l'écran LCD avec 16 colonnes et 2 lignes
  lcd.backlight();   // Allumer le rétroéclairage
  pinMode(PIN_LED, OUTPUT);
  bouton.attachClick(clic);
  lcd.clear();
  lcd.createChar(0, chiffre96);
  lcd.setCursor(0, 0);
  lcd.print("Nom: Diagne");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(14, 1);
  lcd.print("96");
  delay(3000);
  lcd.clear();
}

void lireCapteurs() {
  int valeurCapteur = analogRead(PIN_CAPTEUR_TEMP);
  //int temp = map(valeurCapteur, 0, 1023, 0, 50);
  R2 = R1 * (1023.0 / (float)valeurCapteur - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = T - 273.15;

  lcd.setCursor(0, 0);
  lcd.print("temp:");
  lcd.print(Tc);
  lcd.print("C");
  int currentTime = millis();
  if (Tc > 25) {
    etatClim = true;
    digitalWrite(PIN_LED, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("clim : ON ");

  } else if (Tc < 24) {

    etatClim = false;
    digitalWrite(PIN_LED, LOW);
    lcd.setCursor(0, 1);
    lcd.print("clim : OFF");
  }
}

void afficherVitesseEtDirection() {
  int joystickX = analogRead(PIN_JOYSTICK_X);
  int joystickY = analogRead(PIN_JOYSTICK_Y);
 
  int vitesse = (joystickX > 512) ? map(joystickX, 512, 1023, 0, 120) : map(joystickX, 0, 511, -25, 0);

  int direction = map(joystickY, 0, 1023, 90, -90);

  if (vitesse > 0) {
    lcd.setCursor(0, 1);
    lcd.print("Avance a ");
    lcd.print(vitesse);
    lcd.print(" km/h ");

  } else {
    lcd.setCursor(0, 1);
    lcd.print("Recule a ");
    lcd.print(vitesse);
    lcd.print("km/h ");
  }

  if (direction > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Droite a ");
    lcd.print(direction);
    lcd.print("Deg ");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Gauche a ");
    lcd.print(direction);
    lcd.print("Deg ");
  }
}

void changement01() {
  afficherVitesseEtDirection();
}

void changement02() {
  lireCapteurs();
}

void envoyerDonneesSerie() {
  int joystickY = analogRead(PIN_JOYSTICK_Y);
  int joystickX = analogRead(PIN_JOYSTICK_X);
  Serial.print("etd:2419796,x:");
  Serial.print(joystickX);
  Serial.print(",y:");
  Serial.print(joystickY);
  Serial.print(",sys:");
  Serial.println(etatClim ? 1 : 0);
}

void loop() {
  bouton.tick();
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 100) {
    lastTime = currentTime;
    envoyerDonneesSerie();
  }

  switch (currentTask) {
    case TASK_OFF:
      changement02();
      break;
    case TASK_ON:
      changement01();
      break;
  }
}
