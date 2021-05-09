#pragma once

class C2Json {
public:
    json_t* paramsToJson(const CompressorParmHolder& params);
    void jsonToParams(json_t* json, CompressorParmHolder* outParams);
};

inline json_t* C2Json::paramsToJson(const CompressorParmHolder& params) {
    json_t* rootJ = json_object();

  //  auto params = compressor->getParamHolder();
    json_t* attacks = json_array();
    json_t* releases = json_array();
    json_t* thresholds = json_array();
    json_t* makeups = json_array();
    json_t* enableds = json_array();
    json_t* wetdrys = json_array();
    json_t* ratios = json_array();
    for (int i = 0; i < 16; ++i) {
        json_array_append_new(attacks, json_real(params.getAttack(i)));
        json_array_append_new(releases, json_real(params.getRelease(i)));
        json_array_append_new(thresholds, json_real(params.getThreshold(i)));
        json_array_append_new(makeups, json_real(params.getMakeupGain(i)));
        json_array_append_new(enableds, json_boolean(params.getEnabled(i)));
        json_array_append_new(wetdrys, json_real(params.getWetDryMix(i)));
        json_array_append_new(ratios, json_integer(params.getRatio(i)));
    }
    json_object_set_new(rootJ, "attacks", attacks);
    json_object_set_new(rootJ, "releases", releases);
    json_object_set_new(rootJ, "thresholds", thresholds);
    json_object_set_new(rootJ, "makeups", makeups);
    json_object_set_new(rootJ, "enableds", enableds);
    json_object_set_new(rootJ, "wetdrys", wetdrys);
    json_object_set_new(rootJ, "ratios", ratios);

    return rootJ;
}

inline 
void C2Json::jsonToParams(json_t* rootJ, CompressorParmHolder* params) {
    json_t* attacksJ = json_object_get(rootJ, "attacks");
    json_t* releasesJ = json_object_get(rootJ, "releases");
    json_t* thresholdsJ = json_object_get(rootJ, "thresholds");
    json_t* makeupsJ = json_object_get(rootJ, "makeups");
    json_t* enabledsJ = json_object_get(rootJ, "enableds");
    json_t* ratiosJ = json_object_get(rootJ, "ratios");

    if (!json_is_array(attacksJ) || !json_is_array(releasesJ) || !json_is_array(thresholdsJ) ||
        !json_is_array(makeupsJ) || !json_is_array(enabledsJ) || !json_is_array(ratiosJ)) {
        WARN("parameter json malformed");
        assert(false);
        return;
    }
    if ((json_array_size(attacksJ) < 16) ||
        (json_array_size(releasesJ) < 16) ||
        (json_array_size(thresholdsJ) < 16) ||
        (json_array_size(makeupsJ) < 16) ||
        (json_array_size(enabledsJ) < 16) ||
        (json_array_size(ratiosJ) < 16)) {
        WARN("parameter json malformed2 %d", json_array_size(attacksJ));
        assert(false);
        return;
    }

  //  auto params = compressor->getParamHolder();
    for (int i = 0; i < 15; ++i) {
        auto value = json_array_get(attacksJ, i);
        params->setAttack(i, json_real_value(value));

        value = json_array_get(releasesJ, i);
        params->setRelease(i, json_real_value(value));

        value = json_array_get(thresholdsJ, i);
        params->setThreshold(i, json_real_value(value));

        value = json_array_get(makeupsJ, i);
        params->setMakeupGain(i, json_real_value(value));

        value = json_array_get(enabledsJ, i);
        params->setEnabled(i, json_boolean_value(value));

        value = json_array_get(ratiosJ, i);
        params->setRatio(i, json_integer_value(value));
    }
}
