#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //currently just for strcat and strtok
#include <math.h>       // just for sqrt
#include <stdbool.h>
#include <float.h>      //just for DBL_MAX

#define VERTEX_COUNT 48

typedef struct Vertex{
    int id;
    int x;
    int y;
} Vertex;

double tsp_NN(Vertex vertices[], int);
double tsp_NND(Vertex vertices[], int);

double tsp_NN(Vertex vertices[], int vertex_list_size){
    //Setting up all the distances so we don't have to calculate the same distance multiple times
    double distance_matrix[VERTEX_COUNT][VERTEX_COUNT] = {0}; // distance_matrix[i][j] = distance_matrix[j][i]
    for (int i = 0; i < vertex_list_size; i++){
        for (int j = 0; j < vertex_list_size; j++){
            distance_matrix[i][j] = sqrt(((vertices[i].x - vertices[j].x) * (vertices[i].x - vertices[j].x)) + ((vertices[i].y - vertices[j].y) * (vertices[i].y - vertices[j].y)));
        }
    }

    //The result is dependent on the vertex we start at, so we're starting from each vertex and getting the best solution
    double optimal_distance = DBL_MAX;
    for(int starting_vertex_index = 0; starting_vertex_index < vertex_list_size; starting_vertex_index++){
        
        int sequence_length = 1; //we start with one vertex
        double current_distance = 0;
        
        bool visited[VERTEX_COUNT] = {0};
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
        current_distance = current_distance + distance_matrix[current_vertex_index][starting_vertex_index];
        if(current_distance < optimal_distance){
            optimal_distance = current_distance;
        }
    }
    return optimal_distance;
}

double tsp_NND(Vertex vertices[], int vertex_list_size){
    double distance_matrix[VERTEX_COUNT][VERTEX_COUNT] = {0}; // distance_matrix[i][j] = distance_matrix[j][i]
    for (int i = 0; i < vertex_list_size; i++){
        for (int j = 0; j < vertex_list_size; j++){
            distance_matrix[i][j] = sqrt(((vertices[i].x - vertices[j].x) * (vertices[i].x - vertices[j].x)) + ((vertices[i].y - vertices[j].y) * (vertices[i].y - vertices[j].y)));
        }
    }
    
    double optimal_distance = DBL_MAX;
    for(int starting_vertex_index = 0; starting_vertex_index < vertex_list_size; starting_vertex_index++){
        double current_distance = 0;

    
        bool visited[VERTEX_COUNT] = {0};
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
        current_distance = current_distance + distance_matrix[start_vertex_index][end_vertex_index];
        if(current_distance < optimal_distance){
            optimal_distance = current_distance;
        }
        printf("New best solution, start %i: %lf\n", starting_vertex_index, optimal_distance);
    }

    return optimal_distance;
}

int main(int argc, char *argv[]){
    char input_file_directory[30] = "data\\";             // Maybe could be a #define idk
    strcat(input_file_directory, "att48.in" /*argv[1]*/); // data\<input_file>

    FILE *file = fopen(input_file_directory, "r"); // files from the data directory are read only files
    if (file == NULL){
        printf("Error while searching for the input file.\nCheck if the input file name is typed correctly or type the input file's directory.");
        return -1;
    }

    Vertex vertex_list[VERTEX_COUNT] = {0};
    char line[100] = "";
    int line_count = 0;

    // Fillig vertex_list with vertices (vertex structs) based on the input file data (id, x coordinate, y coordinate)
    // fgets will get a line from the file and it will stop if it hits EOF
    while (fgets(line, sizeof(line), file)){

        // We don't care about the first 6 lines (they are only there for header information purposes)
        if (line_count >= 6 && line_count <= 53){

            // Separating id, x coordinate and y coordinate (strtok is destructive btw)
            char *first_piece = strtok(line, " ");
            char *second_piece = strtok(NULL, " ");
            char *third_piece = strtok(NULL, " ");

            // Nice way to initialize structs I found
            Vertex vertex = {.id = atoi(first_piece), .x = atoi(second_piece), .y = atoi(third_piece)};

            vertex_list[vertex.id - 1] = vertex;
        }
        line_count++;
    }

    double result_distance = tsp_NND(vertex_list, VERTEX_COUNT);
    printf("\nResult distance: %f\n", result_distance);
    fclose(file);
    return 0;
}