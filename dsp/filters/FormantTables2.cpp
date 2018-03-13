
#include <assert.h>
#include "FormantTables2.h"


static const float freqLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels];
static const float bwLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels];
static const float gainLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels];

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