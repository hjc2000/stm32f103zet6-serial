#pragma once
#include <atomic>
#include <base/di/SingletonGetter.h>
#include <bsp-interface/di/interrupt.h>
#include <bsp-interface/di/task.h>
#include <bsp-interface/serial/ISerial.h>
#include <hal.h>
#include <memory>
#include <SerialOptions.h>

namespace hal
{
    class Serial : public bsp::ISerial
    {
    private:
        Serial() = default;

        UART_HandleTypeDef _uart_handle{};
        std::shared_ptr<bsp::IBinarySemaphore> _send_complete_signal = DICreate_BinarySemaphore();
        std::shared_ptr<bsp::IBinarySemaphore> _receive_complete_signal = DICreate_BinarySemaphore();
        std::shared_ptr<bsp::IMutex> _read_lock = DICreate_Mutex();
        int32_t _current_receive_count = 0;

#pragma region 初始化
        void InitializeGpio();
        void InitializeDma();
        void InitializeUart(SerialOptions const &options);
        void InitializeInterrupt();
#pragma endregion

#pragma region 被中断处理函数回调的函数
        static_function void OnReceiveEventCallback(UART_HandleTypeDef *huart, uint16_t pos);
        static_function void OnSendCompleteCallback(UART_HandleTypeDef *huart);
#pragma endregion

    public:
        static_function Serial &Instance()
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
                    DI_InterruptSwitch().DisableGlobalInterrupt();
                }

                void Unlock() override
                {
                    DI_InterruptSwitch().EnableGlobalInterrupt();
                }
            };

            Getter g;
            return g.Instance();
        }

        std::string Name()
        {
            return "serial";
        }

        /// @brief 打开串口
        /// @param options
        void Open(bsp::ISerialOptions const &options) override;

#pragma region Stream
        /// @brief 调用后临时启动 DMA 接收一次数据。
        /// @note 本类没有缓冲机制，所以上层应用如果调用 Read 不及时，会丢失数据。
        /// @note 因为调用一次 Read 具有一定开销，需要设置寄存器，使能中断，设置一些
        /// 状态变量。所以为了提高效率，每次调用 Read 时传入的 buffer 适当大一些，
        /// 并且 count 大一些。
        ///
        /// @param buffer
        /// @param offset
        /// @param count
        /// @return
        int32_t Read(uint8_t *buffer, int32_t offset, int32_t count) override;

        /// @brief 调用后临时启动 DMA 进行一次发送。
        /// @param buffer
        /// @param offset
        /// @param count
        void Write(uint8_t const *buffer, int32_t offset, int32_t count) override;

        void Close() override;
#pragma endregion
    };
} // namespace hal
