/*
 This file is part of micronau.
 Copyright (c) 2013 - David Smitley
 
 Permission is granted to use this software under the terms of the GPL v2 (or any later version)
 
 Details can be found at: www.gnu.org/licenses
*/

#ifndef __PLUGINEDITOR_H_74E5CE11__
#define __PLUGINEDITOR_H_74E5CE11__

#include "../JuceLibraryCode/JuceHeader.h"
#include "micronau.h"
#include "gui/MicronSlider.h"
#include "gui/LcdComboBox.h"
#include "gui/MicronToggleButton.h"
#include "gui/MicronTabBar.h"
#include "gui/SliderBank.h"
#include <vector>
#include <list>

class LcdLabel;
class StdComboBox;
class SliderBank;
class LcdTextEditor;

class ext_slider : public MicronSlider
{
public:
    ext_slider(MicronauAudioProcessor *owner, int nrpn_num) : plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
        setRange (param->getMin(), param->getMax(), 1);
        setDoubleClickReturnValue(true, param->getDefaultValue());
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
	// this setRange override adjusts mouse drag sensitivity so that smaller ranges are more sensitive than larger ranges.
	void setRange (double newMin, double newMax, double newInt) {
            MicronSlider::setRange (newMin, newMax, newInt);
            setMouseDragSensitivity( 20.0*(4.0+log10(newMax - newMin)) );
    }
//    void mouseDoubleClick(const MouseEvent& event);
	const IonSysexParam* getInternalParam() { return param; }
	
private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
};

class ext_combo : public LcdComboBox
{
public:
    ext_combo(MicronauAudioProcessor *owner, int nrpn_num) : plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
 
        vector<ListItemParameter> list_items = param->getList();
        if (list_items.size() != 0) {
            vector<ListItemParameter>::const_iterator i;
            for (int i = 0; i != list_items.size(); i++) {
                addItem(list_items[i].getName(), i+1);
            }
        } else {
            addItem("--", 1000);
            for (int i = param->getMin(); i < param->getMax(); i++) {
                addItem(String(i), i+1);
            }
        }
        nrpn = nrpn_num;
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    int get_min() { return param->getMin();}
    int get_max() { return param->getMax();}
    int get_nrpn() {return nrpn;}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
    vector<ListItemParameter> & get_list_item_names() {return param->getList();}
	const IonSysexParam* getInternalParam() { return param; }

private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
    int nrpn;
};

class ext_button : public MicronToggleButton
{
public:
    ext_button(MicronauAudioProcessor *owner, int nrpn_num, LookAndFeel *lf) : MicronToggleButton(""), plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
        if (lf) {
            setLookAndFeel(lf);
        }
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
    
private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
};

class back_label : public Label
{
public:
    back_label(String s, int x, int y, int w, int h) : Label() {
        setText(s, dontSendNotification);
        setFont (Font ("Arial", 12.00f, Font::bold));
        setBounds(x, y, w, h);
        setJustificationType(Justification::centred);
    }
};

//==============================================================================
class MicronauAudioProcessorEditor  : public AudioProcessorEditor,
										public AudioProcessorListener,
                                        public SliderListener,
                                        public ButtonListener,
                                        public ComboBoxListener,
                                        public TextEditorListener,
                                        public MouseListener,
                                        public Timer
{
public:
    MicronauAudioProcessorEditor (MicronauAudioProcessor* ownerFilter);
    ~MicronauAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics& g);
    void timerCallback();
    void sliderValueChanged (Slider* slider);
	void sliderDragStarted (Slider* slider);
	void sliderDragEnded (Slider* slider);
	void mouseDown(const MouseEvent& event);
    KeyboardFocusTraverser* createFocusTraverser();
    void buttonClicked (Button* button);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void textEditorTextChanged (TextEditor &t);
	void textEditorFocusLost (TextEditor &t);
    void addSlider(ext_slider *s) {sliders.add(s);}
    void audioProcessorParameterChanged (AudioProcessor* processor, int parameterIndex, float newValue) { paramHasChanged = true; }
	void audioProcessorChanged (AudioProcessor* processor) { paramHasChanged = true; }

