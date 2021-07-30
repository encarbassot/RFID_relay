# RFID_relay
# Arduino relé operado por tag RFID 

Este código te permite configurar nuevos tags en la EEPROM para mantener la memoria al retirar la alimentación.

Ha sido principalmente creado para un amigo que quiere encender su moto con un tag.


## Cableado
  RFID-RC522
  * 3.3V->  3v3
  * RST   ->  (D9)
  * GND   ->  GND
  * IRQ   ->  NON_CONECCTED
  * MISO  ->  D12
  * MOSI  ->  D11
  * SCK   ->  D13
  * SDA   ->  (D10)

RELE
  * Vcc -> 5V
  * GND -> GND
  * IN  -> (A5)

## Instalación
Descargar el archivo [RFID_relay.ino](https://github.com/encarbassot/RFID_relay/blob/main/RFID_relay.ino) dentro de una carpeta con el mismo nombre. (RFID_relay/RFID_relay.ino) y abrir el archivo con Arduino. 

#### A continuación se presentan las opciones de configuración:

- ### Permitir escritura de tags en la memoria
```c
#define WRITING 2
```

Sera necesario por lo menos una vez cargar el programa con esta opción para guardar nuestro tag.

Para ello acercaremos nuestro Tag mientras tenemos conectado el pin 2 y GND.

Esta opción también nos permite **borrar toda la memoria**, si el pin 2 y GND están conectados al iniciar el dispositivo (o reiniciar con el botón de la placa)

- ### Abrir rele sin tag
```c
#define JUMP_RELE 3
```

Esta opcion permite emular una lectura correcta de un tag y cambiar el estado del rele, al conectar el pin 3 con GND.

- ### Accionamiento del rele
El rele por defecto se activa al realizar una lectura satisfactoria y se desactiva en la siguiente lectura satisfactoria. Si usted desea que el rele funcione en modo pulsador. entonces necesita activar la siguiente opcion:
```c
#define PRESS 3000
```
esta opcion provocara que dada una lectura satisfactoria el rele se activara 3000ms (3s) y luego se desactiva automaticamente.

- ### SELF_DESTRUCT (Avanzado)
```c
#define SELF_DESTRUCT 4
//#define SELF_REPAIR
```

Esta opcion permite deshabilitar las dos anteriores. Para poder configurar el tag tranquilamente y al estar seguro, conectar el pin 4 a GND. Esto provocará que no se pueda modificar ningun dato ni puentear el rele con el pin 3. Esta opcion se guardará en memoria, esto quiere decir que al retirar la alimentacion, no se borrará.

Para poder volver a cargar algun codigo sera necesario cargar una vez el codigo con **SELF_REPAIR** descomentado. Esto borrara de la memoria el anterior **SELF_DESTRUCT**.




## Uso
Para un uso basico recomendado, primero cargar el programa con la siguiente configuracion.
```c
#define WRITING 2
//#define JUMP_RELE 3
//#define SELF_DESTRUCT 4
//#define SELF_REPAIR
```
Esto permitira poder escrivir en la memoria el tag deseado.

**ALERTA** ahora al conectar pin 2 y GND se podra escrivir un nuevo tag.
Para evitar esto, se recomienda volver a cargar el programa con todas las opciones deshabilitadas.
```c
//#define WRITING 2
//#define JUMP_RELE 3
//#define SELF_DESTRUCT 4
//#define SELF_REPAIR
```
El programa se cargara sin las opciones de escribir nuevos tags, pero no borrara la memoria. De esta forma mantendremos nuestro tag guardado y sin posibilidad de ser modificado

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

inspirado en [https://arduinogetstarted.com/tutorials/arduino-rfid-nfc](https://arduinogetstarted.com/tutorials/arduino-rfid-nfc)
