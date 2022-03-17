#include<stdio.h>
#include<stdlib.h>
#include<string.h> //currently just for strcat

#define VERTEX_COUNT 48

typedef struct Vertex{
    int id;
    int x;
    int y;
}Vertex;

int main(int argc, char* argv[]){    
    if(argc <= 1){
        printf("Not enough arguments, you can check the data folder to see which input files there are. \n\nSyntax is <program_name> <input_file>\n");
        return -1;
    }else{
        char input_file_directory[30] = "data\\";
        strcat(input_file_directory, argv[1]); // data\<input_file>
        
        FILE* file = fopen(input_file_directory, "r");
        if(file == NULL){
            printf("Error while searching for the input file.\nCheck if the input file name is typed correctly or type the input file's directory.");
            return -1;
        }      

        Vertex vertex_list[VERTEX_COUNT] = {0};
        char line[100] = "";
        int line_count = 0;
        
        //Fill vertex_list by reading data from att48.in, while loop until it hits EOF string
        while(fgets(line, sizeof(line), file)){
            
            //We don't care about the first 6 lines
            if(line_count >= 6 && line_count <= 53){
                char* first_piece = strtok(line, " ");
                char* second_piece = strtok(NULL, " ");
                char* third_piece = strtok(NULL, " ");

                Vertex vertex = {.id = atoi(first_piece), .x = atoi(second_piece), .y = atoi(third_piece)};

                vertex_list[vertex.id - 1] = vertex;
            }
            line_count++;
        }

        fclose(file);
    }
    return 0;
}