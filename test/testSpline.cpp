
#include <vector>

using Spline = std::vector< std::pair<double, double> >;


/*

 Function that take input as Control Point x_coordinates and
Control Point y_coordinates and draw bezier curve
void bezierCurve(int x[], int y[])
{
    double xu = 0.0, yu = 0.0, u = 0.0;
    int i = 0;
    for (u = 0.0; u <= 1.0; u += 0.0001) {
        xu = pow(1 - u, 3)*x[0] + 3 * u*pow(1 - u, 2)*x[1] + 3 * pow(u, 2)*(1 - u)*x[2]
            + pow(u, 3)*x[3];
        yu = pow(1 - u, 3)*y[0] + 3 * u*pow(1 - u, 2)*y[1] + 3 * pow(u, 2)*(1 - u)*y[2]
            + pow(u, 3)*y[3];
        SDL_RenderDrawPoint(renderer, (int) xu, (int) yu);
    }
}
*/
static void renderPt(const Spline& spline, double t)
{
    std::pair<double, double> ret;

    ret.first = pow(1 - t, 3) * spline[0].first +
        3 * t * pow(1 - t, 2) * spline[1].first +
        3 * pow(t, 2) * (1 - t) * spline[2].first
        + pow(t, 3) * spline[3].first;

    ret.second = pow(1 - t, 3) * spline[0].second +
        3 * t * pow(1 - t, 2) * spline[1].second +
        3 * pow(t, 2) * (1 - t) * spline[2].second
        + pow(t, 3) * spline[3].second;

    printf("%f, %f\n", ret.first, ret.second);
}

static void render(const Spline& spline)
{
    double t;
    const double delta = .1;
    for (t = 0; t <= 1; t += delta) {
        renderPt(spline, t);
    }

}

static void test0()
{
    Spline x = {
        {0.0, 0.0},
        {.5, .5},
        {.5, .5},
        {1.0, .5}
    };
    render(x);
}


void testSpline()
{
    test0();
}