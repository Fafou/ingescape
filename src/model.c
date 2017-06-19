//
//  mtic_model.c
//
//  Created by Patxi Berard
//  Modified by Vincent Deliencourt
//  Copyright © 2016 IKKY WP4.8. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
//#include "mastic.h"
#include "mastic_private.h"


//Define the pointer on the callback function
typedef void (*calback_ptr_t)( agent_iop* );

//Hashable structure which associate the name of one 'iop' and the pointer of one callback
typedef struct callbacks {
    const char * iop_name;          //Need to be unique : the table hash key
    calback_ptr_t callback_ptr;   //pointer on the callback
    UT_hash_handle hh;
} callbacks;

//The variable which will contain all the callbacks associated to 'iop'
callbacks *agent_callbacks;

// ------------ main functions use in 'MODEL' --------------- //

agent_iop * mtic_find_iop_by_name_on_definition(const char *name, definition* definition, model_state *code){
    agent_iop *found = NULL;
    
    if(name != NULL && definition != NULL){
        //find the input agent_iop
        HASH_FIND_STR( definition->inputs_table, name, found );
        if(found == NULL){
            // look in the outputs
            HASH_FIND_STR( definition->outputs_table, name, found );
        }else
        {
            *code = TYPE_INPUT;
            return found;
        }
        
        if(found == NULL){
            //find the parameters agent_iop
            HASH_FIND_STR( definition->params_table, name, found );
        }else{
            *code = TYPE_OUTPUT;
            return found;
        }
        
        if(found == NULL){
            fprintf(stderr, "ERROR : The name of the iop {%s} doesn't exist. \n",name);
            *code = NOK;
            return found;
        }else
        {
            *code = TYPE_PARAM;
            return found;
        }
    }else{
        fprintf(stderr, "ERROR : The name of the IOP is empty. \n");
        *code = NAME_EMPTY;
        return found;
    }
}

/*
 * Function: mtic_find_iop_by_name
 * ----------------------------
 *
 */
agent_iop * model_findIopByName(const char *name, model_state *code){
    
    return mtic_find_iop_by_name_on_definition(name,mtic_definition_live,code);
}


void update_value(agent_iop *iop, void* value){
    switch (iop->type) {
        case INTEGER_T:
            iop->old_value.i = iop->value.i;
            iop->value.i = *(int*)(value);
            break;
        case DOUBLE_T:
            iop->old_value.d = iop->value.d;
            iop->value.d = *(double*)(value);
            break;
        case BOOL_T:
            iop->old_value.b = iop->value.b;
            iop->value.b = *(bool*)(value);
            break;
        case STRING_T:
            free(iop->old_value.s);
            iop->old_value.s = strdup(iop->value.s);
            free(iop->value.s);
            iop->value.s = strdup(value);
            break;
        case IMPULSION_T:
            free(iop->old_value.impuls);
            iop->old_value.impuls = strdup(iop->value.impuls);
            free(iop->value.impuls);
            iop->value.impuls = strdup(value);
        break;
        case DATA_T:
            free(iop->old_value.data);
            iop->old_value.data = strdup(iop->value.data);
            free(iop->value.data);
            iop->value.data = strdup(value);
        break;
        default:
            break;
    }
}

/*
 * Function: observe :
 * ----------------------------
 *  Observe the iop and associate a callback to it.
 *  When the iop value will change by calling the 'mtic_set' function, the associated callback will be called.
 *
 *  name_iop        : Name of the 'input/output/parameter'
 *
 *  callback_fct    : The pointer to the associated callback
 *
 *  return : model_state (cf. enum) return error is occur
 */
model_state mtic_observe(const char *iop_name,
             void (*callback_fct)(agent_iop *input_iop)){

    //1) find the iop
    model_state code;
    agent_iop *iop = model_findIopByName((char*)iop_name,&code);

    if(iop == NULL)
        return code;

    //2) Associate the input name with the callback which it will be called in SET
    if(callback_fct != NULL){
        callbacks *new_callback = malloc(sizeof(callbacks));
        new_callback->iop_name = strdup(iop_name);  //assign the key
        new_callback->callback_ptr = callback_fct;
        HASH_ADD_STR( agent_callbacks, iop_name,  new_callback);
    }else
        return NO_CALLBACK;

    return OK;
}

/*
 * Function: mtic_get
 * ----------------------------
 *  Find the 'input/output/parameter' value.
 *
 *  name_iop    : Name of the 'input/output/parameter'
 *
 *  state       : The pointer on the state (cf. enum)
 *
 *  return : pointer on the value OR null
 */
void * mtic_get(const char *name_iop, model_state *state){

    agent_iop *iop = model_findIopByName((char*) name_iop, state);

    if(iop == NULL)
        return state;

    //Return the value corresponding
    switch (iop->type) {
        case INTEGER_T:
            return &iop->value.i;
            break;
        case DOUBLE_T:
            return &iop->value.d;
            break;
        case BOOL_T:
            return &iop->value.b;
            break;
        case STRING_T:
            return iop->value.s;
            break;
        case IMPULSION_T:
            return iop->value.impuls;
            break;
        case DATA_T:
            return iop->value.data;
            break;
        default:
            break;
    }

    return NULL;
}

