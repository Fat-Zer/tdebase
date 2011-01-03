

#include <tqstring.h>
#include <tqstringlist.h>
#include <stdlib.h>
#include <stdio.h>



int main(int argc, char **argv) {
TQStringList have;
char buf[1024];


   while (!feof(stdin)) {
      char *cline = fgets(buf, 1000, stdin);
      if (!cline) break;
      if (!have.tqcontains(cline)) {
         have << cline;
         fprintf(stdout, "%s", cline);
      }
   }


return 0;
}


