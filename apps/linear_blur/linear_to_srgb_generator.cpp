#include "Halide.h"

namespace {

class LinearTosRGB : public Halide::Generator<LinearTosRGB> {
public:
    Input<Func>  linear{"linear"};
    Output<Func> srgb{"srgb"};

    void generate() {
        using Halide::_;
        Var x("x"), y("y"), yi("yi");

        srgb(x, y, _) = select(linear(x, y, _) <= .0031308f,
                         linear(x, y, _) * 12.92f,
                         (1 + .055f) * pow(linear(x, y, _), 1.0f / 2.4f) - .055f);

        if (auto_schedule) {
            const int W = 1536, H = 2560, C = 4;
            // Wart: Input<Func> are defined with Vars we don't know.
            // Might be x,y but might be _0,_1. Use the args() to work around.
            linear.estimate(linear.args()[0], 0, W)
                  .estimate(linear.args()[1], 0, H);
            for (int i = 2; i < linear.args().size(); ++i) {
                linear.estimate(linear.args()[i], 0, C);
            }
            srgb.estimate(x, 0, W)
                .estimate(y, 0, H);
            for (int i = 2; i < srgb.args().size(); ++i) {
                srgb.estimate(srgb.args()[i], 0, C);
            }
        } else {
            srgb.split(y, y, yi, 8).parallel(y).vectorize(x, 8);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LinearTosRGB, linear_to_srgb)
