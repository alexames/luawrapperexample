#include "Example.hpp"
#include <string>

// These are just a bunch of generic getters and setters for
// values in the Example class

using namespace std;

bool Example::GetBoolean() const
{
    return boolean;
}

int Example::GetInteger() const
{
    return integer;
}

unsigned int Example::GetUInteger() const
{
    return uinteger;
}

const char* Example::GetCString() const
{
    return cstring;
}

const string& Example::GetCPPString() const
{
    return cppstring;
}

const Vector2D& Example::GetVec() const
{
    return vec;
}

double Example::GetNumber() const
{
    return number;
}

float Example::GetFloatNumber() const
{
    return floatnumber;
}

Example* Example::GetPtr() const
{
    return ptr;
}

void Example::SetBoolean(bool val)
{
    boolean = val;
}

void Example::SetInteger(int val)
{
    integer = val;
}

void Example::SetUInteger(unsigned int val)
{
    uinteger = val;
}

void Example::SetCString(const char* val)
{
    cstring = val;
}

void Example::SetCPPString(const string& val)
{
    cppstring = val;
}

void Example::SetVec(const Vector2D& val)
{
    vec = val;
}

void Example::SetNumber(double val)
{
    number = val;
}

void Example::SetFloatNumber(float val)
{
    floatnumber = val;
}

void Example::SetPtr(Example* val)
{
    ptr = val;
}

