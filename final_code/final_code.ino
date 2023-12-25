//khai bao thu vien DHT11
#include <DHT.h>
//Khai bao thu LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>       // Vào Library Manager tải thư viện Thư viện Màn hình I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);  //Khai báo địa chỉ I2C (0x27 or 0x3F) và Màn hình 16x02

//khai bao cho nhiet do - do am
#define DHTPin A0          // Chân xuất tín hiệu của cảm biến nối chân số 6 Arduino
#define DHTType DHT11      // Khai báo kiểu cảm biến là DHT11
DHT dht(DHTPin, DHTType);  // Khai báo thư viện chân cảm biến và kiểu cảm biến.
 
//khai bao chan cho CBAS
int cbas = A1;  //Cảm biến nối chân số 5 Arduino
int gt_cbas;
int Relay_1 = 8;//ngo ra dieu khien relay den
//khai bao cho chan buzzer
int Relay_2 = 9;//ngo ra dieu khien relay buzzer
//Khai bao thu vien cho cam bien van tay AS608
#include <Adafruit_Fingerprint.h>
//Khai bao chan cho cam bien van tay
int Relay_4 = 12;  //ngo ra dieu khiem relay khoa cua
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)

//Khai bao thu vien Fingerprint va Serial
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// Set up the serial port to use softwareserial for fingerprint sensor
SoftwareSerial mySerial_finger(16, 17);
SoftwareSerial mySerial_sim800l(14, 15);
#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
#define mySerial_finger Serial2
#define mySerial_sim800l Serial3
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial_finger);

char incomingByte;
String inputString;
int _timeout;
String _buffer;
int gt_gas;
int gas = A2;
int Relay_3 = 13;  // ngo ra dieu khien relay quat
int i = 0;
int j = 0;
int k = 0;
// Đọc trạng thái quạt
//int fan = digitalRead(Relay_3);
String number = "0814897423";  //-> thay doi so dien thoai cua ban

void setup() {

  // khởi động cảm biến DHT
  dht.begin();
  //khoi tao LCD
  lcd.init();
  lcd.backlight();
  //khoi tao CBAS
  pinMode(cbas, INPUT);  //Cảm biến nhận tín hiệu
  pinMode(Relay_1, OUTPUT);
  digitalWrite(Relay_1, LOW);
  //khoi tao Buzzer
  pinMode(Relay_2, OUTPUT);
  digitalWrite(Relay_2, LOW);
  //khoi tao cam bien van tay AS608
  pinMode(Relay_4, OUTPUT);
  Serial.begin(9600);
  Serial3.begin(9600);
  while (!Serial);
  delay(100);
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) { delay(1); }
  }
  finger.getTemplateCount();
  Serial.print("Sensor contains ");
  Serial.print(finger.templateCount);
  Serial.println(" template(s)");
  Serial.println("Waiting for valid finger...");

  //khoi tao cho sim800L
  pinMode(Relay_3, OUTPUT);    //khai báo relay là output
  pinMode(gas, INPUT);         //khai bao gas là input
  digitalWrite(Relay_3, LOW);  //tắt relay
  mySerial_sim800l.begin(9600);
  while (!mySerial_sim800l.available()) {
    mySerial_sim800l.println("AT");
    delay(1000);
    Serial.println("Connecting...");  // kết nối
  }
  Serial.println("Connected!");
  mySerial_sim800l.println("AT+CMGF=1");  // khởi động chức năng SMS
  delay(1000);
  mySerial_sim800l.println("AT+CNMI=1,2,0,0,0");
  delay(1000);
  mySerial_sim800l.println("AT+CMGL=\"REC UNREAD\"");  // đọc tin nhắn tới
}

