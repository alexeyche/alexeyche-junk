
#include "lzw.c"

int main()
{
//    int i, fd = open("unixdict.txt", O_RDONLY);
//    
//    
//
//    if (fd == -1) {
//        fprintf(stderr, "Can't read file\n");
//        return 1;
//    };
 
//    struct stat st;
//    fstat(fd, &st);
 
    byte *in = _new(byte, 100);
    
    byte in_data[100] = { 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
                     0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0,   
                     0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0 };

    for(size_t i=0; i<100; i++) {
        in[i] = in_data[i];
    }
//    read(fd, in, st.st_size);
//    _setsize(in, st.st_size);
//    close(fd);
 
    printf("input size:   %d\n", 100);
 
    byte *enc = lzw_encode(in, 9);
    printf("encoded size: %d\n", _len(enc));
 
//    byte *dec = lzw_decode(enc);
//    printf("decoded size: %d\n", _len(dec));
// 
//    for (i = 0; i < _len(dec); i++)
//        if (dec[i] != in[i]) {
//            printf("bad decode at %d\n", i);
//            break;
//        }
// 
//    if (i == _len(dec)) printf("Decoded ok\n");
 
 
    _del(in);
    _del(enc);
//    _del(dec);
 
    return 0;
}
