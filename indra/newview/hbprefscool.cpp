/** 
 * @file hbprefscool.cpp
 * @author Henri Beauchamp
 * @brief Cool VL Viewer preferences panel
 *
 * $LicenseInfo:firstyear=2008&license=viewergpl$
 * 
 * Copyright (c) 2008-2012, Henri Beauchamp.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "hbprefscool.h"

#include "llbufferstream.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llcolorswatch.h"
#include "lldir.h"
#include "llhttpclient.h"
#include "llnotifications.h"
#include "llradiogroup.h"
#include "llsdserialize.h"
#include "llspellcheck.h"
#include "llstring.h"
#include "lltabcontainer.h"
#include "lltexteditor.h"
#include "lluictrlfactory.h"

#include "hbfloaterrlv.h"
#include "llstartup.h"
#include "llviewercontrol.h"
#include "RRInterface.h"

class HBPrefsCoolImpl : public LLPanel
{
public:
	HBPrefsCoolImpl();
	/*virtual*/ ~HBPrefsCoolImpl();

	/*virtual*/ void refresh();
	/*virtual*/ void draw();

	void apply();
	void cancel();

	void setDirty()								{ mIsDirty = true; }
	static void setQueryActive(bool active);

	std::string getDictName(const std::string& language);
	std::string getDictLanguage(const std::string& name);

	static HBPrefsCoolImpl* getInstance()		{ return sInstance; }

