/*
  bxdump - dumps binary data in hex format (with character translation)

  int bxdump(char *p,int size)

     p    - pointer to buffer
     size - size of buffer in bytes
*/

#include <stdio.h>
#include <string.h>

#define BYTELN 16

#define TRUE 1
#define FALSE 0

static char *charset = 
"................\
................\
 !\"#$%&'()*+,-./\
0123456789:;<=>?\
@ABCDEFGHIJKLMNO\
PQRSTUVWXYZ[.]^_\
`abcdefghijklmno\
pqrstuvwxyz...~.";

int bxdump(char *p,int size)
{
   int bytecount,i,eof;
   int repeating = FALSE;
   char *lastbuf = NULL;
   
    bytecount = 0;
    eof = FALSE;
    while (!eof) {
        eof = (bytecount >= size-1);
        if (eof) break;
        if (lastbuf != NULL) {
            if (memcmp(p+bytecount,lastbuf,BYTELN) == 0) {
                if (!repeating) {
                    repeating = TRUE;
                    printf("........\n");
                }
                bytecount += BYTELN;
                continue;
            }
            else {
                repeating = FALSE;
            }
        }
        lastbuf = p+bytecount;
        
        printf("%08d ",bytecount);
        for (i=0;i<BYTELN;i++) {
            if ((i%4) == 0) printf(" ");
            printf("%02X",(unsigned char) p[bytecount]);
            bytecount++;
        }
        bytecount -= BYTELN;
        printf("  *");
        for (i=0;i<BYTELN;i++) {
            if ((unsigned) p[bytecount] > 127) {
                printf("%c",charset[0]);
            }
            else {
                printf("%c",charset[(int) p[bytecount]]);
            }
            bytecount++;
        }
        printf("*\n");
    }
    return(0);
}
