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
        PROCESS_YIELD();
        if(ev==serial_line_event_message) {

        }
    }
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

}


// * ping
// ***********************************************

// returns ping-struct with values
static struct ping createPing(uint8_t destID) {

}

// takes raw broadcast input, interprets as ping and ingores/registers/forwards
static void processPing(char* data) {

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
static void processRevPing(char* data) {

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
static void processMsg(char* data) {

}

// sends message
static void msgOut(struct msg m) {

}
