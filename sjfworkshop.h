#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "node-id.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/serial-line.h"
#include <stdio.h>
#include <stdbool.h>

static const struct msg {
    uint8_t type;
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    char msg[50];
} ;

static const struct ping {
    uint8_t type;
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    uint8_t cost;
} ;

static const struct revPing {
    uint8_t type;
    uint8_t srcID;
    uint8_t destID;
    uint8_t nextNodeID;
} ;

static const struct connection {
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    uint8_t nextNodeID;
} ;

// * global variables
// ***********************************************
static struct broadcast_conn broadcast;
static const struct broadcast_callbacks broadcast_call = { broadcast_recv };
static struct ping pingList[10];
static struct connection connList[5];

// * functions
// ***********************************************

static void establishConn(uint8_t destID);
static void sendMsg(uint8_t destID, struct msg m);

// broadcast
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t * from);

// ping
static struct ping createPing(uint8_t destID);
static void processPing(char* data);
static bool isDuplicate(struct ping p);
static void pingOut(struct ping p);

// revPing
static struct revPing createRevPing(struct ping p);
static void processRevPing(char* data);
static void revPingOut(struct revPing rp);

// connection
static void regConn(struct revPing rp);
static bool connEstablished(uint8_t destID);

// message
static struct msg createMsg(uint8_t destID, char* text);
static void processMsg(char* data);
static void msgOut(struct msg m);
