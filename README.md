# cpputils
various c++ utility classes


## hexdumper

Using iostream to configure hexdump output:

    std::cout << hex::hexstring << hex::dumper(data, size) << "\n";
    std::cout << hex::offset(0x12000) << hex::right << hex::dumper(data, size) << "\n";
    
A more detailed description can be found [in this blog post](http://nlitsme.github.io/posts/hexdumper-for-c%2B%2B-iostreams/)

### todo

 * add support for hexdumping data from streams.

(C) 2016 Willem Hengeveld <itsme@xs4all.nl>
