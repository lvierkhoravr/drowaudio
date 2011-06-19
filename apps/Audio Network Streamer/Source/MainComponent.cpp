/*
  ==============================================================================

    MainComponent.cpp
    Created: 20 May 2011 5:08:02am
    Author:  David Rowland

  ==============================================================================
*/

#include "MainComponent.h"

MainComponent::MainComponent()
:	tabbedComponent(TabbedButtonBar::TabsAtTop)
{
	settingsButton.setClickingTogglesState(true);
	settingsButton.setButtonText("Settings");
	settingsButton.addListener(this);
		
	addAndMakeVisible(&connectionComp);
	addChildComponent(&settingsComp);
	addAndMakeVisible(&settingsButton);
}

MainComponent::~MainComponent()
{
}

void MainComponent::resized()
{
	const int w = getWidth();
	const int h = getHeight();
	const int m = 5;
	
	settingsButton.setBounds(w-100-m, m, 100, 20);
	connectionComp.setBounds(getLocalBounds());//(0, settingsButton.getBottom()+m, w, h-settingsButton.getHeight()-m);
	settingsComp.setBounds(getLocalBounds());//(0, settingsButton.getBottom()+m, w, h-settingsButton.getHeight()-m);
}

void MainComponent::buttonClicked(Button *button)
{
	if (button == &settingsButton)
	{
		if (settingsButton.getToggleState())
		{
			settingsComp.setVisible(true);
			connectionComp.setVisible(false);
		}
		else
		{
			connectionComp.setVisible(true);
			settingsComp.setVisible(false);
		}

	}
}

//==============================================================================
