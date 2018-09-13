#include "sjfworkshop.h"

/*---------------------------------------------------------------------------*/
PROCESS(main_proc, "MAIN PROCESS");
AUTOSTART_PROCESSES(&main_proc);
/*---------------------------------------------------------------------------*/

static const struct broadcast_callbacks broadcast_call = { broadcast_recv };
static uint8_t connIDCounter = 0;


// * MAIN FUNCTION
// *******************************************

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_EXITHANDLER()
    PROCESS_BEGIN();

    // initialize
    broadcast_open(&broadcast, 129, &broadcast_call);

    while(1) {
        PROCESS_YIELD();

        if (strcmp((char*)data, "ping")==0) {
            printf("pinging 3\n");
            establishConn(destination);
        }
        if (strcmp((char*)data, "msg")==0) {
            printf("messaging 3\n");
            sendMsg(connIDCounter, "Hello Node " + destination);
        }
    }
    PROCESS_END();
}

static void establishConn(uint8_t destID) {
    static struct ping p;
    p = createPing(destID);
    regPing(p);
    pingOut(p);
}

static void sendMsg(uint8_t connID,char* text) {
    static struct msg m;
    m = createMsg(connID, text);
    msgOut(m);
}


// * broadcast
// ***********************************************

// called when data is received
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t * from) {
    static struct header h;
    packetbuf_copyto(&h);

    printf("[RXB %d %d %d %d %d]\n", node_id, h.destID, h.srcID, h.connID, h.hopCnt);

    static struct ping p;
    static struct revPing rp;
    static struct msg m;

    switch (h.type) {
        case 1: // ping
            printf("RXB ping\n");
            packetbuf_copyto(&p);
            processPing(p);
            break;
        case 2: // revPing
            printf("RXB revPing\n");
            packetbuf_copyto(&rp);
            processRevPing(rp);
            break;
        case 10: // message
            printf("RXB msg\n");
            packetbuf_copyto(&m);
            processMsg(m);
            break;
        default:
            printf("ERR: INVALID typeHeader\n");
    }
}


// * ping
// ***********************************************

// returns ping-struct with values
static struct ping createPing(uint8_t destID) {
    static struct ping p;
    p.type = 1;
    p.srcID = node_id;
    p.destID = destID;
    p.prevNodeID = node_id;
    p.cost = 1;
    p.hopCnt = 1;
    connIDCounter ++;
    p.connID = connIDCounter;
    return p;
}

// takes raw broadcast input, interprets as ping and ingores/registers/forwards
static void processPing(struct ping p) {
    if (!isDuplicate(p)) {
        regPing(p);
        if(p.destID == node_id){
            printf("ping reached dest, createRevPing\n");
            struct revPing rp = createRevPing(p);
            regConn(rp);
            revPingOut(rp);
        }else{
            printf("ping forwarded\n");
            p.prevNodeID = node_id;
            p.hopCnt++;
            waitRand();
            pingOut(p);
        }

    } else {
        printf("ping ignored\n");
    }
}

static void regPing(struct ping p) {
    printf("ping registered\n");
    static uint8_t i;
    for(i = 1; i<pingListSize; i++) {
        pingList[pingListSize-i] = pingList[pingListSize-i-1];
    }

    pingList[0] = p;
}

// returns whether given ping has already been received
static bool isDuplicate(struct ping p) {
    static uint8_t i;
    for(i = 0; i<pingListSize;i++) {
        if(p.srcID == pingList[i].srcID && p.connID == pingList[i].connID){
            return true;
        }
    }
    return false;
}

// process ping for forwarding and sends it
static void pingOut(struct ping p) {
    packetbuf_copyfrom(&p,sizeof(p));
    broadcast_send(&broadcast);

    printf("[RXT %d %d %d %d %d] ping\n", node_id, p.destID, p.srcID, p.connID, p.hopCnt);
}


// * revPing
// ***********************************************

// returns revPing-struct with values
static struct revPing createRevPing(struct ping p) {
    static struct revPing rp;
    if (p.destID != node_id) {
        printf("ERR: destID != node_id in createRevPing()\n");
    }
    rp.type = 2;
    rp.srcID = p.srcID;
    rp.destID = p.destID;
    rp.nextNodeID = p.destID;
    rp.prevNodeID = p.prevNodeID; 
    rp.hopCnt = 1;
    rp.connID = p.connID;
    return rp;
}

