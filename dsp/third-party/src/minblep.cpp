
namespace rack {
namespace dsp {


// do nothing one to satisfy linker
void minBlepImpulse(int z, int o, float* output) {
    int n = 2 * z * o;
    for (int i = 0; i < n; ++n) {
        output[i] = 0;
    }
}

 }
}