/*
 *    INGEPROBE
 *
 *  Copyright (c) 2016-2020 Ingenuity i/o. All rights reserved.
 *
 *    See license terms for the rights and conditions
 *    defined by copyright holders.
 *
 *
 *    Contributors:
 *      Stephane Vales <vales@ingenuity.io>
 *
 *
 */


#include <zyre.h>
#include <zyre_event.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "uthash/uthash.h"
#include <getopt.h>
#include "call.h"

#define UNUSED(x) (void)x;
#define BUFFER_SIZE 4096

//global application options
int port = 5670;
const char *name = "ingeprobe";
char *networkDevice = NULL;
bool verbose = false;

const char *gossipbind = NULL; //"tcp://10.0.0.8:5659";
const char *gossipconnect = NULL; //"tcp://10.0.0.8:5661";
const char *endpoint = NULL; //"tcp://10.0.0.8:5672";

bool useSecurity = false;
zcert_t *cert = NULL;
zcert_t *brokerCert = NULL;
zactor_t *auth = NULL;
const char *privateKey = "~/Documents/IngeScape/security/ingeprobe.cert_secret";
const char *brokerKey = NULL; //"~/Documents/IngeScape/security/sparingPartner.cert";
const char *publicKeys = "~/Documents/IngeScape/security";
char privateKeyPath[BUFFER_SIZE] = "";
char brokerKeyPath[BUFFER_SIZE] = "";
char publicKeysPath[BUFFER_SIZE] = "";

char *outputTypes[] = {"INT", "DOUBLE", "STRING", "BOOL", "IMPULSION", "DATA"};

//for message passed as run parameter
char *paramMessage = NULL;

//data storage
typedef struct peer peer_t;
typedef struct agent agent_t;

typedef struct context{
    char *name;
    zyre_t *node;
    peer_t *peers;
    agent_t *agents;
    bool useGossip;
} context_t;

#define NAME_BUFFER_SIZE 256
typedef struct peer {
    char *uuid;
    char *name;
    char *endpoint;
    int reconnected;
    char *publisherPort;
    char *logPort;
    char *protocol;
    char *publicKey;
    zsock_t *subscriber;
    zmq_pollitem_t *subscriberPoller;
    zsock_t *logger;
    zmq_pollitem_t *loggerPoller;
    context_t *context;
    UT_hash_handle hh;
} peer_t;

typedef struct agent{
    char *uuid;
    char *name;
    peer_t *peer;
    call_t *calls;
    UT_hash_handle hh;
} agent_t;

///////////////////////////////////////////////////////////////////////////////
// UTILITIES

#define MAX_NUMBER_OF_NETDEVICES 16
#define MAX_NETWORK_DEVICE_LENGTH 1024

void getNetdevicesList(char ***devices, int *nb){
#if (defined WIN32 || defined _WIN32)
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    assert (rc == 0);
    assert (LOBYTE (wsa_data.wVersion) == 2 &&
            HIBYTE (wsa_data.wVersion) == 2);
#endif
    if (devices != NULL)
        *devices = calloc(MAX_NUMBER_OF_NETDEVICES, sizeof(char*));
    int currentDeviceNb = 0;
    
    ziflist_t *iflist = ziflist_new ();
    assert (iflist);
    const char *name = ziflist_first (iflist);
    while (name) {
        //        printf (" - name=%s address=%s netmask=%s broadcast=%s\n",
        //                name, ziflist_address (iflist), ziflist_netmask (iflist), ziflist_broadcast (iflist));
        if (devices != NULL){
            (*devices)[currentDeviceNb] = calloc(MAX_NETWORK_DEVICE_LENGTH+1, sizeof(char));
            strncpy((*devices)[currentDeviceNb], name, MAX_NETWORK_DEVICE_LENGTH);
        }
        currentDeviceNb++;
        name = ziflist_next (iflist);
    }
    ziflist_destroy (&iflist);
    *nb = currentDeviceNb;
#if (defined WIN32 || defined _WIN32)
    WSACleanup();
#endif
}

void getNetaddressesList(char ***addresses, int *nb){
#if (defined WIN32 || defined _WIN32)
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    assert (rc == 0);
    assert (LOBYTE (wsa_data.wVersion) == 2 &&
            HIBYTE (wsa_data.wVersion) == 2);
#endif
    if (addresses != NULL)
        *addresses = calloc(MAX_NUMBER_OF_NETDEVICES, sizeof(char*));
    int currentDeviceNb = 0;
    
    ziflist_t *iflist = ziflist_new ();
    assert (iflist);
    const char *name = ziflist_first (iflist);
    while (name) {
        if (addresses != NULL){
            (*addresses)[currentDeviceNb] = strdup(ziflist_address(iflist));
        }
        currentDeviceNb++;
        name = ziflist_next (iflist);
    }
    ziflist_destroy (&iflist);
    *nb = currentDeviceNb;
#if (defined WIN32 || defined _WIN32)
    WSACleanup();
#endif
}

void freeNetdevicesList(char **devices, int nb){
    int i = 0;
    for (i = 0; i < nb; i++){
        if (devices != NULL && devices[i] != NULL)
            free(devices[i]);
    }
    if (devices != NULL)
        free (devices);
}

void freeNetaddressesList(char **addresses, int nb){
    freeNetdevicesList(addresses, nb);
}

///////////////////////////////////////////////////////////////////////////////
// ZYRE AGENT MANAGEMENT

//manage incoming messages from one of the publisher agent we subscribed to
int manageSubscription (zloop_t *loop, zsock_t *socket, void *arg){
    UNUSED(loop);
    agent_t *agent = (agent_t *)arg;
    zmsg_t *msg = zmsg_recv(socket);
    size_t s = zmsg_size(msg);
    char *outputName = NULL;
    char *string = NULL;
    zframe_t *frame = NULL;
    void *data = NULL;
    char uuid[33] = ""; //33 is UUID length + terminal 0
    size_t size = 0;
    iopType_t type = 0;
    printf("%s published : ", agent->name);
    
    for (size_t i = 0; i < s; i += 3){
        outputName = zmsg_popstr(msg);
        if (streq(agent->peer->protocol, "v2")){
            //in v2 protocol, publication message starts with agent identification
            snprintf(uuid, 33, "%s", outputName);
            agent_t *a = NULL;
            HASH_FIND_STR(agent->peer->context->agents, uuid, a);
            printf("%s.%s", a->name, outputName + 33);
        }else{
            printf("%s", outputName);
        }
        free(outputName);
        string = zmsg_popstr(msg); //output type as string
        type = atoi(string);
        free(string);
        printf(" %s", outputTypes[type-1]);
        switch (type) {
            case IGS_INTEGER_T:
                frame = zmsg_pop(msg);
                data = zframe_data(frame);
                size = zframe_size(frame);
                printf(" %d\n", *((int *)data));
                break;
            case IGS_DOUBLE_T:
                frame = zmsg_pop(msg);
                data = zframe_data(frame);
                size = zframe_size(frame);
                printf(" %f\n", *((double *)data));
                break;
            case IGS_BOOL_T:
                frame = zmsg_pop(msg);
                data = zframe_data(frame);
                size = zframe_size(frame);
                printf(" %d\n", *((bool *)data));
                break;
            case IGS_STRING_T:{
                char *str = zmsg_popstr(msg);
                printf(" %s\n", str);
                free(str);
                break;
            }
            case IGS_IMPULSION_T:
                frame = zmsg_pop(msg);
                data = zframe_data(frame);
                size = zframe_size(frame);
                printf("\n");
                break;
            case IGS_DATA_T:
                frame = zmsg_pop(msg);
                data = zframe_data(frame);
                size = zframe_size(frame);
                string = zframe_strhex(frame);
                printf(" (%lu bytes) %.64s\n", size, string);
                free(string);
                break;
                
            default:
                break;
        }
        if (frame != NULL)
            zframe_destroy(&frame);
    }
    zmsg_destroy(&msg);
    return 0;
}