/*
 * Function: mtic_set
 * ----------------------------
 *  Set the new value to the associated iop.
 *
 *  name_iop    : Name of the 'input/output/parameter'
 *
 *  new_value   : The new value
 *
 *  return : model_state (cf. enum) return error is occur
 */
model_state mtic_set(const char *iop_name, void *new_value){

    //1) find the iop
    model_state code;
    agent_iop *iop = model_findIopByName((char*) iop_name,&code);

    if(iop == NULL)
        return code;

    //2) update the value in the iop_live structure
    update_value(iop, new_value);
    
    // Let us know the value has changed
    char* str_value = mtic_iop_value_to_string(iop);
    mtic_debug("SET(%s,%s).\n",iop_name,str_value);
    free(str_value);

    //3) Callback associated from 'observe' function
    //Check ma liste de callback et appellé celle concerné
    callbacks *fct_to_call;
    HASH_FIND_STR(agent_callbacks,iop_name,fct_to_call);
    if(fct_to_call != NULL)
    {
        fct_to_call->callback_ptr(iop);
    }

    /*
     * If the iop is an output  : publish
     */
    if(code == TYPE_OUTPUT){
        
        // Publish the new value
        network_publishOutput(iop_name);
    }

    return OK;
}

/*
 * Function: mute
 * ----------------------------
 *  Mute an 'input/output/parameter'.
 *
 *  name_iop    : Name of the 'input/output/parameter'
 *
 *  return : 0 is the output has been properly muted, -1 otherwise.
 */

int mtic_mute_internal(const char* iop_name)
{
    int result = -1;
    model_state code;
    // mtic_get iop object
    agent_iop *iop = model_findIopByName((char*)iop_name,&code);
    
    if(iop != NULL)
    {
        // update the is_muted state
        iop->is_muted = true;
        result = 0;
    }
    
    return result;
}

/*
 * Function: unmute
 * ----------------------------
 *  Unmute an 'input/output/parameter'.
 *
 *  name_iop    : Name of the 'input/output/parameter'
 *
 *  return : 0 is the output has been properly unmuted, -1 otherwise.
 */
int mtic_unmute_internal(const char* iop_name)
{
    int result = -1;
    model_state code;
    // mtic_get iop object
    agent_iop *iop = model_findIopByName((char*)iop_name,&code);
    
    if(iop != NULL)
    {
        // update the is_muted state
        iop->is_muted = false;
        result = 0;
    }
    
    return result;
}

/*
 * Function: mtic_muteAll
 * ----------------------------
 *  Mute all outputs.
 *
 *  return : 0 is the outputs have been properly muted, -1 otherwise.
 */
int mtic_muteAll()
{
    int result = 0,result_tmp = 0;

    // Go through the agent outpust to mute them
    struct agent_iop *current_iop, *tmp_iop;
    HASH_ITER(hh, mtic_definition_live->outputs_table, current_iop, tmp_iop) {
        if(current_iop != NULL)
        {
            result_tmp = mtic_mute_internal(current_iop->name);
            // If one one the output has not been muted, we notice it
            if(result_tmp != 0)
            {
                result = result_tmp;
            }
        }
    }
    
    return result;
}

/*
 * Function: mtic_unmuteAll
 * ----------------------------
 *  Unmute all outputs.
 *
 *  return : 0 is the outputs have been properly unmuted, -1 otherwise.
 */
int mtic_unmuteAll()
{
    int result = 0,result_tmp = 0;
    
    // Go through the agent outpust to mute them
    struct agent_iop *current_iop, *tmp_iop;
    HASH_ITER(hh, mtic_definition_live->outputs_table, current_iop, tmp_iop) {
        if(current_iop != NULL)
        {
            result_tmp = mtic_unmute_internal(current_iop->name);
            // If one one the output has not been unmuted, we notice it
            if(result_tmp != 0)
            {
                result = result_tmp;
            }
        }
    }
    
    return result;
}

////////////////////////////////////////////////////////////////////////
// PRIVATE API
////////////////////////////////////////////////////////////////////////
//à remplir ou déplacer ici

/*
 * Function: model_model_IntToString
 * ----------------------------
 *  Carry out conversion from int to string.
 *
 *  return : the int value as a string.
 */
char* model_IntToString(const int value)
{
    // Compute the size of allocate for str.
   int length = snprintf( NULL, 0, "%d", value);

   if(length == 0)
   {
       mtic_debug("value double is NULL. Its length is zero.");
       return NULL;
   }

   // Allocate the memory.
   char* str = malloc(length+1);
   // Write the value into str.
   snprintf( str, length + 1, "%d", value);

   return str;
}

/*
 * Function: model_DoubleToString
 * ----------------------------
 *  Carry out conversion from double to string.
 *
 *  return : the double value as a string.
 */
char* model_DoubleToString(const double value)
{
    // Compute the size of allocate for str.
    int length = snprintf( NULL, 0, "%fl", value);

    if(length == 0)
    {
        mtic_debug("value double is NULL. Its length is zero.");
        return NULL;
    }
    // Allocate the memory.
    char* str = malloc(length+1);
    // Write the value into str.
    snprintf( str, length + 1, "%lf", value);

    return str;
}

////////////////////////////////////////////////////////////////////////
// PUBLIC API
////////////////////////////////////////////////////////////////////////

