#include <utility>
#include <fstream>
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
    MemoryIoWriter clientRequestWriter(clientToServerEncoded);
    MemoryIoReader serverRequestReader(clientToServerEncoded);
    MemoryIoWriter serverResponseWriter(serverToClientEncoded);
    MemoryIoReader clientResponseReader(serverToClientEncoded);
    
    const int TRANSACTION_ID = 1;
    const int UNIT_ID = 1;

    // Send request
    ModbusTcp::write_request(clientRequestWriter, requestMessage);

    // Decode request
    ModbusTcpMessage tcp_request = ModbusTcp::read_request(serverRequestReader);
    REQUEST_TYPE returnedRequestMessage = tcp_request.message<REQUEST_TYPE>();

    // Send response
    ModbusTcp::write_response(serverResponseWriter, tcp_request, responseMessage);

    // Decode response
    ModbusTcpMessage tcp_response = ModbusTcp::read_response(clientResponseReader);
    RESPONSE_TYPE returnedResponseMessage = tcp_response.message<RESPONSE_TYPE>();

    std::pair<REQUEST_TYPE, RESPONSE_TYPE> result(returnedRequestMessage, 
                                                  returnedResponseMessage);
    return result;
}

TEST(Modbus, read_holding_registers) {
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

TEST(Modbus, write_multiple_registers) {
    // The client will write 3 registers starting at 0 with
    // the following data
    const uint16_t REGISTERS[] = { 0x01, 0x0203, 0x4050 };
    const int NR_REGISTERS = 3;
    vector<uint16_t> registers;
    registers.assign(REGISTERS, REGISTERS + NR_REGISTERS);
    WriteMultipleRegistersRequest requestMessage(0, registers);
    WriteMultipleRegistersResponse responseMessage(0, NR_REGISTERS);

    pair<WriteMultipleRegistersRequest, WriteMultipleRegistersResponse> result =
        sendRequestAndGetResponse(requestMessage, responseMessage);

    // Verify request
    ASSERT_EQ(3, result.first.registers.size());
    EXPECT_EQ(0, result.first.starting_address);
    EXPECT_EQ(0x01, result.first.registers[0]);
    EXPECT_EQ(0x0203, result.first.registers[1]);
    EXPECT_EQ(0X4050, result.first.registers[2]);

    // Verify response
    EXPECT_EQ(0, result.second.startingAddress);
    EXPECT_EQ(3, result.second.numberOfRegisters);
}

TEST(Modbus, read_from_file) {
    const char* MODBUS_TEST_DATA = "TestData/modbus_req";
    fstream stream(MODBUS_TEST_DATA, std::ifstream::in);
    ASSERT_TRUE(stream);
    IoStreamReader reader(stream);
    ASSERT_FALSE(reader.hasError());
    ASSERT_FALSE(reader.isEof());
    ASSERT_TRUE(reader);
    int nr_messages = 0;
    while(reader) {
        ModbusTcpMessage message = ModbusTcp::read_request(reader);
        EXPECT_FALSE(reader.hasError());
        nr_messages++;
        switch(message.functionCode) {
        case WriteMultipleRegisters: {
            WriteMultipleRegistersRequest request =
                message.message<WriteMultipleRegistersRequest>();
            break;
        }
        default:
            ASSERT_FALSE(true);
            break;
        }
    }
    EXPECT_EQ(60, nr_messages);
}
