#include "Io.h"
#include "ModbusMessage.h"
#include "ModbusTcpMessage.h"

#ifndef _MODBUS_H
#define _MODBUS_H

namespace j2 {

    /** @brief class for managing communication with PLC via Modbus over TCP */
    class Modbus {
    public:
        ReadHoldingRegistersResponse request(const ReadHoldingRegistersRequest& request) {
            ReadHoldingRegistersResponse response;
            handleRequest(request, &response);
            return response;
        }
        
    private:
        void handleRequest(const ModbusRequest& request, ModbusResponse* response) {
            uint16_t requestTransactionId = _transactionId++;
            ModbusTcpMessage::outgoing(requestTransactionId,
                                       _unitId,
                                       request).serialize(*_writer);
            ModbusTcpMessage::incoming(requestTransactionId,
                                       _unitId,
                                       response).deserialize(*_reader);
        }
    protected:
        uint16_t _transactionId;
        uint16_t _unitId;
        std::tr1::shared_ptr<IoReader> _reader;
        std::tr1::shared_ptr<IoWriter> _writer;
    };

    /** @brief class used to build a Modbus instance */
    class ModbusConfigurer : public Modbus {
    public:
        ModbusConfigurer& transactionId(uint16_t transactionId) {
            _transactionId = transactionId; 
            return *this;
        }

        ModbusConfigurer& unitId(uint16_t unitId) {
            _unitId = unitId;
            return *this;
        }

        ModbusConfigurer& reader(std::tr1::shared_ptr<IoReader> reader) {
            _reader = reader;
            return *this;
        }

        ModbusConfigurer& writer(std::tr1::shared_ptr<IoWriter> writer) {
            _writer = writer;
            return *this;
        }

        Modbus& configure() { 
            return *this; 
        }
    };

} // namespace j2

#endif // _MODBUS_H
