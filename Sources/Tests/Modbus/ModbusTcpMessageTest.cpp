#include <utility>
#include <gtest/gtest.h>
#include "Io.h"
#include "Modbus.h"

using namespace std;
using namespace std::tr1;
using namespace j2;

template<class REQUEST_TYPE, class RESPONSE_TYPE>
pair<REQUEST_TYPE, RESPONSE_TYPE> sendRequestAndGetResponse(REQUEST_TYPE& requestMessage,
                                                            RESPONSE_TYPE& responseMessage)
{
    // These buffers contain the messages written by the client
    // to the server and back respectively
    shared_buffer clientToServerEncoded(new buffer);
    shared_buffer serverToClientEncoded(new buffer);

    // These Writers and Readers wrap the above buffers to simulate comms
    // between client and server
    MemoryIoWriter* clientRequestWriter = new MemoryIoWriter(clientToServerEncoded);
    MemoryIoReader serverRequestReader(clientToServerEncoded);
    MemoryIoWriter serverResponseWriter(serverToClientEncoded);
    MemoryIoReader* clientResponseReader = new MemoryIoReader(serverToClientEncoded);
    
    const int TRANSACTION_ID = 1;
    const int UNIT_ID = 1;

    // Serialize the simulated server response. This will be read immediately
    // when the request is sent below
    ModbusTcpMessage::outgoing(TRANSACTION_ID, UNIT_ID, responseMessage)
        .serialize(serverResponseWriter);

    // Wrap client readers and writers in shared_ptr for resource management
    shared_ptr<IoWriter> sharedWriter(clientRequestWriter);
    shared_ptr<IoReader> sharedReader(clientResponseReader);
    Modbus& modbus = ModbusConfigurer()
        .reader(sharedReader)
        .writer(sharedWriter)
        .transactionId(TRANSACTION_ID)
        .unitId(UNIT_ID)
        .configure();

    // Make request and get response
    RESPONSE_TYPE returnedResponseMessage = modbus.request(requestMessage);

    // Decode and verify the message sent to the server
    REQUEST_TYPE returnedRequestMessage;
    ModbusTcpMessage::incoming(1, 1, &returnedRequestMessage)
        .deserialize(serverRequestReader);    
    return pair<REQUEST_TYPE, RESPONSE_TYPE>(returnedRequestMessage, 
                                             returnedResponseMessage);
}

TEST(Modbus, read_holding_registers_with_template) {
    // The server will respond to a request for 3 registers starting at 0 with
    // the following data
    const uint16_t REGISTERS[] = { 0x01, 0x0203, 0x4050 };
    const int NR_REGISTERS = 3;
    ReadHoldingRegistersRequest requestMessage(0, NR_REGISTERS);
    vector<uint16_t> registers;
    registers.assign(REGISTERS, REGISTERS + NR_REGISTERS);
    ReadHoldingRegistersResponse responseMessage(registers);

    pair<ReadHoldingRegistersRequest, ReadHoldingRegistersResponse> result =
        sendRequestAndGetResponse(requestMessage, responseMessage);

    // Verify request
    EXPECT_EQ(0, result.first.startingAddress);
    EXPECT_EQ(3, result.first.numberOfRegisters);

    // Verify response
    ASSERT_EQ(3, result.second.registers.size());
    EXPECT_EQ(0x01, result.second.registers[0]);
    EXPECT_EQ(0x0203, result.second.registers[1]);
    EXPECT_EQ(0X4050, result.second.registers[2]);
}