//read/write IOP using void*
//generic typeless functions (requires developer to check IOP type for type casting)
//for IMPULSION_T value is always 0
//size is passed by Mastic based on type (for bool, double, int and string) or metadata (for data)
void mtic_readInput(const char *name, void *value, long *size){
    
}
void mtic_readOutput(const char *name, void *value, long *size){
    
}
void mtic_readParameter(const char *name, void *value, long *size){
    
}

//read per type
//implicit conversions are possible. Some need to raise warnings.
//we need to make things clear on structures
//for IMPULSION_T value is always 0
//for DATA_T, size is passed by Mastic

// Read Inputs ...
/**
 * \fn mtic_readInputAsBool(char *name)
 * \brief Find the Agent's input by name and return the input value as a Boolean.
 * \param Take the name of the input to read as it has been defined in the definition.
 * \return Return the input value as true or false.
 */
bool mtic_readInputAsBool(const char *name){

    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the input has been returned.
    if(iop != NULL){
        switch(iop->type){
            case BOOL_T:
                return iop->value.b;
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to bool.
                mtic_debug("mtic_readInputAsBool : Implicit conversion from int to bool for the input {%s}.", name);
                return (bool) iop->value.i;
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to bool.
                mtic_debug("mtic_readInputAsBool : Implicit conversion from double to bool for the input {%s}.", name);
                return (bool) iop->value.d;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to bool.
                // Test first if the input value is {true}, then {false}. Finally if the two previous test fail the function returns {false}.
                if (!stricmp(iop->value.s, "true")){
                    mtic_debug("mtic_readInputAsBool : Implicit conversion from string to bool for the input {%s}.", name);
                    return true;
                }
                else if (!stricmp(iop->value.s, "false")){
                    mtic_debug("mtic_readInputAsBool : Implicit conversion from string to bool for the input {%s}.", name);
                    return false;
                }
                else{
                    mtic_debug("mtic_readInputAsBool : Implicit conversion fails to attempt for the input {%s}. String value is different from {true , false}.", name);
                    return false;
                }
                break;

            default:
                //Handle the case: the input cannot be handled.
                mtic_debug("mtic_readInputAsBool : Agent's input {%s} cannot be returned as a Boolean! By default {false} is returned.", name);
                return false;
                break;
        }
    }
    else{
        //Handle the case: the input is not found.
        mtic_debug("mtic_readInputAsBool : Any Agent's input {%s} cannot be found. By default {false} is returned.", name);
        return false;
    }
}

/**
 * \fn mtic_readInputAsInt(char *name)
 * \brief Find the Agent's input by name and return the input value as an integer.
 * \param Take the name of the input to read as it has been defined in the definition.
 * \return Return the input value as an integer.
 */
int mtic_readInputAsInt(const char *name){

    //Get the pointer IOP Agent selected by name
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the input has been returned.
    if(iop != NULL){
        switch(iop->type){
            case INTEGER_T:
                return iop->value.i;
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to int.
                mtic_debug("mtic_readInputAsInt : Implicit conversion from bool to int for the input {%s}.", name);
                return (int) iop->value.b;
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to int.
                mtic_debug("mtic_readInputAsInt : Implicit conversion from double to int for the input {%s}.", name);
                return (int) iop->value.d;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to int.
                //Try to convert to int (1 expected value)
                int value = 0;
                int test = sscanf(iop->value.s, "%d", &value);

                // Check if one value is returned.
                if(test == 1){
                    mtic_debug("mtic_readInputAsInt : Implicit conversion from string to int for the input {%s}.", name);
                    return value;
                }
                else{
                    mtic_debug("mtic_readInputAsInt : Implicit conversion fails to attempt for the input {%s}. String value is different from an integer.", name);
                    return value;
                }
                break;

            default:
                //Handle the case: the input cannot be handled.
                mtic_debug("mtic_readInputAsInt : Agent's input {%s} cannot be returned as an integer! By default {0} is returned.", name);
                return 0;
                break;
        }
    }
    else{
        //Handle the case: the input is not found.
        mtic_debug("mtic_readInputAsInt : Any Agent's {%s} input cannot be found. By default {0} is returned.", name);
        return 0;
    }
}

/**
 * \fn mtic_readInputAsDouble(char *name)
 * \brief Find the Agent's input by name and return the input value as a double.
 * \param Take name of the input to read as it has been defined in the definition.
 * \return Return the input value as double.
 */
