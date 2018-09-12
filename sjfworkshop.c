#include "sjfworkshop.h"

/*---------------------------------------------------------------------------*/
PROCESS(main_proc, "MAIN PROCESS");
AUTOSTART_PROCESSES(&main_proc);
/*---------------------------------------------------------------------------*/

// * MAIN FUNCTION
// *******************************************

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_EXITHANDLER()
    PROCESS_BEGIN();

    // initialize
    broadcast_open(&broadcast, 129, &broadcast_call);

    while(1) {
        // add program logic
        PROCESS_YIELD();packetbuf_copyto(&dm_rcv);
    PROCESS_END();
}


static void establishConn(uint8_t destID) {

}

static void sendMsg(uint8_t destID, struct msg m) {

}


// * broadcast
// ***********************************************

// called when data is received
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t * from) {
    static uint8_t typeHeader;
    packetbuf_copyto(&typeHeader);

    switch (typeHeader) {
        case 1: // ping
            static struct ping p;
            packetbuf_copyto(&p);
            processPing(p);
            break;
        case 2: // revPing
            static struct revPing rp;
            packetbuf_copyto(&rp);
            processRevPing(rp);
            break;
        case 10: // message
            static struct msg m;
            packetbuf_copyto(&m);
            processMsg(m);
            break;
        default:
            printf("ERR: INVALID typeHeader", );
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
    return p;
}

// takes raw broadcast input, interprets as ping and ingores/registers/forwards
static void processPing(struct ping p) {

}

// returns whether given ping has already been received
static bool isDuplicate(struct ping p) {

}

// process ping for forwarding and sends it
static void pingOut(struct ping p) {

}


// * revPing
// ***********************************************

// returns revPing-struct with values
static struct revPing createRevPing(struct ping p) {

}

// takes raw broadcast input, interprets as revPing and registers conn/forwards
static void processRevPing(struct revPing rp) {

}

// process revPing for forwarding and sends it
static void revPingOut(struct revPing rp) {

}


// * connection
// ***********************************************

// registers connection with revPing given
static void regConn(struct revPing rp) {

}

// returns whether connection to destination has been establisched
static bool connEstablished(uint8_t destID) {

}


// * message
// ***********************************************

// returns message-struct with values given
static struct msg createMsg(uint8_t destID, char* text) {

}

// takes raw broadcast input and interprets it as message
static void processMsg(static struct msg m){

}

// sends message
static void msgOut(struct msg m) {

}
