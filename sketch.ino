
//220202128 Muhabat Baltayeva (iö)
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

#define Ekran_adresi 0x3C
#define Ekran_yuksekligi 64
#define Ekran_genisligi 128
#define Oled_sifirlama -1

Adafruit_SSD1306 oled(Ekran_genisligi, Ekran_yuksekligi, &Wire, Oled_sifirlama);

#define SEGT_F 12
#define SEGT_G 13
#define SEGT_A 7
#define SEGT_B 2
#define SEGT_E 9
#define SEGT_D 10
#define SEGT_C 11
#define Led_1 5
#define Led_2 6
#define Led_3 4

#define Tugla_basla_X 2
#define Tugla_basla_Y 10
#define Tugla_genislik 15
#define Tugla_aciklik 2
#define Tugla_satirlar 2
#define Tugla_yukseklik 6
#define Tugla_sutunlar 5
#define Tahta_yukseklik 4
#define Tahta_genislik 40
#define Slide_Pot A0
#define Top_yaricap 3
#define Tahta_Y_Poz 60
#define Tahta_Hiz 2
#define Foto_Pin A1
#define Joystick_button_pin 2
#define Joystick_X_pin A3 // Joystick X pin

#define LUX_esik 500 // Lux eşik değeri
#define Hizlanma 2
#define Yaricapi_olabilir 3

int canlar = 3; // Can sayısı
int seviye = 1; // Oyun seviyesi
int topHizX = 1;
int topHizY = 1;
bool ballReleased = false;
int puan = 0;         // Oyuncu puanı
int toplam_puan = 0;
int tahtaX = (Ekran_genisligi - Tahta_genislik) / 2;
int topX = Ekran_genisligi / 2;
int topY = Ekran_yuksekligi / 2;

// Tuğlaları tutacak matris
bool tuglalar[Tugla_sutunlar][Tugla_satirlar];

bool aktiflenebilir = false;
int canX, canY;

void setup() {
  Serial.begin(9600);

  pinMode(Led_1, OUTPUT);
  pinMode(Led_2, OUTPUT);
  pinMode(Led_3, OUTPUT);

  pinMode(SEGT_A, OUTPUT);
  pinMode(SEGT_B, OUTPUT);
  pinMode(SEGT_C, OUTPUT);
  pinMode(SEGT_D, OUTPUT);
  pinMode(SEGT_E, OUTPUT);
  pinMode(SEGT_F, OUTPUT);
  pinMode(SEGT_G, OUTPUT);

  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, LOW);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);

  delay(2000);
  displayMenu(); // Menüyü ekrana yazdır
}

void loop() {
  int secenek = getMenuSelection();

  if (secenek == 1) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("Oyun baslıyor...");
    delay(2000);
    startGame(); // Başlat seçeneği seçildiyse oyunu başlat
  } else if (secenek == 2) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("Oyuna gosterdiginiz ilgi icin tesekkurler...");
    oled.display();
    delay(2000);
    oled.clearDisplay();
    oled.display();
    while (1); // Sonsuz döngü, programı durdur
  }
}

void displayMenu() {
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("1. Start Game");
  oled.println("2. Exit");
  oled.display();
}

int getMenuSelection() {
  while (true) {
    int joystickButtonState = digitalRead(Joystick_button_pin);
    if (joystickButtonState == LOW) {
      int joystickX = analogRead(Joystick_X_pin);
      if (joystickX < 300) { // Sol yönde hareket ettiyse
        return 1; // 1 döndür, başlat seçeneğini seçti
      } else if (joystickX > 700) { // Sağ yönde hareket ettiyse
        return 2; // 2 döndür, çıkış seçeneğini seçti
      }
    }
    delay(10);
  }
}

void startGame() {
  oled.clearDisplay();
  resetGame(); // Oyunu sıfırla ve başlat
  displayLives();
  drawTuglalar(); // Tuğlaları ekrana çiz
  oled.display();

  while (canlar > 0) { // Canlar bitene kadar devam et
    moveTahta();
    if (!ballReleased && digitalRead(Joystick_button_pin) == LOW) {
      ballReleased = true;
    }
    if (ballReleased) {
      moveTop();
    }
    delay(10);
  }

  // Oyun bittiğinde ekrana mesajı yazdır
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Oyun Bitti!");
  oled.print("skorunuz: ");
  oled.println(toplam_puan);
  oled.display();
  delay(2000);
}