private:
	void refreshValues();
	void refreshRestrainedLove(bool enable);
	void updateRestrainedLoveUserProfile();

	static void onTabChanged(void* user_data, bool from_click);

	static void onCommitCheckBoxShowButton(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxSpellCheck(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxRestrainedLove(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxSpeedRez(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxPrivateLookAt(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxPrivatePointAt(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxAvatarPhysics(LLUICtrl* ctrl, void* user_data);
	static void onCommitUserProfile(LLUICtrl* ctrl, void* user_data);
	static void onCommitCheckBoxAfterRestart(LLUICtrl* ctrl, void* user_data);
	static void onClickCustomBlackList(void* user_data);
	static void onClickDownloadDict(void* user_data);
	static void onClickResetAvatarSize(void* user_data);

private:
	static HBPrefsCoolImpl* sInstance;
	static S32 sQueries;

	LLTabContainer* mTabContainer;

	bool mFirstRun;
	bool mIsDirty;

	LLSD mDictsList;
	std::set<std::string> mInstalledDicts;

	bool mWatchBlackListFloater;
	S32 mRestrainedLoveUserProfile;

	BOOL mShowChatButton;
	BOOL mShowIMButton;
	BOOL mShowFriendsButton;
	BOOL mShowGroupsButton;
	BOOL mShowFlyButton;
	BOOL mShowSnapshotButton;
	BOOL mShowSearchButton;
	BOOL mShowBuildButton;
	BOOL mShowRadarButton;
	BOOL mShowMiniMapButton;
	BOOL mShowMapButton;
	BOOL mShowInventoryButton;
	BOOL mHideMasterRemote;
	BOOL mUseOldChatHistory;
	BOOL mAutoOpenTextInput;
	BOOL mIMTabsVerticalStacking;
	BOOL mUseOldStatusBarIcons;
	BOOL mUseOldTrackingDots;
	BOOL mHideTeleportProgress;
	BOOL mUseNewSLLoginPage;
	BOOL mStackMinimizedTopToBottom;
	BOOL mStackMinimizedRightToLeft;
	BOOL mAllowMUpose;
	BOOL mAutoCloseOOC;
	BOOL mHighlightOwnNameInChat;
	BOOL mHighlightOwnNameInIM;
	BOOL mHighlightDisplayName;
	BOOL mSpellCheck;
	BOOL mSpellCheckShow;
	BOOL mAddAvatarNamesToIgnore;
	BOOL mAllowMultipleViewers;
	BOOL mFetchInventoryOnLogin;
	BOOL mSpeedRez;
	BOOL mTeleportHistoryDeparture;
	BOOL mPreviewAnimInWorld;
	BOOL mAppearanceAnimation;
	BOOL mPrivateLookAt;
	BOOL mPrivatePointAt;
	BOOL mRevokePermsOnStandUp;
	BOOL mRezWithLandGroup;
	BOOL mNoMultipleShoes;
	BOOL mNoMultipleSkirts;
	BOOL mNoMultiplePhysics;
	BOOL mAvatarSizePersist;
	BOOL mAvatarPhysics;
	BOOL mRestrainedLove;
	BOOL mRestrainedLoveNoSetEnv;
	BOOL mRestrainedLoveAllowWear;
	BOOL mRestrainedLoveForbidGiveToRLV;
	BOOL mRestrainedLoveShowEllipsis;
	BOOL mRestrainedLoveUntruncatedEmotes;
	BOOL mRestrainedLoveCanOoc;
	F32 mAvatarDepth;
	F32 mAvatarWidth;
	F32 mRenderAvatarPhysicsLODFactor;
	U32 mFadeMouselookExitTip;
	U32 mDecimalsForTools;
	U32 mStackScreenWidthFraction;
	U32 mTimeFormat;
	U32 mDateFormat;
	U32 mSpeedRezInterval;
	U32 mPrivateLookAtLimit;
	U32 mPrivatePointAtLimit;
	U32 mRestrainedLoveReattachDelay;
	LLColor4 mOwnNameChatColor;
	std::string mHighlightNicknames;
	std::string mSpellCheckLanguage;
	std::string mRestrainedLoveRecvimMessage;
	std::string mRestrainedLoveSendimMessage;
	std::string mRestrainedLoveBlacklist;
};

class DictionaryDownload : public LLHTTPClient::Responder
{
	LOG_CLASS(DictionaryDownload);

public:
    DictionaryDownload(const std::string& dict_name)
	:	mDictName(dict_name)
	{
		HBPrefsCoolImpl::setQueryActive(true);
	}

	void completedRaw(U32 status, const std::string& reason,
					  const LLChannelDescriptors& channels,
					  const LLIOPipe::buffer_ptr_t& buffer)
    {
		HBPrefsCoolImpl::setQueryActive(false);

		if (status < 200 || status >= 300)
		{
			LLSD args;
			args["NAME"] = mDictName;
			args["STATUS"] = llformat("%d", status);
			args["REASON"] = reason;
			LLNotifications::instance().add("DownloadDictFailure", args);
			return;
		}

		LLBufferStream inputstream(channels, buffer.get());
		std::string filename;
		filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS,
												  "dictionaries",
												  mDictName.c_str());
		llofstream outputstream(filename, std::ios::binary);
		if (outputstream.is_open())
		{
			while (inputstream.good() && outputstream.good())
			{
				outputstream << inputstream.rdbuf();
			}
			outputstream.close();
		}
		else
		{
			LLSD args;
			args["NAME"] = mDictName;
			LLNotifications::instance().add("DictWriteFailure", args);
		}
	}

private:
	std::string mDictName;
};

HBPrefsCoolImpl* HBPrefsCoolImpl::sInstance = NULL;
S32 HBPrefsCoolImpl::sQueries = 0;

HBPrefsCoolImpl::HBPrefsCoolImpl()
:	LLPanel(std::string("Cool Preferences Panel")),
	mWatchBlackListFloater(false),
	mFirstRun(true),
	mIsDirty(true)
{
	sInstance = this;

	LLUICtrlFactory::getInstance()->buildPanel(this,
											   "panel_preferences_cool.xml");

	childSetCommitCallback("show_chat_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_im_button_check",				onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_friends_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_group_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_fly_button_check",				onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_snapshot_button_check",		onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_search_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_build_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_radar_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_minimap_button_check",			onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_map_button_check",				onCommitCheckBoxShowButton, this);
	childSetCommitCallback("show_inventory_button_check",		onCommitCheckBoxShowButton, this);
	childSetCommitCallback("use_old_chat_history_check",		onCommitCheckBoxAfterRestart, this);
	childSetCommitCallback("im_tabs_vertical_stacking_check",	onCommitCheckBoxAfterRestart, this);
	childSetCommitCallback("use_old_statusbar_icons_check",		onCommitCheckBoxAfterRestart, this);
	childSetCommitCallback("use_new_sl_login_page_check",		onCommitCheckBoxAfterRestart, this);
	childSetCommitCallback("spell_check_check"			,		onCommitCheckBoxSpellCheck, this);
	childSetCommitCallback("speed_rez_check",					onCommitCheckBoxSpeedRez, this);
	childSetCommitCallback("private_look_at_check",				onCommitCheckBoxPrivateLookAt, this);
	childSetCommitCallback("private_point_at_check",			onCommitCheckBoxPrivatePointAt, this);
	childSetCommitCallback("avatar_physics_check",				onCommitCheckBoxAvatarPhysics, this);
	childSetCommitCallback("restrained_love_check",				onCommitCheckBoxRestrainedLove, this);
	childSetCommitCallback("user_profile",						onCommitUserProfile, this);
	childSetCommitCallback("restrained_love_no_setenv_check",	onCommitCheckBoxAfterRestart, this);
	childSetCommitCallback("restrained_love_emotes_check",		onCommitCheckBoxAfterRestart, this);
	childSetCommitCallback("restrained_love_can_ooc_check",		onCommitCheckBoxAfterRestart, this);

	childSetAction("dict_download_button",						onClickDownloadDict, this);
	childSetAction("custom_profile_button",						onClickCustomBlackList, this);
	childSetAction("reset_size_button",							onClickResetAvatarSize, this);

	mTabContainer = getChild<LLTabContainer>("Cool Prefs");
	LLPanel* tab = mTabContainer->getChild<LLPanel>("User Interface");
	mTabContainer->setTabChangeCallback(tab, onTabChanged);
	mTabContainer->setTabUserData(tab, this);
	tab = mTabContainer->getChild<LLPanel>("Chat/IM");
	mTabContainer->setTabChangeCallback(tab, onTabChanged);
	mTabContainer->setTabUserData(tab, this);
	tab = mTabContainer->getChild<LLPanel>("Miscellaneous");
	mTabContainer->setTabChangeCallback(tab, onTabChanged);
	mTabContainer->setTabUserData(tab, this);
	tab = mTabContainer->getChild<LLPanel>("RestrainedLove");
	mTabContainer->setTabChangeCallback(tab, onTabChanged);
	mTabContainer->setTabUserData(tab, this);

	if (LLStartUp::getStartupState() != STATE_STARTED)
	{
		LLCheckBoxCtrl* rl = getChild<LLCheckBoxCtrl>("restrained_love_check");
		std::string tooltip = rl->getToolTip();
		tooltip += " " + getString("when_logged_in");
		rl->setToolTip(tooltip);
	}

	refresh();
}

HBPrefsCoolImpl::~HBPrefsCoolImpl()
{
	sInstance = NULL;
}

void HBPrefsCoolImpl::draw()
{
	if (mFirstRun)
	{
		mFirstRun = false;
		mTabContainer->selectTab(gSavedSettings.getS32("LastCoolPrefTab"));
	}
	if (mIsDirty)
	{
		// First get the list of all installed dictionaries
		mInstalledDicts.clear();
		mInstalledDicts = LLSpellCheck::instance().getBaseDicts();

		// Then get the list of all existing dictionaries
		mDictsList.clear();
		std::string dict_list;
		dict_list = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,
												   "dictionaries",
												   "dict_list.xml");
		llifstream inputstream(dict_list, std::ios::binary);
		if (inputstream.is_open())
		{
			LLSDSerialize::fromXMLDocument(mDictsList, inputstream);
			inputstream.close();
		}
		if (mDictsList.size() == 0)
		{
			llwarns << "Failure to load the list of all existing dictionaries."
					<< llendl;
		}

		bool found_one = false;
		std::string name = LLSpellCheck::instance().getCurrentDict();
		std::string language;
		S32 selection = -1;
		LLComboBox* combo = getChild<LLComboBox>("installed_dicts_combo");
		combo->removeall();
		for (std::set<std::string>::iterator it = mInstalledDicts.begin();
			 it != mInstalledDicts.end(); it++)
		{
			language = getDictLanguage(*it);
			if (language.empty())
			{
				language = *it;
			}
			combo->add(language);
			found_one = true;
			if (*it == name)
			{
				selection = combo->getItemCount() - 1;
			}
		}
		if (!found_one)
		{
			combo->add(LLStringUtil::null);
		}
		if (selection >= 0)
		{
			combo->setCurrentByIndex(selection);
		}

		found_one = false;
		combo = getChild<LLComboBox>("download_dict_combo");
		combo->removeall();
		for (LLSD::array_const_iterator it = mDictsList.beginArray();
			 it != mDictsList.endArray(); it++)
		{
			const LLSD& entry = *it;
			name = entry["name"].asString();
			if (name.empty())
			{
				llwarns << "Invalid dictionary list entry: no name." << llendl;
				continue;
			}
			LLStringUtil::toLower(name);
			if (!mInstalledDicts.count(name))
			{
				language = entry["language"].asString();
				if (language.empty())
				{
					llwarns << "Invalid dictionary list entry. No language for: "
							<< name << llendl;
					language = name;
				}
				combo->add(language);
				found_one = true;
			}
		}
		if (!found_one)
		{
			combo->add(LLStringUtil::null);
		}

		bool visible = (sQueries == 0);
		childSetVisible("download_dict_combo", visible);
		childSetVisible("dict_download_button", visible);
		childSetVisible("downloading", !visible);

		mIsDirty = false;
	}
	if (mWatchBlackListFloater && !HBFloaterBlacklistRLV::instanceExists())
	{
		mWatchBlackListFloater = false;
		updateRestrainedLoveUserProfile();
	}
	LLPanel::draw();
}

void HBPrefsCoolImpl::refreshRestrainedLove(bool enable)
{
	// Enable/disable all children in the RestrainedLove panel
	LLPanel* panel = getChild<LLPanel>("RestrainedLove");
	LLView* child = panel->getFirstChild();
	while (child)
	{
		child->setEnabled(enable);
		child = panel->findNextSibling(child);
	}

	// Enable/disable the FetchInventoryOnLogin check box
	if (enable)
	{
		gSavedSettings.setBOOL("FetchInventoryOnLogin",	TRUE);
		childSetValue("fetch_inventory_on_login_check", TRUE);
		childDisable("fetch_inventory_on_login_check");
	}
	else
	{
		childEnable("fetch_inventory_on_login_check");
	}

	// RestrainedLove check box enabled only when logged in.
	childSetEnabled("restrained_love_check",
					LLStartUp::getStartupState() == STATE_STARTED);
}

void HBPrefsCoolImpl::refreshValues()
{
	// User Interface
	mShowChatButton						= gSavedSettings.getBOOL("ShowChatButton");
	mShowIMButton						= gSavedSettings.getBOOL("ShowIMButton");
	mShowFriendsButton					= gSavedSettings.getBOOL("ShowFriendsButton");
	mShowGroupsButton					= gSavedSettings.getBOOL("ShowGroupsButton");
	mShowFlyButton						= gSavedSettings.getBOOL("ShowFlyButton");
	mShowSnapshotButton					= gSavedSettings.getBOOL("ShowSnapshotButton");
	mShowSearchButton					= gSavedSettings.getBOOL("ShowSearchButton");
	mShowBuildButton					= gSavedSettings.getBOOL("ShowBuildButton");
	mShowRadarButton					= gSavedSettings.getBOOL("ShowRadarButton");
	mShowMiniMapButton					= gSavedSettings.getBOOL("ShowMiniMapButton");
	mShowMapButton						= gSavedSettings.getBOOL("ShowMapButton");
	mShowInventoryButton				= gSavedSettings.getBOOL("ShowInventoryButton");
	mHideMasterRemote					= gSavedSettings.getBOOL("HideMasterRemote");
	mFadeMouselookExitTip				= gSavedSettings.getU32("FadeMouselookExitTip");
	mUseOldChatHistory					= gSavedSettings.getBOOL("UseOldChatHistory");
	mAutoOpenTextInput					= gSavedSettings.getBOOL("AutoOpenTextInput");
	mIMTabsVerticalStacking				= gSavedSettings.getBOOL("IMTabsVerticalStacking");
	mUseOldStatusBarIcons				= gSavedSettings.getBOOL("UseOldStatusBarIcons");
	mUseOldTrackingDots					= gSavedSettings.getBOOL("UseOldTrackingDots");
	mDecimalsForTools					= gSavedSettings.getU32("DecimalsForTools");
	mHideTeleportProgress				= gSavedSettings.getBOOL("HideTeleportProgress");
	mUseNewSLLoginPage					= gSavedSettings.getBOOL("UseNewSLLoginPage");
	mStackMinimizedTopToBottom			= gSavedSettings.getBOOL("StackMinimizedTopToBottom");
	mStackMinimizedRightToLeft			= gSavedSettings.getBOOL("StackMinimizedRightToLeft");
	mStackScreenWidthFraction			= gSavedSettings.getU32("StackScreenWidthFraction");

	// Chat, IM & Text
	mAllowMUpose						= gSavedSettings.getBOOL("AllowMUpose");
	mAutoCloseOOC						= gSavedSettings.getBOOL("AutoCloseOOC");
	mHighlightOwnNameInChat				= gSavedSettings.getBOOL("HighlightOwnNameInChat");
	mHighlightOwnNameInIM				= gSavedSettings.getBOOL("HighlightOwnNameInIM");
	mOwnNameChatColor					= gSavedSettings.getColor4("OwnNameChatColor");
	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		mHighlightNicknames				= gSavedPerAccountSettings.getString("HighlightNicknames");
		mHighlightDisplayName			= gSavedPerAccountSettings.getBOOL("HighlightDisplayName");
	}
	mSpellCheck							= gSavedSettings.getBOOL("SpellCheck");
	mSpellCheckShow						= gSavedSettings.getBOOL("SpellCheckShow");
	mAddAvatarNamesToIgnore				= gSavedSettings.getBOOL("AddAvatarNamesToIgnore");
	mSpellCheckLanguage					= gSavedSettings.getString("SpellCheckLanguage");

	// Miscellaneous
	mAllowMultipleViewers				= gSavedSettings.getBOOL("AllowMultipleViewers");
	mFetchInventoryOnLogin				= gSavedSettings.getBOOL("FetchInventoryOnLogin");
	mSpeedRez							= gSavedSettings.getBOOL("SpeedRez");
	mSpeedRezInterval					= gSavedSettings.getU32("SpeedRezInterval");
	mPreviewAnimInWorld					= gSavedSettings.getBOOL("PreviewAnimInWorld");
	mAppearanceAnimation				= gSavedSettings.getBOOL("AppearanceAnimation");
	mPrivateLookAt						= gSavedSettings.getBOOL("PrivateLookAt");
	mPrivatePointAt						= gSavedSettings.getBOOL("PrivatePointAt");
	mPrivateLookAtLimit					= gSavedSettings.getU32("PrivateLookAtLimit");
	mPrivatePointAtLimit				= gSavedSettings.getU32("PrivatePointAtLimit");
	mRevokePermsOnStandUp				= gSavedSettings.getBOOL("RevokePermsOnStandUp");
	mTeleportHistoryDeparture			= gSavedSettings.getBOOL("TeleportHistoryDeparture");
	mRezWithLandGroup					= gSavedSettings.getBOOL("RezWithLandGroup");
	mNoMultipleShoes					= gSavedSettings.getBOOL("NoMultipleShoes");
	mNoMultipleSkirts					= gSavedSettings.getBOOL("NoMultipleSkirts");
	mNoMultiplePhysics					= gSavedSettings.getBOOL("NoMultiplePhysics");
	mAvatarDepth						= gSavedSettings.getF32("AvatarDepth");
	mAvatarWidth						= gSavedSettings.getF32("AvatarWidth");
	mAvatarSizePersist					= gSavedSettings.getBOOL("AvatarSizePersist");
	mAvatarPhysics						= gSavedSettings.getBOOL("AvatarPhysics");
	mRenderAvatarPhysicsLODFactor		= gSavedSettings.getF32("RenderAvatarPhysicsLODFactor");

	// RestrainedLove
	mRestrainedLove						= gSavedSettings.getBOOL("RestrainedLove");
	if (mRestrainedLove)
	{
		mFetchInventoryOnLogin = TRUE;
		gSavedSettings.setBOOL("FetchInventoryOnLogin",	TRUE);
	}
	mRestrainedLoveBlacklist			= gSavedSettings.getString("RestrainedLoveBlacklist");
	mRestrainedLoveNoSetEnv				= gSavedSettings.getBOOL("RestrainedLoveNoSetEnv");
	mRestrainedLoveAllowWear			= gSavedSettings.getBOOL("RestrainedLoveAllowWear");
	mRestrainedLoveReattachDelay		= gSavedSettings.getU32("RestrainedLoveReattachDelay");
	mRestrainedLoveForbidGiveToRLV		= gSavedSettings.getBOOL("RestrainedLoveForbidGiveToRLV");
	mRestrainedLoveShowEllipsis			= gSavedSettings.getBOOL("RestrainedLoveShowEllipsis");
	mRestrainedLoveUntruncatedEmotes	= gSavedSettings.getBOOL("RestrainedLoveUntruncatedEmotes");
	mRestrainedLoveCanOoc				= gSavedSettings.getBOOL("RestrainedLoveCanOoc");
	mRestrainedLoveRecvimMessage		= gSavedSettings.getString("RestrainedLoveRecvimMessage");
	mRestrainedLoveSendimMessage		= gSavedSettings.getString("RestrainedLoveSendimMessage");
}

void HBPrefsCoolImpl::updateRestrainedLoveUserProfile()
{
	std::string blacklist = gSavedSettings.getString("RestrainedLoveBlacklist");
	if (blacklist.empty())
	{
		mRestrainedLoveUserProfile = 0;
	}
	else if (blacklist == RRInterface::sRolePlayBlackList)
	{
		mRestrainedLoveUserProfile = 1;
	}
	else if (blacklist == RRInterface::sVanillaBlackList)
	{
		mRestrainedLoveUserProfile = 2;
	}
	else
	{
		mRestrainedLoveUserProfile = 3;
	}
	LLRadioGroup* radio = getChild<LLRadioGroup>("user_profile");
	radio->selectNthItem(mRestrainedLoveUserProfile);
}

void HBPrefsCoolImpl::refresh()
{
	refreshValues();

	// User Interface

	std::string format = gSavedSettings.getString("ShortTimeFormat");
	if (format.find("%p") == -1)
	{
		mTimeFormat = 0;
	}
	else
	{
		mTimeFormat = 1;
	}

	format = gSavedSettings.getString("ShortDateFormat");
	if (format.find("%m/%d/%") != -1)
	{
		mDateFormat = 2;
	}
	else if (format.find("%d/%m/%") != -1)
	{
		mDateFormat = 1;
	}
	else
	{
		mDateFormat = 0;
	}

	// time format combobox
	LLComboBox* combo = getChild<LLComboBox>("time_format_combobox");
	if (combo)
	{
		combo->setCurrentByIndex(mTimeFormat);
	}

	// date format combobox
	combo = getChild<LLComboBox>("date_format_combobox");
	if (combo)
	{
		combo->setCurrentByIndex(mDateFormat);
	}

	if (LLStartUp::getStartupState() != STATE_STARTED)
	{
		childDisable("highlight_nicknames_text");
		childDisable("highlight_display_name_check");
	}
	else
	{
		childSetValue("highlight_nicknames_text", mHighlightNicknames);
		childSetValue("highlight_display_name_check", mHighlightDisplayName);
	}

	// Spell checking
	childSetEnabled("spell_check_show_check", mSpellCheck);
	childSetEnabled("add_avatar_names_to_ignore_check", mSpellCheck);
	childSetEnabled("installed_dicts_combo", mSpellCheck);
	childSetEnabled("download_dict_combo", mSpellCheck);
	childSetEnabled("dict_download_button", mSpellCheck);

	// Miscellaneous
	childSetEnabled("speed_rez_interval", mSpeedRez);
	childSetEnabled("speed_rez_seconds", mSpeedRez);
	childSetEnabled("private_look_at_limit", mPrivateLookAt);
	childSetEnabled("private_look_at_limit_meters", mPrivateLookAt);
	childSetEnabled("private_point_at_limit", mPrivatePointAt);
	childSetEnabled("private_point_at_limit_meters", mPrivatePointAt);
	childSetEnabled("avatar_physics_lod", mAvatarPhysics);

	// RestrainedLove
	refreshRestrainedLove(mRestrainedLove);
	updateRestrainedLoveUserProfile();
	LLWString message = utf8str_to_wstring(gSavedSettings.getString("RestrainedLoveRecvimMessage"));
	LLWStringUtil::replaceChar(message, '^', '\n');
	childSetText("receive_im_message_editor", wstring_to_utf8str(message));
	message = utf8str_to_wstring(gSavedSettings.getString("RestrainedLoveSendimMessage"));
	LLWStringUtil::replaceChar(message, '^', '\n');
	childSetText("send_im_message_editor", wstring_to_utf8str(message));
}

void HBPrefsCoolImpl::cancel()
{
	// User Interface
	gSavedSettings.setBOOL("ShowChatButton",					mShowChatButton);
	gSavedSettings.setBOOL("ShowIMButton",						mShowIMButton);
	gSavedSettings.setBOOL("ShowFriendsButton",					mShowFriendsButton);
	gSavedSettings.setBOOL("ShowGroupsButton",					mShowGroupsButton);
	gSavedSettings.setBOOL("ShowFlyButton",						mShowFlyButton);
	gSavedSettings.setBOOL("ShowSnapshotButton",				mShowSnapshotButton);
	gSavedSettings.setBOOL("ShowSearchButton",					mShowSearchButton);
	gSavedSettings.setBOOL("ShowBuildButton",					mShowBuildButton);
	gSavedSettings.setBOOL("ShowRadarButton",					mShowRadarButton);
	gSavedSettings.setBOOL("ShowMiniMapButton",					mShowMiniMapButton);
	gSavedSettings.setBOOL("ShowMapButton",						mShowMapButton);
	gSavedSettings.setBOOL("ShowInventoryButton",				mShowInventoryButton);
	gSavedSettings.setBOOL("HideMasterRemote",					mHideMasterRemote);
	gSavedSettings.setU32("FadeMouselookExitTip",				mFadeMouselookExitTip);
	gSavedSettings.setBOOL("UseOldChatHistory",					mUseOldChatHistory);
	gSavedSettings.setBOOL("AutoOpenTextInput",					mAutoOpenTextInput);
	gSavedSettings.setBOOL("IMTabsVerticalStacking",			mIMTabsVerticalStacking);
	gSavedSettings.setBOOL("UseOldStatusBarIcons",				mUseOldStatusBarIcons);
	gSavedSettings.setBOOL("UseOldTrackingDots",				mUseOldTrackingDots);
	gSavedSettings.setU32("DecimalsForTools",					mDecimalsForTools);
	gSavedSettings.setBOOL("HideTeleportProgress",				mHideTeleportProgress);
	gSavedSettings.setBOOL("UseNewSLLoginPage",					mUseNewSLLoginPage);
	gSavedSettings.setBOOL("StackMinimizedTopToBottom",			mStackMinimizedTopToBottom);
	gSavedSettings.setBOOL("StackMinimizedRightToLeft",			mStackMinimizedRightToLeft);
	gSavedSettings.setU32("StackScreenWidthFraction",			mStackScreenWidthFraction);

	// Chat, IM & Text
	gSavedSettings.setBOOL("AllowMUpose",						mAllowMUpose);
	gSavedSettings.setBOOL("AutoCloseOOC",						mAutoCloseOOC);
	gSavedSettings.setBOOL("HighlightOwnNameInChat",			mHighlightOwnNameInChat);
	gSavedSettings.setBOOL("HighlightOwnNameInIM",				mHighlightOwnNameInIM);
	gSavedSettings.setColor4("OwnNameChatColor",				mOwnNameChatColor);
	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		gSavedPerAccountSettings.setString("HighlightNicknames", mHighlightNicknames);
		gSavedPerAccountSettings.setBOOL("HighlightDisplayName", mHighlightDisplayName);
	}
	gSavedSettings.setBOOL("SpellCheck",						mSpellCheck);
	gSavedSettings.setBOOL("SpellCheckShow",					mSpellCheckShow);
	gSavedSettings.setBOOL("AddAvatarNamesToIgnore",			mAddAvatarNamesToIgnore);
	gSavedSettings.setString("SpellCheckLanguage",				mSpellCheckLanguage);

	// Miscellaneous
	gSavedSettings.setBOOL("AllowMultipleViewers",				mAllowMultipleViewers);
	gSavedSettings.setBOOL("FetchInventoryOnLogin",				mFetchInventoryOnLogin);
	gSavedSettings.setBOOL("SpeedRez",							mSpeedRez);
	gSavedSettings.setU32("SpeedRezInterval",					mSpeedRezInterval);
	gSavedSettings.setBOOL("PreviewAnimInWorld",				mPreviewAnimInWorld);
	gSavedSettings.setBOOL("AppearanceAnimation",				mAppearanceAnimation);
	gSavedSettings.setBOOL("PrivateLookAt",						mPrivateLookAt);
	gSavedSettings.setBOOL("PrivatePointAt",					mPrivatePointAt);
	gSavedSettings.setU32("PrivateLookAtLimit",					mPrivateLookAtLimit);
	gSavedSettings.setU32("PrivatePointAtLimit",				mPrivatePointAtLimit);
	gSavedSettings.setBOOL("RevokePermsOnStandUp",				mRevokePermsOnStandUp);
	gSavedSettings.setBOOL("TeleportHistoryDeparture",			mTeleportHistoryDeparture);
	gSavedSettings.setBOOL("RezWithLandGroup",					mRezWithLandGroup);
	gSavedSettings.setBOOL("NoMultipleShoes",					mNoMultipleShoes);
	gSavedSettings.setBOOL("NoMultipleSkirts",					mNoMultipleSkirts);
	gSavedSettings.setBOOL("NoMultiplePhysics",					mNoMultiplePhysics);
	gSavedSettings.setBOOL("AvatarSizePersist",					mAvatarSizePersist);
	gSavedSettings.setF32("AvatarDepth",						mAvatarDepth);
	gSavedSettings.setF32("AvatarWidth",						mAvatarWidth);
	gSavedSettings.setBOOL("AvatarPhysics",						mAvatarPhysics);
	gSavedSettings.setBOOL("AvatarSizePersist",					mAvatarSizePersist);
	gSavedSettings.setF32("RenderAvatarPhysicsLODFactor",		mRenderAvatarPhysicsLODFactor);

	// RestrainedLove
	gSavedSettings.setBOOL("RestrainedLove",					mRestrainedLove);
	gSavedSettings.setString("RestrainedLoveBlacklist",			mRestrainedLoveBlacklist);
	gSavedSettings.setBOOL("RestrainedLoveNoSetEnv",			mRestrainedLoveNoSetEnv);
	gSavedSettings.setBOOL("RestrainedLoveAllowWear",			mRestrainedLoveAllowWear);
	gSavedSettings.setU32("RestrainedLoveReattachDelay",		mRestrainedLoveReattachDelay);
	gSavedSettings.setBOOL("RestrainedLoveForbidGiveToRLV",		mRestrainedLoveForbidGiveToRLV);
	gSavedSettings.setBOOL("RestrainedLoveShowEllipsis",		mRestrainedLoveShowEllipsis);
	gSavedSettings.setBOOL("RestrainedLoveUntruncatedEmotes",	mRestrainedLoveUntruncatedEmotes);
	gSavedSettings.setBOOL("RestrainedLoveCanOoc",				mRestrainedLoveCanOoc);
	gSavedSettings.setString("RestrainedLoveRecvimMessage",		mRestrainedLoveRecvimMessage);
	gSavedSettings.setString("RestrainedLoveSendimMessage",		mRestrainedLoveSendimMessage);
}

void HBPrefsCoolImpl::apply()
{
	// User Interface

	std::string short_date, long_date, short_time, long_time, timestamp;	

	LLComboBox* combo = getChild<LLComboBox>("time_format_combobox");
	if (combo)
	{
		mTimeFormat = combo->getCurrentIndex();
	}

	combo = getChild<LLComboBox>("date_format_combobox");
	if (combo)
	{
		mDateFormat = combo->getCurrentIndex();
	}

	if (mTimeFormat == 0)
	{
		short_time = "%H:%M";
		long_time  = "%H:%M:%S";
		timestamp  = " %H:%M:%S";
	}
	else
	{
		short_time = "%I:%M %p";
		long_time  = "%I:%M:%S %p";
		timestamp  = " %I:%M %p";
	}

	if (mDateFormat == 0)
	{
		short_date = "%Y-%m-%d";
		long_date  = "%A %d %B %Y";
		timestamp  = "%a %d %b %Y" + timestamp;
	}
	else if (mDateFormat == 1)
	{
		short_date = "%d/%m/%Y";
		long_date  = "%A %d %B %Y";
		timestamp  = "%a %d %b %Y" + timestamp;
	}
	else
	{
		short_date = "%m/%d/%Y";
		long_date  = "%A, %B %d %Y";
		timestamp  = "%a %b %d %Y" + timestamp;
	}

	gSavedSettings.setString("ShortDateFormat",	short_date);
	gSavedSettings.setString("LongDateFormat",	long_date);
	gSavedSettings.setString("ShortTimeFormat",	short_time);
	gSavedSettings.setString("LongTimeFormat",	long_time);
	gSavedSettings.setString("TimestampFormat",	timestamp);

	// Chat, IM & Text

	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		gSavedPerAccountSettings.setString("HighlightNicknames",
										   childGetValue("highlight_nicknames_text"));
		gSavedPerAccountSettings.setBOOL("HighlightDisplayName",
										 childGetValue("highlight_display_name_check"));
	}

	combo = getChild<LLComboBox>("installed_dicts_combo");
	std::string dict_name = getDictName(combo->getSelectedItemLabel());
	if (!dict_name.empty())
	{
		gSavedSettings.setString("SpellCheckLanguage", dict_name);
	}

	// RestrainedLove

	LLTextEditor* text = getChild<LLTextEditor>("receive_im_message_editor");
	LLWString message = text->getWText(); 
	LLWStringUtil::replaceTabsWithSpaces(message, 4);
	LLWStringUtil::replaceChar(message, '\n', '^');
	gSavedSettings.setString("RestrainedLoveRecvimMessage",
							 std::string(wstring_to_utf8str(message)));

	text = getChild<LLTextEditor>("send_im_message_editor");
	message = text->getWText(); 
	LLWStringUtil::replaceTabsWithSpaces(message, 4);
	LLWStringUtil::replaceChar(message, '\n', '^');
	gSavedSettings.setString("RestrainedLoveSendimMessage",
							 std::string(wstring_to_utf8str(message)));

	refreshValues();
}

