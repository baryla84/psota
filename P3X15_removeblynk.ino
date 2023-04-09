                              //Zainstlować zestaw płytek esp8266   NARZĘDZIA -> Płytka -> Menedżer płytek   PRZY WGRYWANIU PROGRAM UŻYWAĆ PŁYTKI NodeMCU 1.0
//Zainstalować brakujące biblioteki   NARZĘDZIA -> Zarządzaj bibliotekami      Do obsługi na telefonie używać Firefoxa
#include <ESP8266WiFi.h>
#include <Streaming.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include <ESP8266HTTPClient.h>



elapsedMillis TimeStart; //Timer
OneWire oneWire(D7); //Pomiar temperatury
DallasTemperature sensors(&oneWire); //Przekazania informacji do biblioteki
Servo serwomechanizm;  //Tworzymy obiekt, dzięki któremu możemy odwołać się do serwa
WiFiServer server(80); // Set web server port number to 80

LiquidCrystal_I2C lcd(0x3F, 20, 4);  // definiowanie lcd

DeviceAddress pomiar = { 0x28, 0x98, 0xEF, 0x73, 0x27, 0x19, 0x1, 0x75 };   // nadanie nazwy czujnikowi temp od sn czujnika
DeviceAddress pomiar1 = { 0x28, 0x35, 0x46, 0x80, 0x27, 0x19, 0x1, 0x5A };  // nadanie nazwy czujnikowi temp od sn czujnika
DeviceAddress pomiar2 = { 0x28, 0xFD, 0x17, 0x26, 0x5F, 0x14, 0x1, 0xDF };  // nadanie nazwy czujnikowi temp od sn czujnika
DeviceAddress pomiar3 = { 0x28, 0x16, 0xB0, 0x4F, 0x1A, 0x19, 0x1, 0xE2 };  // nadanie nazwy czujnikowi temp od sn czujnika

// Variable to store the HTTP request
String header;
String outputTEMP;
String kociolTEMP;
String koldolTEMP;
String przekaznikiTEMP;



//Dane połaczenia Wi-Fi
//Domyślny adres IP: 192.168.43.15
//Poniżej parametry do ustawienia staycznego IP dla ESP8266 - NIE ZAWSZE DZIAŁAJĄ ZALEŻY OD ROUTERA, NALEZY WIĘC UŻYĆ DOMYŚLNEGO IP
IPAddress staticIP(192, 168, 1, 14); //ESP static ip
IPAddress gateway(192, 168, 1, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);

//Nazwa urządzenia i dane WIFI
const char* deviceName = "Netis";
const char* ssid = "Piwnica";
const char* password = "samuraicana102";

//Do ustawienia:
volatile int czas = 900000; // to 15 min
volatile int stan = 0;
volatile float T = 0; //nie ustawiać, wartość temperatury kolumny
volatile float Td = 0; // nie ustawiać, wartość temperatury czujnika na dole kolumny
volatile float Tk = 0; //nie ustawiać, wartość temperatury kotła 
volatile float Tp = 0; //nie ustawiać, wartość temeperatury radiatora przekazników SSR
volatile int Z = 180; //Serwo wartość początkowa 180-zamknięte 0-otwarte
volatile float To = 78; //Temperatura optymalna
volatile float Ta = 86; //Temperatura alarmowa
volatile float Tos = 82; //Temperatura ostrzegawcza
volatile float Tt = 0.3; //Tolerancja temperatury
volatile int Zk = 5; //Co ile ma się zmieniać pozycja serwa
volatile int pozycja = Z; //Aktualna pozycja serwa 0-180
volatile int cykl = 0; // Deklaracja licznika cykli
volatile int maxCykl = 10; //Max. wartość licznika cykli do ustawienia
 
// Auxiliar variables to store the current output state
String output5State = "OFF";
String output6State = "OFF";
String output8State = "OFF";