//manage incoming messages from one of the logger agent we subscribed to
int manageLog (zloop_t *loop, zsock_t *socket, void *arg){
    UNUSED(loop);
    peer_t *a = (peer_t *)arg;
    zmsg_t *msg = zmsg_recv(a->logger);
    printf("%s logged : ", a->name);
    char *part = zmsg_popstr(msg);
    while (part != NULL){
        printf(" %s", part);
        free(part);
        part = zmsg_popstr(msg);
    }
    printf("\n");
    zmsg_destroy(&msg);
    return 0;
}

//manage commands entered on the command line from the parent
int manageParent (zloop_t *loop, zsock_t *socket, void *args){
    context_t *context = (context_t *)args;
    zyre_t *node = context->node;
    zmsg_t *msg = zmsg_recv (socket);
    if (!msg){
        printf("Error while reading message from main thread");
        exit(EXIT_FAILURE); //Interrupted
    }
    char *command = zmsg_popstr (msg);
    if (streq (command, "$TERM")){
        return -1;
    }
    else
    {
        if (streq (command, "SHOUT")) {
            char *channel = zmsg_popstr (msg);
            char *string = zmsg_popstr (msg);
            zyre_shouts (node, channel, "%s", string);
            free(channel);
            free(string);
        }
        else if (streq (command, "WHISPER")) {
            char *peer = zmsg_popstr (msg);
            char *string = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                if (strcmp(p->name, peer) == 0 || strcmp(p->uuid, peer) == 0){
                    //NB: no break here beacause we allow whispering several peers
                    //having the same name
                    zyre_whispers (node, p->uuid, "%s", string);
                }
            }
            free(peer);
            free(string);
        }
        else if (streq (command, "WHISPERALL")) {
            char *string = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                zyre_whispers (node, p->uuid, "%s", string);
            }
            free(string);
        }
        else if(streq (command, "LEAVE")){
            char *channel = zmsg_popstr (msg);
            zyre_leave (node, channel);
            free(channel);
        }
        else if(streq (command, "LEAVEALL")){
            char *p;
            zlist_t *all_channels = zyre_peer_groups(node);
            while ((p = (char *)zlist_pop(all_channels))) {
                zyre_leave (node, p);
                free(p);
            }
            zlist_destroy(&all_channels);
        }
        else if(streq (command, "JOIN")){
            char *channel = zmsg_popstr (msg);
            //zyre_set_contest_in_group(node, channel);
            zyre_join (node, channel);
            free(channel);
        }
        else if(streq (command, "JOINALL")){
            char *p;
            zlist_t *all_channels = zyre_peer_groups(node);
            while ((p = (char *)zlist_pop(all_channels))) {
                zyre_join (node, p);
                free(p);
            }
            zlist_destroy(&all_channels);
        }
        else if(streq (command, "AGENTS")){
            peer_t *peer, *ptmp;
            HASH_ITER(hh, context->peers, peer, ptmp){
                printf("in peer '%s' (%s) - %s:\n", peer->name, peer->uuid, peer->endpoint);
                agent_t *a = NULL, *atmp;
                HASH_ITER(hh, context->agents, a, atmp){
                    if (a->peer == peer){
                        printf("\t'%s' (%s)\n", a->name, a->uuid);
                    }
                }
            }
        }
        else if(streq (command, "WRITE")){
            char *target = zmsg_popstr (msg);
            char *args = zmsg_popstr (msg);
            char type[2] = "";
            char iopName[BUFFER_SIZE] = "";
            char value[BUFFER_SIZE] = "";
            peer_t *p = NULL;
            agent_t *a = NULL, *atmp;
            //try to find agent with this name or uuid
            HASH_ITER(hh, context->agents, a, atmp){
                if (streq(a->name, target) || streq(a->uuid, target)){
                    p = a->peer;
                    int res = sscanf(args, "%1s %s %s", type, iopName, value);
                    if (res == 3){
                        snprintf(value, BUFFER_SIZE, "%s", args + strlen(type) + 1 + strlen(iopName) + 1);
                        zmsg_t *msg = zmsg_new();
                        if (streq(type, "i")){
                            zmsg_addstr(msg, "SET_INPUT");
                        }else if (streq(type, "o")){
                            zmsg_addstr(msg, "SET_OUTPUT");
                        }else if (streq(type, "p")){
                            zmsg_addstr(msg, "SET_PARAMETER");
                        }else{
                            printf("invalid type format '%s'\n", type);
                            zmsg_destroy(&msg);
                            break;
                        }
                        zmsg_addstr(msg, iopName);
                        zmsg_addstr(msg, value);
                        zmsg_addstr(msg, a->uuid);
                        zyre_whisper(node, p->uuid, &msg);
                    }else{
                        printf("'%s' has bad format\n", args);
                    }
                }
            }
        }
        else if(streq (command, "CALL")){
            char *callTarget = zmsg_popstr (msg);
            char *callArgs = zmsg_popstr (msg);
            peer_t *p = NULL;
            agent_t *a = NULL, *atmp;
            bool sentCall = false;
            //try to find agent with this name or uuid
            HASH_ITER(hh, context->agents, a, atmp){
                if (streq(a->name, callTarget) || streq(a->uuid, callTarget)){
                    p = a->peer;
                    //Extract call name
                    size_t offset = 0;
                    while (callArgs[offset] != ' ' && callArgs[offset] != '\0') {
                        offset++;
                    }
                    char callName[4096] = "";
                    strncpy(callName, callArgs, offset);
                    call_t *call = NULL;
                    HASH_FIND_STR(a->calls, callName, call);
                    if (call != NULL){
                        zmsg_t *callMsg = zmsg_new();
                        zmsg_addstr(callMsg, "CALL");
                        if (streq(p->protocol, "v2")){
                            //v2 protocol : add our id as caller and callee agent uuid
                            zmsg_addstr(callMsg, zyre_uuid(context->node));
                            zmsg_addstr(callMsg, a->uuid);
                        }else{
                            //v1 protocol
                            //nothing to do
                        }
                        if (addArgumentsToCallMessage(callMsg, call, callArgs)){
                            zyre_whisper(node, p->uuid, &callMsg);
                            sentCall = true;
                        }
                    }
                    free(callArgs);
                    free(callTarget);
                }
            }
            if (!sentCall){
                printf("unknown agent %s\n", callTarget);
            }
        }
        else if(streq (command, "PEERS")){
            zlist_t *peers = zyre_peers(node);
            char *p;
            printf("@peers:\n");
            while ((p = (char *)zlist_pop(peers))) {
                peer_t *a = NULL;
                HASH_FIND_STR(context->peers, p, a);
                if (a != NULL){
                    printf("\t%s -> %s\n", a->name, p);
                }else{
                    printf("\t? -> %s\n", p);
                }
                free(p);
            }
            zlist_destroy(&peers);
        }
        else if(streq (command, "CHANNELS")){
            zlist_t *my_channels = zyre_own_groups(node);
            zlist_t *all_channels = zyre_peer_groups(node);
            char *p;
            printf("@my channels:\n");
            while ((p = (char *)zlist_pop(my_channels))) {
                printf("\t%s\n", p);
                free(p);
            }
            printf("@all channels:\n");
            while ((p = (char *)zlist_pop(all_channels))) {
                printf("\t%s\n", p);
                free(p);
            }
            zlist_destroy(&my_channels);
            zlist_destroy(&all_channels);
        }
        else if(streq (command, "VERBOSE")){
            zyre_set_verbose(node);
        }
        else if (streq (command, "SUBSCRIBE")) {
            char *agentId = zmsg_popstr (msg);
            agent_t *agent, *tmp;
            size_t nbAgents = HASH_COUNT(context->agents);
            if (nbAgents == 0){
                printf("no agent : join INGESCAPE_PRIVATE to receive agents definitions\n");
            }
            HASH_ITER(hh, context->agents, agent, tmp){
                //NB: no break here because we allow subscribing to several peers
                //having the same name
                if (strcmp(agent->name, agentId) == 0 || strcmp(agent->uuid, agentId) == 0){
                    if (agent->peer->publisherPort == NULL){
                        printf("Found agent %s but its peer publisher port is NULL : command has been ignored\n", agentId);
                        continue;
                    }
                    
                    if (agent->peer->subscriber != NULL){
                        //subscriber to this agent is already active:
                        //we just have to add internal pub/sub command
                        zsock_set_subscribe(agent->peer->subscriber, "");
                        continue;
                    }
                    
                    char endpointAddress[128];
                    strncpy(endpointAddress, agent->peer->endpoint, 128);
                    // IP adress extraction
                    char *insert = endpointAddress + strlen(endpointAddress);
                    bool extractOK = true;
                    while (*insert != ':'){
                        insert--;
                        if (insert == endpointAddress){
                            printf("Error: could not extract port from address %s", agent->peer->endpoint);
                            extractOK = false;
                            break;
                        }
                    }
                    if (extractOK){
                        *(insert + 1) = '\0';
                        strcat(endpointAddress, agent->peer->publisherPort);
                        agent->peer->subscriber = zsock_new_sub(endpointAddress, NULL);
                        if (useSecurity){
                            assert(agent->peer->publicKey);
                            zcert_apply(cert, agent->peer->subscriber);
                            zsock_set_curve_serverkey (agent->peer->subscriber, agent->peer->publicKey);
                        }
                        zsock_set_subscribe(agent->peer->subscriber, "");
                        zloop_reader(loop, agent->peer->subscriber, manageSubscription, agent);
                        zloop_reader_set_tolerant(loop, agent->peer->subscriber);
                        printf("Subscriber created for %s\n", agent->name);
                    }
                }
            }
            free(agentId);
        }
        else if (streq (command, "SUBSCRIBE_TO_OUTPUT")) {
            char *agentId = zmsg_popstr (msg);
            char *output = zmsg_popstr (msg);
            size_t nbAgents = HASH_COUNT(context->agents);
            if (nbAgents == 0){
                printf("no agent : first join INGESCAPE_PRIVATE to receive agents definitions\n");
            }
            agent_t *agent, *tmp;
            HASH_ITER(hh, context->agents, agent, tmp){
                //NB: no break here because we allow subscribing to several peers
                //having the same name
                if (strcmp(agent->name, agentId) == 0 || strcmp(agent->uuid, agentId) == 0){
                    if (agent->peer->publisherPort == NULL){
                        printf("found agent %s but its peer publisher port is NULL : command has been ignored\n", agentId);
                        continue;
                    }
                    
                    if (agent->peer->subscriber != NULL){
                        //subscriber to this agent is already active:
                        //we just have to add internal pub/sub command
                        if (streq(agent->peer->protocol, "v1")){
                            zsock_set_subscribe(agent->peer->subscriber, output);
                        }else if (streq(agent->peer->protocol, "v2")){
                            char filterValue[4096 + 33] = ""; //33 is UUID length + separator
                            snprintf(filterValue, 4096 + 33, "%s-%s", agent->uuid, output);
                            zsock_set_subscribe(agent->peer->subscriber, filterValue);
                        }
                        continue;
                    }
                    
                    char endpointAddress[128];
                    strncpy(endpointAddress, agent->peer->endpoint, 128);
                    // IP adress extraction
                    char *insert = endpointAddress + strlen(endpointAddress);
                    bool extractOK = true;
                    while (*insert != ':'){
                        insert--;
                        if (insert == endpointAddress){
                            printf("error: could not extract port from address %s", agent->peer->endpoint);
                            extractOK = false;
                            break;
                        }
                    }
                    if (extractOK){
                        *(insert + 1) = '\0';
                        strcat(endpointAddress, agent->peer->publisherPort);
                        agent->peer->subscriber = zsock_new_sub(endpointAddress, NULL);
                        if (streq(agent->peer->protocol, "v1")){
                            zsock_set_subscribe(agent->peer->subscriber, output);
                        }else if (streq(agent->peer->protocol, "v2")){
                            if (useSecurity){
                                assert(agent->peer->publicKey);
                                zcert_apply(cert, agent->peer->subscriber);
                                zsock_set_curve_serverkey (agent->peer->subscriber, agent->peer->publicKey);
                            }
                            char filterValue[4096 + 33] = ""; //33 is UUID length + separator
                            snprintf(filterValue, 4096 + 33, "%s-%s", agent->uuid, output);
                            zsock_set_subscribe(agent->peer->subscriber, filterValue);
                        }
                        zloop_reader(loop, agent->peer->subscriber, manageSubscription, agent);
                        zloop_reader_set_tolerant(loop, agent->peer->subscriber);
                        printf("subscriber created for %s on output %s\n", agent->name, output);
                    }
                }
            }
            free(agentId);
            free(output);
        }
        else if (streq (command, "UNSUBSCRIBE")) {
            char *agentId = zmsg_popstr (msg);
            agent_t *agent, *tmp;
            HASH_ITER(hh, context->agents, agent, tmp){
                //NB: no break here beacause we allow unsubscribing to several peers
                //having the same name
                if (strcmp(agent->name, agentId) == 0 || strcmp(agent->uuid, agentId) == 0){
                    if (agent->peer->publisherPort == NULL){
                        printf("found peer %s but its publisher port is NULL : command has been ignored\n", agentId);
                        continue;
                    }
                    if (agent->peer->subscriber == NULL){
                        printf("found peer %s but its subscriber socket is NULL : command has been ignored\n", agentId);
                        continue;
                    }else{
                        zloop_reader_end(loop, agent->peer->subscriber);
                        zsock_destroy(&(agent->peer->subscriber));
                    }
                }
            }
            free(agentId);
        }
        else if (streq (command, "LOG")) {
            char *peerId = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                if (strcmp(p->name, peerId) == 0 || strcmp(p->uuid, peerId) == 0){
                    if (p->logPort == NULL){
                        printf("found peer %s but its log port is NULL : command has been ignored\n", peerId);
                        continue;
                    }
                    //NB: no break here beacause we allow subscribing to several peers
                    //having the same name
                    char endpointAddress[128];
                    strncpy(endpointAddress, p->endpoint, 128);
                    
                    // IP adress extraction
                    char *insert = endpointAddress + strlen(endpointAddress);
                    bool extractOK = true;
                    while (*insert != ':'){
                        insert--;
                        if (insert == endpointAddress){
                            printf("error: could not extract port from address %s", p->endpoint);
                            extractOK = false;
                            break;
                        }
                    }
                    if (extractOK){
                        *(insert + 1) = '\0';
                        strcat(endpointAddress, p->logPort);
                        p->logger = zsock_new_sub(endpointAddress, NULL);
                        if (useSecurity){
                            assert(p->publicKey);
                            zcert_apply(cert, p->logger);
                            zsock_set_curve_serverkey (p->logger, p->publicKey);
                        }
                        zsock_set_subscribe(p->logger, "");
                        zloop_reader(loop, p->logger, manageLog, p);
                        zloop_reader_set_tolerant(loop, p->logger);
                        printf("log stream created for %s\n", p->name);
                    }
                }
            }
            free(peerId);
        }
        else if (streq (command, "UNLOG")) {
            char *peerId = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                //NB: no break here because we allow subscribing to several peers
                //having the same name
                if (strcmp(p->name, peerId) == 0 || strcmp(p->uuid, peerId) == 0){
                    if (p->logPort == NULL){
                        printf("found peer %s but its logger port is NULL : command has been ignored\n", peerId);
                        continue;
                    }
                    if (p->logger == NULL){
                        printf("found peer %s but its logger socket is NULL : command has been ignored\n", peerId);
                        continue;
                    }else{
                        zloop_reader_end(loop, p->logger);
                        zsock_destroy(&(p->logger));
                    }
                }
            }
            free(peerId);
        }
        else if (streq (command, "LICENSE")) {
            char *peerId = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                //NB: no break here because multiple peers can have the same name
                if (strcmp(p->name, peerId) == 0 || strcmp(p->uuid, peerId) == 0){
                    zyre_whispers(node, p->uuid, "GET_LICENSE_INFO");
                }
            }
            free(peerId);
        }
        else if (streq (command, "START_AGENT")) {
            char *uuid = zmsg_popstr (msg);
            char *agentName = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                //NB: no break here because multiple peers can have the same name
                if (strcmp(p->name, uuid) == 0 || strcmp(p->uuid, uuid) == 0){
                    zmsg_t *msg = zmsg_new();
                    zmsg_addstr(msg, "START_AGENT");
                    zmsg_addstr(msg, agentName);
                    zyre_whisper(node, p->uuid, &msg);
                }
            }
            free(uuid);
        }
        else if (streq (command, "STOP_AGENT")) {
            char *uuid = zmsg_popstr (msg);
            agent_t *agent, *tmp;
            HASH_ITER(hh, context->agents, agent, tmp){
                //NB: no break here because multiple agents can have the same name
                if (strcmp(agent->name, uuid) == 0 || strcmp(agent->uuid, uuid) == 0){
                    zmsg_t *msg = zmsg_new();
                    zmsg_addstr(msg, "STOP_AGENT");
                    zmsg_addstr(msg, agent->uuid);
                    zyre_whisper(node, agent->peer->uuid, &msg);
                }
            }
            free(uuid);
        }
        else if (streq (command, "STOP_PEER")) {
            char *peerId = zmsg_popstr (msg);
            peer_t *p, *tmp;
            HASH_ITER(hh, context->peers, p, tmp){
                //NB: no break here because multiple peers can have the same name
                if (strcmp(p->name, peerId) == 0 || strcmp(p->uuid, peerId) == 0){
                    zyre_whispers(node, p->uuid, "STOP_PEER");
                }
            }
            free(peerId);
        }
        else {
            printf("error: invalid command to actor\n%s\n", command);
            assert (false);
        }
    }
    
    free (command);
    zmsg_destroy (&msg);
    return 0;
}

