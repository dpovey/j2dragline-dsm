#include "Io.h"

#ifndef _MODBUS_MESSAGE_H
#define _MODBUS_MESSAGE_H

namespace j2 {
    enum ModbusFunctionCode {
        InvalidFunction = 0,
        ReadCoilStatus = 1,
        ReadInputStatus = 2,
        ReadHoldingRegisters = 3,
        ReadInputRegisters = 4,
        ForceSingleCoil = 5,
        PresetSingleRegister = 6,
        ReadExceptionStatus = 7,
        ForceMultipleCoils = 15,
        PresetMultipleRegisters = 16,
        ReportSlaveId = 17        
    };


    class ModbusMessage : public virtual Serializable {
    public:
        virtual ModbusFunctionCode functionCode() const = 0;

        virtual int size() const = 0;
    };

    class ModbusRequest : public ModbusMessage {
    };
    
    class ModbusResponse : public ModbusMessage { 
    };

    class ReadHoldingRegistersRequest : public ModbusRequest {
    public:
        ReadHoldingRegistersRequest() { }

        ReadHoldingRegistersRequest(uint16_t startingAddress, uint16_t numberOfRegisters) :
            startingAddress(startingAddress),
            numberOfRegisters(numberOfRegisters) {
        }
        
        ModbusFunctionCode functionCode() const { return ReadHoldingRegisters; }

        int size() const { return 4; }

        void serialize(IoWriter& writer) const {
            writer
                .write(startingAddress)
                .write(numberOfRegisters);
        }

        void deserialize(IoReader& reader) {
            reader
                .read(&startingAddress)
                .read(&numberOfRegisters);
        }

    public:
        uint16_t startingAddress;
        uint16_t numberOfRegisters;
    };

    class ReadHoldingRegistersResponse : public ModbusResponse {
    public:
        ReadHoldingRegistersResponse() { }

        ReadHoldingRegistersResponse(std::vector<uint16_t> registers) :
            registers(registers)
        {
        }

        ModbusFunctionCode functionCode() const { return ReadHoldingRegisters; }

        int size() const { return 2 + (registers.size() * 2); }


        void serialize(IoWriter& writer) const {
            writer
                .write(uint16_t(registers.size())) 
                .write(registers);                   
        }

        void deserialize(IoReader& reader) {
            registers.clear();
            uint16_t nrRegisters;
            reader.read(&nrRegisters);
            reader.buffer(nrRegisters * 2);
            reader.read(registers, nrRegisters);
        }

    public:
        std::vector<uint16_t> registers;
    };
        
        

} // namespace j2

#endif // _MODBUS_MESSAGE_H