private:

	// layout of all the major component groups
	enum
	{
		// group box margins
		GROUP_BOX_MARGIN_X = 10,
		GROUP_BOX_MARGIN_Y = 20,

		// top and left edges
		LEFT_X = 15,
		TOP_Y = 20,

		// component groups...
		
		MODMAT_X = LEFT_X,
		MODMAT_Y = TOP_Y,
		MODMAT_W = 755,
		MODMAT_H = 100,

		LCD_Y = MODMAT_Y-14,
		LCD_H = 34,

		LOGO_X = 788,
		LOGO_Y = LCD_Y + 2,
		LOGO_W = 68,
		LOGO_H = 32,
		
		MIDI_X = 910,
		MIDI_IN_Y = LCD_Y + 45,
		MIDI_OUT_Y = MIDI_IN_Y + 20,
		MIDI_H = 15,

		SYNC_X = 875,
		SYNC_Y = MIDI_OUT_Y + 20,

		PROG_NAME_Y = SYNC_Y + 32,

		OSCS_X = MODMAT_X,
		OSCS_Y = 145,
		OSCS_W = 210,
		OSCS_H = 200,

		ENVS_X = 460,
		ENVS_Y = 315,
		ENVS_W = 400,
		ENVS_H = 200,

		PREFILT_X = 255,
		PREFILT_Y = OSCS_Y,
		PREFILT_W = 140,
		PREFILT_H = 250,

		FILT_X = 420,
		FILT_Y = PREFILT_Y,
		FILT_W = 230,
		FILT_H = 145,

		POSTFILT_X = 680,
		POSTFILT_Y = FILT_Y,
		POSTFILT_W = 200,
		POSTFILT_H = FILT_H,

		LFO_X = PREFILT_X,
		LFO_Y = 440,
		LFO_W = 185,
		LFO_H = 225,
		
		FM_X = OSCS_X,
		FM_Y = 377,
		FM_W = OSCS_W,
		FM_H = 76,

		VOICE_X = FM_X,
		VOICE_Y = FM_Y + 107,
		VOICE_W = FM_W,
		VOICE_H = 77,

		PORTA_X = VOICE_X,
		PORTA_Y = VOICE_Y + 107,
		PORTA_W = VOICE_W,
		PORTA_H = 55,
		
		XYZ_X = 900,
		XYZ_Y = PROG_NAME_Y+95,
		XYZ_W = 150,
		XYZ_H = 70,

		OUTPUT_X = XYZ_X,
		OUTPUT_Y = XYZ_Y + 97,
		OUTPUT_W = XYZ_W,
		OUTPUT_H = 135,

		RANDOMIZER_X = 930,
		RANDOMIZER_Y = OUTPUT_Y + 160,
		RANDOMIZER_W = 120,
		RANDOMIZER_H = 70,

        FX_X = ENVS_X,
        FX_Y = ENVS_Y + 245,
        FX_W = 440,
        FX_H = 105
	};

    void add_knob(int nrpn, int x, int y, const char *text, Component *parent);
    void add_box(int nprn, int x, int y, int width, const char *text, int loc, Component *parent);
    void add_button(int nrpn, int x, int y, const char *text, bool invert, Component *parent);

	void add_group_box(const String& labelText, int x, int y, int w, int h);
	void add_label(const String& labelText, int x, int y, int w, int h);

	Button* create_guibutton(int x, int y, bool wantMicronButton = false); // create a gui button not associated with an nrpn
	MicronSlider* create_guiknob(int x, int y, const char *text); // create a gui knob not associated with an nrpn

    void create_osc(int x, int y);
    void create_prefilt(int x, int y);
    void create_postfilt(int x, int y);
    void create_mod(int x, int y);
    void create_filter(int x, int y);
    void create_env(int x, int y);
    void create_fm(int x, int y);
    void create_voice(int x, int y);
    void create_portamento(int x, int y);
    void create_xyz(int x, int y);
    void create_output(int x, int y);
    void create_lfo(int x, int y);
	void create_fx_and_tracking_tabs(int x, int y);
    void create_fx1(int x, int y, Component* parent);
    void create_fx2(int x, int y, Component* parent);
	void create_tracking(int x, int y, Component* parent);

	void create_randomizer(int x, int y);
	void randomizeParams();

	void updateGuiComponents();
    void update_tracking();
    void update_midi_menu(int in_out, bool init);

    void select_item_by_name(int in_out, String nm);

	ext_combo* findBoxWithNrpn(int nrpn);

	Image background;
	Image buttonOffImg;
	Image buttonHoverImg;
	Image buttonOnImg;

    ScopedPointer<Drawable> logo;
    ScopedPointer<LookAndFeel> inverted_button_lf;
    OwnedArray<GroupComponent> group_boxes;

    ScopedPointer<Button> sync_nrpn;
    ScopedPointer<Button> sync_sysex;
    ScopedPointer<Button> request;
    ScopedPointer<Button> undo_button;
    ScopedPointer<Button> redo_button;

    ScopedPointer<ComboBox> midi_in_menu;
    ScopedPointer<ComboBox> midi_out_menu;
    ScopedPointer<ComboBox> midi_out_chan;

    ScopedPointer<LcdLabel> param_display;
    ScopedPointer<LcdTextEditor> prog_name;

    MicronauAudioProcessor *owner;
	bool paramHasChanged; // using this flag to avoid repeatedly updating program name which interferes with editing of the name

	ScopedPointer<MicronTabBar> mod_tabs;
	ScopedPointer<MicronTabBar> fx_and_tracking_tabs;
    ScopedPointer<Component> fx1[7];
    ScopedPointer<Component> fx2[7];
	ScopedPointer<SliderBank> trackgen;

	OwnedArray<Label> labelComponents;

    // prototype
    OwnedArray<ext_slider> sliders;
    OwnedArray<ext_combo> boxes;
    OwnedArray<ext_button> buttons;

	// randomizer stuff
	ScopedPointer<Button> randomizeButton;
	ScopedPointer<Button> randomizeLockPitchButton;
	ScopedPointer<MicronSlider>	randomizeAmtSlider;
	Random randGen;

	// undo/redo stuff
	struct Snapshot
	{
		std::string			progname_value;
		std::vector<double>	slider_values;
		std::vector<int>	box_values;
		std::vector<bool>	button_values;
	};
	
	void takeUndoSnapshot();
	void restorePreviousUndoSnapshot(bool redo = false);
	bool canUndo();
	bool canRedo();

	std::list<Snapshot>	undo_history;
	std::list<Snapshot>::iterator undo_cur;
	bool				allowNewSnapshots;
};


#endif  // __PLUGINEDITOR_H_74E5CE11__
