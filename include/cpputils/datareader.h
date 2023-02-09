class Reader {
    const uint8_t *_p;
    const uint8_t *_pend;
public:
    template<typename P>
    Reader(std::span<const uint8_t> data)
        : _p(data.data()), _pend(_p + data.size())
    {
    }
    void reuire(size_t n)
    {
        if (_p + n <= _pend)
            return;
        throw std::runtime_error("eof");
    }

};

class Writer {
    uint8_t *_p;
    uint8_t *_pend;
public:
    Writer(uint8_t *ptr, size_t size)
        : _p(ptr), _pend(ptr + size)
    {
    }
    void reuire(size_t n)
    {
        if (_p + n <= _pend)
            return;
        throw std::runtime_error("eof");
    }
    uint8_t get8()
    {
        return *_p++;
    }

};


class AutoWriter : public Writer {
    std::vector<uint8_t> _buf;
public:
    AutoWriter()
        : _buf(256), Writer(_buf.data(), _buf.size())
    {
    }

    void reuire(size_t n)
    {
        if (_p + n <= _pend)
            return;

        uint64_t ofs = _p - _buf.data();

        _buf.resize((_buf.size()+256)/3*4);

        _p = _buf.data();
        _pend = _p + _buf.size();

        _p += ofs;
    }
};
