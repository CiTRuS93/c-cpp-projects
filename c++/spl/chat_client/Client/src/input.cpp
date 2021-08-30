#include <input.h>

void Input::run()
{
    while (1)
    {
        std::string answer;
        if (!connectionHandler.getLine(answer))
        {
            std::cout << "Disconnected. Exiting...\n"
                      << std::endl;
            break;
        }
        
        int len = answer.length();
        answer.resize(len);
        
        
        
        std::string decodedAns = decode(answer);

    
        std::cout << decodedAns << std::endl;
        answer = "";
        if (flag)
        {
            std::cout << "Exiting...\n"
                      << std::endl;
            connectionHandler.close();
            break;
        }
    }
}
std::string Input::decode(std::string answer)
{
    char bytesArr[2];
    bytesArr[0] = answer[0];
    bytesArr[1] = answer[1];
    
    std::string decodedAns = "";
    short num = bytesToShort(bytesArr);

    answer = answer.substr(2);
    switch (num)
    {
    case 9:
    {
        decodedAns = decodedAns + "NOTIFICAION ";
        bytesArr[0] = answer[0];

        num = byteToShort(bytesArr);
        answer = answer.substr(1);
        if (num == 0)
        {
            decodedAns = decodedAns + "PM ";
        }
        else
        {
            decodedAns = decodedAns + "public ";
        }
        size_t i = answer.find('\0');
        answer = answer.substr(i + 1);
         i = answer.find('\0');
        decodedAns = decodedAns + answer.substr(0, i+1)+' ';
        
        answer = answer.substr(i + 1);
         i = answer.find('\0');
        decodedAns = decodedAns + answer;
        
        break;
    }
    case 10:

    
        decodedAns = decodedAns + "ACK ";
        bytesArr[0] = answer[0];
        bytesArr[1] = answer[1];
        num = bytesToShort(bytesArr);
        decodedAns = decodedAns + std::to_string(num) + ' ';
        //answer = answer.substr(2);
        if(num==3){
            flag=1;
        }
        
        if (num == 8)
        {
            answer = answer.substr(2);
            bytesArr[0] = answer[0];
            bytesArr[1] = answer[1];
            num = bytesToShort(bytesArr);
            decodedAns = decodedAns + std::to_string(num) + ' ';
            answer = answer.substr(2);
            bytesArr[0] = answer[0];
            bytesArr[1] = answer[1];
            num = bytesToShort(bytesArr);
            decodedAns = decodedAns + std::to_string(num) + ' ';
            answer = answer.substr(2);
            bytesArr[0] = answer[0];
            bytesArr[1] = answer[1];
            num = bytesToShort(bytesArr);
            decodedAns = decodedAns + std::to_string(num);
        }
        else if (num == 7 || num == 4)
        {
            answer = answer.substr(2);
            bytesArr[0] = answer[0];
            bytesArr[1] = answer[1];
            num = bytesToShort(bytesArr);
            decodedAns = decodedAns + std::to_string(num) + ' ';
            answer = answer.substr(2);
            size_t i = answer.find('\0');
            decodedAns = decodedAns + answer.substr(0,i);
            
            for(size_t i = 0; i < decodedAns.length(); i++)
            {
                if(decodedAns[i]=='\0')
                    decodedAns[i]=' ';
            }
            

        }
        break;
    case 11:
        decodedAns = decodedAns + "ERROR ";
        bytesArr[0] = answer[0];
        bytesArr[1] = answer[1];
        num = bytesToShort(bytesArr);
        decodedAns = decodedAns + std::to_string(num);
        break;
    }
    return decodedAns;
}
short Input::bytesToShort(char *bytesArr)
{   


    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

short Input::byteToShort(char *bytesArr)
{

    short result = (short)(bytesArr[0] & 0xff);
    return result;
}