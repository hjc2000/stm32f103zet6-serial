#include "Serial.h"
#include <base/LockGuard.h>
#include <bsp-interface/di/dma.h>
#include <bsp-interface/di/gpio.h>
#include <bsp-interface/di/interrupt.h>

using namespace hal;
using namespace bsp;

#pragma region 初始化

void hal::Serial::InitializeGpio()
{
    // PA9
    {
        bsp::IGpioPin *pin = DI_GpioPinCollection().Get("PA9");
        pin->OpenAsAlternateFunctionMode("af_push_pull",
                                         bsp::IGpioPinPullMode::PullUp,
                                         bsp::IGpioPinDriver::PushPull);
    }

    // PA10
    {
        bsp::IGpioPin *pin = DI_GpioPinCollection().Get("PA10");
        pin->OpenAsAlternateFunctionMode("af_input",
                                         bsp::IGpioPinPullMode::PullUp,
                                         bsp::IGpioPinDriver::PushPull);
    }
}

void hal::Serial::InitializeDma()
{
    // 初始化发送 DMA
    {
        auto options = DICreate_DmaOptions();
        options->SetDirection(bsp::IDmaOptions_Direction::MemoryToPeripheral);
        options->SetMemoryDataAlignment(1);
        options->SetMemoryIncrement(true);
        options->SetPeripheralDataAlignment(1);
        options->SetPeripheralIncrement(false);
        options->SetPriority(bsp::IDmaOptions_Priority::Medium);
        DI_DmaChannelCollection().Get("dma1_channel4")->Open(*options, &_uart_handle);
    }

    // 初始化接收 DMA
    {
        auto options = DICreate_DmaOptions();
        options->SetDirection(bsp::IDmaOptions_Direction::PeripheralToMemory);
        options->SetMemoryDataAlignment(1);
        options->SetMemoryIncrement(true);
        options->SetPeripheralDataAlignment(1);
        options->SetPeripheralIncrement(false);
        options->SetPriority(bsp::IDmaOptions_Priority::Medium);
        DI_DmaChannelCollection().Get("dma1_channel5")->Open(*options, &_uart_handle);
    }
}

void hal::Serial::InitializeUart(SerialOptions const &options)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    /*
     * 先立刻释放一次信号量，等会 Write 方法被调用时直接通过，不被阻塞。
     * 然后在发送完成之前，第二次 Write 就会被阻塞了，这还能防止 Write
     * 被多线程同时调用。
     */
    _send_complete_signal->Release();
    _uart_handle.Instance = USART1;
    _uart_handle.Init = options;
    _uart_handle.MspInitCallback = nullptr;
    HAL_UART_Init(&_uart_handle);

    /*
     * HAL_UART_Init 函数会把中断处理函数中回调的函数都设为默认的，所以必须在 HAL_UART_Init
     * 之后对函数指针赋值。
     */
    _uart_handle.RxEventCallback = OnReceiveEventCallback;
    _uart_handle.TxCpltCallback = OnSendCompleteCallback;
}

void hal::Serial::InitializeInterrupt()
{
    DI_IsrManager().AddIsr(static_cast<uint32_t>(IRQn_Type::USART1_IRQn),
                           [this]()
                           {
                               HAL_UART_IRQHandler(&_uart_handle);
                           });

    DI_IsrManager().AddIsr(static_cast<uint32_t>(IRQn_Type::DMA1_Channel4_IRQn),
                           [this]()
                           {
                               HAL_DMA_IRQHandler(_uart_handle.hdmatx);
                           });

    DI_IsrManager().AddIsr(static_cast<uint32_t>(IRQn_Type::DMA1_Channel5_IRQn),
                           [this]()
                           {
                               HAL_DMA_IRQHandler(_uart_handle.hdmarx);
                           });

    DI_EnableInterrupt(IRQn_Type::USART1_IRQn, 10);
    DI_EnableInterrupt(IRQn_Type::DMA1_Channel4_IRQn, 10);
    DI_EnableInterrupt(IRQn_Type::DMA1_Channel5_IRQn, 10);
}

#pragma endregion

#pragma region 被中断处理函数回调的函数

void Serial::OnReceiveEventCallback(UART_HandleTypeDef *huart, uint16_t pos)
{
    Serial::Instance()._current_receive_count = pos;
    Serial::Instance()._receive_complete_signal->ReleaseFromISR();
}

void Serial::OnSendCompleteCallback(UART_HandleTypeDef *huart)
{
    Serial::Instance()._send_complete_signal->ReleaseFromISR();
}

#pragma endregion

hal::Serial &hal::Serial::Instance()
{
    class Getter :
        public base::SingletonGetter<Serial>
    {
    public:
        std::unique_ptr<Serial> Create() override
        {
            return std::unique_ptr<Serial>{new Serial{}};
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

void Serial::Open(bsp::ISerialOptions const &options)
{
    InitializeGpio();
    InitializeDma();
    InitializeUart(static_cast<SerialOptions const &>(options));
    InitializeInterrupt();
}

#pragma region Stream

int32_t Serial::Read(uint8_t *buffer, int32_t offset, int32_t count)
{
    if (count > UINT16_MAX)
    {
        throw std::invalid_argument{"count 太大"};
    }

    base::LockGuard l{*_read_lock};
    while (true)
    {
        {
            bsp::GlobalInterruptGuard g;
            HAL_UARTEx_ReceiveToIdle_DMA(&_uart_handle, buffer + offset, count);

            /*
             * 通过赋值为空指针，把传输半满回调给禁用，不然接收的数据较长，超过缓冲区一半时，
             * 即使是一次性接收的，UART 也会回调 OnReceiveEventCallback 两次。
             *
             * 这个操作需要在临界区中，并且 DMA 的中断要处于 freertos 的管理范围内，否则无效。
             */
            _uart_handle.hdmarx->XferHalfCpltCallback = nullptr;
        }

        _receive_complete_signal->Acquire();
        if (_current_receive_count > 0)
        {
            return _current_receive_count;
        }
    }
}

void Serial::Write(uint8_t const *buffer, int32_t offset, int32_t count)
{
    _send_complete_signal->Acquire();
    HAL_UART_Transmit_DMA(&_uart_handle, buffer + offset, count);
}

void Serial::Close()
{
    HAL_UART_DMAStop(&_uart_handle);
    DI_DisableInterrupt(IRQn_Type::USART1_IRQn);
    DI_DisableInterrupt(IRQn_Type::DMA1_Channel4_IRQn);
    DI_DisableInterrupt(IRQn_Type::DMA1_Channel5_IRQn);
    DI_DmaChannelCollection().Get("dma1_channel4")->Close();
    DI_DmaChannelCollection().Get("dma1_channel5")->Close();
}

#pragma endregion
