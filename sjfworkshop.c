#include "sjfworkshop.h"

/*---------------------------------------------------------------------------*/
PROCESS(main_proc, "MAIN PROCESS");
AUTOSTART_PROCESSES(&main_proc);
/*---------------------------------------------------------------------------*/

static const struct broadcast_callbacks broadcast_call = { broadcast_recv };
static uint8_t connIDCounter = 0;
static uint8_t msgIDCounter = 0;
static struct etimer et; // timer


// * MAIN FUNCTION
// *******************************************

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_EXITHANDLER()
    PROCESS_BEGIN();
    leds_init();
    serial_line_init();
    static uint8_t destID = 0;
    static char* msgText;
    static bool doRepeat = false;

    // initialize
    broadcast_open(&broadcast, 129, &broadcast_call);

    while(1) {
        PROCESS_YIELD(); // get destID
        if(ev==serial_line_event_message) {
            destID = atoi((char *) data);
        }
        PROCESS_YIELD(); // get message
        if(ev==serial_line_event_message) {
            msgText = (char *) data;
        }

        if(strcmp(msgText, "send") == 0) {
            msgText = "tled";
            doRepeat = true;
        } else {
            doRepeat = false;
        }

        do {
            if (!connEstablished(destID)) { // ping 5 times
                static uint8_t i;
                static uint8_t pt;
                pt = pingTries;
                for(i = 0; i < pt; i++) {
                    establishConn(destID);
                    etimer_set(&et, CLOCK_SECOND * pingWaitTime);
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
                    if (connEstablished(destID)) {
                        printf("Established connection to node %d after %d try/tries\n", destID, i+1);
                        sendMsg(getConnIDbyDest(destID), msgText);
                        break;
                    }
                }

                if (!connEstablished(destID)) {
                    printf("Couldn't establish connection to node %d after %d tries\n", destID, pt);
                }

            } else { // send message
                sendMsg(getConnIDbyDest(destID), msgText);
            }
            if(doRepeat) {
                etimer_set(&et, CLOCK_SECOND * 1.0); // set the timer to 1s
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
            }
        } while (doRepeat);

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

    // printf("[RXB %d %d %d %d %d]\n", node_id, h.destID, h.srcID, h.msgID, h.hopCnt);

    static struct ping p;
    static struct revPing rp;
    static struct msg m;

    switch (h.type) {
        case 1: // ping
            // printf("RXB ping\n");
            packetbuf_copyto(&p);
            processPing(p);
            break;
        case 2: // revPing
            // printf("RXB revPing\n");
            packetbuf_copyto(&rp);
            processRevPing(rp);
            break;
        case 10: // message
            // printf("RXB msg\n");
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
    p.msgID = 0;
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
            // printf("ping reached dest, createRevPing\n");
            struct revPing rp = createRevPing(p);
            regConn(rp);
            waitRand();
            revPingOut(rp);
        }else{
            // printf("ping forwarded\n");
            p.prevNodeID = node_id;
            p.hopCnt++;
            waitRand();
            pingOut(p);
        }

    } else {
        // printf("ping ignored\n");
    }
}

static void regPing(struct ping p) {
    // printf("ping registered\n");
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

    // printf("[TXB %d %d %d %d %d] ping\n", node_id, p.destID, p.srcID, p.msgID, p.hopCnt);
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
    rp.msgID = 0;
    rp.hopCnt = 1;
    rp.connID = p.connID;
    return rp;
}

// takes raw broadcast input, interprets as revPing and registers conn/forwards
static void processRevPing(struct revPing rp) {
    if (rp.srcID == node_id) {
        // printf("revPing reached srcID, conn %d established\n", rp.connID);
        regConn(rp);
    } else if (rp.prevNodeID == node_id) {
        // printf("processing/forwarding revPing\n");
        regConn(rp);
        rp.nextNodeID = node_id;
        rp.prevNodeID = connList[0].prevNodeID;
        rp.hopCnt ++;
        revPingOut(rp);
    } else {
        // printf("revPing ignored\n");
    }
}

// process revPing for forwarding and sends it
static void revPingOut(struct revPing rp) {
    packetbuf_copyfrom(&rp,sizeof(rp));
    broadcast_send(&broadcast);

    // printf("[TXB %d %d %d %d %d] revPing\n", node_id, rp.destID, rp.srcID, rp.msgID, rp.hopCnt);
}


// * connection
// ***********************************************

// registers connection with revPing given
static void regConn(struct revPing rp) {
    // printf("conn %d registered\n", rp.connID);
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
    leds_toggle(LEDS_RED);
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
    msgIDCounter ++;
    message.msgID = msgIDCounter;
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
        if(strcmp(m.text, "tled")==0) {
            leds_toggle(LEDS_BLUE);
        }
    } else if (connEstablished(m.destID) && m.nextNodeID == node_id){
        leds_toggle(LEDS_RED);
        // printf("forward msg\n");
        static struct connection c;
        c = getConnByID(m.connID);
        m.hopCnt++;
        m.nextNodeID = c.nextNodeID;
        msgOut(m);
        leds_toggle(LEDS_RED);
    } else {
        // printf("ignored msg\n");
    }
}

// sends message
static void msgOut(struct msg m) {
    packetbuf_copyfrom(&m,sizeof(m));
    broadcast_send(&broadcast);

    //printf("[TXB %d %d %d %d %d] msg\n", node_id, m.destID, m.srcID, m.msgID , m.hopCnt);
}

static void waitRand(){
    static int i;
    for (i = rand()%100; i>1;i--) {
        printf("*");
    }
    printf("\n");
}
