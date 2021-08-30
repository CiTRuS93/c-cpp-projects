#include <stdio.h>
int count_digit(char* string);
int main(int argc,char* argv[])
{
    
    int c= count_digit(argv[1]);
    printf("Number of digits in string = %d\n", c);
    return 0;
}
int count_digit(char* string){
int c=0;

    
    for(int j = 0; string[j] != '\0'; j++)
    {
        if(string[j]>='0'&&string[j]<='9'){
        c++;
        }
    }
    return c;
}
