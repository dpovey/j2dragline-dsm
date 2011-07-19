#include <algorithm>
#include <iostream>
#include <tr1/functional>
#include "Io.h"
#include "ModbusMessage.h"
#include "ModbusTcpMessage.h"

#ifndef _MODBUS_H
#define _MODBUS_H

namespace j2 {

    /** @brief class for managing communication with PLC via Modbus over TCP */
    class ModbusTcp {
    public:
        ModbusTcp() { }

        typedef std::tr1::function<ModbusMessage* (ModbusFunctionCode)> ModbusMessageFactoryMethod;

        static ModbusTcpMessage read(IoReader& reader, 
                                     ModbusMessageFactoryMethod create_message);
        
        static ModbusTcpMessage read_request(IoReader& reader);
        
        static ModbusTcpMessage read_response(IoReader& reader);

        static void write_request(IoWriter& writer, const ModbusRequest& request);

        static void write_response(IoWriter& writer, 
                                   const ModbusTcpMessage& request,
                                   const ModbusResponse& response);
    };

    inline ModbusTcpMessage ModbusTcp::read(IoReader& reader, 
                                            ModbusMessageFactoryMethod create_message) {
        ModbusTcpMessage message;
        message.deserialize(reader);
        ModbusMessage* modbus_message = create_message((ModbusFunctionCode)message.functionCode);
        MemoryIoReader data_reader(message.data());
        modbus_message->deserialize(data_reader);
        message.set_message(modbus_message);
        return message;    
    }
        
    inline ModbusTcpMessage ModbusTcp::read_request(IoReader& reader) {
        return read(reader, ModbusMessage::request_for);
    }
    
    inline ModbusTcpMessage ModbusTcp::read_response(IoReader& reader) {
        return read(reader, ModbusMessage::response_for);
    }

    inline void ModbusTcp::write_request(IoWriter& writer, const ModbusRequest& request) {
        ModbusTcpMessage(uint16_t(0), uint16_t(0), request).serialize(writer);
    }

    inline void ModbusTcp::write_response(IoWriter& writer, 
                                          const ModbusTcpMessage& request,
                                          const ModbusResponse& response) {
        ModbusTcpMessage(request.transactionId, request.unitId, response).serialize(writer);
    }
    

} // namespace j2

#endif // _MODBUS_H
