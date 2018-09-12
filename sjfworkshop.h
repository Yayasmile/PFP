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

} ;

static const struct ping {

} ;

static const struct revPing {

} ;

static const struct nodeTrace {

} ;

static const struct connection {

} ;

static struct broadcast_conn broadcast;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t * from);
static const struct broadcast_callbacks broadcast_call = { broadcast_recv };
static struct ping createPing(uint8_t destID);
static struct ping createMsg(uint8_t destID);
static void pingIn(struct ping p);
static void pingOut(struct ping p);
static void revPingIn(struct revPing rP);
static void revPingOut(struct revPing rP);
static bool isDuplicate(struct ping p);
static void msgIn(struct msg m);
static void msgOut(struct msg m);
static void regConn(struct revPing rP);