/*
 *  AudioFilePlayer.h
 *  dRowAudio
 *
 *	Derived from code by Martin Robinson 2008
 *  Created by David Rowland on 10/01/2009.
 *  Copyright 2009 dRowAudio. All rights reserved.
 *
 */

#ifndef _DROWAUDIO_AUDIOFILEPLAYER__H_
#define _DROWAUDIO_AUDIOFILEPLAYER__H_

//==============================================================================
/** This class can be used to load and play an audio file from disk.
 
    This combines the functionality of an AudioTransportSource, 
    AudioFormatReader and AudioFormatReaderSource.

    @see AudioTransportSource
    @see AudioFormatReader
    @see AudioFormatReaderSource
 */
class AudioFilePlayer : public PositionableAudioSource,
                        public ChangeListener
{
public:
    //==============================================================================
	/** Creates an empty AudioFilePlayer.
     */
	AudioFilePlayer();
    
	/** Destructor.
     
        If you subclass from this, make sure to call
        audioTransportSource->setSource (nullptr) in your destructor so you don't
        mess up the audio chain dependancies and crash
     */
	~AudioFilePlayer();
	
	/** Open and get ready to play a given audio file. 
     */
	bool setFile (const File& newFile);
    
    /** Sets the source of the player using a memory block.
        The player will use this so should not be deleted until a new file is
        set or a nullptr is passed in here to clear the loaded file.
     */
    bool setMemoryBlock (MemoryBlock* inputBlock);
        
	/** Returns the absolute path of the current audio file if it was set with a file.
     */
	File getFile()                                        {	return file;                }
    
    /** Returns the current stream if a the source was set with one.
        It is the caller's responsibility to delete this stream.
     */
    MemoryInputStream* getInputStream();
    
    /** Returns true of the source was set from a MemoryBlock, falst if it was a File.
     */
    bool sourceIsMemoryBlock();
    
    //==============================================================================
    /** Starts playing (if a source has been selected). */
    void start();
    
    /** Stops playing. */
    void stop();
    
    /** Play the audio file from the start. */
	void startFromZero();
	
	/** Pauses or plays the audio file. */
	void pause();
    
    /** Returns true if it's currently playing. */
    bool isPlaying() const noexcept             { return audioTransportSource->isPlaying(); }
    
    //==============================================================================
    /** Changes the current playback position in the source stream.
     */
    void setPosition (double newPosition)       { audioTransportSource->setPosition (newPosition);  }
    
    /** Returns the position that the next data block will be read from in seconds.
     */
    double getCurrentPosition() const           { return audioTransportSource->getCurrentPosition();}
    
    /** Returns the stream's length in seconds.
     */
    double getLengthInSeconds() const           { return audioTransportSource->getLengthInSeconds();}
    
    /** Returns true if the player has stopped because its input stream ran out of data.
     */
    bool hasStreamFinished() const noexcept     { return audioTransportSource->hasStreamFinished(); }
    
    //==============================================================================
	/** Returns the AudioFormatReaderSource currently being used.
     */
	inline AudioFormatReaderSource* getAudioFormatReaderSource()   {   return audioFormatReaderSource; }
	
    /** Sets the AudioFormatManager to use.
     */
    void setAudioFormatManager (AudioFormatManager* newManager,  bool deleteWhenNotNeeded = true);
    
	/** Returns the AudioFormatManager being used.
     */
	inline AudioFormatManager* getAudioFormatManager()             {   return formatManager;           }
        
    /** Returns the AudioTransportSource being used.
     */
    inline AudioTransportSource* getAudioTransportSource()         {   return audioTransportSource;    }

    //==============================================================================
    /** A class for receiving callbacks from a AudioFilePlayer.
	 
        To be told when a player's file changes, you can register an AudioFilePlayer::Listener
        object using AudioFilePlayer::addListener().

        @see AudioFilePlayer::addListener, AudioFilePlayer::removeListener
	 */
    class  Listener
    {
    public:
        //==============================================================================
        /** Destructor. */
        virtual ~Listener() {}
		
        //==============================================================================
        /** Called when the player's file is changed.
         
            You can find out the new file using AudioFilePlayer::getFile().
		 */
        virtual void fileChanged (AudioFilePlayer* player) = 0;
		
        /** Called when the the player is stopped or started.
            You can find out if it is currently stopped with isPlaying().
		 */
        virtual void playerStoppedOrStarted (AudioFilePlayer* player) {}

        /** To avoid having to create a new listener interface for each subclass of AudioFilePlayer
            you can call this and send a SettingCode to your listeners to identify what sort of change occured
            e.g. playback rate, filter gain etc.
         */
        virtual void audioFilePlayerSettingChanged (AudioFilePlayer* player, int settingCode) {}
    };
	
    /** Adds a listener to be called when this slider's value changes. */
    void addListener (Listener* listener);
	
    /** Removes a previously-registered listener. */
    void removeListener (Listener* listener);
	
    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    
    /** Implementation of the AudioSource method. */
    void releaseResources();
    
    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);
    
    //==============================================================================
    /** Sets the next read position in samples.
     */
    void setNextReadPosition (int64 newPosition)    {   audioTransportSource->setNextReadPosition (newPosition);    }
    
    /** Returns the position from which the next block will be returned.
     */
    int64 getNextReadPosition() const   {   return audioTransportSource->getNextReadPosition();    }
    
    /** Returns the total length of the stream (in samples). */
    int64 getTotalLength() const        {   return audioTransportSource->getTotalLength(); }
    
    /** Returns true if this source is actually playing in a loop. */
    bool isLooping() const              {   return audioTransportSource->isLooping();      }
    
    /** Tells the source whether you'd like it to play in a loop. */
    virtual void setLooping (bool shouldLoop);

    /** @internal. */
    void changeListenerCallback (ChangeBroadcaster* source);
    
protected:	
    //==============================================================================
	TimeSliceThread bufferingTimeSliceThread;

	OptionalScopedPointer<AudioFormatManager> formatManager;

    AudioSource* masterSource;
    ScopedPointer<AudioFormatReaderSource> audioFormatReaderSource;
	ScopedPointer<AudioTransportSource> audioTransportSource;

	File file;
    MemoryBlock* currentMemoryBlock;
    MemoryInputStream* memoryInputStream;
    
    ListenerList <Listener> listeners;

    //==============================================================================
    /** Sets up the audio chain when a new source is chosen.
     
        By default this will create a new AudioFormatReader source and attach it to the 
        AudioTransportSource. If you want to add your own sources overide this method.
        If you do change this make sure you set the masterSource member to the top level
        of your audio source chain.
     */
	virtual bool setSourceWithReader (AudioFormatReader* reader);
    
    //==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilePlayer);
};

#endif //_DROWAUDIO_AUDIOFILEPLAYER__H_