double mtic_readInputAsDouble(const char *name){

     //Get the pointer IOP Agent selected by name
     model_state state;
     agent_iop *iop = model_findIopByName((char*) name, &state);

     // Check if the input has been returned.
     if(iop != NULL){
         switch(iop->type){
             case DOUBLE_T:
                 return iop->value.d;
                 break;

             case BOOL_T:
                 //Handle the case: An implicit conversion can be done from bool to double.
                 mtic_debug("mtic_readInputAsDouble : Implicit conversion from bool to double for the input {%s}.", name);
                 return (double) iop->value.b;
                 break;

             case INTEGER_T:
                 //Handle the case: An implicit conversion can be done from int to double.
                 mtic_debug("mtic_readInputAsDouble : Implicit conversion from int to double for the input {%s}.", name);
                 return (double) iop->value.i;
                 break;

             case STRING_T:
                 //Handle the case: An implicit conversion can be done from string to double.
                 //Try to convert to double (1 expected value)
                 double value = 0.0;
                 int test = sscanf(iop->value.s, "%lf", &value);

                 // Check if one value is returned.
                 if(test == 1){
                     mtic_debug("mtic_readInputAsDouble : Implicit conversion from string to double for the input {%s}.", name);
                     return value;
                 }
                 else{
                     mtic_debug("mtic_readInputAsDouble : Implicit conversion fails to attempt for the input {%s}. String value is different from a double.", name);
                     return value;
                 }
                 break;

             default:
                 //Handle the case: the input cannot be handled.
                 mtic_debug("mtic_readInputAsDouble : Agent's input {%s} cannot be returned as a double! By default {0.0} is returned.", name);
                 return 0.0;
                 break;
         }
     }
     else{
         //Handle the case: the input is not found.
         mtic_debug("mtic_readInputAsDouble : Agent's input {%s} cannot be found. By default {0.0} is returned.", name);
         return 0.0;
     }
}

/**
 * \fn mtic_readInputAsString(char *name)
 * \brief Find the Agent's input by name and return the input value as a string.
 *        WARNING: Allocating memory that must be free after use.
 * \param Take name of the input to read as it has been defined in the definition.
 * \return Return the input value as a string.
 */
char* mtic_readInputAsString(const char *name){
    //Get the pointer IOP Agent selected by name
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the input has been returned.
    if(iop != NULL){
        switch(iop->type){
            case STRING_T:
                return strdup(iop->value.s);
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to string.
                mtic_debug("mtic_readInputAsString : Implicit conversion from bool to string for the input {%s}.", name);
                return iop->value.b ? strdup("true") : strdup("false");
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to string.
                mtic_debug("mtic_readInputAsString : Implicit conversion from int to string for the input {%s}.", name);
                return model_IntToString(iop->value.i);
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to string.
                mtic_debug("mtic_readInputAsString : Implicit conversion from double to string for the input {%s}.", name);
                return model_DoubleToString(iop->value.d);
                break;

            default:
                //Handle the case: the input cannot be handled.
                mtic_debug("mtic_readInputAsDouble : Agent's input {%s} cannot be return as a string! By default {NULL} is return.", name);
                return NULL;
                break;
        }
    }
    else{
        //Handle the case: the input is not found.
        mtic_debug("mtic_readInputAsString : Agent's input {%s} cannot be found. By default {NULL} is returned.", name);
        return NULL;
    }
}

void mtic_readInputAsData(const char *name, void *data, long *size){ //allocs data structure to be disposed by caller
    
}

// Read Outputs ...
/**
 * \fn mtic_readOutputAsBool(const char *name)
 * \brief Find the Agent's output by name and return the output value as a Boolean.
 * \param Take the name of the output to read as it has been defined in the definition.
 * \return Return the output value as true or false.
 */
bool mtic_readOutputAsBool(const char *name){
    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the output has been returned.
    if(iop != NULL){
        switch(iop->type){
            case BOOL_T:
                return iop->value.b;
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to bool.
                mtic_debug("mtic_readOutputAsBool : Implicit conversion from int to bool for the output {%s}.", name);
                return (bool) iop->value.i;
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to bool.
                mtic_debug("mtic_readOutputAsBool : Implicit conversion from double to bool for the output {%s}.", name);
                return (bool) iop->value.d;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to bool.
                // Test first if the output value is {true}, then {false}. Finally if the two previous test fail the function returns {false}.
                if (!stricmp(iop->value.s, "true")){
                    mtic_debug("mtic_readOutputAsBool : Implicit conversion from string to bool for the output {%s}.", name);
                    return true;
                }
                else if (!stricmp(iop->value.s, "false")){
                    mtic_debug("mtic_readOutputAsBool : Implicit conversion from string to bool for the output {%s}.", name);
                    return false;
                }
                else{
                    mtic_debug("mtic_readOutputAsBool : Implicit conversion fails to attempt for the output {%s}. String value is different from {true , false}.", name);
                    return false;
                }
                break;

            default:
                //Handle the case: the output cannot be handled.
                mtic_debug("mtic_readOutputAsBool : Agent's output {%s} cannot be returned as a Boolean! By default {false} is returned.", name);
                return false;
                break;
        }
    }
    else{
        //Handle the case: the output is not found.
        mtic_debug("mtic_readOutputAsBool : Any Agent's output {%s} has been returned", name);
        return false;
    }
}

/**
 * \fn mtic_readOutputAsInt(const char *name)
 * \brief Find the Agent's output by name and return the output value as an integer.
 * \param Take the name of the output to read as it has been defined in the definition.
 * \return Return the output value as an integer.
 */
