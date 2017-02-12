#include "cdc_result.h"
#include <iostream>
#include <cstdlib>
#include <sstream>

using std::cout;
using std::endl;

TestInput::TestInput(const std::string& value, const std::string& type, const std::string& name) :
    my_value(value), my_type(type), my_name(name)
{
    if (my_value[0] == '"' || my_value[0] == '\'')
    {
        /** Remove quotes from the value */
        my_value = my_value.substr(1, my_value.length() - 2);
    }
}

TestOutput::TestOutput(const std::string& input, const std::string& name)
{
    json_error_t err;
    json_t *js = json_loads(input.c_str(), 0, &err);

    if (js)
    {
        json_t *value = json_object_get(js, name.c_str());

        if (value)
        {
            std::stringstream ss;

            if (json_is_string(value))
            {
                ss << json_string_value(value);
            }
            else if (json_is_integer(value))
            {
                ss << json_integer_value(value);
            }
            else if (json_is_null(value))
            {
                ss << "NULL";
            }
            else if (json_is_real(value))
            {
                ss << json_real_value(value);
            }
            else
            {
                cout << "Value '" << name << "' is not a primitive type: " << input << endl;
            }

            my_value = ss.str();
        }
        else
        {
            cout << "Value '" << name << "' not found" << endl;
        }

        json_decref(js);
    }
    else
    {
        cout << "Failed to parse JSON: " << err.text << endl;
    }
}

const std::string& TestOutput::getValue() const
{
    return my_value;
}