void resetGame() {
  // Tuğla matrisini başlangıçta doldur
  for (int sutun = 0; sutun < Tugla_sutunlar; sutun++) {
    for (int satir = 0; satir < Tugla_satirlar; satir++) {
      tuglalar[sutun][satir] = true;
    }
  }

  tahtaX = (Ekran_genisligi - Tahta_genislik) / 2;
  topX = Ekran_genisligi / 2;
  topY = Ekran_yuksekligi / 2;
  topHizX = 1 + seviye; // Seviyeye bağlı olarak hızı artır
  topHizY = 1 + seviye; // Seviyeye bağlı olarak hızı artır
  ballReleased = false;
}

void displayLives() {
  oled.setCursor(0, 0);
  oled.print("Can: ");
  oled.println(canlar);

  // LED'leri güncelle
  if (canlar >= 3) {
    digitalWrite(Led_1, HIGH);
    digitalWrite(Led_2, HIGH);
    digitalWrite(Led_3, HIGH);
  } else if (canlar == 2) {
    digitalWrite(Led_1, LOW);
    digitalWrite(Led_2, HIGH);
    digitalWrite(Led_3, HIGH);
  } else if (canlar == 1) {
    digitalWrite(Led_1, LOW);
    digitalWrite(Led_2, LOW);
    digitalWrite(Led_3, HIGH);
  } else {
    digitalWrite(Led_1, LOW);
    digitalWrite(Led_2, LOW);
    digitalWrite(Led_3, LOW);
  }
}

void displayPuan(int puan) {
  digitalWrite(SEGT_A, HIGH);
  digitalWrite(SEGT_B, HIGH);
  digitalWrite(SEGT_C, HIGH);
  digitalWrite(SEGT_D, HIGH);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, HIGH);
  digitalWrite(SEGT_G, HIGH);

  switch (puan) {
    case 1:
      displayOne();
      break;
    case 2:
      displayTwo();
      break;
    case 3:
      displayThree();
      break;
    case 4:
      displayFour();
      break;
    case 5:
      displayFive();
      break;
    case 6:
      displaySix();
      break;
    case 7:
      displaySeven();
      break;
    case 8:
      displayEight();
      break;
    case 9:
      displayNine();
      break;
    default:
      displayZero();
      break;
  }
}

void drawTuglalar() {
  for (int sutun = 0; sutun < Tugla_sutunlar; sutun++) {
    for (int satir = 0; satir < Tugla_satirlar; satir++) {
      if (tuglalar[sutun][satir]) {
        int x = Tugla_basla_X + sutun * (Tugla_genislik + Tugla_aciklik);
        int y = Tugla_basla_Y + satir * (Tugla_yukseklik + Tugla_aciklik);
        // Işık eşiği kontrolü
        if (analogRead(Foto_Pin) < LUX_esik) {
          oled.fillRect(x, y, Tugla_genislik, Tugla_yukseklik, WHITE);
          oled.invertDisplay(false);
        } else {
          oled.invertDisplay(true);
          oled.fillRect(x, y, Tugla_genislik, Tugla_yukseklik, WHITE);
        }
      }
    }
  }
}

void drawTahta() {
  oled.fillRect(tahtaX, Tahta_Y_Poz, Tahta_genislik, Tahta_yukseklik, WHITE);
}

void moveTahta() {
  int slideValue = analogRead(Slide_Pot);

  // Tahta'ın X konumunu slide potansiyometreden gelen değere göre güncelle
  tahtaX = map(slideValue, 0, 1023, 0, Ekran_genisligi - Tahta_genislik);

  oled.clearDisplay();
  drawTuglalar();
  drawTahta();
  drawTop();
  oled.display();
}

void drawTop() {
  oled.fillCircle(topX, topY, Top_yaricap, WHITE);
}

