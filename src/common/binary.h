#pragma once

#include <vector>
#include <iostream>

typedef std::vector<uint8_t> binary_array_t;

class IBinary
{
    IBinary();

public:
    virtual void read(std::istream &is) = 0;
    virtual void write(std::ostream &os) const = 0;

    static std::string to(const void *data, size_t size);
    static std::string to(const binary_array_t &data);
    static binary_array_t from(const std::string &data);

    binary_array_t to();
    void from(const binary_array_t &blob);
    bool save(const std::string &filename);

    bool load(const std::string &filename);

    friend std::istream &operator>>(std::istream &is, IBinary &base);
    friend std::ostream &operator<<(std::ostream &os, const IBinary &base);
};