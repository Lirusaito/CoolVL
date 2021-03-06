/** 
 * @file hbfloaterrlv.h
 * @brief The HBFloaterRLV and HBFloaterBlacklistRLV classes declarations
 *
 * $LicenseInfo:firstyear=2011&license=viewergpl$
 * 
 * Copyright (c) 2011, Henri Beauchamp
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

#ifndef LL_HBFLOATERRLV_H
#define LL_HBFLOATERRLV_H

#include "llfloater.h"

class LLScrollListCtrl;

class HBFloaterRLV : public LLFloater
{
public:
	HBFloaterRLV();
	/*virtual*/ ~HBFloaterRLV();

	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();

	static void showInstance();
	static void setDirty();

private:
	static void onButtonRefresh(void* data);
	static void onButtonClose(void* data);

	LLScrollListCtrl* mRestrictions;
	bool mIsDirty;

	static HBFloaterRLV* sInstance;
};

class HBFloaterBlacklistRLV : public LLFloater
{
public:
	HBFloaterBlacklistRLV();
	/*virtual*/ ~HBFloaterBlacklistRLV();

	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();

	static void showInstance();
	static bool instanceExists()	{ return sInstance != NULL; }
	static void closeInstance();

private:
	static void onButtonApply(void* data);
	static void onButtonCancel(void* data);

	static HBFloaterBlacklistRLV* sInstance;
};

#endif