void loop() {

  //Doc cam bien van tay
  getFingerprintIDez();
  delay(50);  //don't ned to run this at full speed.
  
  //Doc nhiet do - do am
  float h = dht.readHumidity();     // Đọc độ ẩm môi trường
  float t = dht.readTemperature();  //Đọc nhiệt độ C
  Serial.print("DO AM: ");
  Serial.print(h);
  Serial.print("NHIET DO: ");
  Serial.print(t);
  Serial.println();
  delay(1000);

  //hien thi do am len LCD
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.setCursor(2, 0);
  lcd.print(h,0);
  lcd.setCursor(4, 0);
  lcd.print("%");

  //hien nhiet do len LCD
  lcd.setCursor(8, 0);
  lcd.print("T:");
  lcd.setCursor(10, 0);
  lcd.print(t,1);
  lcd.setCursor(14, 0);
  lcd.write(byte(223));  // Ký tự °C trong bảng mã ASCII
  lcd.setCursor(15, 0);
  lcd.print("C");
  delay(100);

  //hien thi nong do khi GAS
  lcd.setCursor(0, 1);
  lcd.print("G:");
  lcd.setCursor(2, 1);
  lcd.print(gt_gas);
  lcd.setCursor(5, 1);
  lcd.print("%");

  // Hiển thị trạng thái quạt lên LCD
  lcd.setCursor(8,1 );
  lcd.print("FAN:");

  //Doc CBAS
  gt_cbas = analogRead(cbas);  //Đọc giá trị analog của cảm biến và gán vào biến giatri
  if (gt_cbas > 450)           //Nếu giá trị quang trở lớn hơn 300
  {
    digitalWrite(Relay_1, HIGH);
    delay(1000);
    
  } else  //Ngược lại
  {
    digitalWrite(Relay_1, LOW);
    delay(1000);
  }

  Serial.print("Gia tri cam bien AS: ");
  Serial.println(gt_cbas);
  delay(450);

  //doc gia tri cam bien khi gas de dieu khien thong qua sim800L
  gt_gas = analogRead(gas);
  Serial.print("Nong do khi gas: ");
  Serial.println(gt_gas);
  if (gt_gas >= 250) {
    
    
    for (j = 0; j <= 1; j = j + 1)  // chi goi 2 lan
    {
      digitalWrite(Relay_2, HIGH);
      delay(1000);
      callNumber();
      for (i = 0; i <= 25; i = i + 1) {  //đếm 25s , trong khi 25s này nó vừa đủ gọi, hết 25s nó sẽ thực hiện bước tt là cúp máy
        Serial.println(i);
        delay(1000);
      }
      mySerial_sim800l.println("ATH");  // thực hiện lệnh cúp máy
    }
    for (k = 0; k < 1; k = k + 1) {
      SendMessage();
    };
  } else  //Ngược lại
  {
     digitalWrite(Relay_2, LOW);
     delay(1000);
  }
  control();  //ham dieu khien relay thong qua SMS
}
   
//ham cam bien van tay
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  digitalWrite(Relay_4, LOW);
  delay(1500);  //Change the door lock delay from here
  digitalWrite(Relay_4, HIGH);
  Serial.println("Unlocked");
  return finger.fingerID;
}

//ham dieu khien relay thong qua SMS
void control() {
  if (mySerial_sim800l.available()) {
    delay(100);
    // Serial Buffer
    while (mySerial_sim800l.available()) {
      incomingByte = mySerial_sim800l.read();
      inputString += incomingByte;
    }
    delay(10);
    Serial.println(inputString);
    inputString.toUpperCase();
    if (inputString.indexOf("OFF") > -1) {
      digitalWrite(Relay_3, LOW);  // kích mức thấp
      lcd.setCursor(12, 1);
      lcd.print("OFF");
    }
    if (inputString.indexOf("ON") > -1) {
      digitalWrite(Relay_3, HIGH);
      lcd.setCursor(12, 1);
      lcd.print("ON   ");
    }
    delay(50);
    //Delete Messages
    if (inputString.indexOf("OK") == -1) {
      mySerial_sim800l.println("AT+CMGDA=\"DEL ALL\"");
      delay(1000);
    }
    inputString = "";
  }
}
String _readSerial() {
  _timeout = 0;
  while (!mySerial_sim800l.available() && _timeout < 12000) {
    delay(13);
    _timeout++;
  }
  if (mySerial_sim800l.available()) {
    return mySerial_sim800l.readString();
  }
  //  loop();
}
void callNumber() {
  mySerial_sim800l.print(F("ATD"));
  mySerial_sim800l.print(number);
  mySerial_sim800l.print(F(";\r\n"));
  _buffer = _readSerial();
  Serial.println(_buffer);
}

void SendMessage() {
  //Serial.println ("Sending Message");
  mySerial_sim800l.println("AT+CMGF=1");  //Sets the GSM Module in Text Mode
  delay(500);
  //Serial.println ("Set SMS Number");
  mySerial_sim800l.println("AT+CMGS=\"" + number + "\"\r");  //Mobile phone number to send message
  delay(500);
  String SMS = "LUONG KHI GAS VUOT NGUONG";
  mySerial_sim800l.println(SMS);
  delay(100);
  mySerial_sim800l.println((char)26);  // ASCII code of CTRL+Z
  delay(500);
  _buffer = _readSerial();
  // loop();
}
