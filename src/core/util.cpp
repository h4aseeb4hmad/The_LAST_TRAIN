#include "util.h"

// Primitive helper utilities, clamping, and screen control
#include<iostream>
using namespace std;

int clampInt(int val, int minVal, int maxVal) //keeping the number of character or values in range
{
    if(val<minVal)
    {
        return minVal;
    }
    if(val>maxVal)
    {
        return maxVal;
    }
    return val;
}

void trimWhitespace(char* str) //removing extra spaces from the beggining and from the end
{
    if(str==nullptr)
    {
        return;
    }
    int length=0;
    while(str[length]!='\0')
    {
        length++;
    }
    int end=length-1;
    while(end>=0 && (str[end]==' ' || str[end]=='\n' || str[end]=='\r'))
    {
        str[end]='\0';
        end--;
    }
    int start=0;
    while(str[start]==' ' || str[start]=='\t' || str[start]=='\n' || str[start]=='\r')
    {
        start++;
    }
    if(start>0)
    {
        int i=0;
        while(str[start+i]!='\0')
        {
            str[i]=str[start+i];
            i++;
        }
        str[i]='\0';
    }
}

bool stringEquals(const char* a, const char* b) //checking if both strings are same character by charater
{
    if(a==nullptr || b==nullptr)
    {
        return false;
    }
    int i=0;
    while(a[i]!='\0' && b[i]!='\0')
    {
        if(a[i]!=b[i])
        {
            return false;
        }
        i++;
    }
    return a[i]==b[i];
}

void printHeader(const char* title)
{
    cout<<"\n==============================\n";
    if(title!=nullptr)
    {
        cout<<" "<<title<<"\n";
    }
    cout<<"==============================\n";
}

void clearScreen()
{
    cout<<"\033[2J\033[H"; //ANSI escape sequence(033 represents escape esc, [2 tells the terminal to clear the screen, [H moves cursor to the top left cornor of the screen)
}