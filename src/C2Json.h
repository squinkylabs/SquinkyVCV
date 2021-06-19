#pragma once

#include "CompressorParamHolder.h"
#include "Squinky.hpp"
#include "jansson.h"

/**
 * JSON utilities to handle verious seiralization tasks for compressor 2
 */
class C2Json {
public:
    json_t* paramsToJson(const CompressorParamHolder& params, int schema);
    void jsonToParamsOrig(json_t* json, CompressorParamHolder* outParams);

    /**
     * @returns the schema number
     */
    int jsonToParams(json_t* json, CompressorParamHolder* outParams);
    bool jsonToParamsNew(json_t* json, CompressorParamHolder* outParams);

    void copyToClip(const CompressorParamChannel&);
    bool getClipAsParamChannel(CompressorParamChannel*);

private:
    const char* attack_ = "attack";
    const char* release_ = "release";
    const char* threshold_ = "threshold";
    const char* makeup_ = "makeup";
    const char* enabled_ = "enabled";
    const char* enabledSC_ = "enabledSC";
    const char* wetdry_ = "wetdry";
    const char* ratio_ = "ratio";
    const char* schema_ = "schema";
    const char* comp2_schema_ = "sq-compII";

    json_t* paramsToJsonOneChannel(const CompressorParamHolder& params, int channel);
    bool jsonToParamOneChannel(json_t* obj, CompressorParamHolder* outParams, int channel);
};

inline bool C2Json::getClipAsParamChannel(CompressorParamChannel* ch) {
    const char* jsonString = glfwGetClipboardString(APP->window->win);
    if (!jsonString) {
        INFO("nothing to paste");
        return false;
    }

    json_error_t error;
    json_t* obj = json_loads(jsonString, 0, &error);
    if (!obj) {
        INFO("data on clip not json us");
        return false;
    }

    json_t* schemaJ = json_object_get(obj, schema_);
    if (!schemaJ) {
        INFO("cllipboard json has no schema");
        return false;
    }
    std::string s = json_string_value(schemaJ);
    if (s != comp2_schema_) {
        INFO("clipboard schema mismatch");
        return false;
    }

    assert(CompressorParamHolder::getNumParams() == 8);
    json_t* attackJ = json_object_get(obj, attack_);
    json_t* releaseJ = json_object_get(obj, release_);
    json_t* thresholdJ = json_object_get(obj, threshold_);
    json_t* makeupJ = json_object_get(obj, makeup_);
    json_t* enabledJ = json_object_get(obj, enabled_);
    json_t* wetdryJ = json_object_get(obj, wetdry_);
    json_t* ratioJ = json_object_get(obj, ratio_);
    json_t* enabledSCJ = json_object_get(obj, enabledSC_);
    if (!attackJ || !releaseJ || !thresholdJ || !makeupJ || !enabledJ || !ratioJ || !wetdryJ || !enabledSCJ) {
        json_decref(obj);
        WARN("json schema mismatch");
        return false;
    }

    ch->attack = json_number_value(attackJ);
    ch->release = json_number_value(releaseJ);
    ch->threshold = json_number_value(thresholdJ);
    ch->makeupGain = json_number_value(makeupJ);
    ch->wetDryMix = json_number_value(wetdryJ);
    ch->enabled = json_boolean_value(enabledJ);
    ch->sidechainEnabled = json_boolean_value(enabledSCJ);
    ch->ratio = json_integer_value(ratioJ);

    json_decref(obj);
    return true;
}

