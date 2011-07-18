#include <Io.h>
#include <gtest/gtest.h>

using namespace std;
using namespace j2;

TEST(MemoryIoReader, can_read_byte) {
    shared_buffer bytes = shared_buffer(new buffer);
    bytes->push_back(0xca);
    MemoryIoReader reader = MemoryIoReader(bytes);
    EXPECT_EQ(0xca, reader.readByte());
    EXPECT_FALSE(reader.hasError());
    EXPECT_TRUE(reader.isEof());
}

TEST(MemoryIoReader, can_read_double_byte) {
    uint8_t TEST_DATA[] = { 0xca, 0xfe };
    MemoryIoReader reader = MemoryIoReader(TEST_DATA, sizeof(TEST_DATA));
    uint16_t doubleByte;
    reader.read(&doubleByte);
    EXPECT_EQ(0xcafe, doubleByte);
    EXPECT_FALSE(reader.hasError());
    EXPECT_TRUE(reader.isEof());
}

TEST(MemoryIoReader, can_read_data) {
    uint8_t TEST_DATA[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
    MemoryIoReader reader = MemoryIoReader(TEST_DATA, sizeof(TEST_DATA));
    vector<uint8_t> expected;
    expected.assign(TEST_DATA, TEST_DATA + sizeof(TEST_DATA));
    vector<uint8_t> read;
    reader.read(read, sizeof(TEST_DATA));
    EXPECT_EQ(expected, read);
    EXPECT_FALSE(reader.hasError());
    EXPECT_TRUE(reader.isEof());
}

TEST(MemoryIoReader, error_when_reading_past_end_of_file) {
    uint8_t TEST_DATA[] = { 0xca, 0xfe };
    MemoryIoReader reader = MemoryIoReader(TEST_DATA, sizeof(TEST_DATA));

    // Read first byte
    reader.readByte();
    EXPECT_TRUE(!reader.hasError() && !reader.isEof());

    // Read second byte, now at EOF
    reader.readByte();
    EXPECT_TRUE(!reader.hasError() && reader.isEof());

    // Read past EOF
    reader.readByte();
    EXPECT_TRUE(reader.hasError() && reader.isEof());    

    // Read past EOF
    reader.readByte();
    EXPECT_TRUE(reader.hasError() && reader.isEof());

    vector<uint8_t> dummy;
    reader.read(dummy, 1000);
    EXPECT_TRUE(reader.hasError() && reader.isEof());

    EXPECT_FALSE(reader);
}

TEST(MemoryIoReader, chaining_reads) {
    uint8_t TEST_DATA[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
    MemoryIoReader reader = MemoryIoReader(TEST_DATA, sizeof(TEST_DATA));
    uint16_t a, c, e;
    uint8_t b, d, f;
    vector<uint8_t> g;
    reader
        .read(&a)
        .read(&b)
        .read(&c)
        .read(&d)
        .read(&e)
        .read(&f)
        .read(g, 1);
    EXPECT_EQ(0x0102, a);
    EXPECT_EQ(0x03, b);
    EXPECT_EQ(0x0405, c);
    EXPECT_EQ(0x06, d);
    EXPECT_EQ(0x0708, e);
    EXPECT_EQ(0x09, f);
    EXPECT_EQ(0x0a, g[0]);    
    EXPECT_FALSE(reader.hasError());
    EXPECT_TRUE(reader.isEof());
}

TEST(MemoryIoWriter, can_write_byte) {
    shared_buffer bytes = shared_buffer(new buffer());
    MemoryIoWriter writer = MemoryIoWriter(bytes);
    writer.writeByte(0xca);
    EXPECT_EQ(0xca, (*bytes)[0]);
    EXPECT_FALSE(writer.hasError());
}

TEST(MemoryIoWriter, can_write_double_byte) {
    shared_buffer bytes = shared_buffer(new buffer());
    MemoryIoWriter writer = MemoryIoWriter(bytes);
    writer.write(uint16_t(0xcafe));

    uint8_t EXPECTED_DATA[] = { 0xca, 0xfe };
    vector<uint8_t> expected;
    expected.assign(EXPECTED_DATA, EXPECTED_DATA + sizeof(EXPECTED_DATA));
    EXPECT_EQ(expected, *bytes);
    EXPECT_FALSE(writer.hasError());
}

TEST(MemoryIoWriter, can_write_data_with_array) {
    uint8_t TEST_DATA[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };

    shared_buffer bytes = shared_buffer(new buffer());
    MemoryIoWriter writer = MemoryIoWriter(bytes);
    writer.write(TEST_DATA, sizeof(TEST_DATA));

    vector<uint8_t> expected;
    expected.assign(TEST_DATA, TEST_DATA + sizeof(TEST_DATA));
    EXPECT_EQ(expected, *bytes);
    EXPECT_FALSE(writer.hasError());
}

TEST(MemoryIoWriter, can_write_data_with_vector) {
    uint8_t TEST_DATA[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
    shared_buffer bytes = shared_buffer(new buffer());
    MemoryIoWriter writer = MemoryIoWriter(bytes);
    vector<uint8_t> expected;
    expected.assign(TEST_DATA, TEST_DATA + sizeof(TEST_DATA));
    writer.write(expected);
    EXPECT_EQ(expected, *bytes);
    EXPECT_FALSE(writer.hasError());
}


TEST(MemoryIoWriter, chaining_writes) {
    uint8_t TEST_DATA[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
    shared_buffer bytes = shared_buffer(new buffer());
    MemoryIoWriter writer = MemoryIoWriter(bytes);
    uint16_t a = 0x0102, c = 0x0405, e = 0x0708;
    uint8_t b = 0x3, d = 0x06, f = 0x09;
    vector<uint8_t> g;
    g.push_back(0x0a);
    writer
        .write(a)
        .write(b)
        .write(c)
        .write(d)
        .write(e)
        .write(f)
        .write(g);
    EXPECT_FALSE(writer.hasError());

    vector<uint8_t> expected;
    expected.assign(TEST_DATA, TEST_DATA + sizeof(TEST_DATA));
    EXPECT_EQ(expected, *bytes);
    EXPECT_FALSE(writer.hasError());    
}

