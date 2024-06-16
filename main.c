#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_FIELD_LENGTH 256
#define MAX_FIELDS 10

typedef struct {
    char fields[MAX_FIELDS][MAX_FIELD_LENGTH];
    int field_count;
} CSVLine;

// Fonction pour lire une ligne CSV et la stocker dans une structure CSVLine
void parse_csv_line(char* line, CSVLine* csv_line) {
    char* token;
    char* rest = line;
    csv_line->field_count = 0;

    while ((token = strtok_r(rest, ",", &rest)) && csv_line->field_count < MAX_FIELDS) {
        strncpy(csv_line->fields[csv_line->field_count], token, MAX_FIELD_LENGTH - 1);
        csv_line->fields[csv_line->field_count][MAX_FIELD_LENGTH - 1] = '\0'; // s'assurer que la chaîne est terminée
        csv_line->field_count++;
    }
}

// Fonction pour écrire une structure CSVLine dans une ligne CSV
void write_csv_line(FILE* file, CSVLine* csv_line) {
    for (int i = 0; i < csv_line->field_count; i++) {
        fprintf(file, "%s", csv_line->fields[i]);
        if (i < csv_line->field_count - 1) {
            fprintf(file, ",");
        }
    }
    fprintf(file, "\n");
}

const char *GetFilePathGameData(int file_numb) {
    DIR *dirp;
    struct dirent *direntp;
    static char result[LINE_MAX];
    
    dirp = opendir("game_data");
    
    if (dirp == NULL) {
        perror("");
        return NULL;
    } else {
        for (int i = 0; i < file_numb; i++) {
            direntp = readdir(dirp);
            if (direntp == NULL) break; 
        }
        if (direntp != NULL) {
            snprintf(result, LINE_MAX, "game_data/%s", direntp->d_name);
        } else {
            result[0] = '\0';
        }
        closedir(dirp);
    }
    return result;
}

const char *ReturnGameDataFolder(short int file_id, short int x, short int y) {
    const char *file_path = GetFilePathGameData(file_id);
    if (!file_path) {
        return NULL;
    }

    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        printf("File not found: %s\n", file_path);
        return NULL;
    }

    static char result[MAX_FIELD_LENGTH];
    char line[MAX_LINE_LENGTH];
    int current_line = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (current_line == x) {
            CSVLine csv_line;
            parse_csv_line(line, &csv_line);
            if (y < csv_line.field_count) {
                strncpy(result, csv_line.fields[y], MAX_FIELD_LENGTH);
                result[MAX_FIELD_LENGTH - 1] = '\0';
                fclose(fp);
                printf("Path: %s\n", file_path);
                return result;
            }
        }
        current_line++;
    }

    fclose(fp);
    return NULL;
}

int FunctionStoreGameData(const char *data, short int file_id, short int coord_x, short int coord_y) {
    const char *file_path = GetFilePathGameData(file_id);
    if (!file_path) {
        return EXIT_FAILURE;
    }

    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("ERROR opening file");
        return EXIT_FAILURE;
    }

    FILE *temp_file = fopen("game_data/temp.csv", "w");
    if (!temp_file) {
        perror("ERROR opening temp file");
        fclose(file);
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH];
    int current_row = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        CSVLine csv_line;
        parse_csv_line(line, &csv_line);

        if (current_row == coord_x && coord_y < csv_line.field_count) {
            strncpy(csv_line.fields[coord_y], data, MAX_FIELD_LENGTH - 1);
            csv_line.fields[coord_y][MAX_FIELD_LENGTH - 1] = '\0';
        }

        write_csv_line(temp_file, &csv_line);
        current_row++;
    }

    fclose(file);
    fclose(temp_file);

    if (remove(file_path) != 0) {
        perror("Erreur lors de la suppression du fichier d'origine");
        return EXIT_FAILURE;
    }

    if (rename("game_data/temp.csv", file_path) != 0) {
        perror("Erreur lors du renommage du fichier temporaire");
        return EXIT_FAILURE;
    }
    printf("Storage successful");

    return EXIT_SUCCESS;
}

const char *CSV_reader(int file_ID, int coord_x, int coord_y) {
    
    if (file_ID > 9 || file_ID < 3) {
        printf("\nERROR: There is no such file!\n");
        return 0;
    }

    printf("ID: %d x: %d y: %d\nLoading...\n", file_ID, coord_x, coord_y);

    const char *ask = ReturnGameDataFolder(file_ID, coord_x, coord_y);
    /*if (ask) {
        printf("Result: %s\n", ask);
    } else {
        printf("ERROR_CSV_Reader_Data not found.\n");
    }*/
    return ask;
}
    
int CSV_writer(char to_store, int file_ID, int coord_x, int coord_y){ //__Storage function call
    
    printf("Storing: %s\n", &to_store);
    int result = FunctionStoreGameData(&to_store, file_ID, coord_x, coord_y);
    if (result == 0) {
        printf("Stored successfully\n");
    } else {
        printf("ERROR: Storage failed\n");
    }

    return 0;
}

char getYesNoResponse() {
    char input[MAX_INPUT];
    char response;

    while (1) {
        printf("y/n ?");
        // Read input scanf
        if (scanf("%9s", input) == 1) {
            // Flush the input buffer to avoid unwanted effects
            while (getchar() != '\n');

            // Convert in lower caracter
            for (int i = 0; input[i]; i++) {
                input[i] = tolower((unsigned char)input[i]);
            }

            if (strcmp(input, "yes") == 0) {
                response = 'y';
                break;
            } else if (strcmp(input, "no") == 0) {
                response = 'n';
                break;
            } else {
                printf("\nInvalide input. Please awnser by y/n.\n");
            }
        } else {
            // Manage error
            printf("ERROR_reading_input.\n");
            clearerr(stdin);  // Remove error
        }
    }

    return response;
}



int main() {

    int file_id;
    int x;
    int y;

    // "CSV's ID: 3 test ; 4 races ; 5 classes ; 6 players ; 7 spells ; 8 bosses ; 9 enemies"
    printf("file_id:");
    scanf("%d", &file_id);
    printf("x:");
    scanf("%d", &x);
    printf("y:");
    scanf("%d", &y);

    const char *result = CSV_reader(file_id, x, y);
    printf("result: %s\n", result);

    // Function call to modify result data
    printf("Modify this data ?");
    char input_keyboard = getYesNoResponse();
    
    if (input_keyboard == 'y') 
    {
        char new_data[LINE_MAX];
        
        printf("Enter a new data: ");
        scanf("%s", new_data);
        FunctionStoreGameData(new_data, file_id, x, y);
    } else {
        printf("You say: NO.\n");
    }


    return 0;
}
