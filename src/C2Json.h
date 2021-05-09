#pragma once

#include "CompressorParamHolder.h"
#include "jansson.h"

class C2Json {
public:
    json_t* paramsToJson(const CompressorParmHolder& params);
    void jsonToParamsOrig(json_t* json, CompressorParmHolder* outParams);
    void jsonToParams(json_t* json, CompressorParmHolder* outParams);
    bool jsonToParamsNew(json_t* json, CompressorParmHolder* outParams);

    void copyToClip(const CompressorParamChannel&);
    bool getClipAsParamChannel(CompressorParamChannel*);

private:
    const char* attack_ = "attack";
    const char* release_ = "release";
    const char* threshold_ = "threshold";
    const char* makeup_ = "makeup";
    const char* enabled_ = "enabled";
    const char* wetdry_ = "wetdry";
    const char* ratio_ = "ratio";

    json_t* paramsToJsonOneChannel(const CompressorParmHolder& params, int channel);
    bool jsonToParamOneChannel(json_t* obj, CompressorParmHolder* outParams, int channel);
};

inline json_t* C2Json::paramsToJson(const CompressorParmHolder& params) {
    json_t* arrayJ = json_array();
    for (int i = 0; i < 16; ++i) {
        json_array_append_new(arrayJ, paramsToJsonOneChannel(params, i));
    }
#if 0
    {
    char* s = json_dumps(arrayJ, 0);
     INFO("json: %s", s);
    }
#endif
    return arrayJ;
}

inline json_t* C2Json::paramsToJsonOneChannel(const CompressorParmHolder& params, int channel) {
    json_t* objJ = json_object();
    json_object_set_new(objJ, attack_, json_real(params.getAttack(channel)));
    json_object_set_new(objJ, release_, json_real(params.getRelease(channel)));
    json_object_set_new(objJ, threshold_, json_real(params.getThreshold(channel)));
    json_object_set_new(objJ, makeup_, json_real(params.getMakeupGain(channel)));
    json_object_set_new(objJ, wetdry_, json_real(params.getRelease(channel)));
    json_object_set_new(objJ, enabled_, json_boolean(params.getEnabled(channel)));
    json_object_set_new(objJ, ratio_, json_integer(params.getRatio(channel)));
    return objJ;
}

inline void C2Json::jsonToParams(json_t* json, CompressorParmHolder* outParams) {
#if 0
    WARN("JSON to params");
    char* p = json_dumps(json, 0);
    INFO("json: %s", p);
    free(p);
#endif
    bool b = jsonToParamsNew(json, outParams);
    if (!b) {
        jsonToParamsOrig(json, outParams);
    }
}

inline bool C2Json::jsonToParamsNew(json_t* json, CompressorParmHolder* outParams) {
    if (!json_is_array(json)) {
        WARN("JSON not array, can't be cur");
        return false;
    }
    for (int i = 0; i < 16; ++i) {
        json_t* obj = json_array_get(json, i);
        if (!obj) {
            WARN("array missing elements");
            return false;
        }
        bool b = jsonToParamOneChannel(obj, outParams, i);
        if (!b) {
            return false;
        }
    }
    return true;
}

inline bool C2Json::jsonToParamOneChannel(json_t* obj, CompressorParmHolder* outParams, int channel) {
    assert(channel >= 0 && channel <= 15);

    json_t* attackJ = json_object_get(obj, attack_);
    json_t* releaseJ = json_object_get(obj, release_);
    json_t* thresholdJ = json_object_get(obj, threshold_);
    json_t* makeupJ = json_object_get(obj, makeup_);
    json_t* enabledJ = json_object_get(obj, enabled_);
    json_t* ratioJ = json_object_get(obj, ratio_);
    if (!attackJ || !releaseJ || !thresholdJ || !makeupJ || !enabledJ || !ratioJ) {
        WARN("new channel deserialize failed");
        json_decref(attackJ);
        json_decref(releaseJ);
        json_decref(thresholdJ);
        json_decref(makeupJ);
        json_decref(enabledJ);
        json_decref(ratioJ);
        return false;
    }
    outParams->setAttack(channel, json_real_value(attackJ));
    outParams->setRelease(channel, json_number_value(releaseJ));
    outParams->setThreshold(channel, json_number_value(thresholdJ));
    outParams->setMakeupGain(channel, json_number_value(makeupJ));
    outParams->setEnabled(channel, json_boolean_value(enabledJ));
    outParams->setRatio(channel, json_integer_value(ratioJ));
    return true;
}

inline void C2Json::jsonToParamsOrig(json_t* rootJ, CompressorParmHolder* params) {
    WARN("jsonToParamsOrig");
    json_t* attacksJ = json_object_get(rootJ, "attacks");
    json_t* releasesJ = json_object_get(rootJ, "releases");
    json_t* thresholdsJ = json_object_get(rootJ, "thresholds");
    json_t* makeupsJ = json_object_get(rootJ, "makeups");
    json_t* enabledsJ = json_object_get(rootJ, "enableds");
    json_t* ratiosJ = json_object_get(rootJ, "ratios");

    if (!json_is_array(attacksJ) || !json_is_array(releasesJ) || !json_is_array(thresholdsJ) ||
        !json_is_array(makeupsJ) || !json_is_array(enabledsJ) || !json_is_array(ratiosJ)) {
        WARN("orig parameter json malformed");
        return;
    }
    if ((json_array_size(attacksJ) < 16) ||
        (json_array_size(releasesJ) < 16) ||
        (json_array_size(thresholdsJ) < 16) ||
        (json_array_size(makeupsJ) < 16) ||
        (json_array_size(enabledsJ) < 16) ||
        (json_array_size(ratiosJ) < 16)) {
        WARN("orig parameter json malformed2 %d", json_array_size(attacksJ));
        return;
    }

    for (int i = 0; i < 15; ++i) {
        auto value = json_array_get(attacksJ, i);
        params->setAttack(i, json_real_value(value));

        value = json_array_get(releasesJ, i);
        params->setRelease(i, json_number_value(value));

        value = json_array_get(thresholdsJ, i);
        params->setThreshold(i, json_number_value(value));

        value = json_array_get(makeupsJ, i);
        params->setMakeupGain(i, json_number_value(value));

        value = json_array_get(enabledsJ, i);
        params->setEnabled(i, json_boolean_value(value));

        value = json_array_get(ratiosJ, i);
        params->setRatio(i, json_integer_value(value));
    }
}

#if 0
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
#endif