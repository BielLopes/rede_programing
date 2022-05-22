#pragma once

struct message_type
{
    int command;
    int lis_sensors[4];
    int list_sensors_length;
    int equipment;
};

struct storage_communication
{
    int equipments[4][4];
    int total_equipment;
};

int just_numbers(char *validation, int is_equipment);

int validate_and_create_message(char *buf, int count, struct message_type *message, int csock);

char *update_bd_and_create_response(struct message_type *message, struct storage_communication *storage);
