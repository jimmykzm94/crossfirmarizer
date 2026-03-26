#include "csv_parser.h"
#include "simulation_state.h"
#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

simulation_state_t simulation_state;

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        perror("Please provide absolute path!");
        return -1;
    }

    printf("Firmware simulation started.\n");

    // Reset
    simulation_state.command.count = 0;
    simulation_state.response.count = 0;

    // 1. Open and parse input CSV
    char input_path[1024];
    strcpy(input_path, argv[1]);
    strcat(input_path, "/test_input.csv");
    parse_input_csv(input_path);

    // 2. Run the firmware's core logic
    app_init();
    app_background();

    // 3. Write results to output CSV
    char output_path[1024];
    strcpy(output_path, argv[1]);
    strcat(output_path, "/test_output.csv");
    write_output_csv(output_path);

    printf("Firmware simulation finished.\n");
    return 0;
}
