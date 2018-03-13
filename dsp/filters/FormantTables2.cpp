
#include <assert.h>
#include "FormantTables2.h"


static const float freqLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels] = {
   // model = 0(bass)
    {
        // F1 a
        //0=a,1=e,2=i, 3=o 4=u
        {600, 400, 250, 400, 350},
        // F2 
        {1040, 1620, 1750, 750, 600},
        // F3 
        {2250, 2400, 2600, 2400, 2400},
        // F4 
        {2450, 2800, 3050, 2600, 2675},
        //F5 
        {2750, 3100, 3340, 2900, 2950}
    },
    //1(tenor)
    {
        // F1
        //0=a,1=e,2=i, 3=o 4=u
        {650, 400, 290, 400, 350 },
        // F2
        {1080, 1700, 1870, 800, 600 },
        // F3
        {2650, 2600, 2800, 2600, 2700},
        // F4
        {2900, 3200, 3250, 2800, 2900},
        //F5
        {3250, 3850, 3540, 3000, 3300}
    },
    //2(countertenor)
    {
        // F1
        //0=a,1=e,2=i, 3=o 4=u
        {660, 440, 270, 430, 370},
        // F2
        {1120, 1800, 1850, 820, 630},
        // F3
        {2750, 2700, 2900, 2700, 2750},
        // F4
        {3000, 3000, 3350, 3000,  3000},
        //F5
        {3350, 3300, 3590, 3300, 3400}
    },
    //3(alto)
    {
        // F1     
        //0=a,1=e,2=i, 3=o 4=u
        {800, 400, 350, 450, 325},
        // F2
        {1150, 1600, 1700, 800, 700},
        // F3
        {2800, 2700, 2700, 2830, 2530},
        // F4
        {3500, 3300, 3700, 3500, 3500},
        //F5
        {4950, 4950, 4950, 4950, 4950}
    },
    // 4(soprano)
    {
        // F1
        //0=a,1=e,2=i, 3=o 4=u
        {800, 350, 270, 450,  325},
        // F2
        {1150, 2000,  2140, 800, 700},
        // F3
        {2900, 2800, 2950, 2830, 2700},
        // F4
        {3900, 3600, 3900, 3800, 3800},
        //F5
        {4950, 4950, 4950, 4950, 4950}
    }
};

static const float bwLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels] = {
    // model = 0(bass)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
    //1(tenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        // F5
        {}
    },
    //2(countertenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
    //3(alto)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
// 4(soprano)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    }
};

static const float gainLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels] = {
    // model = 0(bass)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
    //1(tenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
    //2(countertenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
    //3(alto)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    },
    // 4(soprano)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {},
        // F2
        {},
        // F3
        {},
        // F4
        {},
        //F5
        {}
    }
};

FormantTables2::FormantTables2()
{

    for (int model = 0; model < numModels; ++model) {
        for (int formantBand = 0; formantBand < numFormantBands; ++formantBand) {

            LookupTableParams<float>& fparams = freqInterpolators[model][formantBand];
            const float  *values = freqLookup[model][formantBand];
            // todo: take log
            LookupTable<float>::initDiscrete(fparams, numVowels, values);

            LookupTableParams<float>& bwparams = bwInterpolators[model][formantBand];
            values = bwLookup[model][formantBand];
            // todo: take log
            LookupTable<float>::initDiscrete(bwparams, numVowels, values);

            LookupTableParams<float>& gparams = gainInterpolators[model][formantBand];
            values = gainLookup[model][formantBand];
            // todo: take log
            LookupTable<float>::initDiscrete(gparams, numVowels, values);
        }
    }
}

float FormantTables2::getLogFrequency(int model, int formantBand, float vowel)
{
    assert(model >= 0 && model <= numModels);
    assert(formantBand >= 0 && formantBand <= numFormantBands);
    assert(vowel >= 0 && vowel < numVowels);

    LookupTableParams<float>& params = freqInterpolators[model][formantBand];
    return LookupTable<float>::lookup(params, vowel);
}

float FormantTables2::getNormalizedBandwidth(int model, int formantBand, float vowel)
{
    assert(model >= 0 && model <= numModels);
    assert(formantBand >= 0 && formantBand <= numFormantBands);
    assert(vowel >= 0 && vowel < numVowels);

    LookupTableParams<float>& params = bwInterpolators[model][formantBand];
    return LookupTable<float>::lookup(params, vowel);
}
float FormantTables2::getGain(int model, int formantBand, float vowel)
{
    assert(model >= 0 && model <= numModels);
    assert(formantBand >= 0 && formantBand <= numFormantBands);
    assert(vowel >= 0 && vowel < numVowels);

    LookupTableParams<float>& params = gainInterpolators[model][formantBand];
    return LookupTable<float>::lookup(params, vowel);
}