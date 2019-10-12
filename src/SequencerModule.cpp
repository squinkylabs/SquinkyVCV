
#include <iostream>
#include "Squinky.hpp"

#ifdef _SEQ
#include "DrawTimer.h"
#include "NewSongDataCommand.h"
#include "WidgetComposite.h"
#include "Seq.h"
#include "seq/SeqSettings.h"
#include "seq/NoteDisplay.h"
#include "seq/AboveNoteGrid.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/ToggleButton.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqToggleLED.h"

#include "seq/SequencerSerializer.h"
#ifndef _USERKB
#include "MidiKeyboardHandler.h"
#endif
#include "MidiLock.h"
#include "MidiSong.h"
#include "../test/TestSettings.h"
#include "TimeUtils.h"
#include "MidiFileProxy.h"
#include "SequencerModule.h"
#include <osdialog.h>

#ifdef _TIME_DRAWING
static DrawTimer drawTimer("Seq");
#endif

using Comp = Seq<WidgetComposite>;

SequencerModule::SequencerModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
    runStopRequested = false;
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    ISeqSettings* ss = new SeqSettings(this);
    std::shared_ptr<ISeqSettings> _settings( ss);
    seqComp = std::make_shared<Comp>(this, song);
    sequencer = MidiSequencer::make(song, _settings, seqComp->getAuditionHost());
}

static const char* helpUrl = "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/sq2.md";

struct SequencerWidget : ModuleWidget
{
    SequencerWidget(SequencerModule *);

    void appendContextMenu(Menu *theMenu) override 
    { 
        ::rack::ui::MenuLabel *spacerLabel = new ::rack::ui::MenuLabel(); 
        theMenu->addChild(spacerLabel); 
        ManualMenuItem* manual = new ManualMenuItem("Seq++ manual", helpUrl); 
        theMenu->addChild(manual);  

        SqMenuItem* midifile = new SqMenuItem(
            []() { return false; },
            [this]() { this->loadMidiFile(); }
        );
        midifile->text = "Load midi file";
        theMenu->addChild(midifile); 

        SqMenuItem* midifileSave = new SqMenuItem(
            []() { return false; },
            [this]() { this->saveMidiFile(); }
        );
        midifileSave->text = "Save midi file";
        theMenu->addChild(midifileSave); 
    }

    void loadMidiFile();
    void saveMidiFile();

    /**
     * Helper to add a text label to this widget
     */
#ifdef _LAB
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