int mtic_readOutputAsInt(const char *name){
    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the output has been returned.
    if(iop != NULL){
        switch(iop->type){
            case INTEGER_T:
                return iop->value.i;
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to int.
                mtic_debug("mtic_readOutputAsInt : Implicit conversion from bool to int for the output {%s}.", name);
                return (int) iop->value.b;
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to int.
                mtic_debug("mtic_readOutputAsInt : Implicit conversion from double to int for the output {%s}.", name);
                return (int) iop->value.d;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to int.
                //Try to convert to int (1 expected value)
                int value = 0;
                int test = sscanf(iop->value.s, "%d", &value);

                // Check if one value is returned.
                if(test == 1){
                    mtic_debug("mtic_readOutputAsInt : Implicit conversion from string to int for the output {%s}.", name);
                    return value;
                }
                else{
                    mtic_debug("mtic_readOutputAsInt : Implicit conversion fails to attempt for the output {%s}. String value is different from an integer.", name);
                    return value;
                }
                break;

            default:
                //Handle the case: the output cannot be handled.
                mtic_debug("mtic_readOutputAsInt : Agent's output {%s} cannot be returned as an integer! By default {0} is returned.", name);
                return 0;
                break;
        }
    }
    else{
        //Handle the case: the output is not found.
        mtic_debug("mtic_readOutputAsInt : Any Agent's output {%s} has been returned", name);
        return 0;
    }
}

/**
 * \fn mtic_readOutputAsDouble(const char *name)
 * \brief Find the Agent's output by name and return the output value as a double.
 * \param Take the name of the output to read as it has been defined in the definition.
 * \return Return the output value as a double.
 */
double mtic_readOutputAsDouble(const char *name){
    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the output has been returned.
    if(iop != NULL){
        switch(iop->type){
            case DOUBLE_T:
                return iop->value.d;
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to double.
                mtic_debug("mtic_readOutputAsDouble : Implicit conversion from bool to double for the output {%s}.", name);
                return (double) iop->value.b;
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to double.
                mtic_debug("mtic_readOutputAsDouble : Implicit conversion from int to double for the output {%s}.", name);
                return (double) iop->value.i;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to int.

                //Try to convert to double (1 expected value)
                double value = 0.0;
                int test = sscanf(iop->value.s, "%lf", &value);

                // Check if one value is returned.
                if(test == 1){
                    mtic_debug("mtic_readOutputAsDouble : Implicit conversion from string to double for the output {%s}.", name);
                    return value;
                }
                else{
                    mtic_debug("mtic_readOutputAsDouble : Implicit conversion fails to attempt for the output {%s}. String value is different from a double.", name);
                    return value;
                }
                break;

            default:
                //Handle the case: the output cannot be handled.
                mtic_debug("mtic_readOutputAsDouble : Agent's output {%s} cannot be returned as a double! By default {0.0} is returned.", name);
                return 0.0;
                break;
        }
    }
    else{
        //Handle the case: the output is not found.
        mtic_debug("mtic_readOutputAsDouble : Any Agent's output {%s} has been returned", name);
        return 0.0;
    }
}

/**
 * \fn mtic_readOutputAsString(const char *name)
 * \brief Find the Agent's output by name and return the output value as a string.
 *        WARNING: Allocating memory that must be free after use.
 * \param Take the name of the output to read as it has been defined in the definition.
 * \return Return the output value as a string.
 */
char* mtic_readOutputAsString(const char *name){
    //Get the pointer IOP Agent selected by name
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the output has been returned.
    if(iop != NULL){
        switch(iop->type){
            case STRING_T:
                return strdup(iop->value.s);
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to string.
                mtic_debug("mtic_readOutputAsString : Implicit conversion from bool to string for the output {%s}.", name);
                return iop->value.b ? strdup("true") : strdup("false");
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to string.
                mtic_debug("mtic_readOutputAsString : Implicit conversion from int to string for the output {%s}.", name);
                return model_IntToString(iop->value.i);
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to string.
                mtic_debug("mtic_readOutputAsString : Implicit conversion from double to string for the output {%s}.", name);
                return model_DoubleToString(iop->value.d);
                break;

            default:
                //Handle the case: the output cannot be handled.
                mtic_debug("mtic_readOutputAsString : Agent's output {%s} cannot be returned as a string! By default {NULL} is return.", name);
                return NULL;
                break;
        }
    }
    else{
        //Handle the case: the output is not found.
        mtic_debug("mtic_readOutputAsString : Agent's output {%s} cannot be found", name);
        return NULL;
    }
}

void mtic_readOutputAsData(const char *name, void *data, long *size){ //allocs data structure to be disposed by caller
    
}

// Read Parameters ...
/**
 * \fn mtic_readParameterAsBool(const char *name)
 * \brief Find the Agent's parameter by name and return the output value as a Boolean.
 * \param Take the name of the parameter to read as it has been defined in the definition.
 * \return Return the parameter value as true or false.
 */
