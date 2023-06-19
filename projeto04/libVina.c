#include <stdio.h>
#include <stdlib.h>
#include "libArquivos.h"
#include "libLista.h"
#include "libVina.h"

#define ARGUMENT_OFFSET 3

struct list_t* load_list(FILE *archive, struct list_t *list, size_t archive_pointer) {
    fseek(archive, archive_pointer, SEEK_SET);
    struct file_header_t temp;
    
    while (fread(&temp, sizeof(struct file_header_t), 1, archive) == 1)
        add_list_tail(list, &temp);

    return list;
}

struct list_t* store_list(FILE *archive, struct list_t *list, size_t archive_pointer) {
    fseek(archive, archive_pointer, SEEK_SET);
    struct file_header_t *file_data;
    
    while ((file_data = get_first_element(list))) {
        fwrite(file_data, sizeof(struct file_header_t), 1, archive);
        free(file_data);
    }

    return list;
}

void insert_file(FILE *archive, struct list_t *list, char *filename, size_t *archive_pointer) {
    struct file_header_t *file_data = get_data(filename);
    printf("TESTE: %s\n", filename);
    FILE *member = open_member(filename);
    size_t file_size = file_data->size;
    size_t buffer_size = 1024;
    char buffer[buffer_size];
    file_data->archive_position = *archive_pointer;

    if(member == NULL)
        return;

    while (file_size > 0) {
        size_t read_size = (file_size < buffer_size)? file_size : buffer_size;
        size_t bytes_read = fread(buffer, sizeof(char), read_size, member);
        fwrite(buffer, sizeof(char), bytes_read, archive);
        file_size -= bytes_read;
    }

    (*archive_pointer) += file_data->size;
    add_list_tail(list, file_data);
    fclose(member);
}

void insert_operation(FILE *archive, char **argv, int members_quantity) {
    struct list_t *list = make_list();
    size_t archive_pointer;

    if(fread(&archive_pointer, sizeof(size_t), 1, archive)) {
        load_list(archive, list, archive_pointer);
        fseek(archive, archive_pointer, SEEK_SET);
    } else {
        archive_pointer = sizeof(size_t);
        fwrite(&archive_pointer, sizeof(size_t), 1, archive);
    }

    for (int i = ARGUMENT_OFFSET; i < members_quantity; i++)
        if(! is_element_present(list, argv[i]))
            insert_file(archive, list, argv[i], &archive_pointer);
    //TODO: fazer o caso de o arquivo já estar lá dentro. Acho que é mais fácil
    //com a remoção já feita depois.
    fseek(archive, 0, SEEK_SET);
    fwrite(&archive_pointer, sizeof(size_t), 1, archive);

    store_list(archive, list, archive_pointer);
    list = delete_list(list);
}

void update_file(FILE *archive, char **argv, int members_quantity) {
}

void move_file(FILE *archive, char **argv, int members_quantity) {
}

void extract_file(FILE *archive, struct file_header_t *file_data) {
    FILE *member = make_member(file_data->filename);
    size_t file_size = file_data->size;
    size_t buffer_size = 1024;
    char buffer[buffer_size];

    if(member == NULL)
        return;

    fseek(archive, file_data->archive_position, SEEK_SET);

    while (file_size > 0) {
        size_t read_size = (file_size < buffer_size)? file_size : buffer_size;
        size_t bytes_read = fread(buffer, sizeof(char), read_size, archive);
        fwrite(buffer, sizeof(char), bytes_read, member);
        file_size -= bytes_read;
    }

    fclose(member);
}

void extract_all(FILE *archive, struct list_t *list, size_t archive_pointer) {
    struct file_header_t *temp;
    
    while ((temp = get_first_element(list))) {
        extract_file(archive, temp);
        free(temp);
    }
}

void extract_operation(FILE *archive, char **argv, int members_quantity) {
    size_t archive_pointer;

    if(fread(&archive_pointer, sizeof(size_t), 1, archive)) {
        struct list_t *list = make_list();
        load_list(archive, list, archive_pointer);

        if(members_quantity == 3) {
            extract_all(archive, list, archive_pointer);
        }

        list = delete_list(list);
    } else {
    }
}

void remove_operation(FILE *archive, char **argv, int members_quantity) {
}

void list_files(FILE *archive) {
    size_t archive_pointer;
    fread(&archive_pointer, sizeof(size_t), 1, archive);
    fseek(archive, archive_pointer, SEEK_SET);
    struct file_header_t temp;
    
    while (fread(&temp, sizeof(struct file_header_t), 1, archive) == 1)
        write_file_data(&temp);
}

void help_utility(char *call) {
    printf("Uso:\n");
    printf("%s [-i | -a | -m <target> | -x | -r | -c | -h]", call);
    printf(" <archive> [membro1 membro2 ...]\n");
}
