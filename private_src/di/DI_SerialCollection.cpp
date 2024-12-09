#include <base/container/Dictionary.h>
#include <base/di/SingletonGetter.h>
#include <bsp-interface/di/interrupt.h>
#include <bsp-interface/di/serial.h>
#include <Serial.h>

base::IDictionary<std::string, bsp::ISerial *> const &DI_SerialCollection()
{
    class Initializer
    {
    private:
        Initializer()
        {
            Add(&hal::Serial::Instance());
        }

        void Add(bsp::ISerial *serial)
        {
            _collection.Add(serial->Name(), serial);
        }

    public:
        base::Dictionary<std::string, bsp::ISerial *> _collection;

        static_function Initializer &Instance()
        {
            class Getter :
                public base::SingletonGetter<Initializer>
            {
            public:
                std::unique_ptr<Initializer> Create() override
                {
                    return std::unique_ptr<Initializer>{new Initializer{}};
                }

                void Lock() override
                {
                    DI_DisableGlobalInterrupt();
                }

                void Unlock() override
                {
                    DI_EnableGlobalInterrupt();
                }
            };

            Getter g;
            return g.Instance();
        }
    };

    return Initializer::Instance()._collection;
}
