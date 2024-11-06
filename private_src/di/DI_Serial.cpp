#include <Serial.h>

bsp::ISerial &DI_Serial()
{
    return hal::Serial::Instance();
}
