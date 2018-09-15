
#include <map>
#include <vector>
#include "asserts.h"


using Spline = std::vector< std::pair<double, double> >;


class NonUniformLookup
{
public:
    void add(double x, double y)
    {
        data[x] = y;
    }
    double lookup(double x)
    {
      //  printf("lookup %f\n", x);
        auto l = data.lower_bound(x);
        assert(l != data.end());
       // printf("lower = %f, %f\n", l->first, l->second);
        auto p = l;
        p--;
        if (p == data.end()) {
            assert(l->first == x);
            return l->second;
        }
        assert(p != data.end());
       // printf("p = %f, %f\n", p->first, p->second);

        // construct line  y = y0 +   (y1 -y0)/(x1 - x0) * x-x0;
        // = b + a(x -b);
        const double b = p->second;
        const double a = (l->second - p->second) / (l->first - p->first);

        const double ret = b + a * (x - p->first);
       // printf("ret = %f\n", ret);

        return ret;
    }
private:
    std::map<double, double> data;
};

class AsymGenerator
{
public:
    const static int iNumPoints = 256;
    const static int iSymmetryTables = 16;

    AsymGenerator(float symmetry)
    {

    }
    float lookup(float);

    static void genTableValues(const Spline& spline, int numPoints)
    {
        const double x0 = spline[0].first;

        // first build non-uniform lookup
        NonUniformLookup nu;
        const double delta = 1.0 / (numPoints * 8);     // let's oversample in t space
        for (double t = 0; t <= 1; t += delta) {
            auto pt = calcPoint(spline, t);
            //printf("adding point to table:%f, %f\n", pt.first, pt.second);
            nu.add(pt.first, pt.second);
        }

        // next output uniform
        for (int i = 0; i < numPoints; ++i) {
            double x = x0 + (double(i) / numPoints);
            double y = nu.lookup(x);
            printf("%f", y);
            if (i != numPoints - 1) {
                printf(", ");

                if ((i % 8) == 7) {
                    printf("\n");
                }
            }
        }
    }
    
    static void genTable(int index, double symetry)
    {
        printf("static float symmetry_table_%d[%d] = {\n", index, iNumPoints);

        genTableValues(makeSplineLeft(symetry), iNumPoints / 2);
        printf(",\n");
        genTableValues(makeSplineRight(symetry), iNumPoints / 2);
        printf("\n}\n");
        fflush(stdout); 
    }


    static Spline makeSplineRight(double symmetry)
    {
        Spline ret;
        ret.push_back(std::pair<double, double>(0.0, 0.0));
        ret.push_back(std::pair<double, double>(0.5, 1.0));
        ret.push_back(std::pair<double, double>(0.5, 1.0));
        ret.push_back(std::pair<double, double>(1.0, 1.0));
        return ret;
    }

    static Spline makeSplineLeft(double symmetry)
    {
        // symmetry from 0..1
        Spline ret;
        ret.push_back(std::pair<double, double>(-1, -symmetry));
        ret.push_back(std::pair<double, double>(-.5, -symmetry));
        ret.push_back(std::pair<double, double>(-.5, -symmetry));
        ret.push_back(std::pair<double, double>(0, 0));
        return ret;
    }

    static std::pair<double, double> calcPoint(const Spline& spline, double t)
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
        return ret;
    }
};

static void gen()
{
    for (int i = 0; i < AsymGenerator::iSymmetryTables; ++i) {
        float symmetry = float(i) / float(AsymGenerator::iSymmetryTables - 1);
        AsymGenerator::genTable(i, symmetry);
    }
}

static void testLook0()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(2.5);
    assertClose(x, 2.5, .0001);
}

static void testLook1()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 4);
    l.add(3, 3);

    double x = l.lookup(1.5);
    assertClose(x, 2.5, .0001);
}

static void testLook2()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(.5);
    assertClose(x, .5, .0001);
}

static void testLook3()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(2.5);
    assertClose(x, 2.5, .0001);
}

static void testLook4()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(0);
    assertClose(x, 0, .0001);
}

#if 0
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
#endif




void testSpline()
{
    testLook0();
    testLook1();
    testLook2();
    testLook3();
    testLook4();
   // test0();
    gen();
}