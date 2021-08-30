#include <output.h>
        bool flag=0;
        bool flag2=0;
void output::run()
{
    while (1)
    {
        
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        //int len = line.length();
        std::string ans = encode(line);
        if (!connectionHandler.sendLine(ans))
        {
            std::cout << "Disconnected. Exiting...\n"
                      << std::endl;
            break;
        }
        if(flag&&flag2)
        break;
        // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
        //std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;
    }
    
}
std::string output::encode(std::string str){
        
        std::string ans ="";
        char bytesArr[2];
        short num;
        switch (str[0])
        {
        case 'R':{
                str = str.substr(9);
                num = 1;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                size_t i = str.find(' ');
                ans = ans+str.substr(0,i)+'\0';
                str=str.substr(i+1);
                ans=ans+str+'\0';        
                break;
        }
        case 'L':{
            if (str[3] == 'I')
            {
                flag2=1;
                str = str.substr(6);
                num = 2;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                size_t i = str.find(' ');
                ans = ans+str.substr(0,i)+'\0';
                str=str.substr(i+1);
                ans=ans+str+'\0'; 
            }
            else
            {
                flag=1;
                str = str.substr(6);
                num =3;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
            }
            break;
        }
        case 'F':{
                str = str.substr(7);
                num = 4;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                num = (short)str[0]-48;
                shortToByte(num,bytesArr);
                ans = ans+bytesArr[0];
                str = str.substr(2);
                size_t ind = str.find(' ');
                num=0;
                num = std::stoi(str.substr(0,ind));

                //for(size_t i = 0; i < str.substr(0,ind).length(); i++)
                //{
                   // std::cout<<i<<std::endl;
                 //   num=num*10+((int)str[i]-48);
               // }
                
                str = str.substr(ind+1);
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                
                for(size_t i = 0; i < str.length(); i++)
                {
                    if(str[i]==' ')
                        str[i]='\0';
                }
                
                ans=ans+str +'\0';            

                break;
        }
        case 'P':
            {if (str[1] == 'O')
            {
                str = str.substr(5);
                num =5;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                ans=ans+str+'\0';
                

            }
            else
            {
                str = str.substr(3);
                num = 6;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                size_t ind = str.find(' ');
                ans=ans+str.substr(0,ind)+'\0';
                str = str.substr(ind+1);
                ans=ans+str+'\0';
            }
            break;}
        case 'U':
                {str = str.substr(8);
                num = 7;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1];
                break;}
        case 'S':
                {str = str.substr(5);
                num = 8;
                shortToBytes(num,bytesArr);
                ans = ans+bytesArr[0]+bytesArr[1] + str +'\0';
                break;}
        }
        return ans;

    }
     void output::shortToBytes(short num, char* bytesArr)
    {
        bytesArr[0] = ((num >> 8) & 0xFF);
        
        bytesArr[1] = (num & 0xFF);
        
    }
    void output::shortToByte(short num, char* bytesArr){
        bytesArr[0] = (num & 0xFF);
    }