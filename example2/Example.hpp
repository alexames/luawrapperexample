#ifndef EXAMPLE_HPP_
#define EXAMPLE_HPP_

#include <string>

#include "Vector2D.hpp"

class Example
{
public:
    Example() : boolean(), integer(), uinteger(), cstring(""), cppstring(""), number(), floatnumber(), ptr(), vec() {}

    bool boolean;
    int integer;
    unsigned int uinteger;
    const char* cstring;
    std::string cppstring;
    double number;
    float floatnumber;
    Example* ptr;
    Vector2D vec;

    bool GetBoolean() const;
    int GetInteger() const;
    unsigned int GetUInteger() const;
    const char* GetCString() const;
    const std::string& GetCPPString() const;
    double GetNumber() const;
    float GetFloatNumber() const;
    Example* GetPtr() const;
    const Vector2D& GetVec() const;

    void SetBoolean(bool val);
    void SetInteger(int val);
    void SetUInteger(unsigned int val);
    void SetCString(const char* val);
    void SetCPPString(const std::string& val);
    void SetNumber(double val);
    void SetFloatNumber(float val);
    void SetPtr(Example* val);
    void SetVec(const Vector2D& Member);

    int DoSomething(bool b);

    int DoSomethingElse(int i, int j);
    int DoSomethingElse(float f);
};

#endif // EXAMPLE_HPP_
