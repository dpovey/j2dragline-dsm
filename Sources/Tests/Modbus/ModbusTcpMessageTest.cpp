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
    MemoryIoWriter clientRequestWriter(clientToServerEncoded);
    MemoryIoReader serverRequestReader(clientToServerEncoded);
    MemoryIoWriter serverResponseWriter(serverToClientEncoded);
    MemoryIoReader clientResponseReader(serverToClientEncoded);
    
    const int TRANSACTION_ID = 1;
    const int UNIT_ID = 1;

    Modbus client_modbus(UNIT_ID);
    Modbus server_modbus(UNIT_ID);

    // Send request
    client_modbus.write(clientRequestWriter, requestMessage);

    // Decode request
    REQUEST_TYPE* returnedRequestMessage = 
        dynamic_cast<REQUEST_TYPE*>(server_modbus.read_request(serverRequestReader));

    // Send response
    server_modbus.write(serverResponseWriter, responseMessage);

    // Decode response
    RESPONSE_TYPE* returnedResponseMessage =
        dynamic_cast<RESPONSE_TYPE*>(client_modbus.read_response(clientResponseReader));

    std::pair<REQUEST_TYPE, RESPONSE_TYPE> result(*returnedRequestMessage, 
                                                  *returnedResponseMessage);
    delete returnedRequestMessage;
    delete returnedResponseMessage;
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

// TEST(Modbus, write_multiple_registers) {
//     // The client will write 3 registers starting at 0 with
//     // the following data
//     const uint16_t REGISTERS[] = { 0x01, 0x0203, 0x4050 };
//     const int NR_REGISTERS = 3;
//     vector<uint16_t> registers;
//     registers.assign(REGISTERS, REGISTERS + NR_REGISTERS);
//     WriteMultipleRegistersRequest requestMessage(registers);
//     WriteMultipleRegistersResponse responseMessage(0, NR_REGISTERS);

//     pair<WriteMultipleRegistersRequest, WriteMultipleRegistersResponse> result =
//         sendRequestAndGetResponse(requestMessage, responseMessage);

//     // Verify request    
//     ASSERT_EQ(3, result.first.registers.size());
//     EXPECT_EQ(0x01, result.first.registers[0]);
//     EXPECT_EQ(0x0203, result.first.registers[1]);
//     EXPECT_EQ(0X4050, result.first.registers[2]);

//     // Verify response
//     EXPECT_EQ(0, result.second.startingAddress);
//     EXPECT_EQ(3, result.second.numberOfRegisters);
// }

// TEST(Modbus, read_from_file) {
//     const std::string MODBUS_TEST_DATA = "TestData/modbus_req";
//     ifstream stream(MODBUS_TEST_DATA);
//     IoStreamReader reader(ifstream);
//     Modbus& modbus = ModbusConfigurer()
//         .transactionId(TRANSACTION_ID)
//         .unitId(UNIT_ID)
//         .configure();
//     while(reader) {
//         ModbusMessage& message = modbus.read_request(reader);
//         switch(message->functionCode()) {
//         case ModbusFunctionCode::WriteMultipleRegisters:
//             WriteMultipleRegistersRequest& request = 
//                 dynamic_cast<WriteMultipleRegistersRequest&>message;
//             modbus.write_request(WriteMultipleRegistersResponse(request.registers.size());
//             break;
//         default:
//             ASSERT_FALSE(true);
//         }
//     }

// }
