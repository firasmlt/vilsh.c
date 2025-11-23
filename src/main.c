#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#define MAXTOKENS 100;

size_t tokenizeCommands(char* buf, char** tokens){
  size_t tokens_len = 0;

  char* token = strtok(buf, " ");

  while(token != NULL){
    tokens[tokens_len] = strdup(token);
    tokens_len++;
    token = strtok(NULL, " ");
  }
  return tokens_len;
}


char *valid_commands[] = {"exit", "type", "echo", "clear"};

int isValidCommand(char* command){
  size_t size = sizeof(valid_commands)/sizeof(char*);

  for(int i = 0; i < size; i++){
    if(strcmp(command, valid_commands[i]) == 0){
      return 1;
    }
  }
  return 0;
}

int doesFileExist(char* filepath){
  return access(filepath, X_OK) == 0;
}

char* isValidPathFile(char* name){
  char* path = getenv("PATH");
  if(!path) return NULL;
  char *path_copy = strdup(path);
  if(!path_copy) return NULL;
  char* directory = strtok(path_copy, ":");
  int c = 0;
  char* directories[100];
  while(directory != NULL && c < 100){
    directories[c] = directory;
    c++;
    directory = strtok(NULL, ":");
  }


  for(int i = 0; i < c; i++){
    char filepath[256];
    filepath[0] = '\0';
    strcat(filepath, directories[i]);
    strcat(filepath, "/");
    strcat(filepath, name);

    if(doesFileExist(filepath)){
      free(path_copy);
      return strdup(filepath);
    }
  }
  free(path_copy);
  return NULL;
}



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

    char** tokens = malloc(100 * sizeof(char*));
    size_t tokens_len = tokenizeCommands(buf, tokens);

    if(tokens_len < 1) continue;

    // CLEAR COMMAND
    if(strcmp(tokens[0], "clear") == 0){
      if(tokens_len == 1){
        write(1, "\033[2J\033[H", 7);
      }else{
        printf("clear: Unknown arguments provided; try 'clear' instead\n");
      }
      continue;
    }
    // EXIT COMMAND
    if(strcmp(tokens[0], "exit") == 0 && tokens_len == 1){
      break;
    }

    // ECHO COMMAND
    if(strcmp(tokens[0], "echo") == 0){
      for(size_t i = 1; i < tokens_len; i++){
        if(i!=1) printf(" ");
        printf("%s", tokens[i]);
      }
      printf("\n");
      continue;
    }

    if(strcmp(tokens[0], "type") == 0){
      if(tokens_len == 1){
        continue;
      }
      for(size_t i =1; i < tokens_len; i++){
        char* filePath = isValidPathFile(tokens[i]);
        if(isValidCommand(tokens[i])){
          printf("%s is a shell builtin\n", tokens[i]);
        }else if(filePath){
          printf("%s is %s\n", tokens[i], filePath);
        }else{
          printf("%s: not found\n", tokens[i]);
        }
        free(filePath);
      }
      continue;
    }

    if(isValidPathFile(tokens[0])){
      char output[1024];
      char command[300];
      command[0] = '\0';
      strcat(command, tokens[0]);
      for(int i = 1; i < tokens_len; i++){
        strcat(command, " ");
        strcat(command, tokens[i]);
      }

      FILE *fp = popen(command, "r");
      while(fgets(output, sizeof(output), fp) != NULL){
        printf("%s", output);
      }
      continue;
    }





    printf("%s: command not found\n", tokens[0]);

    free(tokens);
  }

  return 0;
}
