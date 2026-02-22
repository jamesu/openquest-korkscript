#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS

IMPLEMENT_CONOBJECT(Sound);


Sound::Sound()
{
	mPath = StringTable->EmptyString;
	mSound = {};
	mChannel = 0;
}

bool Sound::onAdd()
{
	if (Parent::onAdd())
	{
		updateResources();
		return true;
	}
	return false;
}

void Sound::onRemove()
{
	::UnloadSound(mSound);
	Parent::onRemove();
}

void Sound::updateResources()
{
	mSound = ::LoadSound(mPath);
}

void Sound::play()
{
	if (mChannel < AUDIO_CHANNEL_COUNT)
	{
		::SetSoundVolume(mSound, gGlobals.mChannelVolume[mChannel]);
	}
	::PlaySound(mSound);
}

void Sound::initPersistFields()
{
	Parent::initPersistFields();
	addField("path", TypeString, Offset(mPath, Sound));
	addField("channel", TypeS32, Offset(mChannel, Sound));
}

ConsoleMethodValue(Sound, play, 2, 2, "")
{
	object->play();
	return KorkApi::ConsoleValue();
}

END_SW_NS
