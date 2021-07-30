/************************
 * CREATED BY ELIOPUTTO *
 ************************/

//para compilar el programa es necesario instalar la libreria:
//MFRC522 by GithubCommunity

/*       ****FUNICONAMIENTO****

 SI TIENE WRITING DESCOMENDADO:
  * Se usara el pin del numero asignado a writing como interruptor para la escritura
  * si al iniciar el arduino esta conectado el pin WRITING con GND se BORRARA la memoria
  * si el pin esta conectado y se acerca un tag, sobrescrivira el tag anterior por el actual

 CUANDO EL PIN WRITING NO ESTE CONECTADO o ESTE COMENTADO:
  * al acercar un tag al lector, comprobbara si es el mismo que el guardado
  * y en caso de ser el mismo invertira el estado del rele

 SI TIENE EL JUMP_RELE DESCOMENDATO
  * al conectar el pin asignado a JUMP_RELE
  * actuara como si hubiese hecho la lectura correcta de un tag y invertira el estado del rele

*/

/****** CONNECTIONS ******
  RFID-RC522
  * 3.3V  ->  3v3
  * RST   ->  (D9)
  * GND   ->  GND
  * IRQ   ->  
  * MISO  ->  D12
  * MOSI  ->  D11
  * SCK   ->  D13
  * SDA   ->  (D10)
  */
  #define SS_PIN 10
  #define RST_PIN 9

  /*
  RELE
  * Vcc -> 5V
  * GND -> GND
  * IN  -> (A5)
  */
  #define RELE_PIN A5



/****** CONFIGURATION ******/

#define WRITING 2
//comentar para deshabilitar la ESCRITURA de los datos
//una vez escritos, si se vuelve a cargar el programa no se borrara lo que este guardado
//se recomienda descomentarlo, cargarlo, escrivir tu tag, comentarlo y volver a cargarlo
//de esta forma no se podra modificar

//#define JUMP_RELE 3
//descomentar para cambiar el estado del rele conectando el pin JUMP_RELE con GND

//#define PRESS 3000
//al estar descomentado PRESS, activara el rele al hacer una lectura y luego lo volvera a apagar despes de 3s
//al estar comentado PRESS, activara el rele al hacer una lectura, al volver a hacer una lectura lo desactivara



////////////////////////////////////////////////////////////////////////
////////////////////////////////AVANZADO////////////////////////////////
////////////////////////////////////////////////////////////////////////

//#define SELF_DESTRUCT 4
//al descomentar esta linea, si se conecta el pin SELF_DESTRUCT con GND
//deshabilitara WRITING y JUMP_RELE
//es mas recomendable descomentarlo por codigo.
//sera necesario borrar la eprom para revertir los cambios

//#define SELF_REPAIR
//la unica forma de poder volver a esccrivir nuevos tags despues de
//un SELF_DESTRUCT es cargar una vez el programa con esta opcion descomentada
//Si vuelve a estar interesado en cargar el programa con SELF_DESTRUCT
//sera necesario primero cargar un SELF_REPAIR y despues un SELF_DESTRUCT






//////////////   CODIGO   //////////////

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define EEPROM_POS_TAG 1
#define EEPROM_POS_SELF_DESTRUCT 0

bool releStatus = true;//invertir a false si al encender el rele se activa automaticamente
bool functionsEnabled=true;
int tag[100];
int taglen=0;

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  pinMode(RELE_PIN,OUTPUT);
  digitalWrite(RELE_PIN,releStatus);

  #ifdef JUMP_RELE
    pinMode(JUMP_RELE,INPUT_PULLUP);
  #endif

  #ifdef SELF_DESTRUCT
    pinMode(SELF_DESTRUCT,INPUT_PULLUP);
  #endif
  
  #ifdef SELF_REPAIR
    #ifdef SELF_DESTRUCT
      #error No puedes activar SELF_DESTRUCT durante un SELF_REPAIR
    #else
      EEPROM.write(EEPROM_POS_SELF_DESTRUCT,0);
    #endif
  #else
    //leer si se ha cometido algun selfDestruct
    functionsEnabled = EEPROM.read(EEPROM_POS_SELF_DESTRUCT) == 0;
  #endif

  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  
  #ifdef WRITING
    pinMode(WRITING,INPUT_PULLUP);

    //if function pin not pressed
    if(digitalRead(WRITING)){
      readEEPROM();

    //if function pin wired witd GND
    }else{//clear every single pin
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      Serial.println("EEPROM CLEAR");
    }
  #else
    readEEPROM();
  #endif  
}

