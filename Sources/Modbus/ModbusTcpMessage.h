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
        static ModbusTcpMessage outgoing(uint16_t transactionId, uint16_t unitId, 
                                         const ModbusMessage& message);

        static ModbusTcpMessage incoming(uint16_t transactionId, uint16_t unitId, 
                                         ModbusMessage *message);

        ModbusTcpMessage(ModbusMessage* message) :
            messageIn(message),
            messageOut(0)
        { }
            
        void serialize(IoWriter& writer) const {
            assert(messageOut);
            writer
                .write(transactionId)
                .write(protocolId)
                .write(uint16_t(messageOut->size()))
                .write(unitId)
                .write(uint8_t(messageOut->functionCode()))
                .write(*messageOut);
        }

        void deserialize(IoReader& reader) {
            assert(messageIn);
            // reader
            //     .read(transactionId)
            //     .read(protocolId)
            //     .read(size)
            //     .read(unitId)
            //     .read(functionCode)
            //     .buffer(size)
            //     .read(*messageIn);
            reader.read(&transactionId);
            reader.read(&protocolId);
            reader.read(&size);
            reader.read(&unitId);
            reader.read(&functionCode);
            reader.buffer(size);
            reader.read(*messageIn);


            // TODO: Handle Modbus exceptions

            if (reader.hasError()) {
                throw ModbusException("Invalid modbus header");
            }

            if (protocolId != 0) {
                throw ModbusException("Invalid protocol id: " + protocolId);
            }

            // TODO: Push into reader.errorIf()
            if (messageIn->functionCode() != functionCode) {
                throw ModbusException("Invalid message");
            }
        }

    protected:
        // Incoming message
        ModbusTcpMessage(uint16_t transactionId, uint16_t unitId, ModbusMessage* message) :
            transactionId(transactionId),
            protocolId(0),
            size(message->size()),
            unitId(unitId),
            functionCode(message->functionCode()),
            messageIn(message),
            messageOut(0) 
        { }

        // Outgoing message
        ModbusTcpMessage(uint16_t transactionId, uint16_t unitId, const ModbusMessage* message) :
            transactionId(transactionId),
            protocolId(0),
            size(message->size()),
            unitId(unitId),
            functionCode(message->functionCode()),
            messageIn(0),
            messageOut(message)
        { }

    public:
        uint16_t transactionId;  // Unique transaction id
        uint16_t protocolId;     // Must be zero for Modbus TCP
        uint16_t size;           // Data size
        uint16_t unitId;         // Unit id
        uint8_t functionCode;    // function code
        ModbusMessage* messageIn;
        const ModbusMessage* messageOut;
    };

    inline ModbusTcpMessage ModbusTcpMessage::outgoing(uint16_t transactionId, uint16_t unitId, 
                                                       const ModbusMessage& message) {
        return ModbusTcpMessage(transactionId, unitId, &message);
    }

    inline ModbusTcpMessage ModbusTcpMessage::incoming(uint16_t transactionId, uint16_t unitId, 
                                     ModbusMessage *message) {
        return ModbusTcpMessage(transactionId, unitId, message);
    }

} // namespace j2

#endif // _MODBUS_TCP_MESSAGE
