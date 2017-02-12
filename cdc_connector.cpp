#include "cdc_connector.h"
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <sys/types.h>

#define CDC_CONNECTOR_VERSION "1.0.0"

#define ERRBUF_SIZE 512
#define READBUF_SIZE 1024

static const char OK_RESPONSE[] = "OK\n";

static const char CLOSE_MSG[] = "CLOSE";
static const char REGISTER_MSG[] = "REGISTER UUID=CDC_CONNECTOR-" CDC_CONNECTOR_VERSION ", TYPE=";
static const char REQUEST_MSG[] = "REQUEST-DATA ";

namespace CDC
{

static inline int nointr_read(int fd, void *dest, size_t size)
{
    int rc = read(fd, dest, size);

    while (rc == -1 && errno == EINTR)
    {
        rc = read(fd, dest, size);
    }

    return rc;
}

static inline int nointr_write(int fd, const void *src, size_t size)
{
    int rc = write(fd, src, size);

    while (rc == -1 && errno == EINTR)
    {
        rc = write(fd, src, size);
    }

    return rc;
}

static std::string bin2hex(const uint8_t *data, size_t len)
{
    std::string result;
    static const char hexconvtab[] = "0123456789abcdef";

    for (int i = 0; i < len; i++)
    {
        result += hexconvtab[data[i] >> 4];
        result += hexconvtab[data[i] & 0x0f];
    }

    return result;
}

std::string generateAuthString(const std::string& user, const std::string& password)
{
    uint8_t digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const uint8_t*> (password.c_str()), password.length(), digest);

    std::string auth_str = user;
    auth_str += ":";

    std::string part1 = bin2hex((uint8_t*)auth_str.c_str(), auth_str.length());
    std::string part2 = bin2hex(digest, sizeof(digest));

    return part1 + part2;
}

/**
 * Public functions
 */

Connection::Connection(const std::string& address,
                       uint16_t port,
                       const std::string& user,
                       const std::string& password,
                       uint32_t flags) :
    my_fd(-1),
    my_address(address),
    my_port(port),
    my_user(user),
    my_password(password),
    my_flags(flags) { }

Connection::~Connection()
{
    closeConnection();
}

bool Connection::createConnection()
{
    bool rval = false;
    struct sockaddr_in remote = {};

    remote.sin_port = htons(my_port);
    remote.sin_family = AF_INET;

    if (inet_aton(my_address.c_str(), (struct in_addr*)&remote.sin_addr.s_addr) == 0)
    {
        my_error = "Invalid address: ";
        my_error += my_address;
    }
    else
    {
        int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (fd == -1)
        {
            char err[ERRBUF_SIZE];
            my_error = "Failed to create socket: ";
            my_error += strerror_r(errno, err, sizeof (err));
        }

        my_fd = fd;

        if (connect(fd, (struct sockaddr*) &remote, sizeof (remote)) == -1)
        {
            char err[ERRBUF_SIZE];
            my_error = "Failed to connect: ";
            my_error += strerror_r(errno, err, sizeof (err));
        }
        else if (doAuth())
        {
            rval = doRegistration();
        }
    }

    return rval;
}

void Connection::closeConnection()
{
    if (my_fd != -1)
    {
        nointr_write(my_fd, CLOSE_MSG, sizeof (CLOSE_MSG) - 1);
        close(my_fd);
        my_fd = -1;
    }
}

bool Connection::requestData(const std::string& table, const std::string& gtid)
{
    bool rval = true;

    std::string req_msg(REQUEST_MSG);
    req_msg += table;

    if (gtid.length())
    {
        req_msg += " ";
        req_msg += gtid;
    }

    if (nointr_write(my_fd, req_msg.c_str(), req_msg.length()) == -1)
    {
        rval = false;
        char err[ERRBUF_SIZE];
        my_error = "Failed to write request: ";
        my_error += strerror_r(errno, err, sizeof (err));
    }

    if (rval)
    {
        /** Read the Avro schema */
        rval = readRow(my_schema);
    }

    return rval;
}

bool Connection::readRow(std::string& dest)
{
    bool rval = true;

    while (true)
    {
        char buf;
        int rc = nointr_read(my_fd, &buf, 1);

        if (rc == -1)
        {
            rval = false;
            char err[ERRBUF_SIZE];
            my_error = "Failed to read row: ";
            my_error += strerror_r(errno, err, sizeof (err));
            break;
        }

        if (buf == '\n')
        {
            break;
        }
        else
        {
            dest += buf;
        }
    }

    return rval;
}

/**
 * Private functions
 */

bool Connection::doAuth()
{
    bool rval = false;
    std::string auth_str = generateAuthString(my_user, my_password);

    /** Send the auth string */
    if (nointr_write(my_fd, auth_str.c_str(), auth_str.length()) == -1)
    {
        char err[ERRBUF_SIZE];
        my_error = "Failed to write authentication data: ";
        my_error += strerror_r(errno, err, sizeof (err));
    }
    else
    {
        /** Read the response */
        char buf[READBUF_SIZE];
        int bytes;

        if ((bytes = nointr_read(my_fd, buf, sizeof (buf))) == -1)
        {
            char err[ERRBUF_SIZE];
            my_error = "Failed to read authentication response: ";
            my_error += strerror_r(errno, err, sizeof (err));
        }
        else if (memcmp(buf, OK_RESPONSE, sizeof (OK_RESPONSE) - 1) != 0)
        {
            buf[bytes] = '\0';
            my_error = "Authentication failed: ";
            my_error += buf;
        }
        else
        {
            rval = true;
        }
    }

    return rval;
}

bool Connection::doRegistration()
{
    bool rval = false;
    std::string reg_msg(REGISTER_MSG);

    const char *type = "";

    if (my_flags & CDC_REQUEST_TYPE_JSON)
    {
        type = "JSON";
    }
    else if (my_flags & CDC_REQUEST_TYPE_AVRO)
    {
        type = "AVRO";
    }

    reg_msg += type;

    /** Send the registration message */
    if (nointr_write(my_fd, reg_msg.c_str(), reg_msg.length()) == -1)
    {
        char err[ERRBUF_SIZE];
        my_error = "Failed to write registration message: ";
        my_error += strerror_r(errno, err, sizeof (err));
    }
    else
    {
        /** Read the response */
        char buf[READBUF_SIZE];
        int bytes;

        if ((bytes = nointr_read(my_fd, buf, sizeof (buf))) == -1)
        {
            char err[ERRBUF_SIZE];
            my_error = "Failed to read registration response: ";
            my_error += strerror_r(errno, err, sizeof (err));
        }
        else if (memcmp(buf, OK_RESPONSE, sizeof (OK_RESPONSE) - 1) != 0)
        {
            buf[bytes] = '\0';
            my_error = "Registration failed: ";
            my_error += buf;
        }
        else
        {
            rval = true;
        }
    }

    return rval;
}

}
