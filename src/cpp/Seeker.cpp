
#include <stdlib.h>
#include "Seeker.h"
#include "easing.h"

Seeker::Seeker(IClock *clock, ICompleter *completer, IProxy<int,2> *proxy,
               ISeekerTarget *target, Vector2i seeker_start_xy, float speed) :
    Ticker(clock, completer),
    target(target),
    proxy(proxy),
    speed(speed),
    last_xy(seeker_start_xy),
    last_tick_time(0),
    pause_start_time(0) {
}

Seeker::~Seeker() {
    delete proxy;
    delete target;
}

void Seeker::start(Uint32 now) {
    Ticker::start(now);
    last_tick_time = now;
}

void Seeker::stop() {
    Ticker::stop();
}

void Seeker::pause(Uint32 now) {
    Ticker::pause(now);
    pause_start_time = now;
}

void Seeker::resume(Uint32 now) {
    Ticker::resume(now);
    last_tick_time += now - pause_start_time;
    pause_start_time = 0;
}

void Seeker::on_tick(Uint32 now) {
    Vector2i target_xy = target->get_target_xy();
    float    last_dist = distance(target_xy, last_xy);
    if (last_dist <= 1) {
        stop();
        on_complete(now);
        return;
    }

    Uint32   delta_t = now - last_tick_time;
    float    ratio   = speed * ((float) delta_t) / last_dist;
    Vector2i next_xy = last_xy + (target_xy - last_xy) * ratio;
    proxy->update(next_xy);

    float next_dist = distance(target_xy, next_xy);
    if (next_dist <= 1) {
        stop();
        on_complete(now);
        return;
    }

    Vector2i last_diff = target_xy - last_xy;
    Vector2i next_diff = target_xy - next_xy;

    int ldx = last_diff[0];
    int ldy = last_diff[1];
    int ndx = next_diff[0];
    int ndy = next_diff[1];

    /* we could overshoot the target, in which case dir vector changes sign */
    if (
        (
            ((ldx > 0 && ndx < 0) || (ldx < 0 && ndx > 0)) &&
            ((ldy > 0 && ndy < 0) || (ldy < 0 && ndy > 0))
        ) ||
        (
            ((ldx == 0) && (ndx == 0) && (ldy*ndy < 0)) ||
            ((ldy == 0) && (ndy == 0) && (ldx*ndx < 0))
        )
    ) {
        stop();
        on_complete(now);
        // maybe update proxy now with target xy?
        return;
    }

    last_xy        = next_xy;
    last_tick_time = now;
}


