#include <bsp-interface/di/serial.h>
#include <SerialOptions.h>

std::shared_ptr<bsp::ISerialOptions> DICreate_ISerialOptions()
{
    return std::shared_ptr<bsp::ISerialOptions>{new hal::SerialOptions{}};
}
