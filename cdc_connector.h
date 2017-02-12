#include <cstdint>
#include <string>

/** Request format flags */
#define CDC_REQUEST_TYPE_JSON (1 << 0)
#define CDC_REQUEST_TYPE_AVRO (1 << 1)

namespace CDC
{

class Connection
{
public:
    Connection(const std::string& address,
               uint16_t port,
               const std::string& user,
               const std::string& password,
               uint32_t flags = CDC_REQUEST_TYPE_JSON);
    virtual ~Connection();
    bool createConnection();
    bool requestData(const std::string& table, const std::string& gtid = "");
    bool readRow(std::string& dest);
    void closeConnection();
    const std::string& getSchema() const
    {
        return my_schema;
    }
    const std::string& getError() const
    {
        return my_error;
    }

private:
    int my_fd;
    uint32_t my_flags;
    uint16_t my_port;
    std::string my_address;
    std::string my_user;
    std::string my_password;
    std::string my_error;
    std::string my_schema;

    bool doAuth();
    bool doRegistration();
};

}
