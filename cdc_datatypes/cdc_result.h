#ifndef CDC_RESULT_H
#define CDC_RESULT_H

#include <jansson.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

class TestOutput
{
private:
    std::string my_value;

public:
    TestOutput(const std::string& input, const std::string& name);
    const std::string& getValue() const;
};

class TestInput
{
private:
    std::string my_value;
    std::string my_type;
    std::string my_name;

public:
    TestInput(const std::string& value, const std::string& type, const std::string& name = "a");
    const std::string& getName() const
    {
        return my_name;
    }
    const std::string& getValue() const
    {
        return my_value;
    }
    const std::string& getType() const
    {
        return my_type;
    }

    bool operator ==(const TestOutput& output) const
    {
        return my_value == output.getValue();
    }

    bool operator !=(const TestOutput& output) const
    {
        return !(*this == output);
    }
};

#ifdef __cplusplus
}
#endif

#endif /* CDC_RESULT_H */

