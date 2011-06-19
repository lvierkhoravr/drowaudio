/*
  ==============================================================================

    SettingsComp.cpp
    Created: 11 Jun 2011 12:37:21am
    Author:  David Rowland

  ==============================================================================
*/

#include "SettingsComp.h"

SettingsComponent::SettingsComponent()
:	settingsTree(Settings::getInstance()->getValueTree())
{
	addAndMakeVisible(&compressAudioButton);
	compressAudioButton.setButtonText("Compress Transfer When Sending");
	compressAudioButton.getToggleStateValue().referTo(settingsTree.getPropertyAsValue(SettingsNames[Settings::compress], nullptr));
	compressAudioButton.setTooltip("Checking this will perform basic non-lossy compression on the audio that is transferred. "
								   "This will reduce network load but increase CPU usage.");
	
	audioSettings = new AudioSettingsComponent(*Settings::getInstance()->audioManager,
											   1, 2,
											   1, 2,
											   false,
											   false,
											   true,
											   false);
	addAndMakeVisible(audioSettings);
	//audioSettings->setSize(100, 200);
	
	Settings::getInstance()->audioManager->addChangeListener(this);
}

SettingsComponent::~SettingsComponent()
{
	Settings::getInstance()->audioManager->removeChangeListener(this);
}

void SettingsComponent::resized()
{
	const int w = getWidth();
	const int h = getHeight();
	const int m = 5;
	const int cx = w * 0.5;
	
	compressAudioButton.setBounds(cx-125, 40+2*m, 250, 20);
	audioSettings->setBounds(m, compressAudioButton.getBottom()+20+2*m, w-2*m, 343);
}

void SettingsComponent::paint(Graphics &g)
{
	const int m = 5;
	
	const int fontH = 20;
	g.setColour(Colours::black);
	g.drawText("Compression Settings:",
			   m, compressAudioButton.getY() - fontH - m,
			   Font(fontH).getStringWidth("Compression Settings:"), fontH, Justification(Justification::centredLeft), true);
	g.drawText("Audio Settings:",
			   m, compressAudioButton.getBottom()+m,
			   Font(fontH).getStringWidth("Audio Settings:"), fontH, Justification(Justification::centredLeft), true);
	
	g.setColour(Colours::azure.darker(0.1));
	Rectangle<int> audioBounds(audioSettings->getBounds());
	g.fillRoundedRectangle(audioBounds.getX(), audioBounds.getY(), audioBounds.getWidth(), audioSettings->getHeight(), 5);
}

void SettingsComponent::changeListenerCallback(ChangeBroadcaster *changedBroadcaster)
{
//	DBG("change callback");
//	void* pointer = Settings::getInstance()->audioManager;
//	
//	if (changedBroadcaster == pointer)
//	{
//		DBG("audio manager changed "<<audioSettings->getHeightOfComponents());
//		audioSettings->resized();
//		DBG("now: "<<audioSettings->getHeightOfComponents());
//		resized();
//		repaint();
//		triggerAsyncUpdate();
//	}
}

void SettingsComponent::handleAsyncUpdate()
{
	DBG("async");
	resized();
}

void SettingsComponent::childBoundsChanged(Component *child)
{
	if (child == &*audioSettings) {
		DBG("audio settings size changed"<<audioSettings->getHeight());
	}
}