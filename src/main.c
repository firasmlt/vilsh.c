#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char buf[4094];
  while(1){
    printf("$ ");
    if(!fgets(buf, sizeof(buf), stdin)){
      printf("\n");
      break;
    }

    size_t len = strlen(buf);

    if(len > 0 && buf[len -1] == '\n'){
      buf[len - 1] = '\0';
    }else{
      int c;
      while((c = getchar()) != '\n' && c != EOF){}
    }

    if(buf[0] == '\0') continue;



    if(strcmp(buf, "clear") == 0){
      write(1, "\033[2J\033[H", 7);
      continue;
    }

    if(strcmp(buf, "exit") == 0){
      break;
    }



    printf("%s: command not found\n", buf);
  }

  return 0;
}