void loop() {
  
  #ifdef JUMP_RELE
    if(digitalRead(JUMP_RELE)==LOW && functionsEnabled){
      releSwitch();
      delay(2000);
    }
  #endif

  #if defined SELF_DESTRUCT && (defined JUMP_RELE || defined WRITING)   
    if(digitalRead(SELF_DESTRUCT)==LOW){
      selfDestruct();
    }
  #endif

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) { // new tag is available && NUID has been readed

    #ifdef WRITING
      //function pin not wired
      if(digitalRead(WRITING)){//compare and open rele
        compareAndOpen(rfid.uid.size,rfid.uid.uidByte);

      //PIN 2 PUENTEADO
      }else{//write to eprom
        writeEEPROM(rfid.uid.size,rfid.uid.uidByte);
      }
    #else
      compareAndOpen(rfid.uid.size,rfid.uid.uidByte);
    #endif

    //cerrar lectura
    rfid.PICC_HaltA(); // halt PICC
    rfid.PCD_StopCrypto1(); // stop encryption on PCD
    
  }
}

void compareAndOpen(int len, byte * _tag){
  Serial.print("COMPARE ");
  bool equal = true;

  //solo si tiene la misma longitud
  if(taglen==len){
    
    for (int i = 0; i < len; i++) {
      Serial.print(_tag[i] < 0x10 ? " 0" : " ");Serial.print(_tag[i], HEX);
      equal = (_tag[i] == tag[i]) && equal;//comparacion
    }

    if(equal){//si todos los bits son iguales le damos chicha al rele
      releSwitch();
      return;
    }
  }
  Serial.println(" FAIL");
}

void readEEPROM(){
  Serial.print("EEPROM ");
  taglen = EEPROM.read(EEPROM_POS_TAG);

  //lee la eeprom y la almacena en variable
  int value;
  for(char i = 0; i<taglen; i++){
    value = EEPROM.read(EEPROM_POS_TAG+i+1);
    Serial.print(value<10?" 0":" ");
    Serial.print(value,HEX);
    tag[i]=value;
  }
  Serial.println();
}

#ifdef WRITING
void writeEEPROM(int len, byte * _tag){
  if(functionsEnabled){
  //guardamos la longitud del tag
  taglen = len;
  //y la escrivimos en la primera posicion de la memoria
  EEPROM.write(EEPROM_POS_TAG, taglen);

  for (int i = 0; i < len; i++) {
    //imprimimos
    Serial.print(_tag[i], HEX);
    //guardamos
    tag[i]=_tag[i];
    //y escrivimos cada byte
    EEPROM.write(i+1+EEPROM_POS_TAG,_tag[i]);
  }
  Serial.println(" EEPROM writen");
  }
}
#endif

#if defined SELF_DESTRUCT && (defined JUMP_RELE || defined WRITING)   
  void selfDestruct(){
    functionsEnabled=false;
    EEPROM.write(EEPROM_POS_SELF_DESTRUCT, 1);

  }
#endif

void releSwitch(){
  releStatus = !releStatus;
  digitalWrite(RELE_PIN,releStatus);
  #ifdef PRESS
    delay(PRESS);
    releStatus = !releStatus;
    digitalWrite(RELE_PIN,releStatus);
  #endif
  Serial.println(" SUCCESSS RELE SWITCH");
}

/*SOURCE
 * inspired by: https://arduinogetstarted.com/tutorials/arduino-rfid-nfc
 */