//manage messages received on the bus
int manageIncoming (zloop_t *loop, zsock_t *socket, void *args){
    context_t *context = (context_t *)args;
    zyre_t *node = context->node;
    zyre_event_t *zyre_event = zyre_event_new (node);
    const char *event = zyre_event_type(zyre_event);
    const char *peerId = zyre_event_peer_uuid(zyre_event);
    const char *name = zyre_event_peer_name (zyre_event);
    const char *address = zyre_event_peer_addr (zyre_event);
    const char *channel = zyre_event_group (zyre_event);
    zmsg_t *msg = zyre_event_msg (zyre_event);
    //size_t msg_size = zmsg_content_size(msg);

    peer_t *peer = NULL;
    HASH_FIND_STR(context->peers, peerId, peer);
    
    if (streq (event, "ENTER")){
        if (!context->useGossip){
            printf ("B->%s has entered the network with peer id %s and endpoint %s\n", name, peerId, address);
        }else{
            printf ("G->%s has entered the network with peer id %s and endpoint %s\n", name, peerId, address);
        }
        //printf ("->%s has entered the network with peer id %s and endpoint %s\n", name, peer, address);
        if (peer == NULL){
            peer = calloc(1, sizeof(peer_t));
            peer->reconnected = 0;
            peer->uuid = strndup(peerId, NAME_BUFFER_SIZE);
            peer->name = strndup(name, NAME_BUFFER_SIZE);
            peer->endpoint = strndup(address, NAME_BUFFER_SIZE);
            peer->context = context;
            HASH_ADD_STR(context->peers, uuid, peer);
        }else{
            //Agent already exists, we set its reconnected flag
            //(this is used below to avoid agent destruction on EXIT received after timeout)
            peer->reconnected++;
        }
        
        
        zhash_t *headers = zyre_event_headers (zyre_event);
        assert(headers);
        char *k;
        const char *v;
        zlist_t *keys = zhash_keys(headers);
        size_t s = zlist_size(keys);
        if (s > 0){
            printf("@%s's headers are:\n", name);
        }
        while ((k = (char *)zlist_pop(keys))) {
            v = zyre_event_header (zyre_event,k);
            if(streq(k,"publisher")){
                //this is a ingescape agent, we store its publishing port
                if (peer->publisherPort != NULL) {
                    free(peer->publisherPort);
                }
                peer->publisherPort = strndup(v,6); //port is 5 digits max
            }else if(streq(k,"logger")){
                //this is a ingescape agent, we store its publishing port
                if (peer->logPort != NULL) {
                    free(peer->logPort);
                }
                peer->logPort = strndup(v,6); //port is 5 digits max
            }else if(streq(k,"protocol")){
                //this is a ingescape agent, we store its publishing port
                if (peer->protocol != NULL) {
                    free(peer->protocol);
                }
                peer->protocol = strndup(v, 6); //max protocol length
            }else if(streq(k,"X-PUBLICKEY")){
                peer->publicKey = strndup(v, 64);
            }
            printf("\t%s -> %s\n", k, v);
            free(k);
        }
        zlist_destroy(&keys);
    } else if (streq (event, "JOIN")){
        printf ("+%s has joined %s\n", name, channel);
    } else if (streq (event, "LEAVE")){
        printf ("-%s has left %s\n", name, channel);
    } else if (streq (event, "SHOUT")){
        char *message = zmsg_popstr(msg);
        if (message == NULL){
            //nothing
        }else if (streq(message, "REMOTE_AGENT_EXIT")){
            char *uuid = zmsg_popstr(msg);
            agent_t *agent = NULL;
            HASH_FIND_STR(context->agents, uuid, agent);
            if (agent){
                HASH_DEL(context->agents,agent);
                printf("[IGS] %s (%s) exited\n", agent->name, agent->uuid);
                free(agent->name);
                free(agent->uuid);
                if (agent->calls != NULL){
                    freeCalls(&(agent->calls));
                }
                free(agent);
            }
            zmsg_pushstr(msg, uuid); //put uuid back into message
            free(uuid);
        }
        printf("#%s:%s(%s) - %s ", channel, name, peerId, message);
        if (message)
            free(message);
        while ((message = zmsg_popstr(msg))){
            printf("| %s ", message);
            free(message);
        }
        printf("\n");
        
    } else if (streq (event, "WHISPER")){
        zmsg_t *dup = zmsg_dup(msg);
        char *message = zmsg_popstr(dup);
        if (message == NULL){
            //nothing
        }else if (peer->protocol && streq(peer->protocol, "v2") && streq(message, "EXTERNAL_DEFINITION#")){
            char* strDefinition = zmsg_popstr(dup);
            char *uuid = zmsg_popstr(dup);
            char *remoteAgentName = zmsg_popstr(dup);
            agent_t *agent = NULL;
            HASH_FIND_STR(context->agents, uuid, agent);
            if (agent == NULL){
                agent = calloc(1, sizeof(agent_t));
                agent->uuid = uuid;
                agent->name = remoteAgentName;
                HASH_ADD_STR(context->agents, uuid, agent);
                peer_t *p = NULL;
                HASH_FIND_STR(context->peers, peerId, p);
                assert(p);
                agent->peer = p;
            }else if (!streq(remoteAgentName, agent->name)){
                if (agent->name != NULL)
                    free(agent->name);
                agent->name = remoteAgentName;
            }
            parseCallsFromDefinition(strDefinition, &(agent->calls));
            free(strDefinition);
        }else if (strncmp(message, "EXTERNAL_DEFINITION#", strlen("EXTERNAL_DEFINITION#")) == 0){
            //v1 or v0 protocol
            char *definition = message + strlen("EXTERNAL_DEFINITION#");
            agent_t *agent = NULL;
            HASH_FIND_STR(context->agents, peerId, agent);
            if (agent == NULL){
                agent = calloc(1, sizeof(agent_t));
                peer_t *p = NULL;
                HASH_FIND_STR(context->peers, peerId, p);
                assert(p);
                agent->peer = p;
            }
            if (agent->name != NULL)
                free(agent->name);
            if (agent->uuid != NULL)
                free(agent->uuid);
            agent->name = strdup(name);
            agent->uuid = strdup(peerId);
            HASH_ADD_STR(context->agents, uuid, agent);
            parseCallsFromDefinition(definition, &(agent->calls));
        }
        if (message)
            free(message);
        zmsg_destroy(&dup);
        
        //print message details
        while ((message = zmsg_popstr(msg))) {
            printf("#%s(%s) - %s ", name, peerId, message);
            free(message);
            while ((message = zmsg_popstr(msg))){
                printf("| %s ", message);
                free(message);
            }
            printf("\n");
            free (message);
        }
        
    } else if (streq (event, "EXIT")){
        if (!context->useGossip){
            printf ("B<-%s exited\n", name);
        }else{
            printf ("G<-%s exited\n", name);
        }
        if (peer != NULL){
            if (peer->reconnected > 0){
                //do not destroy: we are getting a timemout now whereas
                //the agent is reconnected
                peer->reconnected--;
            }else{
                agent_t *a, *atmp;
                HASH_ITER(hh, context->agents, a, atmp){
                    if (a->peer == peer){
                        //this agent lost its peer : remove
                        printf("cleaning agent %s(%s)\n", a->name, a->uuid);
                        HASH_DEL(context->agents,a);
                        free(a->name);
                        free(a->uuid);
                        if (a->calls != NULL){
                            freeCalls(&(a->calls));
                        }
                        free(a);
                    }
                }
                printf("cleaning peer %s(%s)\n", peer->name, peer->uuid);
                HASH_DEL(context->peers, peer);
                free(peer->name);
                free(peer->uuid);
                free(peer->endpoint);
                if (peer->publisherPort != NULL){
                    free(peer->publisherPort);
                }
                if (peer->subscriber != NULL){
                    zloop_poller_end(loop, peer->subscriberPoller);
                    zsock_destroy(&(peer->subscriber));
                }
                if (peer->subscriberPoller != NULL){
                    free(peer->subscriberPoller);
                }
                if (peer->logPort != NULL){
                    free(peer->logPort);
                }
                if (peer->logger != NULL){
                    zloop_poller_end(loop, peer->loggerPoller);
                    zsock_destroy(&(peer->logger));
                }
                if (peer->loggerPoller != NULL){
                    free(peer->loggerPoller);
                }
                if (peer->protocol != NULL){
                    free(peer->protocol);
                }
                free(peer);
            }
        }
    } else if (streq (event, "SILENT")){
        printf ("%s (%s) is being silent\n", name, peerId);
    } else if (streq (event, "LEADER")){
        printf ("\\o/ %s(%s) is leader in election '%s'\n", name, peerId, channel);
    }
    zyre_event_destroy(&zyre_event);
    return 0;
}