// takes raw broadcast input, interprets as revPing and registers conn/forwards
static void processRevPing(struct revPing rp) {
    if (rp.srcID == node_id) {
        printf("revPing reached srcID, conn %d established\n", rp.connID);
        regConn(rp);
    } else if (rp.prevNodeID == node_id) {
        printf("processing/forwarding revPing\n");
        regConn(rp);
        rp.nextNodeID = node_id;
        rp.prevNodeID = connList[0].prevNodeID;
        rp.hopCnt ++;
        revPingOut(rp);
    } else {
        printf("revPing ignored\n");
    }
}

// process revPing for forwarding and sends it
static void revPingOut(struct revPing rp) {
    packetbuf_copyfrom(&rp,sizeof(rp));
    broadcast_send(&broadcast);

    printf("[RXT %d %d %d %d %d] revPing\n", node_id, rp.destID, rp.srcID, rp.connID, rp.hopCnt);
}


// * connection
// ***********************************************

// registers connection with revPing given
static void regConn(struct revPing rp) {
    printf("conn %d registered\n", rp.connID);
    leds_toggle(LEDS_RED);
    static uint8_t i;
    bool isInPingList = false;

    for(i = 0; i<pingListSize; i++) {
        if(rp.connID == pingList[i].connID && rp.srcID == pingList[i].srcID) {
            isInPingList = true;
            break;
        }
    }

    if (isInPingList) {
        static struct connection c;
        c.connID = rp.connID;
        c.srcID = rp.srcID;
        c.destID = rp.destID;
        c.nextNodeID = rp.nextNodeID;
        c.prevNodeID = pingList[i].prevNodeID;

        for(i = 1; i<connListSize; i++) {
            connList[connListSize-i] = connList[connListSize-i-1];
        }

        connList[0] = c;
    } else {
        printf("ERR: unexpected revPing in regConn\n");
    }
}

// returns whether connection to destination has been established
static bool connEstablished(uint8_t destID) {
    return (getConnIDbyDest(destID)!=0);
}

static uint8_t getConnIDbyDest(uint8_t destID) {
    static uint8_t i;

    for(i = 0; i<connListSize; i++) {
        if (connList[i].destID == destID) {
            return connList[i].connID;
        }
    }
    return 0;
}

static struct connection getConnByID(uint8_t connID) {
    static uint8_t i;

    for(i = 0; i<connListSize; i++) {
        if (connList[i].connID == connID) {
            return connList[i];
        }
    }
    printf("ERR: connection not found in getConnByID\n");

    static struct connection c;
    return c;
}


// * message
// ***********************************************

// returns message-struct with values given
static struct msg createMsg(uint8_t connID, char* text) {
    static struct msg message;
    static struct connection c;
    c = getConnByID(connID);

    if (c.connID == 0) {
        printf("ERR: no connection established in createMsg()\n");
    }

    message.type = 10;
    message.connID = c.connID;
    message.srcID = c.srcID;
    message.destID = c.destID;
    message.nextNodeID = c.nextNodeID;
    message.hopCnt = 1;
    sprintf(message.text, text);

    if (c.connID != connID || node_id != c.srcID) {
        printf("ERR: c.connID != connID || node_id != c.srcID in createMsg()\n");
    }

    return message;
}

// takes raw broadcast input and interprets it as message
static void processMsg(struct msg m){
    if (m.destID == node_id) {
        printf("***MESSAGE***\n* %s\n*************\n",m.text);
    } else if (connEstablished(m.destID) && m.nextNodeID == node_id){
        printf("forward msg\n");
        static struct connection c;
        c = getConnByID(m.connID);
        m.hopCnt++;
        m.nextNodeID = c.nextNodeID;
        msgOut(m);
    } else {
        printf("ignored msg\n");
    }
}

// sends message
static void msgOut(struct msg m) {
    packetbuf_copyfrom(&m,sizeof(m));
    broadcast_send(&broadcast);

    printf("[TXB %d %d %d %d %d] msg\n", node_id, m.destID, m.srcID, m.connID, m.hopCnt);
}

static void waitRand(){
    static int i;
    for (i = rand()%100; i>1;i--) {
        printf("*");
    }
    printf("\n");
}
