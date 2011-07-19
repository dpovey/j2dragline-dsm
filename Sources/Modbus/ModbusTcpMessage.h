#include <vector>
#include <stdexcept>
#include <inttypes.h>
#include <assert.h>
#include "Io.h"
#include "ModbusMessage.h"
 
#ifndef _MODBUS_TCP_MESSAGE
#define _MODBUS_TCP_MESSAGE

namespace j2 {

    class ModbusException: public std::runtime_error {
    public:
        ModbusException(const std::string &err) : runtime_error(err) {}
    };
    
    class ModbusTcpMessage : public Serializable {
    public:
        ModbusTcpMessage() : data(new buffer) {
        }

        ModbusTcpMessage(uint16_t transactionId,
                         uint16_t unitId,
                         const ModbusMessage& message) :
            transactionId(transactionId),
            protocolId(0),
            size(message.size() + sizeof(unitId) + sizeof(functionCode)),
            unitId(unitId),
            functionCode(message.functionCode()),
            data(new buffer)            
        {
            data->reserve(message.size() - (sizeof(unitId) + sizeof(functionCode)));
            MemoryIoWriter writer(data);
            message.serialize(writer);
        }
            
        void serialize(IoWriter& writer) const {
            writer
                .write(transactionId)
                .write(protocolId)
                .write(size) 
                .write(unitId)
                .write(functionCode)
                .write(*data);
        }

        void deserialize(IoReader& reader) {
            reader.read(&transactionId);
            reader.read(&protocolId);
            reader.read(&size);
            reader.buffer(size);
            reader.read(&unitId);
            reader.read(&functionCode);
            int remaining = size - (sizeof(unitId) + sizeof(functionCode));
            if (remaining) data->reserve(remaining);
            reader.read(*data, remaining);
            
            puts("-----------------------------------------");

            // TODO: Handle Modbus exceptions

            if (reader.hasError()) {
                throw ModbusException("Invalid modbus header");
            }

            if (protocolId != 0) {
                throw ModbusException("Invalid protocol id: " + protocolId);
            }
        }
        
        void set_message(ModbusMessage* message) {
            _message = std::tr1::shared_ptr<ModbusMessage>(message);
        }

        template <class T>
        const T& message() const {
            return *std::tr1::dynamic_pointer_cast<T>(_message);
        }

    public:
        uint16_t transactionId;       // Unique transaction id
        uint16_t protocolId;          // Must be zero for Modbus TCP
        uint8_t functionCode;         // function
        uint16_t size;                // Data size
        uint8_t unitId;               // Unit id
        shared_buffer data;           // Message data
        
    private:
        std::tr1::shared_ptr<ModbusMessage> _message; // Message
    };


    

} // namespace j2

#endif // _MODBUS_TCP_MESSAGE