bool mtic_readParameterAsBool(const char *name){
    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the parameter has been returned.
    if(iop != NULL){
        switch(iop->type){
            case BOOL_T:
                return iop->value.b;
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to bool.
                mtic_debug("mtic_readParameterAsBool : Implicit conversion from int to bool for the output {%s}.", name);
                return (bool) iop->value.i;
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to bool.
                mtic_debug("mtic_readParameterAsBool : Implicit conversion from double to bool for the parameter {%s}.", name);
                return (bool) iop->value.d;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to bool.
                // Test first if the output value is {true}, then {false}. Finally if the two previous test fail the function returns {false}.
                if (!stricmp(iop->value.s, "true")){
                    mtic_debug("mtic_readParameterAsBool : Implicit conversion from string to bool for the parameter {%s}.", name);
                    return true;
                }
                else if (!stricmp(iop->value.s, "false")){
                    mtic_debug("mtic_readParameterAsBool : Implicit conversion from string to bool for the parameter {%s}.", name);
                    return false;
                }
                else{
                    mtic_debug("mtic_readParameterAsBool : Implicit conversion fails to attempt for the parameter {%s}. String value is different from {true , false}.", name);
                    return false;
                }
                break;

            default:
                //Handle the case: the parameter cannot be handled.
                mtic_debug("mtic_readParameterAsBool : Agent's parameter {%s} cannot be returned as a Boolean! By default {false} is returned.", name);
                return false;
                break;
        }
    }
    else{
        //Handle the case: the parameter is not found.
        mtic_debug("mtic_readParameterAsBool : Any Agent's parameter {%s} has been returned", name);
        return false;
    }
}

/**
 * \fn mtic_readParameterAsInt(const char *name)
 * \brief Find the Agent's parameter by name and return the parameter value as an integer.
 * \param Take the name of the parameter to read as it has been defined in the definition.
 * \return Return the parameter value as an integer.
 */
int mtic_readParameterAsInt(const char *name){
    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the parameter has been returned.
    if(iop != NULL){
        switch(iop->type){
            case INTEGER_T:
                return iop->value.i;
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to int.
                mtic_debug("mtic_readParameterAsInt : Implicit conversion from bool to int for the parameter {%s}.", name);
                return (int) iop->value.b;
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to int.
                mtic_debug("mtic_readParameterAsInt : Implicit conversion from double to int for the parameter {%s}.", name);
                return (int) iop->value.d;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to int.
                //Try to convert to int (1 expected value)
                int value = 0;
                int test = sscanf(iop->value.s, "%d", &value);

                // Check if one value is returned.
                if(test == 1){
                    mtic_debug("mtic_readParameterAsInt : Implicit conversion from string to int for the parameter {%s}.", name);
                    return value;
                }
                else{
                    mtic_debug("mtic_readParameterAsInt : Implicit conversion fails to attempt for the parameter {%s}. String value is different from an integer.", name);
                    return value;
                }
                break;

            default:
                //Handle the case: the parameter cannot be handled.
                mtic_debug("mtic_readParameterAsInt : Agent's parameter {%s} cannot be returned as an integer! By default {0} is returned.", name);
                return 0;
                break;
        }
    }
    else{
        //Handle the case: the parameter is not found.
        mtic_debug("mtic_readParameterAsInt : Any Agent's parameter {%s} has been returned", name);
        return 0;
    }
}

/**
 * \fn mtic_readParameterAsDouble(const char *name)
 * \brief Find the Agent's parameter by name and return the output value as a double.
 * \param Take the name of the output to read as it has been defined in the definition.
 * \return Return the parameter value as a double.
 */
double mtic_readParameterAsDouble(const char *name){
    //Get the pointer IOP Agent selected by name.
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the parameter has been returned.
    if(iop != NULL){
        switch(iop->type){
            case DOUBLE_T:
                return iop->value.d;
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to double.
                mtic_debug("mtic_readParameterAsDouble : Implicit conversion from bool to double for the parameter {%s}.", name);
                return (double) iop->value.b;
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to double.
                mtic_debug("mtic_readParameterAsDouble : Implicit conversion from int to double for the parameter {%s}.", name);
                return (double) iop->value.i;
                break;

            case STRING_T:
                //Handle the case: An implicit conversion can be done from string to int.
                //Try to convert to double (1 expected value)
                double value = 0.0;
                int test = sscanf(iop->value.s, "%lf", &value);

                // Check if one value is returned.
                if(test == 1){
                    mtic_debug("mtic_readParameterAsDouble : Implicit conversion from string to double for the parameter {%s}.", name);
                    return value;
                }
                else{
                    mtic_debug("mtic_readParameterAsDouble : Implicit conversion fails to attempt for the parameter {%s}. String value is different from a double.", name);
                    return value;
                }
                break;

            default:
                //Handle the case: the parameter cannot be handled.
                mtic_debug("mtic_readParameterAsDouble : Agent's parameter {%s} cannot be returned as a double! By default {0.0} is returned.", name);
                return 0.0;
                break;
        }
    }
    else{
        //Handle the case: the parameter is not found.
        mtic_debug("mtic_readParameterAsDouble : Any Agent's parameter {%s} has been returned", name);
        return 0.0;
    }
}

/**
 * \fn mtic_readParameterAsString(const char *name)
 * \brief Find the Agent's parameter by name and return the output value as a string.
 *        WARNING: Allocating memory that must be free after use.
 * \param Take the name of the parameter to read as it has been defined in the definition.
 * \return Return the parameter value as a string.
 */
