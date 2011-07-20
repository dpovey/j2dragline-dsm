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
        explicit ModbusException(const std::string &err) : runtime_error(err) {}
    };
    
    class ModbusTcpMessage : public Serializable {
    public:
        ModbusTcpMessage() : _data(new buffer) {
        }

        ModbusTcpMessage(uint16_t transactionId,
                         uint16_t unitId,
                         const ModbusMessage& message) :
            transactionId(transactionId),
            protocolId(0),
            size(message.size() + sizeof(unitId) + sizeof(functionCode)),
            unitId(unitId),
            functionCode(message.functionCode()),
            _data(new buffer)            
        {
            _data->reserve(message.size() - (sizeof(unitId) + sizeof(functionCode)));
            MemoryIoWriter writer(_data);
            message.serialize(writer);
        }
            
        void serialize(IoWriter& writer) const {
            writer
                .write(transactionId)
                .write(protocolId)
                .write(size) 
                .write(unitId)
                .write(functionCode)
                .write(*_data);
        }

        void deserialize(IoReader& reader) {
            reader.read(&transactionId);
            reader.read(&protocolId);
            reader.read(&size);
            reader.buffer(size);
            reader.read(&unitId);
            reader.read(&functionCode);
            int remaining = size - (sizeof(unitId) + sizeof(functionCode));
            if (remaining > 0) {
                _data->reserve(remaining);
                reader.read(*_data, remaining);
            }
  
            if (protocolId != 0) {
                reader.error(ModbusException("Invalid protocol id"));
            }
        }
        
        void message(ModbusMessage* message) {
            _message = std::tr1::shared_ptr<ModbusMessage>(message);
        }

        template <class T>
        const T& message() const {
            return *std::tr1::dynamic_pointer_cast<T>(_message);
        }

        const buffer& data() const { return *_data; }

    public:
        uint16_t transactionId;       // Unique transaction id
        uint16_t protocolId;          // Must be zero for Modbus TCP
        uint8_t functionCode;         // function
        uint16_t size;                // Data size
        uint8_t unitId;               // Unit id
        
    private:
        shared_buffer _data;           // Message data
        std::tr1::shared_ptr<ModbusMessage> _message; // Message
    };


    

} // namespace j2

#endif // _MODBUS_TCP_MESSAGE