void moveTop() {
  oled.fillCircle(topX, topY, Top_yaricap, BLACK);

  topX += topHizX; // Seviyeye bağlı olarak hızı artır
  topY += topHizY; // Seviyeye bağlı olarak hızı artır

  // Check collision with walls
  if (topX + Top_yaricap >= Ekran_genisligi || topX - Top_yaricap <= 0) {
    topHizX = -topHizX;
  }

  // Check collision with ceiling
  if (topY - Top_yaricap <= 0) {
    topHizY = -topHizY;
  }

  // Check collision with paddle
  if (topY + Top_yaricap >= Tahta_Y_Poz && topY - Top_yaricap <= Tahta_Y_Poz + Tahta_yukseklik) {
    if (topX + Top_yaricap >= tahtaX && topX - Top_yaricap <= tahtaX + Tahta_genislik) {
      topHizY = -topHizY;
    }
  }

  // Check collision with bricks
  int topSutun = (topX - Tugla_basla_X) / (Tugla_genislik + Tugla_aciklik);
  int topSatir = (topY - Tugla_basla_Y) / (Tugla_yukseklik + Tugla_aciklik);

  if (topSatir >= 0 && topSatir < Tugla_satirlar && topSutun >= 0 && topSutun < Tugla_sutunlar && tuglalar[topSutun][topSatir]) {
    tuglalar[topSutun][topSatir] = false; // Tuğlayı yok et
    topHizY = -topHizY; // Topun yönünü tersine çevir
    toplam_puan++;
    puan++; // Puanı artır

    if (puan >= 10) {
      puan = puan % 10;
    }
    displayPuan(puan); // Puanı ekrana yazdır

    // Can düşme olasılığı hesapla
    if (random(1, 11) == 1) {
      createNewCan(topSutun, topSatir); // Kırılan tuğlanın olduğu konumdan canı düşür
    }
  }

  // Check collision with bottom
  if (topY + Top_yaricap >= Ekran_yuksekligi) {
    // Ball went out of bounds, reduce lives
    canlar--;
    displayLives();

    // Reset ball position
    topX = Ekran_genisligi / 2;
    topY = Ekran_yuksekligi / 2;
    topHizX = 1 + seviye; // Seviyeye bağlı olarak hızı artır
    topHizY = 1 + seviye; // Seviyeye bağlı olarak hızı artır
    ballReleased = true;

    // Pause for a moment before restarting
    delay(1000);
  }

  drawTop();
  moveCan();
  oled.display();

  // Check if all bricks are destroyed for level transition
  if (checkBricksDestroyed()) {
    seviye++;
    if (seviye > 3) {
      // Oyunu bitir
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.println("Tebrikler, Oyunu Tamamladınız!");
      oled.print("skorunuz: ");
      oled.println(toplam_puan);
      oled.display();
      delay(15000);
      resetGame();
    } else {
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.print("Seviye ");
      oled.println(seviye);
      oled.display();
      delay(2000);
      resetGame(); // Oyunu sıfırla ve yeni seviyeye geç
    }
  }
}

bool checkBricksDestroyed() {
  for (int sutun = 0; sutun < Tugla_sutunlar; sutun++) {
    for (int satir = 0; satir < Tugla_satirlar; satir++) {
      if (tuglalar[sutun][satir]) {
        return false; // Hala bir tuğla varsa false döndür
      }
    }
  }
  return true; // Tüm tuğlalar kırıldıysa true döndür
}

void displayZero() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, LOW);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);
}

void displayOne() {
  digitalWrite(SEGT_A, HIGH);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, HIGH);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, HIGH);
  digitalWrite(SEGT_G, HIGH);
}

void displayTwo() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, HIGH);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, LOW);
  digitalWrite(SEGT_F, HIGH);
  digitalWrite(SEGT_G, LOW);
}

void displayThree() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, HIGH);
  digitalWrite(SEGT_G, LOW);
}

void displayFour() {
  digitalWrite(SEGT_A, HIGH);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, HIGH);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);
}

void displayFive() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, HIGH);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);
}

void displaySix() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, HIGH);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, LOW);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);
}

void displaySeven() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, HIGH);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, HIGH);
  digitalWrite(SEGT_G, HIGH);
}

void displayEight() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, LOW);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);
}

void displayNine() {
  digitalWrite(SEGT_A, LOW);
  digitalWrite(SEGT_B, LOW);
  digitalWrite(SEGT_C, LOW);
  digitalWrite(SEGT_D, LOW);
  digitalWrite(SEGT_E, HIGH);
  digitalWrite(SEGT_F, LOW);
  digitalWrite(SEGT_G, LOW);
}

void createNewCan(int sutun, int satir) {
  aktiflenebilir = true;
  canX = Tugla_basla_X + sutun * (Tugla_genislik + Tugla_aciklik) + Tugla_genislik / 2;
  canY = Tugla_basla_Y + satir * (Tugla_yukseklik + Tugla_aciklik) + Tugla_yukseklik / 2;
}

void drawCan() {
  if (aktiflenebilir) {
    oled.fillCircle(canX, canY, Yaricapi_olabilir, WHITE);
  }
}

void moveCan() {
  if (aktiflenebilir) {
    oled.fillCircle(canX, canY, Yaricapi_olabilir, BLACK);
    canY += Hizlanma;
    if (canY + Yaricapi_olabilir >= Ekran_yuksekligi) {
      aktiflenebilir = false;
      canlar++;
      displayLives();
    }
    drawCan();
  }
}









