#include "SerialOptions.h"
#include <stdexcept>

hal::SerialOptions::SerialOptions()
{
	UseDefaultOptions();
}

void hal::SerialOptions::UseDefaultOptions()
{
	_hal_uart_init.OverSampling = UART_OVERSAMPLING_16;
	SetDirection(bsp::ISerialDirection::RX_TX);
	SetBaudRate(115200);
	SetDataBits(8);
	SetParity(bsp::ISerialParity::None);
	SetStopBits(bsp::ISerialStopBits::One);
	SetHardwareFlowControl(bsp::ISerialHardwareFlowControl::None);
}

bsp::ISerialDirection hal::SerialOptions::Direction()
{
	switch (_hal_uart_init.Mode)
	{
	case UART_MODE_RX:
	{
		return bsp::ISerialDirection::RX;
	}
	case UART_MODE_TX:
	{
		return bsp::ISerialDirection::TX;
	}
	default:
	case UART_MODE_TX_RX:
	{
		return bsp::ISerialDirection::RX_TX;
	}
	}
}

void hal::SerialOptions::SetDirection(bsp::ISerialDirection value)
{
	switch (value)
	{
	case bsp::ISerialDirection::RX:
	{
		_hal_uart_init.Mode = UART_MODE_RX;
		break;
	}
	case bsp::ISerialDirection::TX:
	{
		_hal_uart_init.Mode = UART_MODE_TX;
		break;
	}
	default:
	case bsp::ISerialDirection::RX_TX:
	{
		_hal_uart_init.Mode = UART_MODE_TX_RX;
		break;
	}
	}
}

uint32_t hal::SerialOptions::BaudRate() const
{
	return _hal_uart_init.BaudRate;
}

void hal::SerialOptions::SetBaudRate(uint32_t value)
{
	_hal_uart_init.BaudRate = value;
}

uint8_t hal::SerialOptions::DataBits() const
{
	switch (_hal_uart_init.WordLength)
	{
	default:
	case UART_WORDLENGTH_8B:
	{
		return 8;
	}
	case UART_WORDLENGTH_9B:
	{
		return 9;
	}
	}
}

void hal::SerialOptions::SetDataBits(uint8_t value)
{
	switch (value)
	{
	default:
	case 8:
	{
		_hal_uart_init.WordLength = UART_WORDLENGTH_8B;
		break;
	}
	case 9:
	{
		_hal_uart_init.WordLength = UART_WORDLENGTH_9B;
		break;
	}
	}
}

bsp::ISerialParity hal::SerialOptions::Parity() const
{
	switch (_hal_uart_init.Parity)
	{
	default:
	case UART_PARITY_NONE:
	{
		return bsp::ISerialParity::None;
	}
	case UART_PARITY_EVEN:
	{
		return bsp::ISerialParity::Even;
	}
	case UART_PARITY_ODD:
	{
		return bsp::ISerialParity::Odd;
	}
	}
}

void hal::SerialOptions::SetParity(bsp::ISerialParity value)
{
	switch (value)
	{
	default:
	case bsp::ISerialParity::None:
	{
		_hal_uart_init.Parity = UART_PARITY_NONE;
		break;
	}
	case bsp::ISerialParity::Even:
	{
		_hal_uart_init.Parity = UART_PARITY_EVEN;
		break;
	}
	case bsp::ISerialParity::Odd:
	{
		_hal_uart_init.Parity = UART_PARITY_ODD;
		break;
	}
	}
}

bsp::ISerialStopBits hal::SerialOptions::StopBits() const
{
	switch (_hal_uart_init.StopBits)
	{
	default:
	case UART_STOPBITS_1:
	{
		return bsp::ISerialStopBits::One;
	}
	case UART_STOPBITS_2:
	{
		return bsp::ISerialStopBits::Tow;
	}
	}
}

void hal::SerialOptions::SetStopBits(bsp::ISerialStopBits value)
{
	switch (value)
	{
	default:
	case bsp::ISerialStopBits::One:
	{
		_hal_uart_init.StopBits = UART_STOPBITS_1;
		break;
	}
	case bsp::ISerialStopBits::Tow:
	{
		_hal_uart_init.StopBits = UART_STOPBITS_2;
		break;
	}
	}
}

bsp::ISerialHardwareFlowControl hal::SerialOptions::HardwareFlowControl() const
{
	switch (_hal_uart_init.HwFlowCtl)
	{
	default:
	case UART_HWCONTROL_NONE:
	{
		return bsp::ISerialHardwareFlowControl::None;
	}
	case UART_HWCONTROL_RTS:
	{
		return bsp::ISerialHardwareFlowControl::RTS;
	}
	case UART_HWCONTROL_CTS:
	{
		return bsp::ISerialHardwareFlowControl::CTS;
	}
	case UART_HWCONTROL_RTS_CTS:
	{
		return bsp::ISerialHardwareFlowControl::RTS_CTS;
	}
	}
}

void hal::SerialOptions::SetHardwareFlowControl(bsp::ISerialHardwareFlowControl value)
{
	switch (value)
	{
	default:
	case bsp::ISerialHardwareFlowControl::None:
	{
		_hal_uart_init.HwFlowCtl = UART_HWCONTROL_NONE;
		break;
	}
	case bsp::ISerialHardwareFlowControl::RTS:
	{
		_hal_uart_init.HwFlowCtl = UART_HWCONTROL_RTS;
		break;
	}
	case bsp::ISerialHardwareFlowControl::CTS:
	{
		_hal_uart_init.HwFlowCtl = UART_HWCONTROL_CTS;
		break;
	}
	case bsp::ISerialHardwareFlowControl::RTS_CTS:
	{
		_hal_uart_init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
		break;
	}
	}
}
