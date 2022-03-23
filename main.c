#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //currently just for strcat, strtok and strcmp
#include <math.h>       //just for sqrt
#include <stdbool.h>
#include <float.h>      //just for DBL_MAX
#include <time.h>       //for generating random seeds

#define VERTEX_COUNT 48 //Placeholder
#define HEADER_LINE_COUNT 6

typedef struct Vertex{
    int id;
    double x;
    double y;
} Vertex;

double tsp_NN(Vertex vertices[], int);
double tsp_NND(Vertex vertices[], int);

double tsp_NN(Vertex vertices[], int vertex_list_size){
    //Dynamically allocating 2D matrix
    double** distance_matrix = calloc(vertex_list_size, sizeof(double*));
    for(int i = 0; i < vertex_list_size; i++){
        distance_matrix[i] = calloc(vertex_list_size, sizeof(double));
    }

    //Setting up all the distances so we don't have to calculate the same distance multiple times
    for (int i = 0; i < vertex_list_size; i++){
        for (int j = 0; j < vertex_list_size; j++){
            //SEGMENTATION FAULT RIGHT HERE WHEN USING PLA33810
            distance_matrix[i][j] = sqrt(((vertices[i].x - vertices[j].x) * (vertices[i].x - vertices[j].x)) + ((vertices[i].y - vertices[j].y) * (vertices[i].y - vertices[j].y)));
        }
    }

    time_t t;
    srand((unsigned) time(NULL)); // Setting up random seed
    int starting_vertex_index = rand() % vertex_list_size;
        
    int sequence_length = 1; //we start with one vertex
    double current_distance = 0;
    
    bool* visited = calloc(vertex_list_size, sizeof(bool));
    visited[starting_vertex_index] = true; // we always start from the first vertex

    int current_vertex_index = starting_vertex_index; // We start from 0
    int next_index = 0;
    while (sequence_length < vertex_list_size){
        double min_distance = DBL_MAX;
        for (int i = 0; i < vertex_list_size; i++){
            // We only care about vertices that weren't visited yet
            if (!visited[i]){
                if (distance_matrix[current_vertex_index][i] < min_distance){
                    min_distance = distance_matrix[current_vertex_index][i];
                    next_index = i;
                }
            }
        }
        current_vertex_index = next_index;
        visited[current_vertex_index] = true;
        current_distance = current_distance + min_distance;
        sequence_length++;
    }
    current_distance = current_distance + distance_matrix[starting_vertex_index][current_vertex_index]; //Connecting end-points

    for(int i = 0; i < vertex_list_size; i++){
        free(distance_matrix[i]);
        distance_matrix[i] = NULL;
    }
    free(distance_matrix);
    distance_matrix = NULL;

    free(visited);
    visited = NULL;

    return current_distance;
}

double tsp_NND(Vertex vertices[], int vertex_list_size){
    //Dynamically allocating 2D matrix
    double** distance_matrix = calloc(vertex_list_size, sizeof(double*));
    for(int i = 0; i < vertex_list_size; i++){
        distance_matrix[i] = calloc(vertex_list_size, sizeof(double));
    }

    //Setting up all the distances so we don't have to calculate the same distance multiple times
    for (int i = 0; i < vertex_list_size; i++){
        for (int j = 0; j < vertex_list_size; j++){
            distance_matrix[i][j] = sqrt(((vertices[i].x - vertices[j].x) * (vertices[i].x - vertices[j].x)) + ((vertices[i].y - vertices[j].y) * (vertices[i].y - vertices[j].y)));
        }
    }
    
    time_t t;
    srand((unsigned) time(NULL)); // Setting up random seed
    int starting_vertex_index = rand() % vertex_list_size;

    double current_distance = 0;
    
    bool* visited = calloc(vertex_list_size, sizeof(bool));
    visited[starting_vertex_index] = true; // we always start from the first vertex
    
    int sequence_length = 1;
    int end_vertex_index = starting_vertex_index; // We start from 0
    int start_vertex_index = starting_vertex_index; // Start and end points are the same in the beginning
    int next_index = 0;
    bool need_to_update_end_point = false; //Just a boolean so we can know if we need to update the end-point or the start-point (exclusive OR)
    
    while(sequence_length < vertex_list_size){
        double min_distance = DBL_MAX;
        //Searching for vertex which is closest to the start-point or the end-point
        for(int i = 0; i < vertex_list_size; i++){
            if(!visited[i]){
                //Checking the vertices adjacent to the end-point
                if(distance_matrix[start_vertex_index][i] < min_distance){
                    min_distance = distance_matrix[start_vertex_index][i];
                    next_index = i;
                    need_to_update_end_point = false;
                }
                //Checking the vertices adjacent to the start-point
                if(distance_matrix[end_vertex_index][i] < min_distance){
                    min_distance = distance_matrix[end_vertex_index][i];
                    next_index = i;
                    need_to_update_end_point = true;
                }
            }
        }
        if(need_to_update_end_point){
            end_vertex_index = next_index;
            visited[end_vertex_index] = true;
        }else{
            start_vertex_index = next_index;
            visited[start_vertex_index] = true;
        }
        current_distance = current_distance + min_distance;    
        sequence_length++;
    }
    current_distance = current_distance + distance_matrix[start_vertex_index][end_vertex_index]; //Connecting end-points
    return current_distance;
}

int main(int argc, char *argv[]){
    char input_file_directory[30] = "data\\";             // Maybe could be a #define idk
    strcat(input_file_directory, "pla33810.in");          // data\<input_file>

    FILE *file = fopen(input_file_directory, "r"); // files from the data directory are read only files
    if (file == NULL){
        printf("Error while searching for the input file.\nCheck if the input file name is typed correctly or type the input file's directory.");
        return -1;
    }

    char line[100] = "";
    int line_count = 0;
    
    // Just counting the number of lines in the file (so we can get the amount of vertices)
    while (fgets(line, sizeof(line), file)){
        line_count++;
    }
    
    int vertex_count = line_count - 7; //Amount of lines that contain vertex data from input file
    Vertex* vertex_list = calloc(vertex_count, sizeof(Vertex));
    
    //Resetting file pointer to the start of file again
    // rewind(file) would also do it
    fseek(file, 0, SEEK_SET);

    line_count = 0;
    // fgets will get a line from the file and it will stop if it hits EOF
    // Fillig vertex_list with vertices (vertex structs) based on the input file data (id, x coordinate, y coordinate)
    while (fgets(line, sizeof(line), file))
    {
        // We don't care about the first 6 lines (they are only there for header information purposes) and
        // neither the last one (EOF string)
        if (line_count >= HEADER_LINE_COUNT && strcmp(line, "EOF\n") != 0 && strcmp(line, "EOF \n") != 0){
            // Separating id, x coordinate and y coordinate (strtok is destructive btw)
            char *first_piece = strtok(line, " ");
            char *second_piece = strtok(NULL, " ");
            char *third_piece = strtok(NULL, " ");
            printf("Vertex %i -> ID: %s , X: %s , Y: %s\n", line_count - 5, first_piece, second_piece, third_piece);

            // Nice way to initialize structs I found
            Vertex vertex = {.id = (double)atoi(first_piece), 
                            .x = (double)atoi(second_piece), 
                            .y = (double)atoi(third_piece)};            
            
            vertex_list[vertex.id - 1] = vertex;
        }
        line_count++;
    }
    
    double result_distance = tsp_NN(vertex_list, vertex_count);
    printf("\nResult distance: %f\n", result_distance);
    
    free(vertex_list);
    vertex_list = NULL;
    fclose(file);
    return 0;
}