std::string HBPrefsCoolImpl::getDictName(const std::string& language)
{
	std::string result;
	for (LLSD::array_const_iterator it = mDictsList.beginArray();
		 it != mDictsList.endArray(); it++)
	{
		const LLSD& entry = *it;
		if (entry["language"].asString() == language)
		{
			result = entry["name"].asString();
			break;
		}
	}
	return result;
}

std::string HBPrefsCoolImpl::getDictLanguage(const std::string& name)
{
	std::string result;
	for (LLSD::array_const_iterator it = mDictsList.beginArray();
		 it != mDictsList.endArray(); it++)
	{
		const LLSD& entry = *it;
		if (entry["name"].asString() == name)
		{
			result = entry["language"].asString();
			break;
		}
	}
	return result;
}


// static
void HBPrefsCoolImpl::onTabChanged(void* user_data, bool from_click)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	if (self && self->mTabContainer)
	{
		gSavedSettings.setS32("LastCoolPrefTab",
							  self->mTabContainer->getCurrentPanelIndex());
	}
}

//static
void HBPrefsCoolImpl::setQueryActive(bool active)
{
	if (active)
	{
		sQueries++;
	}
	else
	{
		sQueries--;
		if (sQueries < 0)
		{
			llwarns << "Lost the count of the dictionary download queries !"
					<< llendl;
			sQueries = 0;
		}
		if (sQueries == 0)
		{
			LLNotifications::instance().add("DownloadDictFinished");
		}
	}
	if (HBPrefsCoolImpl::getInstance())
	{
		HBPrefsCoolImpl::getInstance()->setDirty();
	}
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxAfterRestart(LLUICtrl* ctrl,
												   void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	BOOL saved = FALSE;
	std::string control = check->getControlName();
	if (control == "UseOldChatHistory")
	{
		saved = self->mUseOldChatHistory;
	}
	else if (control == "IMTabsVerticalStacking")
	{
		saved = self->mIMTabsVerticalStacking;
	}
	else if (control == "UseOldStatusBarIcons")
	{
		saved = self->mUseOldStatusBarIcons;
	}
	else if (control == "UseNewSLLoginPage")
	{
		saved = self->mUseNewSLLoginPage;
	}
	else if (control == "RestrainedLoveNoSetEnv")
	{
		saved = self->mRestrainedLoveNoSetEnv;
	}
	else if (control == "RestrainedLoveUntruncatedEmotes")
	{
		saved = self->mRestrainedLoveUntruncatedEmotes;
	}
	else if (control == "RestrainedLoveCanOoc")
	{
		saved = self->mRestrainedLoveUntruncatedEmotes;
	}
 	if (saved != check->get())
	{
		LLNotifications::instance().add("InEffectAfterRestart");
	}
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxShowButton(LLUICtrl* ctrl,
												 void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enabled = check->get();
	if (enabled && gSavedSettings.getBOOL("ShowToolBar") == FALSE)
	{
		gSavedSettings.setBOOL("ShowToolBar", TRUE);
	}
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxSpellCheck(LLUICtrl* ctrl,
												 void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enabled = check->get();
	self->childSetEnabled("spell_check_show_check", enabled);
	self->childSetEnabled("add_avatar_names_to_ignore_check", enabled);
	self->childSetEnabled("installed_dicts_combo", enabled);
	self->childSetEnabled("download_dict_combo", enabled);
	self->childSetEnabled("dict_download_button", enabled);
}

//static
void HBPrefsCoolImpl::onClickDownloadDict(void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	if (self)
	{
		LLComboBox* combo = self->getChild<LLComboBox>("download_dict_combo");
		if (combo)
		{
			std::string label = combo->getSelectedItemLabel();
			if (!label.empty())
			{
				std::string dict_name = self->getDictName(label);
				std::string url = gSavedSettings.getString("SpellCheckDictDownloadURL");
				LLHTTPClient::get(url + dict_name + ".aff",
								  new DictionaryDownload(dict_name + ".aff"));
				LLHTTPClient::get(url + dict_name + ".dic",
								  new DictionaryDownload(dict_name + ".dic"));
			}
		}
	}
}

//static
void HBPrefsCoolImpl::onClickResetAvatarSize(void* user_data)
{
	// Reset to the standard bounding box
	LLControlVariable* controlp = gSavedSettings.getControl("AvatarDepth");
	if (controlp)
	{
		controlp->resetToDefault(true);
	}
	controlp = gSavedSettings.getControl("AvatarWidth");
	if (controlp)
	{
		controlp->resetToDefault(true);
	}
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxSpeedRez(LLUICtrl* ctrl, void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enabled = check->get();
	self->childSetEnabled("speed_rez_interval", enabled);
	self->childSetEnabled("speed_rez_seconds", enabled);
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxPrivateLookAt(LLUICtrl* ctrl,
													void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enabled = check->get();
	self->childSetEnabled("private_look_at_limit", enabled);
	self->childSetEnabled("private_look_at_limit_meters", enabled);
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxPrivatePointAt(LLUICtrl* ctrl,
													 void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enabled = check->get();
	self->childSetEnabled("private_point_at_limit", enabled);
	self->childSetEnabled("private_point_at_limit_meters", enabled);
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxAvatarPhysics(LLUICtrl* ctrl,
													void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enabled = check->get();
	self->childSetEnabled("avatar_physics_lod", enabled);
}

//static
void HBPrefsCoolImpl::onCommitCheckBoxRestrainedLove(LLUICtrl* ctrl,
													 void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;
	if (!self || !check) return;

	bool enable = check->get();
	self->refreshRestrainedLove(enable);
	if ((bool)self->mRestrainedLove != enable)
	{
		LLNotifications::instance().add("InEffectAfterRestart");
	}
}

//static
void HBPrefsCoolImpl::onCommitUserProfile(LLUICtrl* ctrl, void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	LLRadioGroup* radio = (LLRadioGroup*)ctrl;
	if (!self || !radio) return;

	std::string blacklist;
	U32 profile = radio->getSelectedIndex();
	switch (profile)
	{
		case 0:
			blacklist.clear();
			break;
		case 1:
			blacklist = RRInterface::sRolePlayBlackList;
			break;
		case 2:
			blacklist = RRInterface::sVanillaBlackList;
			break;
		default:
			blacklist = gSavedSettings.getString("RestrainedLoveBlacklist");
			break;
	}
	gSavedSettings.setString("RestrainedLoveBlacklist",	blacklist);

	if (self->mRestrainedLoveUserProfile != profile)
	{
		LLNotifications::instance().add("InEffectAfterRestart");
	}
	self->mRestrainedLoveUserProfile = profile;
}

//static
void HBPrefsCoolImpl::onClickCustomBlackList(void* user_data)
{
	HBPrefsCoolImpl* self = (HBPrefsCoolImpl*)user_data;
	if (!self) return;
	HBFloaterBlacklistRLV::showInstance();
	self->mWatchBlackListFloater = true;
}

//---------------------------------------------------------------------------

HBPrefsCool::HBPrefsCool()
:	impl(* new HBPrefsCoolImpl())
{
}

HBPrefsCool::~HBPrefsCool()
{
	delete &impl;
}

void HBPrefsCool::apply()
{
	HBFloaterBlacklistRLV::closeInstance();
	impl.apply();
}

void HBPrefsCool::cancel()
{
	HBFloaterBlacklistRLV::closeInstance();
	impl.cancel();
}

LLPanel* HBPrefsCool::getPanel()
{
	return &impl;
}
