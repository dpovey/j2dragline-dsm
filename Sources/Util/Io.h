#include <iostream>
#include <vector>
#include <stdexcept>
#include <tr1/memory>
#include <inttypes.h>
#include <assert.h>
#include "boost/optional.hpp"
#include "boost/any.hpp"

#ifndef _J2_IO_H
#define _J2_IO_H

namespace j2 {

    /** @brief a buffer */
    typedef std::vector<uint8_t> buffer;
    
    /** @brief Pointer to a shared buffer */
    typedef std::tr1::shared_ptr<buffer> shared_buffer;

    class Io {
    public:
        virtual void close() = 0;
    
        // Note: semantics of EOF on read is different to usual, it should detect whether the
        // read will succeed, rather than be set after a read fails.  This may require a 
        // lookahead, but makes life easier for callers.
        virtual bool isEof() const = 0;

        virtual bool hasError() const { return bool(_error); }

        template <typename T>
        T error() const { 
            if (_error) return boost::any_cast<T>(*_error);
            throw std::runtime_error("No error defined");
        }

        void error(boost::any value) {
            _error = boost::optional<boost::any>(value);
        }

        virtual operator bool() const { return !(isEof() || hasError()); }

    private:
        boost::optional<boost::any> _error;

    };

    class IoWriter;
    class IoReader;
    class MemoryIoReader;
    class MemoryIoWriter;

    class Serializable {
    public:
        virtual void serialize(IoWriter& writer) const = 0;
        virtual void deserialize(IoReader& reader) = 0;
    };

    class IoReader : public virtual Io {
    public:
        /** 
         * @brief Read and return a byte.
         * Subclasses of @c IoReader must implement at least this method.
         */
        virtual uint8_t readByte() = 0;
        
        /** @brief Read an 8 bit value. */
        virtual IoReader& read(uint8_t* byte) {
            *byte = readByte();
            return *this;
        }
    
        /** @brief Read a 16-bit value. */
        virtual IoReader& read(uint16_t* doubleByte) {
            *doubleByte = (readByte() << 8) | readByte();
            return *this;
        }
    
        /** @brief Read exactly length bytes of data and append to the vector passed in. */
        template <typename T, typename LEN>
        IoReader& read(std::vector<T>& data, LEN length) {
            if (length <= 0) return *this;
            T item;
            data.reserve(length);
            for (int i = 0; i < length && !isEof(); i++) {
                read(&item);
                data.push_back(item);
            }
            return *this;
        }

        /** @brief Write a serializable object */
        virtual IoReader& read(Serializable& serializable) {
            serializable.deserialize(*this);
            // TODO: Handle exceptions
            return *this;
        }

        /** @brief Hint to underlying writer that we should buffer at least size bytes */
        virtual IoReader& buffer(int size) {
            return *this;
        }

    };

    class IoWriter : public virtual Io {
    public:
        /** 
         * @brief Write a byte.
         * Subclasses of @c Io must implement at least this method.
         */
        virtual void writeByte(uint8_t byte) = 0;
        
        /** @brief Write an 8 bit value. */
        virtual IoWriter& write(uint8_t byte) {
            writeByte(byte);
            return *this;
        }
    
        /** @brief Write a 16-bit value. */
        virtual IoWriter& write(uint16_t doubleByte) {
            writeByte(doubleByte >> 8);
            writeByte(doubleByte & 0xff);
            return *this;
        }
    
        /** @brief Read exactly length bytes of data and append to the vector passed in. */
        template <typename T>
        IoWriter& write(const std::vector<T>& data) {
            for (typename std::vector<T>::const_iterator iter = data.begin(); 
                 iter != data.end() && !isEof();
                 iter++) {
                write(*iter);
            }
            return *this;
        }

        /** @brief Read exactly length bytes of data and append to the vector passed in. */
        template <typename T>
        IoWriter& write(const T data[], int length) {
            for (int i = 0; i < length && !isEof(); i++) {
                write(data[i]);
            }
            return *this;
        }

        /** @brief Write a serializable object */
        virtual IoWriter& write(const Serializable& serializable) {
            serializable.serialize(*this);
            // TODO: Handle exceptions
            return *this;
        }
    };


    /** An Io wrapper that reads from a collection/array etc of bytes. */
    class MemoryIoReader : public IoReader {
    public:
        MemoryIoReader(const j2::buffer& data) :
            _bytes(new j2::buffer) {
            _i = 0;
            _bytes->assign(data.begin(), data.end());
        }

        MemoryIoReader(shared_buffer data) :
            _bytes(data) {
            _i = 0;
        }

        /* Initialize the MemoryIoReader from the given bytes and size. */
        MemoryIoReader(const uint8_t data[], 
                       int size) : _bytes(new j2::buffer) {
            _bytes->assign(data, data+size);
            _i = 0;
        } 

        virtual uint8_t readByte() {
            if (hasError()) return 0;
            if (isEof()) {
                puts("eof");
                // Attempted to read past end of vector
                error(true);
                return 0;
            }
            return (*_bytes)[_i++];
        }

        virtual void close() { }
        
        virtual bool isEof() const {
            return _i >= _bytes->size();
        }

    private:
        shared_buffer _bytes;
        int _i;
    };

    /** An Io wrapper that appends to a vector of bytes. */
    class MemoryIoWriter : public IoWriter {
    public:
        MemoryIoWriter(shared_buffer data) : _bytes(data) { }

        virtual void writeByte(uint8_t byte) {
            if (hasError()) return;
            _bytes->push_back(byte);
        }

        virtual void close() { }

        virtual bool isEof() const { return false; }

    private:
        shared_buffer _bytes;
    };

    class IoStreamWriter : public IoWriter {
    public:
        IoStreamWriter(std::ostream& stream) : stream(stream) { }

        virtual void writeByte(uint8_t byte) {
            stream.write((const char *)&byte, 1);
        }

        virtual void close() { }
        
        virtual bool isEof() const { return stream.eof(); }
        
    private:
        std::ostream& stream;
    };


    class IoStreamReader : public IoReader {
    public:
        IoStreamReader(std::istream& stream) : stream(stream), bad(false), bytes_read(0) { }

        virtual uint8_t readByte() {           
            if (hasError()) return 0;
            if (isEof()) {
                error(true);
                return 0;
            }
            uint8_t byte;
            stream.read((char *)&byte, 1);
            bytes_read++;
            return byte;
        }

        virtual bool hasError() const {
            return (stream.bad() || Io::hasError());
        }

        virtual void close() { }
        
        virtual bool isEof() const { return stream.eof() || stream.peek() == -1; }
        
    private:
        std::istream& stream;
        bool bad;
        int bytes_read;
    };

    inline void hexdump(const buffer& buf) {
        for (buffer::const_iterator iter = buf.begin(); iter != buf.end(); iter++) {
            printf("0x%02x ", *iter);
        }
        printf("\n");
    }

} // namespace j2

#endif // _J2_IO_H
