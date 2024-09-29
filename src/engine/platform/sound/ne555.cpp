// lol 555 emulator
// nope im not emulating pin 5

#include <stdio.h>

struct ne555_passives {
    float ra;
    float rb;
    double c; // in milifarads for uh
              // why would you 1F cap on a 555?
};

class ne555 {
    private:
        ne555_passives passives;
        float t1;
        float t2;
        double time;
    public:
        bool out, reset;
        void set_passives(float ra, float rb, double c) {
            passives.ra = ra;
            passives.rb = rb;
            passives.c = c/1000.0f;

            t1=0.693f*(passives.ra+passives.rb)*passives.c;
            t2=0.693f*passives.rb*passives.c;
        };
        void tick(double timeDelta) {
            if (!reset) {
                time=0.0f;
                return;
            }
            time+=timeDelta;
            if (out) {
                if (time>t1) {
                    out=false;
                    time=0.0f;
                }
            } else {
                if (time>t2) {
                    out=true;
                    time=0.0f;
                }
            }
        };
        ne555(float ra, float rb, double c) {
            set_passives(ra, rb, c);
            time=0.0f;
        };
};