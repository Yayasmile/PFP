#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "node-id.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/serial-line.h"
#include <stdio.h>

static const struct msg {

} ;

static const struct ping {

} ;

static const struct connection {

} ;

static struct broadcast_conn broadcast;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t * from);

static void sendPing(struct ping p) {

}

static const struct broadcast_callbacks broadcast_call = { broadcast_recv };