#pragma once
#include <bsp-interface/serial/ISerialOptions.h>
#include <hal.h>

namespace hal
{
	class SerialOptions
		: public bsp::ISerialOptions
	{
	private:
		UART_InitTypeDef _hal_uart_init;

	public:
		SerialOptions();

		operator UART_InitTypeDef() const
		{
			return _hal_uart_init;
		}

		void UseDefaultOptions() override;

		bsp::ISerialDirection Direction() override;
		void SetDirection(bsp::ISerialDirection value) override;

		/// @brief 波特率。
		/// @return
		uint32_t BaudRate() const override;
		void SetBaudRate(uint32_t value) override;

		/// @brief 数据位的个数。
		/// @return
		uint8_t DataBits() const override;
		void SetDataBits(uint8_t value) override;

		/// @brief 校验位。
		/// @return
		bsp::ISerialParity Parity() const override;
		void SetParity(bsp::ISerialParity value) override;

		/// @brief 停止位个数。
		/// @return
		bsp::ISerialStopBits StopBits() const override;
		void SetStopBits(bsp::ISerialStopBits value) override;

		/// @brief 硬件流控。
		/// @return
		bsp::ISerialHardwareFlowControl HardwareFlowControl() const override;
		void SetHardwareFlowControl(bsp::ISerialHardwareFlowControl value) override;
	};
}
