/** 
 * @file llagentaccess.cpp
 * @brief LLAgentAccess class implementation - manages maturity and godmode info
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */
#include "llviewerprecompiledheaders.h"

#include "llagentaccess.h"

#include "indra_constants.h"

#include "llviewercontrol.h"

LLAgentAccess::LLAgentAccess()
:	mAccess(SIM_ACCESS_PG),
	mAdminOverride(false),
	mGodLevel(GOD_NOT)
{
}

bool LLAgentAccess::getAdminOverride() const	
{ 
	return mAdminOverride; 
}

void LLAgentAccess::setAdminOverride(bool b)
{ 
	mAdminOverride = b; 
}

void LLAgentAccess::setGodLevel(U8 god_level)
{ 
	mGodLevel = god_level; 
}

bool LLAgentAccess::isGodlike() const
{
	if (mAdminOverride) return true;
	return mGodLevel > GOD_NOT;
}

bool LLAgentAccess::isGodlikeWithoutAdminMenuFakery() const
{
	return mGodLevel > GOD_NOT;
}

U8 LLAgentAccess::getGodLevel() const
{
	if (mAdminOverride) return GOD_FULL;
	return mGodLevel;
}

bool LLAgentAccess::wantsPGOnly() const
{
	return (prefersPG() || isTeen()) && !isGodlike();
}

bool LLAgentAccess::canAccessMature() const
{
	// if you prefer mature, you're either mature or adult, and
	// therefore can access all mature content
	return isGodlike() || (prefersMature() && !isTeen());
}

bool LLAgentAccess::canAccessAdult() const
{
	// if you prefer adult, you must BE adult.
	return isGodlike() || (prefersAdult() && isAdult());
}

bool LLAgentAccess::prefersPG() const
{
	static LLCachedControl<U32> maturity(gSavedSettings, "PreferredMaturity");
	return maturity < SIM_ACCESS_MATURE;
}

bool LLAgentAccess::prefersMature() const
{
	static LLCachedControl<U32> maturity(gSavedSettings, "PreferredMaturity");
	return maturity >= SIM_ACCESS_MATURE;
}

bool LLAgentAccess::prefersAdult() const
{
	static LLCachedControl<U32> maturity(gSavedSettings, "PreferredMaturity");
	return maturity >= SIM_ACCESS_ADULT;
}

bool LLAgentAccess::isTeen() const
{
	return mAccess < SIM_ACCESS_MATURE;
}

bool LLAgentAccess::isMature() const
{
	return mAccess >= SIM_ACCESS_MATURE;
}

bool LLAgentAccess::isAdult() const
{
	return mAccess >= SIM_ACCESS_ADULT;
}

void LLAgentAccess::setTeen(bool teen)
{
	if (teen)
	{
		mAccess = SIM_ACCESS_PG;
	}
	else
	{
		mAccess = SIM_ACCESS_MATURE;
	}
}

//static 
U8 LLAgentAccess::convertTextToMaturity(char text)
{
	if ('A' == text)
	{
		return SIM_ACCESS_ADULT;
	}
	else if ('M' == text)
	{
		return SIM_ACCESS_MATURE;
	}
	else if ('P' == text)
	{
		return SIM_ACCESS_PG;
	}
	return SIM_ACCESS_MIN;
}

void LLAgentAccess::setMaturity(char text)
{
	mAccess = LLAgentAccess::convertTextToMaturity(text);
	U8 preferred_access = (U8)gSavedSettings.getU32("PreferredMaturity");
	while (!canSetMaturity(preferred_access))
	{
		if (preferred_access == SIM_ACCESS_ADULT)
		{
			preferred_access = SIM_ACCESS_MATURE;
		}
		else
		{
			// Mature or invalid access gets set to PG
			preferred_access = SIM_ACCESS_PG;
		}
	}
	gSavedSettings.setU32("PreferredMaturity", preferred_access);
}

bool LLAgentAccess::canSetMaturity(U8 maturity)
{
	if (isAdult() || isGodlike())
	{
		// Adults and "gods" can always set their Maturity level
		return true;
	}
	if (maturity == SIM_ACCESS_PG ||
		(maturity == SIM_ACCESS_MATURE && isMature()))
	{
		return true;
	}
	else
	{
		return false;
	}
}
