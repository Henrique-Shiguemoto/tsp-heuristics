#include <stdio.h>
#include <stdlib.h>
#include <string.h>                 //currently just for strcat, strtok and strcmp
#include <math.h>                   //just for sqrt
#include <stdbool.h>
#include <float.h>                  //just for DBL_MAX
#include <time.h>

#define HEADER_LINE_COUNT 6         //This is just the number of lines in the header of the input files

typedef struct Vertex{
    int id;
    double x;
    double y;
} Vertex;

typedef struct Cycle{
    double result;
    int cycle_size;
    Vertex vertex_cycle[];
} Cycle;

void print_vertex(Vertex);
Cycle* initialize_cycle(int);
void destroy_cycle(Cycle*);
double calculate_distance(Vertex vertices[], int, int);
Cycle* tsp_NN(Vertex vertices[], int);
Cycle* tsp_NND(Vertex vertices[], int);
void print_cycle(Cycle*);

void print_vertex(Vertex v){
    printf("ID = %i\t", v.id);
    printf("X = %lf\t", v.x);
    printf("Y = %lf\n", v.y);
}

Cycle* initialize_cycle(int size){
    Cycle* cycle = malloc(sizeof(double) + sizeof(int) + sizeof(Vertex) * size);
    
    if(!cycle){
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    cycle->result = 0;
    cycle->cycle_size = size;

    return cycle;
}

void destroy_cycle(Cycle* cycle){
    //Rule of thumb: ONE FREE PER MALLOC
    free(cycle);
    cycle = NULL;
}

double calculate_distance(Vertex vertices[], int i, int j){
    return sqrt(pow(vertices[i].x - vertices[j].x, 2) + pow(vertices[i].y - vertices[j].y, 2));
}

Cycle* tsp_NN(Vertex vertices[], int vertex_list_size){
    Cycle* cycle = initialize_cycle(vertex_list_size);
    
    srand((unsigned) time(NULL));
    int starting_vertex_index = rand() % vertex_list_size;

    int sequence_length = 1; //we start with one vertex
    double current_distance = 0;

    bool* visited = calloc(vertex_list_size, sizeof(bool));
    visited[starting_vertex_index] = true; // we always start from the first vertex
    int cycle_iterator = 0;
    cycle->vertex_cycle[cycle_iterator] = vertices[starting_vertex_index]; //Starting the cycle build
    
    int current_vertex_index = starting_vertex_index;
    int next_index = 0;
    while (sequence_length < vertex_list_size){
        double min_distance = DBL_MAX;
        for (int i = 0; i < vertex_list_size; i++){
            // We only care about vertices that weren't visited yet
            if (!visited[i]){
                double distance = calculate_distance(vertices, current_vertex_index, i);
                if(distance < min_distance){
                    min_distance = distance;
                    next_index = i;
                }
            }
        }
        current_vertex_index = next_index;        

        visited[current_vertex_index] = true;
        cycle_iterator++;
        cycle->vertex_cycle[cycle_iterator] = vertices[current_vertex_index];
        
        current_distance = current_distance + min_distance;
        sequence_length++;
    }

    current_distance = current_distance + calculate_distance(vertices, starting_vertex_index, current_vertex_index);
    cycle->result = current_distance;

    free(visited);
    visited = NULL;

    return cycle;
}

Cycle* tsp_NND(Vertex vertices[], int vertex_list_size){
    Cycle* cycle = initialize_cycle(vertex_list_size);
    
    srand((unsigned) time(NULL));
    int starting_vertex_index = rand() % vertex_list_size;

    int end = starting_vertex_index; 
    int start = starting_vertex_index; // Start and end points are the same in the beginning

    int sequence_length = 1;
    double current_distance = 0;
    
    bool* visited = calloc(vertex_list_size, sizeof(bool));
    visited[starting_vertex_index] = true; // we always start from the first vertex
    int cycle_iterator = 0;
    cycle->vertex_cycle[cycle_iterator] = vertices[starting_vertex_index];

    int next_index = 0;
    bool need_to_update_end_point = false; //Just a boolean so we can know if we need to update the end-point or the start-point (exclusive OR)

    while(sequence_length < vertex_list_size){
        double min_distance = DBL_MAX;
        //Searching for vertex which is closest to the start-point or the end-point
        for(int i = 0; i < vertex_list_size; i++){
            if(!visited[i]){
                //Checking the vertices adjacent to the end-point
                double distance_from_start = calculate_distance(vertices, start, i);
                double distance_from_end = calculate_distance(vertices, end, i);
                if(distance_from_start < min_distance){
                    min_distance = distance_from_start;
                    next_index = i;
                    need_to_update_end_point = false;
                }
                if(distance_from_end < min_distance){
                    min_distance = distance_from_end;
                    next_index = i;
                    need_to_update_end_point = true;
                }
            }
        }

        if(need_to_update_end_point){
            end = next_index;
            visited[end] = true;
            cycle_iterator++;
            cycle->vertex_cycle[cycle_iterator] = vertices[end];
        }else{
            start = next_index;
            visited[start] = true;
            cycle_iterator++;
            for(int i = cycle_iterator; i > 0; i--){
                cycle->vertex_cycle[i] = cycle->vertex_cycle[i - 1];
            }
            cycle->vertex_cycle[0] = vertices[start];
        }
        current_distance = current_distance + min_distance;
        sequence_length++;
    }

    //Connecting end-points
    current_distance = current_distance + calculate_distance(vertices, start, end);
    cycle->result = current_distance;

    free(visited);
    visited = NULL;

    return cycle;
}

void print_cycle(Cycle* cycle){
    for(int i = 0; i < cycle->cycle_size; i++){
        printf("cycle->vertex_cycle[%i].id = %i\n", i, cycle->vertex_cycle[i].id);
    }
    printf("\nResult distance: %lf\n", cycle->result);
}

int main(int argc, char *argv[]){
    
    if(argc < 2){
        printf("This program needs at least 2 arguments to run: <exe_name> <input_file>");
        exit(EXIT_FAILURE);
    }

    char input_file_directory[30] = "data\\";    // Maybe could be a #define idk
    strcat(input_file_directory, argv[1]);       // data\<input_file>

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
    fseek(file, 0, SEEK_SET);

    line_count = 0;
    // Fillig vertex_list with vertices (vertex structs) based on the input file data (id, x coordinate, y coordinate)
    while (fgets(line, sizeof(line), file))
    {
        // We don't care about the first 6 lines (they are only there for header information purposes) and neither the last one (EOF string)
        if (line_count >= HEADER_LINE_COUNT && strcmp(line, "EOF\n") != 0 && strcmp(line, "EOF \n") != 0 && strcmp(line, "EOF") != 0){
            // Separating id, x coordinate and y coordinate (strtok is destructive btw)
            char *first_piece = strtok(line, " ");
            char *second_piece = strtok(NULL, " ");
            char *third_piece = strtok(NULL, " ");
            char *err;

            Vertex vertex = {.id = atoi(first_piece), 
                            .x = strtod(second_piece, &err), 
                            .y = strtod(third_piece, &err)};
            //print_vertex(vertex);
            vertex_list[vertex.id - 1] = vertex;
        }
        line_count++;
    }

    printf("Starting tsp_NN...             ");
    Cycle* resultant_cycle_NN = tsp_NN(vertex_list, vertex_count);
    printf("Result: %lf\n", resultant_cycle_NN->result);

    printf("Starting tsp_NND...            ");
    Cycle* resultant_cycle_NND = tsp_NND(vertex_list, vertex_count);
    printf("Result: %lf\n", resultant_cycle_NND->result);
    
    destroy_cycle(resultant_cycle_NN);
    destroy_cycle(resultant_cycle_NND);
    free(vertex_list);
    vertex_list = NULL;
    
    fclose(file);
    return 0;
}