char* mtic_readParameterAsString(const char *name){
    //Get the pointer IOP Agent selected by name
    model_state state;
    agent_iop *iop = model_findIopByName((char*) name, &state);

    // Check if the parameter has been returned.
    if(iop != NULL){
        switch(iop->type){
            case STRING_T:
                return strdup(iop->value.s);
                break;

            case BOOL_T:
                //Handle the case: An implicit conversion can be done from bool to string.
                mtic_debug("mtic_readParameterAsString : Implicit conversion from bool to string for the parameter {%s}.", name);
                return iop->value.b ? strdup("true") : strdup("false");
                break;

            case INTEGER_T:
                //Handle the case: An implicit conversion can be done from int to string.
                mtic_debug("mtic_readParameterAsString : Implicit conversion from int to string for the parameter {%s}.", name);
                return model_IntToString(iop->value.i);
                break;

            case DOUBLE_T:
                //Handle the case: An implicit conversion can be done from double to string.
                mtic_debug("mtic_readParameterAsString : Implicit conversion from double to string for the parameter {%s}.", name);
                return model_DoubleToString(iop->value.d);
                break;

            default:
                //Handle the case: the parameter cannot be handled.
                mtic_debug("mtic_readParameterAsString : Agent's parameter {%s} cannot be returned as a string! By default {NULL} is return.", name);;
                return NULL;
                break;
        }
    }
    else{
        //Handle the case: the input is not found.
        mtic_debug("mtic_readParameterAsString : Agent's parameter {%s} cannot be found", name);
        return NULL;
    }
}

void mtic_readParameterAsData(const char *name, void *data, long *size){ //allocs data structure to be disposed by caller
    
}

//write using void*
//for IMPULSION_T value is just ignored
//for DATA_T, these functions should be forbidden (need to know data size)
//size shall be given to Mastic
//Mastic shall clone value and shall dispose of it when stopped
int mtic_writeInput(const char *name, void *value, long size){
    return 1;
}
int mtic_writeOutput(const char *name, void *value, long size){
    return 1;
}
int mtic_writeParameter(const char *name, void *value, long size){
    return 1;
}

//write using internal conversions (Mastic does the conversion job)
//we need to make things clear on structures
//for IMPULSION_T value is just ignored
//Mastic shall clone value and shall dispose of it when stopped
int mtic_writeInputAsBool(const char *name, bool value){
    return 1;
}
int mtic_writeInputAsInt(const char *name, int value){
    return 1;
}
int mtic_writeInputAsDouble(const char *name, double value){
    return 1;
}
int mtic_writeInputAsString(const char *name, char *value){
    return 1;
}
int mtic_writeInputAsData(const char *name, void *value, long size){
    return 1;
}

int mtic_writeOutputAsBool(const char *name, bool value){
    return 1;
}
int mtic_writeOutputAsInt(const char *name, int value){
    return 1;
}
int mtic_writeOutputAsDouble(const char *name, double value){
    return 1;
}
int mtic_writeOutputAsString(const char *name, char *value){
    return 1;
}
int mtic_writeOutputAsData(const char *name, void *value, long size){
    return 1;
}

int mtic_writeParameterAsBool(const char *name, bool value){
    return 1;
}
int mtic_writeParameterAsInt(const char *name, int value){
    return 1;
}
int mtic_writeParameterAsDouble(const char *name, double value){
    return 1;
}
int mtic_writeParameterAsString(const char *name, char *value){
    return 1;
}
int mtic_writeParameterAsData(const char *name, void *value, long size){
    return 1;
}


//check IOP type, lists and existence

/**
 * \fn iopType_t mtic_getTypeForInput(const char *name)
 * \brief this function returns the value type of the input (integer, bool, etc .)
 *
 * \param name The string which contains the name of the input. Can't be NULL or empty.
 * \return The Input value type.
 * 0 The parameter name cannot be NULL or empty.
 * -1 The definition live is NULL.
 * -2 The parameter named is not found
 */
iopType_t mtic_getTypeForInput(const char *name){

    //Check the name
    if((name == NULL) || (strlen(name) == 0)){
        mtic_debug("The input name cannot be NULL or empty\n");
        return 0;
    }

    //Find the Iop
    agent_iop *inputFound = NULL;

    //check if def live iexist
    if(mtic_definition_live == NULL){
        mtic_debug("Definition live is NULL.");
        return -1;
    }

    //Find the input by the name in the table of the my agent's definition
    HASH_FIND_STR(mtic_definition_live->inputs_table, name, inputFound);

    if(inputFound == NULL){
        mtic_debug("The input named : %s is not found in the table.\n", name);
        return -2;
    }

    //Return the type of the value
    return inputFound->type;
}

/**
 * \fn iopType_t mtic_getTypeForOutput(const char *name)
 * \brief this function returns the value type of the output (integer, bool, etc .)
 *
 * \param name The string which contains the name of the output. Can't be NULL or empty.
 * \return The output value type.
 * 0 The parameter name cannot be NULL or empty.
 * -1 The definition live is NULL.
 * -2 The parameter named is not found
 */
iopType_t mtic_getTypeForOutput(const char *name){

    //Check the name
    if((name == NULL) || (strlen(name) == 0)){
        mtic_debug("The output name cannot be NULL or empty\n");
        return 0;
    }

    //Find the Iop
    agent_iop *outputFound = NULL;

    //check if def live iexist
    if(mtic_definition_live == NULL){
        mtic_debug("Definition live is NULL.");
        return -1;
    }

    //Find the output by the name in the table of the my agent's definition
    HASH_FIND_STR(mtic_definition_live->outputs_table, name, outputFound);

    if(outputFound == NULL){
        mtic_debug("The output named : %s is not found in the table.\n", name);
        return -2;
    }

    //Return the type of the value
    return outputFound->type;
}

