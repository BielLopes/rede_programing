/*
    A duas estuturas de dados utilizadas:

    * message_type: respresenta as informações necessárias para qualquer menssagen que é entregue para o servidor.    
    * storage_communication: o formato mais simples possível de se armazenar o estado de cada um dos 16 sensores possíveis e controlar a quantidade de sensores. (mesmo que existam 16 sensores distintos, apenas 15 podem ser instanciados)
    
    obs: mais informações na DOC
*/
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

int validate_and_create_message(char *buf, struct message_type *message);

char *update_bd_and_create_response(struct message_type *message, struct storage_communication *storage);