//Function used to stop main thread when ingeprobe is used just
//to send a message and stop
int stopMainThreadLoop(zloop_t *loop, int timer_id, void *args){
    UNUSED(loop);
    UNUSED(timer_id);
    UNUSED(args);
    return -1;
}

int sendMessage(zloop_t *loop, int timer_id, void *args){
    UNUSED(loop);
    UNUSED(timer_id);
    context_t *context = (context_t *)args;
    zyre_t *node = context->node;
    char target[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    int matches = sscanf(paramMessage, "%s", target);
    if (matches == 1){
        strncpy(message, paramMessage + strlen(target) + 1, BUFFER_SIZE);
        if (strlen(target) > 0 && target[0] == '#'){
            zyre_shouts(node, target+1, "%s", message);
        }else if (strlen(target) > 0){
            zyre_whispers(node, target, "%s", message);
        }
    }
    //this returned value will stop the actor loop
    return -1;
}

static void zyre_actor(zsock_t *pipe, void *args){
    context_t *context = (context_t *)args;
    
    if (useSecurity){
        auth = zactor_new(zauth, NULL);
        assert(auth);
        assert(zstr_send(auth, "VERBOSE") == 0);
        assert(zsock_wait(auth) >= 0);
        //assert(zstr_sendx(auth, "ALLOW", ip_address1, ip_address2, ..., NULL) == 0);
        //assert(zsock_wait(auth) >= 0);
        assert(zstr_sendx(auth, "CURVE", publicKeysPath, NULL) == 0);
        assert(zsock_wait(auth) >= 0);
    }
    
    zyre_t *node = zyre_new (context->name);
    if (useSecurity){
        zyre_set_zcert(node, cert);
        zyre_set_zap_domain(node, "INGESCAPE");
    }
    //zyre_join(node, "INGESCAPE_PRIVATE");
    context->node = node;
    
    if (!context->useGossip){
        //beacon
        zyre_set_port(node, port);
        printf("using broadcast discovery with port %i", port);
        zyre_set_interface(node, networkDevice);
        printf(" on device %s\n", networkDevice);
    }else{
        //gossip
        if (endpoint != NULL){
            int res = zyre_set_endpoint(node, "%s", endpoint);
            if (res != 0){
                printf("impossible to create our endpoint %s... aborting.", endpoint);
                return;
            }
            printf("using endpoint %s\n", endpoint);
            if (gossipconnect == NULL && gossipbind == NULL){
                printf("endpoint specified but no attached gossip information, %s cannot reach any other agent : aborting", name);
                return;
            }
        }
        if (gossipconnect != NULL){
            if (useSecurity){
                zyre_gossip_connect_curve(node, zcert_public_txt(brokerCert), "%s", gossipconnect);
            }else{
                zyre_gossip_connect(node, "%s", gossipconnect);
            }
            printf("connecting to broker at %s\n", gossipconnect);
        }
        if (gossipbind != NULL){
            zyre_gossip_bind(node, "%s", gossipbind);
            printf("creating broker %s\n", gossipbind);
        }
    }
    
    if (verbose){
        zyre_set_verbose(node);
    }
    zyre_start (node);
    zsock_signal (pipe, 0); //notify main thread that we are ready
    zyre_print(node);
    
    zloop_t *loop = zloop_new ();
    zloop_set_verbose (loop, verbose);
    
    zloop_reader(loop, pipe, manageParent, context);
    zloop_reader_set_tolerant(loop, pipe);
    zloop_reader(loop, zyre_socket(node), manageIncoming, context);
    zloop_reader_set_tolerant(loop, zyre_socket(node));
    
    if (paramMessage != NULL && strlen(paramMessage) > 0){
        zloop_timer(loop, 10, 1, sendMessage, context);
    }
    
    zloop_start (loop);
    
    printf("shutting down...\n");
    //clean
    zloop_destroy (&loop);
    assert (loop == NULL);
    peer_t *current, *tmp;
    HASH_ITER(hh, context->peers, current, tmp) {
        HASH_DEL(context->peers,current);
        free(current->name);
        free(current->uuid);
        free(current->endpoint);
        if (current->publicKey)
            free(current->publicKey);
        if (current->publisherPort != NULL)
            free(current->publisherPort);
        if (current->subscriber != NULL)
            zsock_destroy(&(current->subscriber));
        if (current->subscriberPoller != NULL)
            free(current->subscriberPoller);
        if (current->logPort != NULL)
            free(current->logPort);
        if (current->logger != NULL)
            zsock_destroy(&(current->logger));
        if (current->loggerPoller != NULL)
            free(current->loggerPoller);
        if (current->protocol != NULL)
            free(current->protocol);
        free(current);
    }
    agent_t *agent, *atmp;
    HASH_ITER(hh, context->agents, agent, atmp) {
        HASH_DEL(context->agents,agent);
        free(agent->name);
        free(agent->uuid);
        if (agent->calls != NULL){
            freeCalls(&(agent->calls));
        }
        free(agent);
    }
    zyre_stop (node);
    zyre_destroy (&node);
    if (cert)
        zcert_destroy(&cert);
    if (auth)
        zactor_destroy(&auth);
    free(context);
}

///////////////////////////////////////////////////////////////////////////////
// COMMAND LINE AND INTERPRETER OPTIONS
//
void print_usage(){
    printf("Usage example: ingeprobe --verbose --port 5670 --name ingeprobe\n");
    printf("--verbose : enable verbose mode in the application\n");
    printf("--name peer_name : published name of this peer (default : ingeprobe)\n");
    printf("--noninteractiveloop : non-interactive loop for use as a background application\n");
    printf("--message \"[#channel|peer] message\" : message to send to a channel (indicated with #) or a peer (peer id) at startup and then stop\n");
    
    printf("\nSelf-discovery (UDP broadcast) :\n");
    printf("--device : name of the network device to be used (shall be set if several devices available)\n");
    printf("--port port_number : port used for autodiscovery between peers (default : 5670)\n");
    printf("OR\n");
    printf("Brokers and endpoints :\n");
    printf("a TCP endpoint looks like : tcp://10.0.0.7:49155\n");
    printf("--endpoint endpoint : our ingescape endpoint address (overrides --device and --port)\n");
    printf("--bind endpoint : our address as a broker (optional, use only if you want to be a broker)\n");
    printf("--connect endpoint : address of a broker to connect to\n");
    printf("NB: using the endpoint disables self-discovery and brokers must be used\n");
    
    printf("\nSecurity (optional)\n");
    printf("--security : enables security\n");
    printf("--privatekey path : path to the private key we shall use (default: %s)\n", privateKey);
    printf("--brokerkey path : path to the public key we shall use for the broker if needed (optional)\n");
    printf("--publickeys path : path to the directory where to find public keys (default: %s)\n", publicKeys);
}

/*
 Other commands to add:
 - map, clear mapping, unmap
 - mute, unmute, freeze, unfreeze
 - make /log enable the log stream
 */
void print_commands(){
    printf("---------------------------------\n");
    printf("Supported commands:\n");
    printf("/quit : cleanly stop the application\n");
    printf("/verbose : triggers verbose mode for detailed activity information\n");
    printf("\nzyre layer:\n");
    printf("/peers : list all connected peers at the time (name -> uuid)\n");
    printf("/channels : list all existing channels at the time\n");
    printf("/join channel_name : joins a specific channel\n");
    printf("/joinall : join all existing channels at the time\n");
    printf("/leave channel_name : leave a specific channel\n");
    printf("/leaveall : leave all existing channels at the time\n");
    printf("/whisper peer message : sends a message to a specific peer\n\t(peer can be name or uuid)\n");
    printf("/shout channel_name message : sends a message to a specific channel\n");
    printf("/whisperall message : sends a message to all peers individually\n");
    printf("/license peer : gets license information for this peer\n\t(peer can be  name or uuid)\n");
    printf("/stop_peer peer : stops this peer and all attached agents\n\t(peer can be  name or uuid)\n");
    printf("\ningescape layer:\n");
    printf("/agents : list all agents by peer\n");
    printf("/start peer agent : starts an agent on designated peer\n\t(agent is name, peer can be  name or uuid)\n");
    printf("/stop agent : stops this agent\n\t(agent can be  name or uuid)\n");
    printf("/subscribe agent : subscribes to all ingescape outputs for this agent\n\t(agent can be  name or uuid)\n");
    printf("/subscribe agent output : subscribes to ingescape agent specific output\n\t(agent can be  name or uuid)\n");
    printf("/unsubscribe agent : cancel all subscriptions to ingescape agent outputs\n\t(agent can be  name or uuid)\n");
    printf("/log peer : subscribes to ingescape agent log stream\n\t(peer can be name or uuid)\n");
    printf("/unlog peer : cancel subscription to ingescape agent log stream\n\t(peer can be name or uuid)\n");
    printf("/write agent i|o|p iop_name value : sends value to agent (name or uuid) for specified input/output/parameter\n");
    printf("/call agent call_name ... : sends a call to agent for specified call_name wth parameters\n");
    printf("\n");
}

//helper to convert paths starting with ~ to absolute paths
void makeFilePath(const char *from, char *to, size_t size_of_to) {
    if (from[0] == '~') {
        from++;
#ifdef _WIN32
        char *home = getenv("USERPROFILE");
#else
        char *home = getenv("HOME");
#endif
        if (home == NULL) {
            printf("could not find path for home directory\n");
        }
        else {
            strncpy(to, home, size_of_to);
            strncat(to, from, size_of_to);
        }
    }
    else {
        strncpy(to, from, size_of_to);
    }
}

///////////////////////////////////////////////////////////////////////////////
// MAIN & OPTIONS & COMMAND INTERPRETER
//
//
int main (int argc, char *argv [])
{
    //manage options
    int opt= 0;
    bool noninteractiveloop = false;
    
    //Specifying the expected options
    //The two options l and b expect numbers as argument
    static struct option long_options[] = {
        {"verbose",   no_argument, 0,  'v' },
        {"device",      required_argument, 0,  'd' },
        {"port",      required_argument, 0,  'p' },
        {"name",      required_argument, 0,  'n' },
        {"message",      required_argument, 0,  'm' },
        {"noninteractiveloop",      no_argument, 0,  'i' },
        {"help",      no_argument, 0,  'h' },
        {"bind",      required_argument, 0,  's' },
        {"connect",      required_argument, 0,  'g' },
        {"endpoint",      required_argument, 0,  'e' },
        {"privatekey",      required_argument, 0,  '0' },
        {"brokerkey",      required_argument, 0,  '1' },
        {"publickeys",      required_argument, 0,  '2' },
        {"security",      no_argument, 0,  '3' },
    };
    
    int long_index =0;
    while ((opt = getopt_long(argc, argv,"p",long_options, &long_index )) != -1) {
        switch (opt) {
            case 'p' :
                port = atoi(optarg);
                //printf("port: %i\n", port);
                break;
            case 'd' :
                networkDevice = optarg;
                //printf("device: %s\n", networkDevice);
                break;
            case 'n' :
                name = optarg;
                //printf("name: %s\n", name);
                break;
            case 's' :
                gossipbind = optarg;
                //printf("name: %s\n", name);
                break;
            case 'g' :
                gossipconnect = optarg;
                //printf("name: %s\n", name);
                break;
            case 'e' :
                endpoint = optarg;
                //printf("name: %s\n", name);
                break;
            case 'v' :
                verbose = true;
                //printf("verbose\n");
                break;
            case 'm' :
                paramMessage = strdup(optarg);
                break;
            case 'i' :
                noninteractiveloop = true;
                break;
            case '0' :
                privateKey = optarg;
                break;
            case '1' :
                brokerKey = optarg;
                break;
            case '2' :
                publicKeys = optarg;
                break;
            case '3' :
                useSecurity = true;
                break;
            case 'h' :
                print_usage();
                exit (0);
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }
    
    if (publicKeys)
        makeFilePath(publicKeys, publicKeysPath, BUFFER_SIZE);
    if (privateKey)
        makeFilePath(privateKey, privateKeyPath, BUFFER_SIZE);
    if (brokerKey)
        makeFilePath(brokerKey, brokerKeyPath, BUFFER_SIZE);
    
    if (!endpoint && !networkDevice){
        //we have no device to start with: try to find one
        char **devices = NULL;
        char **addresses = NULL;
        int nbD = 0;
        int nbA = 0;
        getNetdevicesList(&devices, &nbD);
        getNetaddressesList(&addresses, &nbA);
        assert(nbD == nbA);
        if (nbD == 1){
            //we have exactly one compliant network device available: we use it
            networkDevice = strdup(devices[0]);
            printf("using %s as default network device (this is the only one available)\n", networkDevice);
        }else if (nbD == 2 && (strcmp(addresses[0], "127.0.0.1") == 0 || strcmp(addresses[1], "127.0.0.1") == 0)){
            //we have two devices, one of which is the loopback
            //pick the device that is NOT the loopback
            if (strcmp(addresses[0], "127.0.0.1") == 0){
                networkDevice = strdup(devices[1]);
            }else{
                networkDevice = strdup(devices[0]);
            }
            printf("using %s as default network device (this is the only one available that is not the loopback)\n", networkDevice);
        }else{
            if (nbD == 0){
                printf("No network device found: aborting.\n");
            }else{
                printf("No network device passed as command line parameter and several are available.\n");
                printf("Please use one of these network devices:\n");
                for (int i = 0; i < nbD; i++){
                    printf("\t%s\n", devices[i]);
                }
                printf("\n");
                print_usage();
            }
            exit(EXIT_FAILURE);
        }
        freeNetdevicesList(devices, nbD);
        freeNetaddressesList(addresses, nbD);
    }
    
    if (useSecurity){
        if(zsys_file_exists(privateKeyPath)){
            cert = zcert_load(privateKeyPath);
            if (!cert){
                printf("'%s' does not contain a valid certificate\n", privateKeyPath);
                return EXIT_FAILURE;
            }
            if (strlen(publicKeysPath) == 0 || !zsys_file_exists(publicKeysPath)){
                printf("public keys path '%s' does not exist\n", publicKeysPath);
                return EXIT_FAILURE;
            }
            if (strlen(brokerKeyPath) > 0){
                if (!zsys_file_exists(brokerKeyPath)){
                    printf("broker key path '%s' does not exist\n", brokerKeyPath);
                    return EXIT_FAILURE;
                }else{
                    brokerCert = zcert_load(brokerKeyPath);
                    if (!brokerCert || !zcert_public_txt(brokerCert)){
                        printf("'%s' does not contain a valid public key\n", brokerKeyPath);
                        return EXIT_FAILURE;
                    }
                }
            }
        }else{
            printf("private key does not exist at '%s'\n", privateKeyPath);
            return EXIT_FAILURE;
        }
    }
    
    if (paramMessage != NULL){
        noninteractiveloop = true;
    }
    
    //init zyre
    zactor_t *actor = NULL;
    context_t *context = calloc(1, sizeof(context_t));
    if (!endpoint){
        assert(context);
        context->name = strdup(name);
        context->useGossip = false;
        actor = zactor_new (zyre_actor, context);
        assert (actor);
    }else{
        if (gossipconnect == NULL && gossipbind == NULL){
            printf("error : endpoint specified but no attached broker parameters, %s won't reach any other agent, aborting.", name);
            return EXIT_FAILURE;
        }else{
            assert(context);
            context->name = strdup(name);
            context->useGossip = true;
            actor = zactor_new (zyre_actor, context);
            assert (actor);
        }
    }
    
    if (noninteractiveloop){
        zloop_t *loop = zloop_new();
        if (paramMessage != NULL){
            zloop_timer(loop, 10, 1, stopMainThreadLoop, NULL);
        }
        zloop_start(loop);
        zloop_destroy(&loop);
    }else{
        //mainloop
        printf("%s is running...\nType /help for available commands\n", name);
        while (!zsys_interrupted) {
            char message [BUFFER_SIZE];
            char command[BUFFER_SIZE];
            char param1[BUFFER_SIZE];
            char param2[BUFFER_SIZE];
            int usedChar = 0;
            if (!fgets (message, BUFFER_SIZE, stdin))
                break;
            if ((message[0] == '/')&&(strlen (message) > 1)) {
                int matches = sscanf(message + 1, "%s %s%n%s", command, param1, &usedChar, param2);
                if (matches > 2) {
                    // copy the remaining of the message in param 2
                    strncpy(param2, message + usedChar + 2, BUFFER_SIZE);
                    // remove '\n' at the end
                    param2[strnlen(param2, BUFFER_SIZE) - 1] = '\0';
                }
                // Process command
                if (matches == 1) {
                    if (strcmp(command, "verbose") == 0){
                        zstr_sendx (actor, "VERBOSE", NULL);
                        
                    }else if (strcmp(command, "peers") == 0){
                        zstr_sendx (actor, "PEERS", NULL);
                        
                    }else if (strcmp(command, "channels") == 0){
                        zstr_sendx (actor, "CHANNELS", NULL);
                        
                    }else if (strcmp(command, "joinall") == 0){
                        zstr_sendx (actor, "JOINALL", NULL);
                        
                    }else if (strcmp(command, "leaveall") == 0){
                        zstr_sendx (actor, "LEAVEALL", NULL);
                        
                    }else if (strcmp(command, "agents") == 0){
                        zstr_sendx (actor, "AGENTS", NULL);
                        
                    }else if (strcmp(command, "help") == 0){
                        print_commands();
                        
                    }else if (strcmp(command, "quit") == 0){
                        zstr_sendx (actor, "$TERM", NULL);
                        break;
                    }
                }else if (matches == 2) {
                    //printf("Received command: %s + %s\n", command, param1);
                    if (strcmp(command, "join") == 0){
                        zstr_sendx (actor, "JOIN", param1, NULL);
                        
                    } else if (strcmp(command, "leave") == 0){
                        zstr_sendx (actor, "LEAVE", param1, NULL);
                        
                    }else if (strcmp(command, "whisperall") == 0){
                        zstr_sendx (actor, "WHISPERALL", param1, NULL);
                        
                    }else if (strcmp(command, "subscribe") == 0){
                        zstr_sendx (actor, "SUBSCRIBE", param1, NULL);
                        
                    }else if (strcmp(command, "log") == 0){
                        zstr_sendx (actor, "LOG", param1, NULL);
                        
                    }else if (strcmp(command, "unsubscribe") == 0){
                        zstr_sendx (actor, "UNSUBSCRIBE", param1, NULL);
                        
                    }else if (strcmp(command, "unlog") == 0){
                        zstr_sendx (actor, "UNLOG", param1, NULL);
                        
                    }else if (strcmp(command, "license") == 0){
                        zstr_sendx (actor, "LICENSE", param1, NULL);
                        
                    }else if (strcmp(command, "stop_peer") == 0){
                        zstr_sendx (actor, "STOP_PEER", param1, NULL);
                        
                    }else if (strcmp(command, "stop") == 0){
                        zstr_sendx (actor, "STOP_AGENT", param1, NULL);
                        
                    }
                }else if (matches == 3) {
                    //printf("Received command: %s + %s + %s\n", command, param1, param2);
                    if (strcmp(command, "whisper") == 0){
                        zstr_sendx (actor, "WHISPER", param1, param2, NULL);
                        
                    } else if (strcmp(command, "shout") == 0){
                        zstr_sendx (actor, "SHOUT", param1, param2, NULL);
                        
                    } else if (strcmp(command, "subscribe") == 0){
                        zstr_sendx (actor, "SUBSCRIBE_TO_OUTPUT", param1, param2, NULL);
                        
                    } else if (strcmp(command, "write") == 0){
                        zstr_sendx (actor, "WRITE", param1, param2, NULL);
                        
                    } else if (strcmp(command, "call") == 0){
                        zstr_sendx (actor, "CALL", param1, param2, NULL);
                        
                    } else if (strcmp(command, "start") == 0){
                        zstr_sendx (actor, "START_AGENT", param1, param2, NULL);
                        
                    }
                }else{
                    printf("Error: message returned %d matches (%s)\n", matches, message);
                }
            }
        }
    }
    zactor_destroy (&actor);

    #ifdef _WIN32
    zsys_shutdown();
    #endif

    return 0;
}
