#pragma once

/**
 * Base class for composites embeddable in a VCV Widget
 * This is used for "real" implementations
 */
class WidgetComposite
{
public:
    WidgetComposite(Module * parent) : 
        inputs(parent->inputs),
        outputs(parent->outputs),
<<<<<<< HEAD
        params(parent->params),
        lights(parent->lights)
    {
        printf("In widget init, #Lights=%lld\n", lights.size());
=======
        params(parent->params)
    {
>>>>>>> master
    }
protected:
    std::vector<Input>& inputs;
    std::vector<Output>& outputs;
    std::vector<Param>& params;
<<<<<<< HEAD
    std::vector<Light>& lights;
=======
>>>>>>> master

};
