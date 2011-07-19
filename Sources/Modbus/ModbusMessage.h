#include "Io.h"

#ifndef _MODBUS_MESSAGE_H
#define _MODBUS_MESSAGE_H

namespace j2 {
    enum ModbusFunctionCode {
        InvalidFunction = 0,
        ReadHoldingRegisters = 3,
        ReadExceptionStatus = 7,
        WriteMultipleRegisters = 16,
        ReportSlaveId = 17        
    };

    class ModbusRequest;
    class ModbusResponse;

    class ModbusMessage : public virtual Serializable {
    public:
        static ModbusRequest* request_for(ModbusFunctionCode code);
        static ModbusResponse* response_for(ModbusFunctionCode code);
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

        int size() const { 
            return sizeof(uint16_t) +
                (registers.size() * sizeof(uint16_t)); }


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


    class WriteMultipleRegistersRequest : public ModbusRequest {
    public:
        WriteMultipleRegistersRequest() { }

        WriteMultipleRegistersRequest(uint16_t starting_address,
                                  std::vector<uint16_t> registers) :
        starting_address(starting_address),
        registers(registers),
        nrBytes(registers.size()*2)
        {
        }

        ModbusFunctionCode functionCode() const { return WriteMultipleRegisters; }

        int size() const { return 
                sizeof(starting_address) +
                sizeof(uint16_t) + // nr registers
                sizeof(nrBytes) +
                + nrBytes;
        }

        void serialize(IoWriter& writer) const {
            assert(registers.size() < 256);
            writer
                .write(starting_address)
                .write(uint16_t(registers.size()))
                .write(nrBytes)
                .write(registers);                   
        }

        void deserialize(IoReader& reader) {
            registers.clear();
            uint16_t nrRegisters;
            reader.read(&starting_address);
            reader.read(&nrRegisters);
            reader.read(&nrBytes);
            assert(nrBytes == nrRegisters * 2);
            reader.buffer(nrBytes);
            reader.read(registers, nrRegisters);
        }

    public:
        uint16_t starting_address;
        std::vector<uint16_t> registers;
        uint8_t nrBytes;
    };


    class WriteMultipleRegistersResponse : public ModbusResponse {
    public:
        WriteMultipleRegistersResponse() { }

        WriteMultipleRegistersResponse(uint16_t startingAddress, uint16_t numberOfRegisters) :
            startingAddress(startingAddress),
            numberOfRegisters(numberOfRegisters) {
        }
        
        ModbusFunctionCode functionCode() const { return WriteMultipleRegisters; }

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

    inline ModbusRequest* ModbusMessage::request_for(ModbusFunctionCode code) {
        switch(code) {
        case ReadHoldingRegisters: return new ReadHoldingRegistersRequest;
        case WriteMultipleRegisters: return new WriteMultipleRegistersRequest;
        default: return 0; // TODO Exception
        }
    }
        
    inline ModbusResponse* ModbusMessage::response_for(ModbusFunctionCode code) {
        switch(code) {
        case ReadHoldingRegisters: return new ReadHoldingRegistersResponse;
        case WriteMultipleRegisters: return new WriteMultipleRegistersResponse;
        default: return 0; // TODO Exception
        }
    }

} // namespace j2

#endif // _MODBUS_MESSAGE_H
