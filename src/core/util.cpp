#include "util.h"
#include <iostream>
using namespace std;
// Primitive helper utilities, clamping, and screen control
// clampint keeps value in range if new value under or over flows.
int clampint(int val, int minval, int maxval)
{
    if(val < minval)
        return minval;
    if(val > maxval)
        return maxval;

    return val;
}

int stringlength(const char* str)
{
    if(str == nullptr)
        return 0;
    
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

static bool isspacechar(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void trimwhitespace(char* str)
{
    if(str == nullptr)
        return;
    
    int start = 0;
    while(isspacechar(str[start]))
    {
        start++;
    }

    int end = stringlength(str) - 1;
    while(end >= start && isspacechar(str[end]))
    {
        end--;
    }

    int newlength = 0;
    for(int i=start; i<=end; i++)
    {
        str[newlength] = str[i];
        newlength++;
    }
    
    str[newlength] = '\0';
}

bool stringequals(const char* a, const char* b)
{
    if(a==nullptr || b==nullptr)
        return a==b;
    
    int i=0;
    while(a[i] != '\0' && b[i] != '\0')
    {
        if(a[i] != b[i])
            return false;
        i++;
    }
    return a[i]==b[i];

}

void copystring(char* dest, const char* src, int destsize)
{
    if(dest==nullptr || destsize <= 0)
        return;

    int i=0;
    if(src != nullptr)
    {
        while(src[i] != '\0' && i < destsize-1)
        {
            dest[i] = src[i];
            i++;
        }
    }
    dest[i]='\0';
}

void tolowercase(char* str)
{
    if(str == nullptr)
        return;

    for(int i=0; str[i] != '\0'; i++)
    {
        if(str[i] >= 'A' && str[i] <= 'Z')
        {
            str[i] = str[i] + ('a'-'A');
        }
    }
}
bool parseint(const char* text, int& value)
{
    if(text == nullptr || text[0] == '\0')
        return false;
    if(stringlength(text) > 6)
        return false;

    int result=0;
    for(int i=0; text[i] != '\0'; i++)
    {
        if(text[i] < '0' || text[i] > '9')
            return false;
        int digit = text[i] - '0';
        result = result * 10 + digit;
    }
    value = result;
    return true;
}

void printheader(const char* title)
{
    cout << "===========================================\n";
    if(title != nullptr)
        cout << " " << title << "\n";

    cout << "===========================================\n";
}

void clearscreen()
{
    cout << "\n\n\n";
}

