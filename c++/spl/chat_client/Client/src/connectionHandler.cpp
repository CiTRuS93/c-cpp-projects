#include <connectionHandler.h>

using boost::asio::ip::tcp;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
bool flag3 =0;
ConnectionHandler::ConnectionHandler(string host, short port) : host_(host), port_(port), io_service_(), socket_(io_service_) {}

ConnectionHandler::~ConnectionHandler()
{
    close();
}

bool ConnectionHandler::connect()
{
    std::cout << "Starting connect to "
              << host_ << ":" << port_ << std::endl;
    try
    {
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
        boost::system::error_code error;
        socket_.connect(endpoint, error);
        if (error)
            throw boost::system::system_error(error);
    }
    catch (std::exception &e)
    {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead)
{
    size_t tmp = 0;
    boost::system::error_code error;
    try
    {
        while (!error && bytesToRead > tmp)
        {
            tmp += socket_.read_some(boost::asio::buffer(bytes + tmp, bytesToRead - tmp), error);
        }
        if (error)
            throw boost::system::system_error(error);
    }
    catch (std::exception &e)
    {
        //std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite)
{
    int tmp = 0;
    boost::system::error_code error;
    try
    {
        while (!error && bytesToWrite > tmp)
        {
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
        if (error){
            close();
            throw boost::system::system_error(error);
        }
    }
    catch (std::exception &e)
    {
        close();
       // std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::getLine(std::string &line)
{
    return getFrameAscii(line, '\0');
}

bool ConnectionHandler::sendLine(std::string &line)
{
    return sendFrameAscii(line, '\0');
}

bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter)
{
    char c = delimiter;
    delimiter=c;
    char ch ;
    int i = 0;
    // Stop when we encounter the null character.
    // Notice that the null character is not appended to the frame string.
    try
    {
        do
        {
            getBytes(&ch, 1);

            frame.append(1, ch);
            i++;
            
        } while (!decodeable(frame)); //while (delimiter != ch);
    }
    catch (std::exception &e)
    {
        //std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;

        return false;
    }

    return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter)
{
    char c = delimiter;
    delimiter=c;
    bool result = sendBytes(frame.c_str(), frame.length());
    if (!result)
        return false;
    return result;
}

// Close down the connection properly.
void ConnectionHandler::close()
{
    try
    {
        socket_.close();
    }
    catch (...)
    {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}
bool ConnectionHandler::decodeable(std::string str)
{

    if (str.length() < 4)
    {
        return false;
    }
    char bytesArr[2];
    bytesArr[0] = str[0];
    bytesArr[1] = str[1];
    short num = bytesToShort(bytesArr);
    

    switch (num)
    {
    case 9:
    {

        int numOfZero = 0;

        for (size_t i = 3; i < str.length(); i++)
        {
            if (str[i] == '\0')
                numOfZero++;
        }
        if (numOfZero >= 2)
        {
            
            return true;
        }
        return false;

        break;
    }
    case 10:

        if (str.length() >= 4)
        {
            bytesArr[0] = str[2];
            bytesArr[1] = str[3];
            num = bytesToShort(bytesArr);
        }
        else
            return false;
        //answer = answer.substr(2);

        if(num==3){
            close();
            flag3=1;
        }
        if (num == 8)
        {
            if (str.length() == 10)
                return true;
            return false;
        }
        else if (num == 7 || num == 4)
        {
            int numOfZero=0;
            for (size_t i = 6; i < str.length(); i++)
            {
                if (str[i] == '\0')
                    numOfZero++;
            }
            if (numOfZero >= 1)
            {
                
                return true;
            }
            return false;
        }
        else
            return true;
        break;
    case 11:
        if (str.length() == 4)
            return true;
        return false;
        break;
    }
    return false;
}
short ConnectionHandler::bytesToShort(char *bytesArr)
{

    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

short ConnectionHandler::byteToShort(char *bytesArr)
{

    short result = (short)(bytesArr[0] & 0xff);
    return result;
}