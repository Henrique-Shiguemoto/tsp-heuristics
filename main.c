#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //currently just for strcat, strtok and strcmp
#include <math.h>       //just for sqrt
#include <stdbool.h>
#include <float.h>      //just for DBL_MAX

#define HEADER_LINE_COUNT 6

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

Cycle* initialize_cycle(int);
void destroy_cycle(Cycle*);
double calculate_cycle_distance(Cycle*);
Cycle* tsp_2Opt_Optimal(Cycle*);
Cycle* two_opt_swap(Cycle*, int, int);
bool two_change_greedy(Cycle*);
Cycle* tsp_2Opt_Greedy(Cycle*);
Cycle* tsp_NN(Vertex vertices[], int);
Cycle* tsp_NND(Vertex vertices[], int);
void printCycle(Cycle*);

Cycle* initialize_cycle(int size){
    Cycle* cycle = malloc(sizeof(double) + sizeof(int) + sizeof(Vertex) * size);
    
    if(!cycle){
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    cycle->result = 0;
    cycle->cycle_size = size;

    for(int i = 0; i < size; i++){
        cycle->vertex_cycle[i].id = 0;
        cycle->vertex_cycle[i].x = 0;
        cycle->vertex_cycle[i].y = 0;
    }

    return cycle;
}

void destroy_cycle(Cycle* cycle){
    //Rule of thumb: ONE FREE PER MALLOC
    free(cycle);
    cycle = NULL;
}

double calculate_cycle_distance(Cycle* cycle){
    double result = 0;
    
    for (int i = 0; i < cycle->cycle_size - 1; i++)
    {
        result = result + sqrt((cycle->vertex_cycle[i].x - cycle->vertex_cycle[i + 1].x) * (cycle->vertex_cycle[i].x - cycle->vertex_cycle[i + 1].x) + 
                               (cycle->vertex_cycle[i].y - cycle->vertex_cycle[i + 1].y) * (cycle->vertex_cycle[i].y - cycle->vertex_cycle[i + 1].y));
    }

    result = result + sqrt((cycle->vertex_cycle[0].x - cycle->vertex_cycle[cycle->cycle_size - 1].x) * (cycle->vertex_cycle[0].x - cycle->vertex_cycle[cycle->cycle_size - 1].x) + 
                           (cycle->vertex_cycle[0].y - cycle->vertex_cycle[cycle->cycle_size - 1].y) * (cycle->vertex_cycle[0].y - cycle->vertex_cycle[cycle->cycle_size - 1].y));
    
    return result;
}

Cycle* tsp_2Opt_Optimal(Cycle* input_cycle){
    Cycle* best_cycle = initialize_cycle(input_cycle->cycle_size);

    bool improved = true;
    while(improved){
        improved = false;
        double current_distance = input_cycle->result;
        int size = input_cycle->cycle_size;
        for(int i = 0; i < size - 1; i++){
            for(int j = i + 1; j < size; j++){
                
                Cycle* cycle = two_opt_swap(input_cycle, i, j);
                cycle->result = calculate_cycle_distance(cycle);
                
                if(cycle->result < current_distance){
                    //best_cycle = cycle
                    //We NEED a deep copy here because we have to use destroy_cycle(cycle) when we get out of the scope inner
                    // for loop scope
                    best_cycle->result = cycle->result;
                    for(int k = 0; k < best_cycle->cycle_size; k++){
                        best_cycle->vertex_cycle[k] = cycle->vertex_cycle[k];
                    }
                    
                    current_distance = best_cycle->result;
                    improved = true;
                }

                destroy_cycle(cycle);
            }
        }
        //Updating the input_cycle
        if(improved){
            input_cycle = best_cycle;
        }
    }
    return input_cycle;
}

Cycle* two_opt_swap(Cycle* current_cycle, int i, int j){
    Cycle* cycle = initialize_cycle(current_cycle->cycle_size);
    
    //In this function we are removing the edges (i - 1, i) and (j, j + 1) 
    // and adding the edges (i - 1, j) and (i, j + 1)

    for(int n = 0; n <= i - 1; n++){
        cycle->vertex_cycle[n] = current_cycle->vertex_cycle[n];
    }

    int temp = 0;
    for (int n = i; n <= j; n++)
    {
        cycle->vertex_cycle[n] = current_cycle->vertex_cycle[j - temp];
        temp++;
    }

    for(int n = j + 1; n < current_cycle->cycle_size; n++){
        cycle->vertex_cycle[n] = current_cycle->vertex_cycle[n];
    }
    
    return cycle;
}

Cycle* tsp_2Opt_Greedy(Cycle* input_cycle){
    Cycle* best_cycle = initialize_cycle(input_cycle->cycle_size);
    
    int allowed_times_improved = 10;
    int times_improved = 0;

    bool improved = true;
    while(improved && times_improved < allowed_times_improved){
        improved = false;
        double current_distance = input_cycle->result;
        int size = input_cycle->cycle_size;
        for(int i = 0; i < size - 1; i++){
            for(int j = i + 1; j < size; j++){
                Cycle* cycle = two_opt_swap(input_cycle, i, j);
                cycle->result = calculate_cycle_distance(cycle);
                
                if(cycle->result < current_distance){                    
                    //best_cycle = cycle;
                    //We NEED a deep copy here because we have to use destroy_cycle(cycle) when we get out of the scope inner
                    // for loop scope
                    best_cycle->result = cycle->result;
                    for(int k = 0; k < best_cycle->cycle_size; k++){
                        best_cycle->vertex_cycle[k] = cycle->vertex_cycle[k];
                    }
                    
                    current_distance = best_cycle->result;
                    times_improved++;
                    improved = true;
                }
                destroy_cycle(cycle);
            }
        }
        //Updating the input_cycle
        if(improved){
            input_cycle = best_cycle;
        }
    }
    return input_cycle;
}

Cycle* tsp_NN(Vertex vertices[], int vertex_list_size){
    Cycle* cycle = initialize_cycle(vertex_list_size);
    
    int starting_vertex_index = 1; // We start from the second vertex in this example

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
                double distance = sqrt((vertices[current_vertex_index].x - vertices[i].x)*(vertices[current_vertex_index].x - vertices[i].x) + 
                                        (vertices[current_vertex_index].y - vertices[i].y)*(vertices[current_vertex_index].y - vertices[i].y));
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

    current_distance = current_distance + sqrt((vertices[starting_vertex_index].x - vertices[current_vertex_index].x)*
                                                (vertices[starting_vertex_index].x - vertices[current_vertex_index].x) + 
                                                (vertices[starting_vertex_index].y - vertices[current_vertex_index].y)*
                                                (vertices[starting_vertex_index].y - vertices[current_vertex_index].y));
    cycle->result = current_distance;

    free(visited);
    visited = NULL;

    return cycle;
}

Cycle* tsp_NND(Vertex vertices[], int vertex_list_size){
    Cycle* cycle = initialize_cycle(vertex_list_size);
    
    int starting_vertex_index = 1; // We start from the second vertex in this example
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
                double distance_from_start = sqrt((vertices[start].x - vertices[i].x)*(vertices[start].x - vertices[i].x) +
                                                    (vertices[start].y - vertices[i].y)*(vertices[start].y - vertices[i].y));
                double distance_from_end = sqrt((vertices[end].x - vertices[i].x)*(vertices[end].x - vertices[i].x) +
                                                    (vertices[end].y - vertices[i].y)*(vertices[end].y - vertices[i].y));
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
    current_distance = current_distance + sqrt((vertices[start].x - vertices[end].x)*
                                                (vertices[start].x - vertices[end].x) + 
                                                (vertices[start].y - vertices[end].y)*
                                                (vertices[start].y - vertices[end].y));
    cycle->result = current_distance;

    free(visited);
    visited = NULL;

    return cycle;
}

void printCycle(Cycle* cycle){
    for(int i = 0; i < cycle->cycle_size; i++){
        printf("cycle->vertex_cycle[%i].id = %i\n", i, cycle->vertex_cycle[i].id);
    }
    printf("\nResult distance: %lf\n", cycle->result);
}

int main(int argc, char *argv[]){
    char input_file_directory[30] = "data\\";         // Maybe could be a #define idk
    strcat(input_file_directory, "test.in");          // data\<input_file>

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
        if (line_count >= HEADER_LINE_COUNT && strcmp(line, "EOF\n") != 0 && strcmp(line, "EOF \n") != 0){
            // Separating id, x coordinate and y coordinate (strtok is destructive btw)
            char *first_piece = strtok(line, " ");
            char *second_piece = strtok(NULL, " ");
            char *third_piece = strtok(NULL, " ");
            
            Vertex vertex = {.id = atoi(first_piece), 
                            .x = (double)atoi(second_piece), 
                            .y = (double)atoi(third_piece)};

            vertex_list[vertex.id - 1] = vertex;
        }
        line_count++;
    }

    Cycle* resultant_cycle = tsp_NND(vertex_list, vertex_count);

    resultant_cycle = tsp_2Opt_Optimal(resultant_cycle);
    printf("%lf", resultant_cycle->result);

    destroy_cycle(resultant_cycle);
    free(vertex_list);
    vertex_list = NULL;
    
    fclose(file);
    return 0;
}