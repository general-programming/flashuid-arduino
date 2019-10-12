# flashuid-arduino

here we go again.

## why?

easy card uid cloning (mifare classic 1k ofc)

## requirements
  * arduino uno
  * MFRC522 board (you must connect to the default SPI ports on the arduino)
  * 2x16 lcd display lcd with i2c interface (you must connect to most used ports for i2c on the arduino)

## libraries
  * [MRFC522](https://github.com/miguelbalboa/rfid)
  * [LiquidCrystal I2C](https://github.com/johnrickman/LiquidCrystal_I2C)

## usage
  * tap original card to the reader
  * wait 2 seconds, tap the uid changeable card to the reader

## special greetz
  * the nfc hacking friend, [ave](https://gitlab.com/a)
  * [deviant ollam](https://twitter.com/deviantollam)