/**
 * \fn iopType_t mtic_getTypeForParameter(const char *name)
 * \brief this function returns the value type of the parameter (integer, bool, etc .)
 *
 * \param name The string which contains the name of the parameter. Can't be NULL or empty.
 * \return The parameter value type.
 * 0 The parameter name cannot be NULL or empty.
 * -1 The definition live is NULL.
 * -2 The parameter named is not found
 */
iopType_t mtic_getTypeForParameter(const char *name){

    //Check the name
    if((name == NULL) || (strlen(name) == 0)){
        mtic_debug("The parameter name cannot be NULL or empty\n");
        return 0;
    }

    //Find the Iop
    agent_iop *parameterFound = NULL;

    //check if def live iexist
    if(mtic_definition_live == NULL){
        mtic_debug("Definition live is NULL.");
        return -1;
    }

    //Find the output by the name in the table of the my agent's definition
    HASH_FIND_STR(mtic_definition_live->params_table, name, parameterFound);

    if(parameterFound == NULL){
        mtic_debug("The parameter named : %s is not found in the table.\n", name);
        return -2;
    }

    //Return the type of the value
    return parameterFound->type;
}

void mtic_getInputsList(char **list, long nbOfElements){
    
}
void mtic_getOutputsList(char **list, long nbOfElements){
    
}
void mtic_getParametersList(char **list, long nbOfElements){
    
}

/**
 * \fn bool mtic_checkInputExistence(const char *name)
 * \brief this function returns the state of the input existence.
 *
 * \param name The string which contains the name of the input. Can't be NULL or empty.
 * \return A boolean "true" if the input exist.
 */
bool mtic_checkInputExistence(const char *name){

    //Check the name
    if((name == NULL) || (strlen(name) == 0)){
        mtic_debug("The input name cannot be NULL or empty\n");
        return false;
    }

    //Find the Iop
    agent_iop *inputFound = NULL;

    //check if def live iexist
    if(mtic_definition_live == NULL){
        mtic_debug("Definition live is NULL.");
        return false;
    }

    //Find the input by the name in the table of the my agent's definition
    HASH_FIND_STR(mtic_definition_live->inputs_table, name, inputFound);

    if(inputFound == NULL){
        mtic_debug("The input named : %s is not found in the table.\n", name);
        return false;
    }

    return true;
}

/**
 * \fn bool mtic_checkOutputExistence(const char *name)
 * \brief this function returns the state of the output existence.
 *
 * \param name The string which contains the name of the output. Can't be NULL or empty.
 * \return A boolean "true" if the output exist.
 */
bool mtic_checkOutputExistence(const char *name){

    //Check the name
    if((name == NULL) || (strlen(name) == 0)){
        mtic_debug("The output name cannot be NULL or empty\n");
        return false;
    }

    //Find the Iop
    agent_iop *outputFound = NULL;

    //check if def live iexist
    if(mtic_definition_live == NULL){
        mtic_debug("Definition live is NULL.");
        return false;
    }

    //Find the input by the name in the table of the my agent's definition
    HASH_FIND_STR(mtic_definition_live->outputs_table, name, outputFound);

    if(outputFound == NULL){
        mtic_debug("The output named : %s is not found in the table.\n", name);
        return false;
    }

    return true;
}

/**
 * \fn bool mtic_checkParameterExistence(const char *name)
 * \brief this function returns the state of the parameter existence.
 *
 * \param name The string which contains the name of the parameter. Can't be NULL or empty.
 * \return A boolean "true" if the output exist.
 */
bool mtic_checkParameterExistence(const char *name){
    //Check the name
    if((name == NULL) || (strlen(name) == 0)){
        mtic_debug("The parameter name cannot be NULL or empty\n");
        return false;
    }

    //Find the Iop
    agent_iop *parameterFound = NULL;

    //check if def live iexist
    if(mtic_definition_live == NULL){
        mtic_debug("Definition live is NULL.");
        return false;
    }

    //Find the input by the name in the table of the my agent's definition
    HASH_FIND_STR(mtic_definition_live->params_table, name, parameterFound);

    if(parameterFound == NULL){
        mtic_debug("The parameter named : %s is not found in the table.\n", name);
        return false;
    }

    return true;
}

//observe IOP
//calback format for IOP observation
typedef void (*mtic_observeCallback)(iop_t iop, const char *name, iopType_t valueType, void *value, void * myData);
int mtic_observeInput(const char *name, mtic_observeCallback cb, void *myData){
    return 1;
}
int mtic_observeOutput(const char *name, mtic_observeCallback cb, void * myData){
    return 1;
}
int mtic_observeParameter(const char *name, mtic_observeCallback cb, void * myData){
    return 1;
}


//mute or unmute an IOP
int mtic_muteOutput(const char *name){
    return 1;
}
int mtic_unmuteOutput(const char *name){
    return 1;
}
bool mtic_isOutputMuted(const char *name){
    return 1;
}

//mute the agent
int mtic_mute(){
    return 1;
}

int mtic_unmute(){
    return 1;
}

bool mtic_isMuted(){
    return 1;
}

