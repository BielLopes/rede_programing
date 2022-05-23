#include "common.h"
#include "application.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int just_numbers(char *validation, int is_equipment)
{
    /*
        Essa função verifica se o Id enviado pelo cliente está em um formato válido
    */
    if (is_equipment)
    {
        if (strlen(validation) != 3)
            return -1;
    }
    else
    {
        if (strlen(validation) != 2)
            return -1;
    }

    if (validation[0] < '0' || validation[1] > '9')
        return -1;

    if (validation[1] < '0' || validation[1] > '9')
        return -1;

    if (is_equipment)
        if (validation[2] != '\n')
            return -1;

    return 0;
}

int validate_and_create_message(char *buf, struct message_type *message)
{
    /*
        Essa função tem como objetivo verificar se a string de entrada pode ser convertida para a estrutura message_type a partir da manipulação da mesma.
    */
    char *aux = (char *)malloc(sizeof(char) * BUFSZ);
    char *to_free = aux;
    strcpy(aux, buf);
    strtok(aux, " ");

    if (0 == strcmp(aux, "add"))
    {
        message->command = 0;
    }
    else if (0 == strcmp(aux, "remove"))
    {
        message->command = 1;
    }
    else if (0 == strcmp(aux, "list"))
    {
        message->command = 2;
    }
    else if (0 == strcmp(aux, "read"))
    {
        message->command = 3;
    }
    else
    {
        return -1;
    }

    aux = strtok(NULL, " ");

    if (message->command < 3)
    {
        if (message->command < 2)
        {
            if (0 != strcmp(aux, "sensor"))
            {
                return -1;
            }
        }
        else
        {
            if (0 != strcmp(aux, "sensors"))
            {
                return -1;
            }
        }
        aux = strtok(NULL, " ");
    }

    if (message->command != 2)
    {
        if (0 != just_numbers(aux, 0)) // Verifica se a string tem apenas dois números.
        {
            return 1;
        }
        int sensor_id = -1;
        while (1)
        {
            sensor_id = atoi(aux);
            if (sensor_id > 0)
            {
                if (0 != just_numbers(aux, 0))
                {
                    return 1;
                }
                if (sensor_id > 4)
                {
                    return 1;
                }
                message->lis_sensors[message->list_sensors_length] = sensor_id - 1;
                message->list_sensors_length++;
                aux = strtok(NULL, " ");
            }
            else
            {
                break;
            }
        }
    }

    if (0 != strcmp(aux, "in"))
    {
        return -1;
    }

    aux = strtok(NULL, " ");

    if (0 != just_numbers(aux, 1))
    {
        return 2;
    }
    int equipment_id = atoi(aux);
    message->equipment = equipment_id - 1;
    if (equipment_id <= 0 || equipment_id > 4)
    {
        return 2;
    }

    aux = strtok(NULL, " ");
    if (aux != NULL)
    {
        return -1;
    }

    if (message->command == 0 && message->list_sensors_length > 3)
     return -1;

    free(to_free);
    return 0;
}

