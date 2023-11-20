/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

----------------------------------------------------------------------------------------

Adicionámos ao sdmessage.proto, dentro da MessageT, um novo opcode:
    OP_STATS = 70;
alterámos o seguite opcode:
    OP_ERROR	= 99;
e os seguintes c_typeS:
    CT_NONE	= 70;
	CT_STATS = 80;

Por último, definimos uma mensagem StatisticsT da seguinte forma:
    message statistics_t  /* Formato da mensagem StatisticsT */
        {
            int32 num_operations = 1;
            int32 time = 2;
            int32 num_clients = 3;
    }
-------------------------------------------------------------------------------------------

No módulo client_stub definimos uma função adicional que 
obtia as estatísticas do servidor.

------------------------------------------------------------------------------------------

No módulo network_server definimos duas variáveis globais de dados partilhados pelas threads:
    struct table_t *global_table;
    struct statistics_t *stats;

Definimos uma função, handle_client, a ser executada nas threads, que serve de
thread secundária de atendimento dos clientes.
Dentro da função network_main_loop, inicializámos a nossa variável global stats.

------------------------------------------------------------------------------------------

Criámos um novo módulo mutex, no qual definimos diversas funções que tratam da gestão de concorrência
das threads aos dados partilhados, que referimos anteriormente.

-------------------------------------------------------------------------------------------

No módulo table_skel, dentro da função invoke acrescentámos
a nova operação stats. Identificámos as operações críticas, que não devem ser interrompidas, 
e gerimos a concorrência nos acessos aos dados partilhados, através das funções implementadas no
módulo mutex.
Definimos, por último, uma nova função get_time_micros()
que permite obter o tempo atual em microsegundos.

------------------------------------------------------------------------------------------

Criámos um novo módulo stats, em que definimos uma nova estrutura:
    struct statistics_t {
	    int num_operations;
        unsigned long time; 
        int num_clients; 
    }
e funções para criar e destruir esta estrutura.

----------------------------------------------------------------------------------------

Incluímos o tratamento da operação stats no código do cliente (table_client.c)

--------------------------------------------------------------------------------------

Os restantes módulos mantiveram-se iguais.





