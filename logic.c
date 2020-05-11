#include "logic.h"

char* get_state_name(logic_state_t i)
{
    static char *names[] = {
        "\"idle\"",
        "\"triggred\"",
        "\"countdown_3\"",
        "\"countdown_2\"",
        "\"countdown_1\"",
        "\"capture\"",
        "\"preview\"",
        "\"procces\"",
        "\"print\""};
    if(i >= 0 && i < sizeof(names)/sizeof(char*)){
        return names[i];
    }
    return "UNDEFINED";
}

void run_logic(shared_memory_t *shared_memory)
{
    static logic_state_t prev_logic_state = -1;
    if(prev_logic_state != shared_memory->logic_state){
        prev_logic_state = shared_memory->logic_state;
        printf("%sEntered %s state!\n",PL,get_state_name(prev_logic_state));
    }
}