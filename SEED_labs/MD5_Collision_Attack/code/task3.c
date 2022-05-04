#include <stdio.h>
#include <stdlib.h>
/* 这只是伪代码，具体请自行实现 */
main(int argc, char *argv[])
{
  FILE* fp = fopen(argv[0], "rb");
  if(fseek(fp, -5L*sizeof(unsigned char), SEEK_END) != 0) {
  	perror("seek error.");
	exit(1);
  }
  unsigned char last_byte;
  fread(&last_byte, sizeof(unsigned char), 1, fp);
  printf("%X\n", last_byte);
  if (last_byte > 0x80)
        printf("正常程序\n");
  else
        printf("恶意程序\n");
  fclose(fp);
  return;
}
