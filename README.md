stripe-ctf
==========

It began like this: -

```bash
#!/bin/bash
while read p; do
   if grep -qiw $p /usr/share/dict/words; then
       echo -n "<"$p"> "
   else
       echo -n $p" "
   fi
done < <( cat short.txt | tr -s '[[:space:]]' '\n' )
```

Then we got here: -

```c
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
 
#include "art.h"
 
int main()
{
    art_tree t;
    int res = init_art_tree(&t);
    int ts = 0;
 
    if (res == 0) { printf("Res is 0\n"); }
 
    // Insert all the words from the dictionary into the trie
    int len;
    char buf[1555555]; // 2493109
    FILE *f = fopen("/usr/share/dict/words", "r");
 
    uintptr_t line = 1;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';
        void * ains = art_insert(&t, buf, len, (void*)line);
        if (ains == NULL) {
            ts++;
            //printf("ains is 0\n");
        }
        line++;
    }
    printf("Loaded %d words into the tree\n", ts);
    // Seek back to the start
    fseek(f, 0, SEEK_SET);
 
    // Search for each line
    int len2;
    FILE *w = fopen("/Users/jason/level0/foo", "r");
    char buf2[1555555]; // 2493109
 
    int line2 = 1;
    while (fgets(buf2, sizeof buf2, w)) {
        len2 = strlen(buf2);
        buf2[len2-1] = '\0';
 
        // Yeah thats right, http://cl.ly/image/212c0K0E1y1V
        for(char *p = buf2;*p;++p) *p=*p>0x40&&*p<0x5b?*p|0x60:*p;
 
        uintptr_t val = (uintptr_t)art_search(&t, buf2, len2);
        printf("looking for [%s] %lu \n", buf2, val);
 
        if (val > 0) {
            printf("found %s\n", buf2);
        }
        line2++;
    }
    printf("Loaded %d words from the foo\n", line2);
 
    return 0;
}
```

And now we are here.