     Label* addLabelLeft(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->alignment = Label::LEFT_ALIGNMENT;
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
#endif

    void step() override;

    NoteDisplay* noteDisplay = nullptr;
    AboveNoteGrid* headerDisplay = nullptr;
    ToggleButton*  scrollControl = nullptr;
    SequencerModule* _module = nullptr;

    void addJacks(SequencerModule *module);
    void addControls(SequencerModule *module, std::shared_ptr<IComposite> icomp);
    void addStepRecord(SequencerModule *module);
    void toggleRunStop(SequencerModule *module);

#ifdef _TIME_DRAWING
    // Seq: avg = 399.650112, stddev = 78.684572 (us) Quota frac=2.397901
    void draw(const DrawArgs &args) override
    {
        DrawLocker l(drawTimer);
        ModuleWidget::draw(args);
    }
#endif
};

#if 0
std::string _removeFileName(const std::string s, std::vector<char> separators)
{
    // find the eerything up to and including the last separator
    for (char separator : separators) {
        auto pos = s.rfind(separator);
        if (pos != std::string::npos) {
            return s.substr(0, pos+1);
        }
    }

    // if we didn't find any separators, then use empty path
    return"";   
}

// windows experiment

std::string removeFileName(const std::string s)
{
#ifdef ARCH_WIN
    return _removeFileName(s, {'\\', ':'});
#else
    return _removeFileName(s, {'/'});
#endif
}
#endif

void SequencerWidget::saveMidiFile()
{
    static const char SMF_FILTERS[] = "Standard MIDI file (.mid):mid";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename = "Untitled.mid";

    std::string dir = _module->sequencer->context->settings()->getMidiFilePath();

	sqDEFER({
		osdialog_filters_free(filters);
	});

	char* pathC = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
  
	if (!pathC) {
		// Fail silently
		return;
	}
    std::string pathStr = pathC;;
	sqDEFER({
		std::free(pathC);
	});

	if (::rack::string::filenameExtension(::rack::string::filename(pathStr)) == "") {
		pathStr += ".mid";
	}

    // TODO: add on file extension
    // TODO: save folder
    bool b = MidiFileProxy::save(_module->sequencer->song, pathStr.c_str());
    if (!b) {
        sqWARN("unable to write midi file to %s", pathStr.c_str());
    } else {
        std::string fileFolder = rack::string::directory(pathStr);
        _module->sequencer->context->settings()->setMidiFilePath(fileFolder);
    }
}

void SequencerWidget::loadMidiFile()
{
    static const char SMF_FILTERS[] = "Standard MIDI file (.mid):mid";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    std::string dir = _module->sequencer->context->settings()->getMidiFilePath();

	sqDEFER({
		osdialog_filters_free(filters);
	});

	char* pathC = osdialog_file(OSDIALOG_OPEN, dir.c_str(), filename.c_str(), filters);
  
	if (!pathC) {
		// Fail silently
		return;
	}
	sqDEFER({
		std::free(pathC);
	});

    MidiSongPtr song = MidiFileProxy::load(pathC);

    std::string temp(pathC);
    std::string fileFolder = rack::string::directory(temp);
    if (song) {
        _module->postNewSong(song, fileFolder);
    }  
}

void SequencerWidget::step()
 {
    ModuleWidget::step();

    // Advance the scroll position
    if (scrollControl && _module && _module->isRunning()) {
        
        const int y = scrollControl->getValue();
        if (y) {
            float curTime = _module->getPlayPosition();
            if (y == 2) {
                auto curBar = TimeUtils::time2bar(curTime);
                curTime = TimeUtils::bar2time(curBar);
            }
            auto seq = _module->getSeq();
            seq->editor-> advanceCursorToTime(curTime, false);
        }
    }

    // give this guy a chance to do some processing on the UI thread.
    if (_module) {
#ifdef _USERKB
        noteDisplay->onUIThread(_module->seqComp, _module->sequencer);
#else
        MidiKeyboardHandler::onUIThread(_module->seqComp, _module->sequencer);
#endif
    }
}

void SequencerWidget::toggleRunStop(SequencerModule *module)
{
    module->toggleRunStop();
}

void sequencerHelp()
{
    SqHelper::openBrowser(helpUrl);
}

SequencerWidget::SequencerWidget(SequencerModule *module) : _module(module)
{
    setModule(module);
    if (module) {
        module->widget = this;
    }
    // was 14, before 8
    // 8 for panel, 28 for notes
    const int panelWidthHP = 8;
    const int width = (panelWidthHP + 28) * RACK_GRID_WIDTH; 
    box.size = Vec(width, RACK_GRID_HEIGHT);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setPanel(this, "res/seq_panel.svg");
    box.size.x = width;     // restore to the full width that we want to be
    {
        const float topDivider = 60;
        const float x = panelWidthHP * RACK_GRID_WIDTH;
        const float width = 28 * RACK_GRID_WIDTH;
        const Vec notePos = Vec(x, topDivider);
        const Vec noteSize = Vec(width, RACK_GRID_HEIGHT - topDivider);

        const Vec headerPos = Vec(x, 0);
        const Vec headerSize = Vec(width, topDivider);

        MidiSequencerPtr seq;
        if (module) {
            seq = module->sequencer;
        } else {
            // make enough of a sequence to render
            MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
            std::shared_ptr<TestSettings> ts = std::make_shared<TestSettings>();
            std::shared_ptr<ISeqSettings> _settings = std::dynamic_pointer_cast<ISeqSettings>(ts);
            seq = MidiSequencer::make(song, _settings, nullptr);
        }
        headerDisplay = new AboveNoteGrid(headerPos, headerSize, seq);
        noteDisplay = new NoteDisplay(notePos, noteSize, seq, module);
        addChild(noteDisplay);
        addChild(headerDisplay);
    }

    addControls(module, icomp);
    addJacks(module);
    addStepRecord(module);
 
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void SequencerWidget::addControls(SequencerModule *module, std::shared_ptr<IComposite> icomp)
{
    const float controlX = 20;

    float y = 50;
#ifdef _LAB
    addLabelLeft(Vec(controlX - 4, y),
        "Clock rate");
#endif
    y += 20;
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(controlX, y),
        module,
        Comp::CLOCK_INPUT_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getClockRates());
    addParam(p);

    y += 28;
#ifdef _LAB
    addLabelLeft(Vec(controlX - 4, y),
        "Polyphony");
#endif
    y += 20;
    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(controlX, y),
        module,
        Comp::NUM_VOICES_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getPolyLabels());
    addParam(p);
   
