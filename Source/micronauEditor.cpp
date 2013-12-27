/*
 This file is part of micronau.
 Copyright (c) 2013 - David Smitley
 
 Permission is granted to use this software under the terms of the GPL v2 (or any later version)
 
 Details can be found at: www.gnu.org/licenses
*/

#include "micronau.h"
#include "micronauEditor.h"
#include "gui/MicronSlider.h"
#include "gui/MicronToggleButton.h"
#include "gui/LcdLabel.h"
#include "gui/StdComboBox.h"
#include "gui/LookAndFeel.h"


//==============================================================================
MicronauAudioProcessorEditor::MicronauAudioProcessorEditor (MicronauAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
	LookAndFeel::setDefaultLookAndFeel( PluginLookAndFeel::getInstance() );

	background = ImageCache::getFromMemory (BinaryData::background_png,
                                     BinaryData::background_pngSize);

    owner = ownerFilter;
    
    // create all of the gui elements and hook them up to the processor
    for (int i = 0; i < 3; i++) {
        create_osc(i);
    }
    
    create_prefilt(290, 140);
    create_filter(410, 135);
    create_mod(0, 55, 25);
    
    sync_nrpn = new TextButton("sync nrpn");
    sync_nrpn->addListener(this);
    addAndMakeVisible(sync_nrpn);
    
    sync_sysex = new TextButton("sync sysex");
    sync_sysex->addListener(this);
    addAndMakeVisible(sync_sysex);
    
	param_display = new LcdLabel("panel", "micronAU\nretroware");
    param_display->setJustificationType (Justification::centredLeft);
    param_display->setEditable (false, false, false);
    param_display->setColour (TextEditor::textColourId, Colours::black);
    param_display->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    param_display->setFont (Font (18.00f, Font::plain));
	addAndMakeVisible(param_display);

    midi_in_menu = new StdComboBox ();
    midi_in_menu->setEditableText (false);
    midi_in_menu->addListener(this);
    addAndMakeVisible (midi_in_menu);

    midi_out_menu = new StdComboBox ();
    midi_out_menu->setEditableText (false);
    midi_out_menu->addListener(this);
    addAndMakeVisible (midi_out_menu);
    
    // This is where our plugin's editor size is set.
    setSize (1060, 670);

    //
    startTimer (50);
}

MicronauAudioProcessorEditor::~MicronauAudioProcessorEditor()
{
}

void MicronauAudioProcessorEditor::add_knob(int nprn, int x, int y, const char *text){
    ext_slider *s;
    s = new ext_slider(owner, nprn);
    sliders.add(s);
    s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);

    if (text) {
        s->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 15);
        s->setLabel(text);
    }

    s->addListener (this);
    s->setBounds(x, y, 40, 40);
    addAndMakeVisible(s);
}

void MicronauAudioProcessorEditor::add_box(int nrpn, int x, int y, int width){
    ext_combo *c;
    
    c = new ext_combo(owner, nrpn);
    c->setBounds(x, y, width, 15);
    c->addListener(this);
    addAndMakeVisible(c);
    boxes.add(c);
}

void MicronauAudioProcessorEditor::create_mod(int n, int x, int y)
{
    for (int i = 0; i < 6; i++) {
        Label *l;

        add_knob((i*4)+(n*24)+694, x + (i*118), y, "level");
        add_knob((i*4)+(n*24)+695, x + (i*118), y + 40, "offset");

        add_box((i*4) + (n*6) + 692, x + 40 + (i*118), y + 4, 75);
        l = new back_label("source", x + 40 + (i*118), y + 4 + 15, 75, 15);
        addAndMakeVisible(l);

        add_box((i*4) + (n*6) + 693, x + 40 + (i*118), y + 4 + 40, 75);
        l = new back_label("destination", x + 40 + (i*118), y + 4 + 15 + 40, 75, 15);
        addAndMakeVisible(l);
    }
}

void MicronauAudioProcessorEditor::create_osc(int n)
{
    int x, y, y_base;
    const char *labels[] = {"shape","octave", "semi", "fine", "wheel"};
    Label *l;
    
    x = 15;
    y = 130;
    y_base = y + n * 65;
    
    for (int i = 0; i < 5; i++) {
        add_knob((n*6)+i+524, x + (i*40), y_base + 20, labels[i]);
    }

    add_box((n*6)+523, x + 50, y_base, 55);

    String s = "osc";
    s += (n+1);
    l = new back_label(s, x, y_base, 55, 15);
    addAndMakeVisible(l);

    l = new back_label("waveform", x+105, y_base, 55, 15);
    addAndMakeVisible(l);

}

void MicronauAudioProcessorEditor::create_prefilt(int x, int y)
{
    const char *labels[] = {"osc1", "osc2", "osc3", "ring", "noise", "ext in"};
    Label *l;
    for (int i = 0; i < 6; i++) {
        add_knob(541 + i, x, y + (i * 40), NULL);
        add_knob(547 + i, x + 50, y + (i * 40), NULL);
        l = new back_label(labels[i], x-40, y + i*40 + 12, 40, 15);
        addAndMakeVisible(l);
    }

    l = new back_label("level", x, y - 15, 40, 15);
    addAndMakeVisible(l);

    l = new back_label("balance", x+40, y - 15, 60, 15);
    addAndMakeVisible(l);
}