// Assign output variables to GPIO pins
const int output5 = 14;
const int output6 = 12;
const int output8 = 15;
const int buzer = 4;





void setup()
{
  sensors.begin();
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  WiFi.disconnect(); 
  
  serwomechanizm.attach(5);  //Serwomechanizm podłączony do pinu D1
  serwomechanizm.write(pozycja); //Bazowanie serwa
    
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output6, OUTPUT);
  pinMode(output8, OUTPUT);
  pinMode(buzer, OUTPUT);
  
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output6, LOW);
  digitalWrite(output8, LOW);
  digitalWrite(buzer, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(deviceName); 
  WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println(""); 
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
 
  

  // inicjacja lcd

  Wire.begin(2,0);
  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight 
  

}



void loop()
{
  
  temperatura(); 

  switch (stan)
  {
    case 1:
          digitalWrite(output5, HIGH);
          digitalWrite(output6, HIGH);
          digitalWrite(output8, HIGH);
          
          output5State = "ON";
          output6State = "ON";
          output8State = "ON";

          lcd.setCursor(10, 0); // komunikacja z LCD
          lcd.print("STAN 1"); // komunikacja z LCD

          
          
          Z = 180;
          pozycja = Z;                   
          ruch_serwem();
          
          if(T >= To)
          {
            TimeStart = 0;
            stan = 2;
            Serial.println("STAN 2");
          }        
          break;
          
    case 2:
          digitalWrite(output5, HIGH);
          digitalWrite(output6, HIGH);
          digitalWrite(output8, HIGH);
          
          output5State = "ON";
          output6State = "ON";
          output8State = "ON";

          lcd.setCursor(10, 0); // komunikacja z LCD
          lcd.print("STAN 2"); // komunikacja z LCD
          
          Serial.print("Czas: ");
          Serial.println(TimeStart);
          
          if (TimeStart >= czas)
          {
            Z = 180;
            pozycja = Z;
            ruch_serwem();                     
            alarmON();
          }
          //Ręczne przejście do 3
          break; 
                      
    case 3:         
          digitalWrite(output5, LOW);
          digitalWrite(output6, HIGH);
          digitalWrite(output8, HIGH);
          
          output5State = "OFF";
          output6State = "ON";
          output8State = "ON";

          lcd.setCursor(10, 0); // komunikacja z LCD
          lcd.print("STAN 3"); // komunikacja z LCD

          //EWENTUALNIE ZMODYFIKOWAĆ WARUNKI OTW/ZAM ZAWORU PONIŻEJ
          
          //if(Z <= 180) // Można odkomentować alarm
          //{
          // alarmON();
          //}

          //Wykonanie warunków tylko wtedy gdy określony cykl
          if(cykl == maxCykl)
          {
              if(Z > 179)
              {
                Z =179;
              }
              else if(Z < 1)
              {
                Z = 0;
              }
              else if(Td < To)
              {
                Z =Z-Zk;
                pozycja = Z;
                ruch_serwem();
              }
              else if(Td > (To+3*Tt))
              {
                Z=Z+(3*Zk);
                pozycja = Z;
                ruch_serwem();
              }
              else if(Td > (To+2*Tt))
              {
                Z=Z+(2*Zk);
                pozycja = Z;
                ruch_serwem();
              }
              else if(Td > To + Tt)
              {
                Z=Z+Zk;
                pozycja = Z;
                ruch_serwem();
              }
              
          }

          //Ręczne przejście do 4
          break;
          
   case 4:                  
          digitalWrite(output5, LOW);
          digitalWrite(output6, HIGH);
          digitalWrite(output8, HIGH);

          
          output5State = "OFF";
          output6State = "ON";
          output8State = "ON";
          
          lcd.setCursor(10, 0); // komunikacja z LCD
          lcd.print("STAN 4"); // komunikacja z LCD
                            
          Z = 180;
          pozycja = Z;
          ruch_serwem();                
          break;
  }


    
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client)
  {                             
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
       if (client.available()) {            // if there's bytes to read from the client
        char c = client.read();             // read a byte
        header += c;                        // if the byte is a newline character
        if (c == '\n')
        {                    
          if (currentLine.length() == 0)
          {                     
            if (header.indexOf("GET /5/on") >= 0)
            {
              output5State = "ON";
              digitalWrite(output5, HIGH);
            }
            else if (header.indexOf("GET /5/off") >= 0)
            {
              output5State = "OFF";
              digitalWrite(output5, LOW);
            }
            else if (header.indexOf("GET /6/on") >= 0)
            {
              output6State = "ON";
              digitalWrite(output6, HIGH);
            }
            else if (header.indexOf("GET /6/off") >= 0)
            {
              output6State = "OFF";
              digitalWrite(output6, LOW);
            }
            else if (header.indexOf("GET /8/on") >= 0)
            {
              output8State = "ON";
              digitalWrite(output8, HIGH);
            }
            else if (header.indexOf("GET /8/off") >= 0)
            {
              output8State = "OFF";
              digitalWrite(output8, LOW);
            }
            
            else if (header.indexOf("GET /refresh") >= 0)
            {
            }
            else if (header.indexOf("GET /down") >= 0)
            {
              pozycja = pozycja - Zk;
              Z = Z - Zk;
              ruch_serwem();
            }
            else if (header.indexOf("GET /up") >= 0)
            {
              pozycja = pozycja + Zk;
              Z = Z + Zk;
              ruch_serwem();
            }
            else if (header.indexOf("GET /auto") >= 0)
            {
              stan = 1;
              Serial.println("STAN 1");
            }
            else if (header.indexOf("GET /manual") >= 0)
            {
              output5State = "OFF";
              digitalWrite(output5, LOW);
              output6State = "OFF";
              digitalWrite(output6, LOW);
              stan = 0;
              output8State = "OFF";
              digitalWrite(output8, LOW);
              stan = 0;
              Serial.println("STAN 0");


              
          lcd.setCursor(10, 0); // komunikacja z LCD
          lcd.print("MANUAL"); // komunikacja z LCD
              
            }
            else if (header.indexOf("GET /alarm") >= 0)
            {
              alarmOFF();
            }
            else if (header.indexOf("GET /Tt/down") >= 0)
            {
              Tt = Tt - 0.3;
            }
            else if (header.indexOf("GET /Ta/down") >= 0)
            {
              Ta = Ta - 1;
            }
            else if (header.indexOf("GET /To/down") >= 0)
            {
              To = To - 1;
            }
            else if (header.indexOf("GET /Tt/up") >= 0)
            {
              Tt = Tt + 0.3;
            }
            else if (header.indexOf("GET /Ta/up") >= 0)
            {
              Ta = Ta + 1;
            }
            else if (header.indexOf("GET /To/up") >= 0)
            {
              To = To + 1;
            }
            else if (header.indexOf("GET /stan/1") >= 0)
            {
              stan = 1;
              Serial.println("STAN 1");
            }
            else if (header.indexOf("GET /stan/2") >= 0)
            {
              stan = 2;
              Serial.println("STAN 2");
            }
            else if (header.indexOf("GET /stan/3") >= 0)
            {
              alarmOFF(); 
              stan = 3;
              Serial.println("STAN 3");
              Z=125;
              pozycja = Z; 
              ruch_serwem(); 
            }
            else if (header.indexOf("GET /stan/4") >= 0)
            {
              alarmOFF();
              stan = 4;
              Serial.println("STAN 4");
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 36px;");
            client.println("text-decoration: none; font-size: 28px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Temperatura szczytu kolumny: " + outputTEMP +  "</h1>");
            client.println("<body><h1>Temperatura na dole kolumny: " + koldolTEMP +  "</h1>");          
            client.println("<body><h1>Temperatura kotla: " + kociolTEMP +  "</h1>");
            client.println("<body><h1>Temperatura SSR: " + przekaznikiTEMP +  "</h1>");
            client.println("<p>STAN: " + String(stan) + "</p>");            
                  
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>Przekaznik 1: " + output5State + "</p>");
            // If the output5State is off, it displays the ON button       
            if (output5State=="OFF") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 6 
            client.println("<p>Przekaznik 2: " + output6State + "</p>");
            // If the output6State is off, it displays the ON button       
            if (output6State=="OFF")
            {
              client.println("<p><a href=\"/6/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/6/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
                // Display current state, and ON/OFF buttons for GPIO 8 
            client.println("<p>Przekaznik 3: " + output8State + "</p>");
            // If the output8State is off, it displays the ON button       
            if (output8State=="OFF")
            {
              client.println("<p><a href=\"/8/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/8/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // --><--
            client.println("<p></p>");
            client.println("<p>Pozycja serwa: " + String(pozycja) + "</p>");
            // If the output6State is off, it displays the ON button       
            client.println("<p><a href=\"/down\"><button class=\"button\"><--</button></a><a href=\"/up\"><button class=\"button\">--></button></a></p>");

            // Button STAN
            client.println("<p>Zmiana trybu pracy na:</p>");                
            if (stan == 0)
            {
              client.println("<p><a href=\"/auto\"><button class=\"button\">AUTO</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/manual\"><button class=\"button button2\">MANUAL</button></a></p>");
            }
            
            client.println("<p>Tolerancja temperatury: " + String(Tt) + " </p>");
            // If the output6State is off, it displays the ON button       
            client.println("<p><a href=\"/Tt/down\"><button class=\"button\"><--</button></a><a href=\"/Tt/up\"><button class=\"button\">--></button></a></p>");

            client.println("<p>Temperatura alarmowa: " + String(Ta) + " </p>");
            // If the output6State is off, it displays the ON button       
            client.println("<p><a href=\"/Ta/down\"><button class=\"button\"><--</button></a><a href=\"/Ta/up\"><button class=\"button\">--></button></a></p>");

            client.println("<p>Temperatura optymalna: " + String(To) + " </p>");
            // If the output6State is off, it displays the ON button       
            client.println("<p><a href=\"/To/down\"><button class=\"button\"><--</button></a><a href=\"/To/up\"><button class=\"button\">--></button></a></p>");

            // Alarm
            client.println("<p></p>");                 
            client.println("<p><a href=\"/alarm\"><button class=\"button\">AlarmReset</button></a></p>");

            // STAN 1
            client.println("<p></p>");                 
            client.println("<p><a href=\"/stan/1\"><button class=\"button\">STAN 1</button></a></p>");
            
            // STAN 2
            client.println("<p></p>");                 
            client.println("<p><a href=\"/stan/2\"><button class=\"button\">STAN 2</button></a></p>");

            // STAN 3
            client.println("<p></p>");                 
            client.println("<p><a href=\"/stan/3\"><button class=\"button\">STAN 3</button></a></p>");

            // STAN 4
            client.println("<p></p>");                 
            client.println("<p><a href=\"/stan/4\"><button class=\"button\">STAN 4</button></a></p>");
            
            // Refresh
            client.println("<p></p>");                   
            client.println("<p><a href=\"/refresh\"><button class=\"button\">Refresh</button></a></p>");            
            
            // The HTTP response ends with another blank line            
            client.println();
            client.println("</body></html>");
            
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    //Serial.println("Client disconnected.");
  }
}

void temperatura()
{
  sensors.requestTemperatures();
  T = sensors.getTempC(pomiar);
  outputTEMP = sensors.getTempC(pomiar);
  Serial.println(sensors.getTempC(pomiar));

  Td = sensors.getTempC(pomiar1);
  koldolTEMP = sensors.getTempC(pomiar1);
  Serial.println(sensors.getTempC(pomiar1));

  Tk = sensors.getTempC(pomiar2);
  kociolTEMP = sensors.getTempC(pomiar2);     
  Serial.println(sensors.getTempC(pomiar2) );   

  Tp = sensors.getTempC(pomiar3);
  przekaznikiTEMP = sensors.getTempC(pomiar3);
  Serial.println(sensors.getTempC(pomiar3));

  
// wyswietlnie na lcd
  lcd.setCursor(0, 0);
  lcd.print("T1"); // Start Printing
  lcd.setCursor(3, 0);
  lcd.print(outputTEMP);
  lcd.setCursor(0, 1);
  lcd.print("T2"); // Start Printing
  lcd.setCursor(3, 1);
  lcd.print(koldolTEMP);
  lcd.setCursor(0, 2);
  lcd.print("T3"); // Start Printing
  lcd.setCursor(3, 2);
  lcd.print(kociolTEMP);



//przesyłanie danych na app blynk
 // Blynk.virtualWrite(7, outputTEMP);
//  Blynk.virtualWrite(8, koldolTEMP);
//  Blynk.virtualWrite(9, kociolTEMP);
//  Blynk.virtualWrite(10, przekaznikiTEMP);  
//  Blynk.virtualWrite(4, stan);




  byte grzalka1 = digitalRead(output5); // odczyt grzałki 
   if (grzalka1 == LOW)               // wyswietlanie stanu grzalki
  {
  lcd.setCursor(10, 1);
  lcd.print("          "); 
  }
  if (grzalka1 == HIGH)
  {
  lcd.setCursor(10, 1);
  lcd.print("GRZALKA 1"); 
  }

  byte grzalka2 = digitalRead(output6); // odczyt grzałki 
   if (grzalka2 == LOW)               // wyswietlanie stanu grzalki
  {
  lcd.setCursor(10, 2);
  lcd.print("          "); 
  }
  if (grzalka2 == HIGH)
  {
  lcd.setCursor(10, 2);
  lcd.print("GRZALKA 2"); 
  }

  byte grzalka3 = digitalRead(output8);
  if (grzalka3 == LOW)              // wyswietlanie stanu grzalki
  {
  lcd.setCursor(10, 3);
  lcd.print("          "); 
  }
  if (grzalka3 == HIGH)
  {
  lcd.setCursor(10, 3);
  lcd.print("GRZALKA 3"); 
  }
  
  if(T>=Ta)
  {
    alarmGLOBAL();
  }
  if(Tp >= 80)
  {
    alarmGLOBAL();
  }
  
 

  if(Tp >= 65)
 
  

  //Licznik cykli
  if (cykl < maxCykl)
  {
    cykl = cykl + 1;
  }
  else if (cykl >= maxCykl)
  {
    cykl = 0;
  } 
}

void ruch_serwem()

{
  if (pozycja > 180)
  {
    pozycja = 180;
  }
  else if (pozycja < 0)
  {
    pozycja = 0;
  }
  else if (pozycja >= 0 && pozycja <= 180)
  {
    serwomechanizm.write(pozycja);

    
//Wyswietlanie na lcd

  lcd.setCursor(0, 3);
  lcd.print("Serwo"); // Start Printing
  lcd.setCursor(6, 3);

if (Z <= 9)
  {
    lcd.print("  ");
    lcd.print(pozycja);
  }
  else if (Z <= 99)
  {
    lcd.print(" ");
    lcd.print(Z);
  }
  else if (Z >= 100)
  {
    lcd.print(Z);
  }
// blynk wyswietlanie
// Blynk.virtualWrite(3, pozycja);
    
  }               
}




void alarmON()
{
  digitalWrite(buzer, HIGH);
}

void alarmOFF()
{
  digitalWrite(buzer, LOW);
}

void alarmGLOBAL()
{
  digitalWrite(buzer, HIGH);
  digitalWrite(output5, LOW);
  digitalWrite(output6, LOW);
  digitalWrite(output8, LOW);

}




    