    y += 28;
    const float yy = y;
#ifdef _LAB
    addLabel(Vec(controlX - 8, y),
        "Run");
#endif
    y += 20;
    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(controlX, y),
        module,
        Seq<WidgetComposite>::RUN_STOP_LIGHT));
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    tog->setHandler( [this, module](bool ctrlKey) {
        this->toggleRunStop(module);
    });
    addChild(tog);
   
    y = yy;
    float controlDx = 40;

    {
#ifdef _LAB
    addLabel(
        Vec(controlX + controlDx - 8, y),
        "Scroll");
#endif
    y += 20;
    scrollControl = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(controlX + controlDx, y),
        module,
        Comp::PLAY_SCROLL_PARAM);
    scrollControl->addSvg("res/square-button-01.svg");
    scrollControl->addSvg("res/square-button-02.svg");
    addParam(scrollControl);
    }
}

void SequencerWidget::addStepRecord(SequencerModule *module)
{
    const float jacksDx = 40;
    const float jacksX = 20;
    const float jacksY = 230;
    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY),
        module,
        Comp::CV_INPUT));  

     addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY),
        module,
        Comp::GATE_INPUT));  
}

void SequencerWidget::addJacks(SequencerModule *module)
{
    const float jacksY1 = 286-2;
    const float jacksY2 = 330+2;
    const float jacksDx = 40;
    const float jacksX = 20;
#ifdef _LAB
    const float labelX = jacksX - 20;
    const float dy = -32;
#endif

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY1),
        module,
        Comp::CLOCK_INPUT));
#ifdef _LAB
    addLabel(
        Vec(3 + labelX + 0 * jacksDx, jacksY1 + dy),
        "Clk");
#endif

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY1),
        module,
        Comp::RESET_INPUT));
#ifdef _LAB
    addLabel(
        Vec(-4 + labelX + 1 * jacksDx, jacksY1 + dy),
        "Reset");
#endif

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 2 * jacksDx, jacksY1),
        module,
        Comp::RUN_INPUT));
#ifdef _LAB
    addLabel(
        Vec(labelX + 1 + 2 * jacksDx, jacksY1 + dy),
        "Run");
#endif

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jacksX, jacksY2),
        module,
        Seq<WidgetComposite>::CV_OUTPUT));
#ifdef _LAB
    addLabel(
        Vec(labelX+2, jacksY2 + dy),
        "CV");
#endif

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY2),
        module,
        Seq<WidgetComposite>::GATE_OUTPUT));
#ifdef _LAB
    addLabel(
        Vec(labelX + 1 * jacksDx, jacksY2 + dy),
        "Gate");
#endif
    addChild(createLight<MediumLight<GreenLight>>(
        Vec(jacksX + 2 * jacksDx -6 , jacksY2 -6),
        module,
        Seq<WidgetComposite>::GATE_LIGHT));
}

void SequencerModule::dataFromJson(json_t *data)
{
    MidiSequencerPtr newSeq = SequencerSerializer::fromJson(data, this);
    setNewSeq(newSeq);
}

void SequencerModule::setNewSeq(MidiSequencerPtr newSeq)
{
    MidiSongPtr oldSong = sequencer->song;
    sequencer = newSeq;
    if (widget) {
        widget->noteDisplay->setSequencer(newSeq);
        widget->headerDisplay->setSequencer(newSeq);
    }

    {
        // Must lock the songs when swapping them or player 
        // might glitch (or crash).
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(sequencer->song->lock);
        seqComp->setSong(sequencer->song);
    }
}

void SequencerModule::postNewSong(MidiSongPtr newSong, const std::string& fileFolder)
{
    auto updater = [](bool set, MidiSequencerPtr seq, MidiSongPtr newSong, SequencerWidget* widget) {

        assert(widget);
        assert(seq);
        assert(newSong);
        if (set && seq) {
            seq->selection->clear();        // clear so we aren't pointing to notes from prev seq
            seq->setNewSong(newSong);
        }

        if (!set && widget) {
            widget->noteDisplay->songUpdated();
            widget->headerDisplay->songUpdated();
        }
    };

    NewSongDataDataCommandPtr cmd = NewSongDataDataCommand::makeLoadMidiFileCommand(newSong, updater);
    sequencer->undo->execute(sequencer, widget, cmd);
    sequencer->context->settings()->setMidiFilePath(fileFolder);
}

void SequencerModule::onReset()
{
    Module::onReset();
    std::shared_ptr<MidiSong> newSong = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    ISeqSettings* ss = new SeqSettings(this);
    std::shared_ptr<ISeqSettings> _settings( ss);
    MidiSequencerPtr newSeq  = MidiSequencer::make(newSong, _settings, seqComp->getAuditionHost());
    setNewSeq(newSeq);
}

Model *modelSequencerModule = 
    createModel<SequencerModule, SequencerWidget>("squinkylabs-sequencer");

#endif