char *update_bd_and_create_response(struct message_type *message, struct storage_communication *storage)
{
    /*
        O nome é super intuitivo, atualiza o estado dos sensores se necessário e forma a string de resposta para o cliente
    */
    char *response = (char *)malloc(sizeof(char) * BUFSZ);
    response[0] = '\0';
    switch (message->command)
    {
    case 0: // add
    {
        if (storage->total_equipment + message->list_sensors_length >= 16)
        {
            strcat(response, "limit exceeded");
            break;
        }

        strcat(response, "sensor ");

        int number_already_exist = 0;
        int already_exists_vector[4] = {0, 0, 0, 0};
        for (int i = 0; i < message->list_sensors_length; i++)
        {
            if (storage->equipments[message->equipment][message->lis_sensors[i]])
            {
                already_exists_vector[i] = 1;
                number_already_exist++;
            }
            else
            {
                storage->equipments[message->equipment][message->lis_sensors[i]] = 1;
                storage->total_equipment++;
            }
        }

        if (number_already_exist == message->list_sensors_length)
        {
            for (int i = 0; i < message->list_sensors_length; i++)
            {
                char aux[10];
                sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                strcat(response, aux);
            }

            char aux[30];
            sprintf(aux, "already exists in 0%d ", message->equipment + 1);
            strcat(response, aux);
        }
        else
        {
            for (int i = 0; i < message->list_sensors_length; i++)
            {
                if (!already_exists_vector[i])
                {
                    char aux[10];
                    sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                    strcat(response, aux);
                }
            }
            strcat(response, "added");

            if (number_already_exist > 0)
            {
                strcat(response, " ");

                for (int i = 0; i < message->list_sensors_length; i++)
                {
                    if (already_exists_vector[i])
                    {
                        char aux[10];
                        sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                        strcat(response, aux);
                    }
                }

                char aux[30];
                sprintf(aux, "already exists in 0%d ", message->equipment + 1);
                strcat(response, aux);
            }
        }

        break;
    }
    case 1: // remove
    {
        strcat(response, "sensor ");

        int number_dont_exist = 0;
        int dont_exists_vector[4] = {0, 0, 0, 0};
        for (int i = 0; i < message->list_sensors_length; i++)
        {
            if (!storage->equipments[message->equipment][message->lis_sensors[i]])
            {
                dont_exists_vector[i] = 1;
                number_dont_exist++;
            }
            else
            {
                storage->equipments[message->equipment][message->lis_sensors[i]] = 0;
                storage->total_equipment--;
            }
        }

        if (number_dont_exist == message->list_sensors_length)
        {
            for (int i = 0; i < message->list_sensors_length; i++)
            {
                char aux[10];
                sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                strcat(response, aux);
            }

            char aux[30];
            sprintf(aux, "does not exist in 0%d ", message->equipment + 1);
            strcat(response, aux);
        }
        else
        {
            for (int i = 0; i < message->list_sensors_length; i++)
            {
                if (!dont_exists_vector[i])
                {
                    char aux[10];
                    sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                    strcat(response, aux);
                }
            }
            strcat(response, "removed");

            if (number_dont_exist > 0)
            {
                strcat(response, " ");

                for (int i = 0; i < message->list_sensors_length; i++)
                {
                    if (dont_exists_vector[i])
                    {
                        char aux[10];
                        sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                        strcat(response, aux);
                    }
                }

                char aux[30];
                sprintf(aux, "already exists in 0%d ", message->equipment + 1);
                strcat(response, aux);
            }
        }

        break;
    }
    case 2: // list
    {
        int number_dont_list = 0;
        for (int i = 0; i < 4; i++)
        {
            if (storage->equipments[message->equipment][i])
            {
                char aux[10];
                sprintf(aux, "0%d ", i + 1);
                strcat(response, aux);
            }
            else
            {
                number_dont_list++;
            }
        }

        if (number_dont_list == 4)
        {
            strcat(response, "none");
        }
        else
        {
            response[strlen(response) - 1] = '\0';
        }
        break;
    }
    case 3:
    {
        int number_dont_exit = 0;
        int dont_exists_vector[4] = {0, 0, 0, 0};
        for (int i = 0; i < message->list_sensors_length; i++)
        {
            if (!storage->equipments[message->equipment][message->lis_sensors[i]])
            {
                dont_exists_vector[i] = 1;
                number_dont_exit++;
            }
        }

        if (number_dont_exit == message->list_sensors_length)
        {
            strcat(response, "sensor(s) ");
            for (int i = 0; i < message->list_sensors_length; i++)
            {
                char aux[10];
                sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                strcat(response, aux);
            }
            strcat(response, "not installed");
        }
        else
        {
            for (int i = 0; i < message->list_sensors_length; i++)
            {
                if (!dont_exists_vector[i])
                {
                    char aux[10];
                    srand((unsigned int)time(NULL));
                    sprintf(aux, "%.2f ", ((float)rand() / (float)(RAND_MAX)) * 10.);
                    strcat(response, aux);
                }
            }

            if (number_dont_exit > 0)
            {
                strcat(response, "and ");
                for (int i = 0; i < message->list_sensors_length; i++)
                {
                    if (dont_exists_vector[i])
                    {
                        char aux[10];
                        sprintf(aux, "0%d ", message->lis_sensors[i] + 1);
                        strcat(response, aux);
                    }
                }
                strcat(response, "not installed");
            }
            else
            {
                response[strlen(response) - 1] = '\0';
            }
        }
        break;
    }
    default:
        break;
    }
    return response;
}