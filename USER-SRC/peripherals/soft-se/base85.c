#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "base85.h"

static unsigned long pow85[] = {
	52200625, 614125, 7225, 85, 1
};

union u {unsigned long vi; unsigned char c[sizeof(unsigned long)];}; 
union v {unsigned long ni; unsigned char d[sizeof(unsigned long)];};
unsigned long base85_swap (unsigned long nLongNumber)
{
      union u un; 
      union v vn; 
      un.vi = nLongNumber; 
      vn.d[0]=un.c[3]; 
      vn.d[1]=un.c[2]; 
      vn.d[2]=un.c[1]; 
      vn.d[3]=un.c[0]; 
      return (vn.ni); 
}
int base85_encode( unsigned char *dst, int *dlen, unsigned char *src, int slen )
{
      int i, bulket, count;
      unsigned long int number;
      unsigned long int *pnumber;
      unsigned char *pout , *pin;
      char buf[5], *s ;
      s = buf;
      pout = dst;
      pin = src;
    
      bulket = slen/4;
      count = slen%4;
      *pout++= '<';
      *pout++= '~';
      for (i = 0; i < bulket; i++)
      {
            pnumber = (unsigned long int *) pin; 
            number =base85_swap(*pnumber);
            if (number == 0)
            *(pout++) = 'z';
            else
            {
                  *(pout+4) = (unsigned char)('!' + (number % 85)); number /= 85; //'!' == 33
                  *(pout+3) = (unsigned char)('!' + number % 85); number /= 85;
                  *(pout+2) = (unsigned char)('!' + number % 85); number /= 85;
                  *(pout+1) = (unsigned char)('!' + number % 85); number /= 85;
                  *(pout)   = (unsigned char)('!' + number % 85); number /= 85;        
                  pout+=5;
            }
            pin+=4;
      }
      if (count)
      {
            number = 0;
            memset(buf, 0, 5);
            memcpy(buf, pin, count);
            pnumber = (unsigned long int *) buf; 
            number =base85_swap(*pnumber);
            i = 5;
            do 
            {
                  *s++ = (char)(number % 85);
                  number /= 85;
            } while (--i > 0);
            i = count;
            do 
            {
                  s--;
                  *pout++ = *s+'!';
          
            } while (i-- > 0);

      }
      *pout++ = '~';
      *pout++ = '>';
      *dlen = pout - dst;
      *pout = 0;
      return *dlen;
}

int base85_decode( unsigned char *dst, int *dlen, unsigned char *src, int slen )
{
      int count = 0;
      int c = 0;
      int i = 0;
      unsigned long int number = 0;
      unsigned char *pout, *pin;
      unsigned long int *pnumber;
      unsigned char *p;
      pout = dst;
      pin = src;
      *dlen = 0;

      if (*(pin++) != '<')
            return -1;
      if (*(pin++) != '~')
            return -1;
      for (i = 0; i < slen; i++)
      {
            switch (c = *(pin++))
            {
            default:
            {
                  if (c < '!' || c > 'u') 
                  {
                        return -1;
                  }
                  number += (c - '!') * pow85[count++];
                  if (count == 5) 
                  {
                        pnumber = (unsigned long int *)pout;
                        *pnumber=base85_swap(number);
                        count = 0;
                        number = 0;
                        pout+=4;
                  }
                  break;
            }
            case 'z':
            {
                  if (count != 0) 
                  {
                        return -1;
                  }            
                  *pout++=0;
                  *pout++=0;
                  *pout++=0;
                  *pout++=0;         
                  break;
            }
            case '~':
            {
                  if (*(pin) == '>') 
                  {
                        if (count > 0) 
                        {
                              count--;
                              number += pow85[count];
                              number = base85_swap(number);
                              p =(unsigned char*)&number;
                              switch (count) 
                              {
                              case 3:
                                    memcpy(pout,&p[0],3);                     
                                    pout+=3;
                                    break;
                              case 2:              
                                    memcpy(pout,&p[0],2);                              
                                    pout+=2;
                                    break;
                              case 1:         
                                    memcpy(pout,&p[0],1);                              
                                    pout++;
                                    break;
                              }
                        }
                        *dlen = pout - dst;
                        dst[*dlen] = 0;
                        return *dlen;
                  }
            
            }
            case '\n': case '\r': case '\t': case ' ':
            case '\0': case '\f': case '\b': case 0177:
            break;
            }
      }
      return -1;
}

