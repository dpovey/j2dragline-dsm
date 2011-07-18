#include <algorithm>
#include "Io.h"
#include "ModbusMessage.h"
#include "ModbusTcpMessage.h"

#ifndef _MODBUS_H
#define _MODBUS_H

namespace j2 {

    /** @brief class for managing communication with PLC via Modbus over TCP */
    class Modbus {
    public:
        explicit Modbus(uint16_t unitId) :
            _transactionId(0),
            _unitId(unitId) { }

        ModbusRequest* read_request(IoReader& reader) {
            ModbusTcpMessage modbus_tcp_message;
            modbus_tcp_message.deserialize(reader);

            update_transaction_id(modbus_tcp_message.transactionId);

            // TODO check for error
            ModbusRequest* result;

            switch(modbus_tcp_message.functionCode) {
            case WriteMultipleRegisters:
                result = new WriteMultipleRegistersRequest();
                break;
            case ReadHoldingRegisters:
                result = new ReadHoldingRegistersRequest();
                break;
            default:
                //TODO: Exception?
                break;
            }
            MemoryIoReader data_reader(modbus_tcp_message.data);
            result->deserialize(data_reader);
            return result;
        }

        ModbusResponse* read_response(IoReader& reader) {
            ModbusTcpMessage modbus_tcp_message;
            modbus_tcp_message.deserialize(reader);
            // TODO check for error

            update_transaction_id(modbus_tcp_message.transactionId);

            ModbusResponse* result;
            switch(modbus_tcp_message.functionCode) {
            case WriteMultipleRegisters:
                result = new WriteMultipleRegistersResponse();
                break;
            case ReadHoldingRegisters:
                result = new ReadHoldingRegistersResponse();
                break;
            default:
                //TODO: Exception?
                break;
            }
            MemoryIoReader data_reader(modbus_tcp_message.data);                        
            result->deserialize(data_reader);
            return result;
        }

        void write(IoWriter& writer, const ModbusMessage& request) {
            ModbusTcpMessage(_transactionId, _unitId, request).serialize(writer);
        }

    private:
        void update_transaction_id(uint16_t received_transaction_id) {
            _transactionId = std::max(_transactionId, uint16_t(received_transaction_id + 1));
        }

    protected:
        uint16_t _transactionId;
        uint16_t _unitId;
    };

} // namespace j2

#endif // _MODBUS_H