void MicronauAudioProcessorEditor::create_filter(int x, int y)
{
    for (int i = 0; i < 2; i++) {
        int x_offset = 50;
        
        add_knob((i*6)+556, x + x_offset, y+(i*80), "cutoff");
        add_knob((i*6)+557, x + x_offset + 40, y+(i*80), "cutoff");
        add_knob((i*6)+559, x + x_offset + 80, y+(i*80), "envamt");
        add_knob((i*6)+558, x + x_offset + 120, y+(i*80), "keytrk");

        add_box((i*6)+555, x + 88, y + 42 + (i * 80), 135);
    }
    add_knob(553, x, y+17, "f1>f2");
    add_knob(670, x, y+62, "offset");

    MicronToggleButton *b = new MicronToggleButton("");
    b->setBounds(x + 10, y+100, 20, 20);
    addAndMakeVisible(b);
    // add_button(560);
}

//==============================================================================
void MicronauAudioProcessorEditor::paint (Graphics& g)
{
	g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), RectanglePlacement(RectanglePlacement::stretchToFit));
    g.setColour (Colours::black);
}

void MicronauAudioProcessorEditor::resized()
{
	param_display->setBounds(885,15,150,45);

    sync_nrpn->setBounds(910, 120, 30, 20);
    sync_sysex->setBounds(960, 120, 30, 20);

    midi_in_menu->setBounds(910, 70, 100, 20);
    midi_out_menu->setBounds(910, 95, 100, 20);
}

void MicronauAudioProcessorEditor::timerCallback()
{
    // update gui if parameters have changed
    for (int i = 0; i < sliders.size(); i++) {
        sliders[i]->setValue(sliders[i]->get_value(), dontSendNotification);
    }

    for (int i = 0; i < boxes.size(); i++) {
        boxes[i]->setSelectedItemIndex(boxes[i]->get_value(), dontSendNotification);
    }
    
    update_midi_menu(MIDI_IN_IDX);
    update_midi_menu(MIDI_OUT_IDX);
}

void MicronauAudioProcessorEditor::update_midi_menu(int in_out)
{
    ComboBox *menu;
    StringArray x;
    switch (in_out) {
        case MIDI_IN_IDX:
            x = MidiInput::getDevices();
            menu = midi_in_menu;
            break;
        case MIDI_OUT_IDX:
            x = MidiOutput::getDevices();
            menu = midi_out_menu;
            break;
        default:
            return;
    }

    bool midi_changed = false;
    if (x.size() + 1 != menu->getNumItems()) {
        midi_changed = true;
    } else {
        for (int i = 0; i < x.size(); i++) {
            if (x[i] != menu->getItemText(i+1)) {
                midi_changed = true;
                break;
            }
        }
    }
    if (midi_changed) {
        int idx = menu->getSelectedItemIndex();
        String current_midi;
        if (idx == -1) {
            current_midi = "None";
        } else {
            current_midi = menu->getItemText(idx);
        }
        menu->clear();
        menu->addItem("None", 1000);
        for (int i = 0; i < x.size(); i++) {
            menu->addItem(x[i], i+1);
        }
        select_item_by_name(in_out, current_midi);
    } else {
        select_item_by_name(in_out, owner->get_midi_port(in_out));
    }
}

void MicronauAudioProcessorEditor::sliderValueChanged (Slider *slider)
{
    ext_slider *s = dynamic_cast<ext_slider*>( slider );
	if (s)
	{
		s->set_value(s->getValue());
		param_display->setText(s->get_name() + "\n" + s->get_txt_value(s->getValue()), dontSendNotification);
	}
}

void MicronauAudioProcessorEditor::sliderDragStarted (Slider* slider)
{	// when user just touches a slider, update its value so it may be seen in the parameter display box.
	sliderValueChanged(slider);
}


void MicronauAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == sync_nrpn) {
        owner->sync_via_nrpn();
    }
    if (button == sync_sysex) {
        owner->sync_via_sysex();
    }
}

void MicronauAudioProcessorEditor::comboBoxChanged (ComboBox* box)
{
    int idx = box->getSelectedItemIndex();
    if (box == midi_out_menu) {
        owner->set_midi_port(MIDI_OUT_IDX, box->getItemText(idx));
    }
    if (box == midi_in_menu) {
        owner->set_midi_port(MIDI_IN_IDX, box->getItemText(idx));
    }
    ext_combo *b = dynamic_cast<ext_combo *>( box );
	if (b)
	{
		b->set_value(b->getSelectedItemIndex());
	}
}

void MicronauAudioProcessorEditor::select_item_by_name(int in_out, String nm)
{
    int i;
    ComboBox *menu;
    switch (in_out) {
        case MIDI_IN_IDX:
            menu = midi_in_menu;
            break;
        case MIDI_OUT_IDX:
            menu = midi_out_menu;
            break;
        default:
            return;
    }
    for (i = 0; i < menu->getNumItems(); i++) {
        if (menu->getItemText(i) == nm) {
            menu->setSelectedItemIndex(i);
            return;
        }
    }
    menu->setSelectedItemIndex(0);
}