inline void C2Json::copyToClip(const CompressorParamChannel& ch) {
    json_t* root = json_object();

    assert(CompressorParamHolder::getNumParams() == 8);
    json_object_set_new(root, attack_, json_real(ch.attack));
    json_object_set_new(root, release_, json_real(ch.release));
    json_object_set_new(root, threshold_, json_real(ch.threshold));
    json_object_set_new(root, makeup_, json_real(ch.makeupGain));
    json_object_set_new(root, wetdry_, json_real(ch.wetDryMix));
    json_object_set_new(root, enabled_, json_boolean(ch.enabled));
    json_object_set_new(root, enabledSC_, json_boolean(ch.sidechainEnabled));
    json_object_set_new(root, ratio_, json_integer(ch.ratio));
    json_object_set_new(root, schema_, json_string(comp2_schema_));

    char* clipJson = json_dumps(root, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
    glfwSetClipboardString(APP->window->win, clipJson);

    json_decref(root);  // we don't need this any more
    free(clipJson);
}

inline json_t* C2Json::paramsToJson(const CompressorParamHolder& params, int schema) {
    json_t* arrayJ = json_array();
    for (int i = 0; i < 16; ++i) {
        json_array_append_new(arrayJ, paramsToJsonOneChannel(params, i));
    }
    // 17th element is scheme int.
    json_array_append_new(arrayJ, json_integer(schema));
    return arrayJ;
}

inline json_t* C2Json::paramsToJsonOneChannel(const CompressorParamHolder& params, int channel) {
    json_t* objJ = json_object();
    assert(CompressorParamHolder::getNumParams() == 8);
    json_object_set_new(objJ, attack_, json_real(params.getAttack(channel)));
    json_object_set_new(objJ, release_, json_real(params.getRelease(channel)));
    json_object_set_new(objJ, threshold_, json_real(params.getThreshold(channel)));
    json_object_set_new(objJ, makeup_, json_real(params.getMakeupGain(channel)));
    json_object_set_new(objJ, wetdry_, json_real(params.getWetDryMix(channel)));
    json_object_set_new(objJ, enabled_, json_boolean(params.getEnabled(channel)));
    json_object_set_new(objJ, enabledSC_, json_boolean(params.getSidechainEnabled(channel)));
    json_object_set_new(objJ, ratio_, json_integer(params.getRatio(channel)));
    return objJ;
}

inline int C2Json::jsonToParams(json_t* json, CompressorParamHolder* outParams) {
     int schema = 0;
    bool b = jsonToParamsNew(json, outParams);
    if (!b) {
        jsonToParamsOrig(json, outParams);
    } else {
        INFO("will try to deser json schmea");
       
        json_t* obj = json_array_get(json, 16);
        if (obj && json_is_integer(obj)) {
            INFO("found it");
            schema = json_integer_value(obj);
        }
    }
    INFO("reutrning schema %d", schema);
    return schema;
}

inline bool C2Json::jsonToParamsNew(json_t* json, CompressorParamHolder* outParams) {
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

inline bool C2Json::jsonToParamOneChannel(json_t* obj, CompressorParamHolder* outParams, int channel) {
    assert(channel >= 0 && channel <= 15);

    json_t* wetdryJ = json_object_get(obj, wetdry_);
    json_t* attackJ = json_object_get(obj, attack_);
    json_t* releaseJ = json_object_get(obj, release_);
    json_t* thresholdJ = json_object_get(obj, threshold_);
    json_t* makeupJ = json_object_get(obj, makeup_);
    json_t* enabledJ = json_object_get(obj, enabled_);
    json_t* enabledSCJ = json_object_get(obj, enabledSC_);
    json_t* ratioJ = json_object_get(obj, ratio_);
    if (!attackJ || !releaseJ || !thresholdJ || !makeupJ || !enabledJ || !ratioJ || !wetdryJ || !enabledSCJ) {
        WARN("new channel deserialize failed");
        return false;
    }
    outParams->setAttack(channel, json_number_value(attackJ));
    outParams->setRelease(channel, json_number_value(releaseJ));
    outParams->setThreshold(channel, json_number_value(thresholdJ));
    outParams->setMakeupGain(channel, json_number_value(makeupJ));
    outParams->setEnabled(channel, json_boolean_value(enabledJ));
    outParams->setSidechainEnabled(channel, json_boolean_value(enabledSCJ));
    outParams->setRatio(channel, json_integer_value(ratioJ));
    outParams->setWetDry(channel, json_number_value(wetdryJ));
    return true;
}

// This is just for compatibility with old test versions
inline void C2Json::jsonToParamsOrig(json_t* rootJ, CompressorParamHolder* params) {
    json_t* attacksJ = json_object_get(rootJ, "attacks");
    json_t* releasesJ = json_object_get(rootJ, "releases");
    json_t* thresholdsJ = json_object_get(rootJ, "thresholds");
    json_t* makeupsJ = json_object_get(rootJ, "makeups");
    json_t* enabledsJ = json_object_get(rootJ, "enableds");
    json_t* ratiosJ = json_object_get(rootJ, "ratios");
    json_t* wetdryJ = json_object_get(rootJ, "wetdrys");

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
        value = json_array_get(wetdryJ, i);
        params->setWetDry(i, json_number_value(value));
        params->setSidechainEnabled(i, false);
        assert(CompressorParamHolder::getNumParams() == 8);
    }
}
