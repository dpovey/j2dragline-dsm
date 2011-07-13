#include <assert.h>
#include <inttypes.h>

namespace j2 {

// Generic data class
class Data
{
public:
    Data() : _size(0), _data(0) {}

    Data(int size) : _size(size) 
    {
        assert(size >= 0);
	_data = new uint8_t[size];
    } 

    Data(const uint8_t data[], int size) : _size(size) { 
        assert(size >= 0);
        _data = new uint8_t[size];
        memcpy(_data, data, size);
    }

    virtual ~Data() { delete[] _data; }

    Data(const Data& bytes) : _size(bytes._size) {
        assert(_size > 0);
        _data = new uint8_t[_size];
        memcpy(_data, bytes._data, _size);
    }

    void operator=(const Data& bytes) {
       if (this == &bytes) return;
       delete[] _data; 
       _data = new uint8_t[bytes._size];
       _size = bytes._size;
       memcpy(_data, bytes._data, _size);
    }

    bool operator==(const Data& other) const {
        return this == &other ||
               (_size == other._size && 
               memcmp(_data, other._data, _size) == 0);
    }

    bool operator!=(const Data& other) const {
        return !(*this == other);
    }

    uint8_t& operator[](int pos) { 
        if (pos < 0) pos = _size - pos;
	assert(pos < _size); 
	return _data[pos];
    }

    const uint8_t& operator[](int pos) const { 
        if (pos < 0) pos = _size - pos;
	assert(pos < _size); 
	return _data[pos];
    }

    Data slice(int start=0, int finish=-1) const { 
        if (start < 0) start  = _size - start;
        if (finish < 0) finish = _size - finish;
        assert(start < _size);
        assert(finish <= _size);
        int sliceSize = (finish + 1) - start;
        assert(sliceSize > 0);
        return Data(_data + start, sliceSize);
    }

    void copyTo(void* to, int offset, int size) const {
        assert(offset + size < _size);
        memcpy(to, _data + offset, size);
    }

    uint16_t shortAt(int offset) const {
        assert(offset > 0 && offset + 1 < _size);
        return ((uint16_t)_data[offset] << 8) | _data[offset+1];
    }

    int size() const { return _size; }

private:
    uint8_t* _data;
    int _size;
};

};
