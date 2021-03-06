#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "node-id.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/serial-line.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static const struct msg {
    uint8_t type;
    uint8_t connID;
    uint8_t srcID;
    uint8_t destID;
    uint8_t nextNodeID;
    uint8_t hopCnt;
    uint8_t msgID;
    char text[50];
} msg;

static const struct ping {
    uint8_t type;
    uint8_t connID;
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    uint8_t hopCnt;
    uint8_t msgID;
    uint8_t cost;
} ping;

static const struct revPing {
    uint8_t type;
    uint8_t connID;
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    uint8_t hopCnt;
    uint8_t msgID;
    uint8_t nextNodeID;
} revPing;

static const struct connection {
    uint8_t connID;
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    uint8_t nextNodeID;
} connection;

static const struct header {
    uint8_t type;
    uint8_t connID;
    uint8_t srcID;
    uint8_t destID;
    uint8_t prevNodeID;
    uint8_t hopCnt;
    uint8_t msgID;
} header;

// * global variables
// ***********************************************
static struct broadcast_conn broadcast;
static struct ping pingList[pingListSize];
static struct connection connList[connListSize];



// * functions
// ***********************************************

static void establishConn(uint8_t destID);
static void sendMsg(uint8_t connID,char* text);

// broadcast
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t * from);

// ping
static struct ping createPing(uint8_t destID);
static void processPing(struct ping p);
static bool isDuplicate(struct ping p);
static void regPing(struct ping p);
static void pingOut(struct ping p);

// revPing
static struct revPing createRevPing(struct ping p);
static void processRevPing(struct revPing rp);
static void revPingOut(struct revPing rp);

// connection
static void regConn(struct revPing rp);
static bool connEstablished(uint8_t destID);
static uint8_t getConnIDbyDest(uint8_t destID);
static struct connection getConnByID(uint8_t connID);

// message
static struct msg createMsg(uint8_t destID, char* text);
static void processMsg(struct msg m);
static void msgOut(struct msg m);

static void waitRand();