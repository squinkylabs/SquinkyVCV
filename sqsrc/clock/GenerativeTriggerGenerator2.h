#pragma once

#include "AudioMath.h"
#include "StochasticProductionRule.h"
#include "StochasticGrammar2.h"
#include "TriggerSequencer.h"


/* Knows how to generate trigger sequence data
 * when evaluating a grammar
 */

class GTGEvaluator2 : public StochasticProductionRule::EvaluationState
{
public:
    GTGEvaluator2(AudioMath::RandomUniformFunc xr, TriggerSequencer::Event * buf) :
        StochasticProductionRule::EvaluationState(xr),
        _buf(buf),
        _delay(0)
    {
    }

   // void writeSymbol(int fakeGKey) override
    void writeSymbol(const StochasticNote& note) override
    {
        // first: write out a trigger at "current delay"
        _buf->evt = TriggerSequencer::TRIGGER;
        _buf->delay = _delay;
        ++_buf;

        // then set current delay to duration of key
      //  _delay = ProductionRuleKeys::getDuration(key);
     //assert(false);    // what do I do (above)
        _delay = note.duration;
        SQINFO("will delay %d. is that correct?", _delay);
    }

    // call this to write final event
    void writeEnd()
    {
        _buf->evt = TriggerSequencer::END;
        _buf->delay = _delay;
    }
private:
    TriggerSequencer::Event * _buf;
    int _delay;
};



/* wraps up some stochastic gnerative grammar stuff feeding
 * a trigger sequencer
 */
class GenerativeTriggerGenerator2
{
public:
    GenerativeTriggerGenerator2(AudioMath::RandomUniformFunc r, StochasticGrammarPtr grammar) :
        _r(r),
        _grammar(grammar)
    {
        _data[0].delay = 0;
        _data[0].evt = TriggerSequencer::END;
        _seq = new TriggerSequencer(_data);
    }
    ~GenerativeTriggerGenerator2()
    {
        delete _seq;
    }


    void setGrammar(StochasticGrammarPtr grammar)
    {
        _grammar = grammar;
    //    _numRules = numRules;
     //   _initKey = initialState;
    }

    // returns true if trigger generated
    bool clock()
    {
        SQINFO("clock");
        _seq->clock();
        bool ret = _seq->getTrigger();
        if (_seq->getEnd()) {
              SQINFO("get end");
            // when we finish playing the seq, generate a new random one
            generate();
            ret |= _seq->getTrigger();
            //printf("this should be getTrigger!!!\n");
        }
        return ret;
    }
private:
    TriggerSequencer * _seq;
    TriggerSequencer::Event _data[33];
    AudioMath::RandomUniformFunc _r;
    StochasticGrammarPtr _grammar;      // who owns this? bts
    void generate()
    {
        SQINFO("gen 98");
        GTGEvaluator2 es(_r, _data);
        es.grammar = _grammar;
        SQINFO("gmr = %p", _grammar.get());
        auto baseRule = es.grammar->getRootRule();
        StochasticProductionRule::evaluate(es, baseRule);      // let's assume 0 is the initial key
  SQINFO("gen 103");
        es.writeEnd();
        TriggerSequencer::isValid(_data);
        _seq->reset(_data);
        assert(!_seq->getEnd());
#if 0
        printf("just generated trigger seq\n");
        TriggerSequencer::Event * p;
        for (p = _data; p->evt != TriggerSequencer::END; ++p) {
            printf("evt=%d, delay=%d\n", p->evt, p->delay);
        }
        printf("delay to end = %d\n", p->delay);
#endif
  SQINFO("gen 116");
    }
};
