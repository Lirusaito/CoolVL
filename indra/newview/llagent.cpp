/**
 * @file llagent.cpp
 * @brief LLAgent class implementation
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

#include "stdtypes.h"
#include "stdenums.h"

#include "llagent.h"

#include "imageids.h"
#include "indra_constants.h"
#include "llbutton.h"
#include "llcamera.h"
#include "llcoordframe.h"
#include "llcriticaldamp.h"
#include "llevent.h"
#include "llfocusmgr.h"
#include "llglheaders.h"
#include "llmath.h"
#include "llmenugl.h"
#include "llmd5.h"
#include "llnotifications.h"
#include "llparcel.h"
#include "llpermissions.h"
#include "llquantize.h"
#include "llquaternion.h"
#include "llregionhandle.h"
#include "llrendersphere.h"
#include "llsdutil.h"
#include "llsmoothstep.h"
#include "llui.h"				// for make_ui_sound
#include "lluictrlfactory.h"
#include "m3math.h"
#include "m4math.h"
#include "message.h"
#include "roles_constants.h"
#include "v3math.h"
#include "v4math.h"

#include "llagentwearables.h"
#include "llappviewer.h"
#include "llbox.h"
#include "llcallingcard.h"
#include "llchatbar.h"
#include "llconsole.h"
#include "lldrawable.h"
#include "llface.h"
#include "llfirstuse.h"
#include "llfloater.h"
#include "llfloateractivespeakers.h"
#include "llfloateravatarinfo.h"
#include "llfloaterbuildoptions.h"
#include "llfloatercamera.h"
#include "llfloaterchat.h"
#include "llfloatercustomize.h"
#include "llfloaterdirectory.h"
#include "llfloatergroupinfo.h"
#include "llfloatergroups.h"
#include "llfloaterland.h"
#include "llfloatermap.h"
#include "llfloatermute.h"
#include "llfloatersnapshot.h"
#include "llfloatertools.h"
#include "llfloaterworldmap.h"
#include "llfollowcam.h"
#include "llgroupmgr.h"
#include "llhomelocationresponder.h"
#include "llhudeffectlookat.h"
#include "llhudmanager.h"
#include "llimview.h"
#include "llinventorymodel.h"
#include "lljoystickbutton.h"
#include "llmorphview.h"
#include "llmoveview.h"
#include "llselectmgr.h"
#include "llsky.h"
#include "llstatusbar.h"
#include "llstartup.h"
#include "lltexturestats.h"
#include "lltool.h"
#include "lltoolcomp.h"
#include "lltoolfocus.h"
#include "lltoolgrab.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolview.h"
#include "llurldispatcher.h"
#include "llviewercamera.h"
#include "llviewercontrol.h"
#include "llviewerinventory.h"
#include "llviewerjoystick.h"
#include "llviewermediafocus.h"
#include "llviewermenu.h"
#include "llviewernetwork.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerparceloverlay.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llviewerdisplay.h"
#include "llvoavatarself.h"
#include "llvoground.h"
#include "llvosky.h"
#include "llwearable.h"
#include "llwearablelist.h"
#include "llworld.h"
#include "llworldmap.h"
#include "pipeline.h"

using namespace LLOldEvents;
using namespace LLVOAvatarDefines;

extern LLMenuBarGL* gMenuBarView;

//drone wandering constants
const F32 MAX_WANDER_TIME = 20.f;							// seconds
const F32 MAX_HEADING_HALF_ERROR = 0.2f;					// radians
const F32 WANDER_MAX_SLEW_RATE = 2.f * DEG_TO_RAD;			// radians / frame
const F32 WANDER_TARGET_MIN_DISTANCE = 10.f;				// meters

// Autopilot constants
const F32 AUTOPILOT_HEADING_HALF_ERROR = 10.f * DEG_TO_RAD;	// radians
const F32 AUTOPILOT_MAX_SLEW_RATE = 1.f * DEG_TO_RAD;		// radians / frame
const F32 AUTOPILOT_STOP_DISTANCE = 2.f;					// meters
const F32 AUTOPILOT_HEIGHT_ADJUST_DISTANCE = 8.f;			// meters
const F32 AUTOPILOT_MIN_TARGET_HEIGHT_OFF_GROUND = 1.f;		// meters
const F32 AUTOPILOT_MAX_TIME_NO_PROGRESS = 1.5f;			// seconds

// face editing constants
const LLVector3d FACE_EDIT_CAMERA_OFFSET(0.4f, -0.05f, 0.07f);
const LLVector3d FACE_EDIT_TARGET_OFFSET(0.f, 0.f, 0.05f);

// Mousewheel camera zoom
const F32 MIN_ZOOM_FRACTION = 0.25f;
const F32 INITIAL_ZOOM_FRACTION = 1.f;
const F32 MAX_ZOOM_FRACTION = 8.f;
const F32 METERS_PER_WHEEL_CLICK = 1.f;

const F32 MAX_TIME_DELTA = 1.f;

const F32 CAMERA_ZOOM_HALF_LIFE = 0.07f;	// seconds
const F32 FOV_ZOOM_HALF_LIFE = 0.07f;	// seconds

const F32 CAMERA_FOCUS_HALF_LIFE = 0.f;//0.02f;
const F32 CAMERA_LAG_HALF_LIFE = 0.25f;
const F32 MIN_CAMERA_LAG = 0.5f;
const F32 MAX_CAMERA_LAG = 5.f;

const F32 CAMERA_COLLIDE_EPSILON = 0.1f;
const F32 MIN_CAMERA_DISTANCE = 0.1f;
const F32 AVATAR_ZOOM_MIN_X_FACTOR = 0.55f;
const F32 AVATAR_ZOOM_MIN_Y_FACTOR = 0.7f;
const F32 AVATAR_ZOOM_MIN_Z_FACTOR = 1.15f;

const F32 MAX_CAMERA_DISTANCE_FROM_AGENT = 50.f;

const F32 MAX_CAMERA_SMOOTH_DISTANCE = 50.0f;

const F32 HEAD_BUFFER_SIZE = 0.3f;
const F32 CUSTOMIZE_AVATAR_CAMERA_ANIM_SLOP = 0.2f;

const F32 LAND_MIN_ZOOM = 0.15f;
const F32 AVATAR_MIN_ZOOM = 0.5f;
const F32 OBJECT_MIN_ZOOM = 0.02f;

const F32 APPEARANCE_MIN_ZOOM = 0.39f;
const F32 APPEARANCE_MAX_ZOOM = 8.f;

// fidget constants
const F32 MIN_FIDGET_TIME = 8.f; // seconds
const F32 MAX_FIDGET_TIME = 20.f; // seconds

const S32 MAX_NUM_CHAT_POSITIONS = 10;
const F32 GROUND_TO_AIR_CAMERA_TRANSITION_TIME = 0.5f;
const F32 GROUND_TO_AIR_CAMERA_TRANSITION_START_TIME = 0.5f;

const F32 MAX_VELOCITY_AUTO_LAND_SQUARED = 4.f * 4.f;

const F32 MAX_FOCUS_OFFSET = 20.f;

const F32 OBJECT_EXTENTS_PADDING = 0.5f;

const F32 MIN_RADIUS_ALPHA_SIZZLE = 0.5f;

const F64 CHAT_AGE_FAST_RATE = 3.0;

// The agent instance.
LLAgent gAgent;

//
// Statics
//

const F32 LLAgent::TYPING_TIMEOUT_SECS = 5.f;

std::map<std::string, std::string> LLAgent::sTeleportErrorMessages;
std::map<std::string, std::string> LLAgent::sTeleportProgressMessages;

class LLAgentFriendObserver : public LLFriendObserver
{
public:
	LLAgentFriendObserver() {}
	virtual ~LLAgentFriendObserver() {}
	virtual void changed(U32 mask);
};

void LLAgentFriendObserver::changed(U32 mask)
{
	// if there's a change we're interested in.
	if ((mask & LLFriendObserver::POWERS) != 0)
	{
		gAgent.friendsChanged();
	}
}

// Constructors and Destructors

// JC - Please try to make this order match the order in the header
// file.  Otherwise it's hard to find variables that aren't initialized.
//-----------------------------------------------------------------------------
// LLAgent()
//-----------------------------------------------------------------------------
LLAgent::LLAgent()
:	mDrawDistance(DEFAULT_FAR_PLANE),

	mGroupPowers(0),
	mHideGroupTitle(FALSE),
	mGroupID(),

	mMapOriginX(0.F),
	mMapOriginY(0.F),
	mMapWidth(0),
	mMapHeight(0),

	mLookAt(NULL),
	mPointAt(NULL),

	mHUDTargetZoom(1.f),
	mHUDCurZoom(1.f),
	mInitialized(FALSE),
	mForceMouselook(FALSE),

	mDoubleTapRunTimer(),
	mDoubleTapRunMode(DOUBLETAP_NONE),

	mbAlwaysRun(false),
	mbRunning(false),

	mAgentAccess(new LLAgentAccess()),
	mTeleportState(TELEPORT_NONE),
	mRegionp(NULL),

	mAgentOriginGlobal(),
	mPositionGlobal(),

	mDistanceTraveled(0.F),
	mLastPositionGlobal(LLVector3d::zero),

	mRenderState(0),
	mTypingTimer(),

	mCameraMode(CAMERA_MODE_THIRD_PERSON),
	mLastCameraMode(CAMERA_MODE_THIRD_PERSON),
	mViewsPushed(FALSE),

	mCustomAnim(FALSE),
	mShowAvatar(TRUE),
	mCameraAnimating(FALSE),
	mAnimationCameraStartGlobal(),
	mAnimationFocusStartGlobal(),
	mAnimationTimer(),
	mAnimationDuration(0.33f),

	mCameraFOVZoomFactor(0.f),
	mCameraCurrentFOVZoomFactor(0.f),
	mCameraFocusOffset(),
	mCameraFOVDefault(DEFAULT_FIELD_OF_VIEW),

	mCameraOffsetDefault(),
	mCameraFocusOffsetDefault(),
	mCameraCollidePlane(),

	mCurrentCameraDistance(2.f),		// meters, set in init()
	mTargetCameraDistance(2.f),
	mCameraZoomFraction(1.f),			// deprecated
	mThirdPersonHeadOffset(0.f, 0.f, 1.f),
	mSitCameraEnabled(FALSE),
	mCameraSmoothingLastPositionGlobal(),
	mCameraSmoothingLastPositionAgent(),
	mCameraSmoothingStop(FALSE),

	mCameraUpVector(LLVector3::z_axis), // default is straight up

	mFocusOnAvatar(TRUE),
	mFocusGlobal(),
	mFocusTargetGlobal(),
	mFocusObject(NULL),
	mFocusObjectDist(0.f),
	mFocusObjectOffset(),
	mFocusDotRadius(0.1f),			// meters
	mTrackFocusObject(TRUE),
	mUIOffset(0.f),

	mFrameAgent(),

	mIsBusy(FALSE),

	mAtKey(0), // Either 1, 0, or -1... indicates that movement-key is pressed
	mWalkKey(0), // like AtKey, but causes less forward thrust
	mLeftKey(0),
	mUpKey(0),
	mYawKey(0.f),
	mPitchKey(0),

	mOrbitLeftKey(0.f),
	mOrbitRightKey(0.f),
	mOrbitUpKey(0.f),
	mOrbitDownKey(0.f),
	mOrbitInKey(0.f),
	mOrbitOutKey(0.f),

	mPanUpKey(0.f),
	mPanDownKey(0.f),
	mPanLeftKey(0.f),
	mPanRightKey(0.f),
	mPanInKey(0.f),
	mPanOutKey(0.f),

	mControlFlags(0x00000000),
	mbFlagsDirty(FALSE),
	mbFlagsNeedReset(FALSE),

	mbJump(FALSE),

	mAutoPilot(FALSE),
	mAutoPilotFlyOnStop(FALSE),
	mAutoPilotTargetGlobal(),
	mAutoPilotStopDistance(1.f),
	mAutoPilotUseRotation(FALSE),
	mAutoPilotTargetFacing(LLVector3::zero),
	mAutoPilotTargetDist(0.f),
	mAutoPilotNoProgressFrameCount(0),
	mAutoPilotRotationThreshold(0.f),
	mAutoPilotFinishedCallback(NULL),
	mAutoPilotCallbackData(NULL),

	mCapabilities(),

	mEffectColor(0.f, 1.f, 1.f, 1.f),

	mHaveHomePosition(FALSE),
	mHomeRegionHandle(0),
	mNearChatRadius(CHAT_NORMAL_RADIUS / 2.f),

	mNextFidgetTime(0.f),
	mCurrentFidget(0),
	mFirstLogin(FALSE),
	mGenderChosen(FALSE),
	mAppearanceSerialNum(0),
	mbTeleportKeepsLookAt(false)
{
	U32 i;
	for (i = 0; i < TOTAL_CONTROLS; ++i)
	{
		mControlsTakenCount[i] = 0;
		mControlsTakenPassedOnCount[i] = 0;
	}

	mFollowCam.setMaxCameraDistantFromSubject(MAX_CAMERA_DISTANCE_FROM_AGENT);
}

// Requires gSavedSettings to be initialized.
//-----------------------------------------------------------------------------
// init()
//-----------------------------------------------------------------------------
void LLAgent::init()
{
	mDrawDistance = gSavedSettings.getF32("RenderFarClip");

	// *Note: this is where LLViewerCamera::getInstance() used to be constructed.

	LLViewerCamera::getInstance()->setView(DEFAULT_FIELD_OF_VIEW);
	// Leave at 0.1 meters until we have real near clip management
	LLViewerCamera::getInstance()->setNear(0.1f);
	// if you want to change camera settings, do so in camera.h
	LLViewerCamera::getInstance()->setFar(mDrawDistance);
	// default, overridden in LLViewerWindow::reshape
	LLViewerCamera::getInstance()->setAspect(gViewerWindow->getDisplayAspectRatio());
	// default, overridden in LLViewerWindow::reshape
	LLViewerCamera::getInstance()->setViewHeightInPixels(768);

	setFlying(gSavedSettings.getBOOL("FlyingAtExit"));

	mCameraFocusOffsetTarget = LLVector4(gSavedSettings.getVector3("CameraOffsetBuild"));
	mCameraOffsetDefault = gSavedSettings.getVector3("CameraOffsetDefault");
	mCameraFocusOffsetDefault = gSavedSettings.getVector3("FocusOffsetDefault");
	mCameraCollidePlane.clearVec();
	mCurrentCameraDistance = mCameraOffsetDefault.magVec() * gSavedSettings.getF32("CameraOffsetScale");
	mTargetCameraDistance = mCurrentCameraDistance;
	mCameraZoomFraction = 1.f;
	mTrackFocusObject = gSavedSettings.getBOOL("TrackFocusObject");

	//LLDebugVarMessageBox::show("Camera Lag", &CAMERA_FOCUS_HALF_LIFE, 0.5f, 0.01f);

	mEffectColor = gSavedSettings.getColor4("EffectColor");

	LLControlVariable* maturity = gSavedSettings.getControl("PreferredMaturity");
	if (maturity)
	{
		maturity->getValidateSignal()->connect(boost::bind(&LLAgent::validateMaturity,
														   this, _2));
		maturity->getSignal()->connect(boost::bind(&LLAgent::handleMaturity,
												   this, _2));
	}

	mInitialized = TRUE;
}

//-----------------------------------------------------------------------------
// cleanup()
//-----------------------------------------------------------------------------
void LLAgent::cleanup()
{
	setSitCamera(LLUUID::null);
	if (mLookAt)
	{
		mLookAt->markDead();
		mLookAt = NULL;
	}
	if (mPointAt)
	{
		mPointAt->markDead();
		mPointAt = NULL;
	}
	mRegionp = NULL;
	setFocusObject(NULL);
}

//-----------------------------------------------------------------------------
// LLAgent()
//-----------------------------------------------------------------------------
LLAgent::~LLAgent()
{
	cleanup();

	delete mAgentAccess;
	mAgentAccess = NULL;

	// *Note: this is where LLViewerCamera::getInstance() used to be deleted.
}

// Change camera back to third person, stop the autopilot,
// deselect stuff, etc.
//-----------------------------------------------------------------------------
// resetView()
//-----------------------------------------------------------------------------
void LLAgent::resetView(BOOL reset_camera, BOOL change_camera)
{
	if (mAutoPilot)
	{
		stopAutoPilot(TRUE);
	}

	if (!gNoRender)
	{
		LLSelectMgr::getInstance()->unhighlightAll();

		// By popular request, keep land selection while walking around. JC
		//LLViewerParcelMgr::getInstance()->deselectLand();

		// Force deselect when walking and attachment is selected; this is so
		// people don't wig out when their avatar moves without animating
		if (LLSelectMgr::getInstance()->getSelection()->isAttachment())
		{
			LLSelectMgr::getInstance()->deselectAll();
		}

		// Hide all popup menus
		if (gMenuHolder)
		{
			gMenuHolder->hideMenus();
		}
	}

	if (change_camera && !LLPipeline::sFreezeTime)
	{
		changeCameraToDefault();

		if (LLViewerJoystick::getInstance()->getOverrideCamera())
		{
			handle_toggle_flycam();
		}

		// reset avatar mode from eventual residual motion
		if (LLToolMgr::getInstance()->inBuildMode())
		{
			LLViewerJoystick::getInstance()->moveAvatar(true);
		}

		if (gFloaterTools)
		{
			gFloaterTools->close();
		}

		if (gViewerWindow)
		{
			gViewerWindow->showCursor();
		}

		// Switch back to basic toolset
		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	}

	if (reset_camera && !LLPipeline::sFreezeTime)
	{
		if (gViewerWindow && !gViewerWindow->getLeftMouseDown() && cameraThirdPerson())
		{
			// leaving mouse-steer mode
			LLVector3 agent_at_axis = getAtAxis();
			agent_at_axis -= projected_vec(agent_at_axis, getReferenceUpVector());
			agent_at_axis.normalize();
			gAgent.resetAxes(lerp(getAtAxis(), agent_at_axis,
								  LLCriticalDamp::getInterpolant(0.3f)));
		}

		setFocusOnAvatar(TRUE, ANIMATE);
	}

	mHUDTargetZoom = 1.f;
}

// Handle any actions that need to be performed when the main app gains focus
// (such as through alt-tab).
//-----------------------------------------------------------------------------
// onAppFocusGained()
//-----------------------------------------------------------------------------
void LLAgent::onAppFocusGained()
{
//MK
	if (gRRenabled)
	{
		return;
	}
//mk
	if (CAMERA_MODE_MOUSELOOK == mCameraMode)
	{
		changeCameraToDefault();
		LLToolMgr::getInstance()->clearSavedTool();
	}
}

void LLAgent::ageChat()
{
	if (isAgentAvatarValid())
	{
		// get amount of time since I last chatted
		F64 elapsed_time = (F64)gAgentAvatarp->mChatTimer.getElapsedTimeF32();
		// add in frame time * 3 (so it ages 4x)
		gAgentAvatarp->mChatTimer.setAge(elapsed_time + (F64)gFrameDTClamped * (CHAT_AGE_FAST_RATE - 1.0));
	}
}

// Allow camera to be moved somewhere other than behind avatar.
//-----------------------------------------------------------------------------
// unlockView()
//-----------------------------------------------------------------------------
void LLAgent::unlockView()
{
	if (getFocusOnAvatar())
	{
		if (isAgentAvatarValid())
		{
			setFocusGlobal(LLVector3d::zero, gAgentAvatarp->mID);
		}
		setFocusOnAvatar(FALSE, FALSE);	// no animation
	}
}

//-----------------------------------------------------------------------------
// moveAt()
//-----------------------------------------------------------------------------
void LLAgent::moveAt(S32 direction, bool reset)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mAtKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_AT_POS | AGENT_CONTROL_FAST_AT);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_AT_NEG | AGENT_CONTROL_FAST_AT);
	}

	if (reset)
	{
		resetView();
	}
}

//-----------------------------------------------------------------------------
// moveAtNudge()
//-----------------------------------------------------------------------------
void LLAgent::moveAtNudge(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mWalkKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_AT_POS);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_AT_NEG);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveLeft()
//-----------------------------------------------------------------------------
void LLAgent::moveLeft(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mLeftKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_LEFT_POS | AGENT_CONTROL_FAST_LEFT);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_LEFT_NEG | AGENT_CONTROL_FAST_LEFT);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveLeftNudge()
//-----------------------------------------------------------------------------
void LLAgent::moveLeftNudge(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mLeftKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_LEFT_POS);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_LEFT_NEG);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveUp()
//-----------------------------------------------------------------------------
void LLAgent::moveUp(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mUpKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_UP_POS | AGENT_CONTROL_FAST_UP);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_UP_NEG | AGENT_CONTROL_FAST_UP);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveYaw()
//-----------------------------------------------------------------------------
void LLAgent::moveYaw(F32 mag, bool reset_view)
{
	mYawKey = mag;

	if (mag > 0)
	{
		setControlFlags(AGENT_CONTROL_YAW_POS);
	}
	else if (mag < 0)
	{
		setControlFlags(AGENT_CONTROL_YAW_NEG);
	}

    if (reset_view)
	{
        resetView();
	}
}

//-----------------------------------------------------------------------------
// movePitch()
//-----------------------------------------------------------------------------
void LLAgent::movePitch(S32 direction)
{
	setKey(direction, mPitchKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_PITCH_POS);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_PITCH_NEG);
	}
}

// Does this parcel allow you to fly?
BOOL LLAgent::canFly()
{
//MK
	if (gRRenabled && gAgent.mRRInterface.mContainsFly)
	{
		return FALSE;
	}
//mk
	if (isGodlike()) return TRUE;

	LLViewerRegion* regionp = getRegion();
	if (regionp && regionp->getBlockFly()) return FALSE;

	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (!parcel) return FALSE;

	// Allow owners to fly on their own land.
	if (LLViewerParcelMgr::isParcelOwnedByAgent(parcel, GP_LAND_ALLOW_FLY))
	{
		return TRUE;
	}

	return parcel->getAllowFly();
}

//-----------------------------------------------------------------------------
// setFlying()
//-----------------------------------------------------------------------------
void LLAgent::setFlying(BOOL fly)
{
	if (fly)
	{
		if (isAgentAvatarValid() && gAgentAvatarp->mIsSitting)
		{
			// don't allow taking off while sitting
			return;
		}
//MK
		if (gRRenabled && gAgent.mRRInterface.mContainsFly)
		{
			return;
		}
//mk
		BOOL was_flying = getFlying();
		if (!canFly() && !was_flying)
		{
			// parcel doesn't let you start fly
			// gods can always fly
			// and it's OK if you're already flying
			make_ui_sound("UISndBadKeystroke");
			return;
		}
		if (!was_flying)
		{
			LLViewerStats::getInstance()->incStat(LLViewerStats::ST_FLY_COUNT);
		}
		setControlFlags(AGENT_CONTROL_FLY);
		gSavedSettings.setBOOL("FlyBtnState", TRUE);
	}
	else
	{
		clearControlFlags(AGENT_CONTROL_FLY);
		gSavedSettings.setBOOL("FlyBtnState", FALSE);
	}
	mbFlagsDirty = TRUE;
}

// UI based mechanism of setting fly state
//-----------------------------------------------------------------------------
// toggleFlying()
//-----------------------------------------------------------------------------
void LLAgent::toggleFlying()
{
	BOOL fly = !(mControlFlags & AGENT_CONTROL_FLY);

	setFlying(fly);
	resetView();
}

//-----------------------------------------------------------------------------
// setRegion()
//-----------------------------------------------------------------------------
void LLAgent::setRegion(LLViewerRegion *regionp)
{
	llassert(regionp);
	if (mRegionp != regionp)
	{
		// std::string host_name;
		// host_name = regionp->getHost().getHostName();

		std::string ip = regionp->getHost().getString();
//MK
		if (!gRRenabled || !gAgent.mRRInterface.mContainsShowloc)
		{
//mk
			llinfos << "Moving agent into region: " << regionp->getName()
				<< " located at " << ip << llendl;
//MK
		}
//mk
		if (mRegionp)
		{
			// We've changed regions, we're now going to change our agent coordinate frame.
			mAgentOriginGlobal = regionp->getOriginGlobal();
			LLVector3d agent_offset_global = mRegionp->getOriginGlobal();

			LLVector3 delta;
			delta.setVec(regionp->getOriginGlobal() - mRegionp->getOriginGlobal());

			setPositionAgent(getPositionAgent() - delta);

			LLVector3 camera_position_agent = LLViewerCamera::getInstance()->getOrigin();
			LLViewerCamera::getInstance()->setOrigin(camera_position_agent - delta);

			// Update all of the regions.
			LLWorld::getInstance()->updateAgentOffset(agent_offset_global);

			// Hack to keep sky in the agent's region, otherwise it may get deleted - DJS 08/02/02
			// *TODO: possibly refactor into gSky->setAgentRegion(regionp)? -Brad
			if (gSky.mVOSkyp)
			{
				gSky.mVOSkyp->setRegion(regionp);
			}
			if (gSky.mVOGroundp)
			{
				gSky.mVOGroundp->setRegion(regionp);
			}

		}
		else
		{
			// First time initialization.
			// We've changed regions, we're now going to change our agent coordinate frame.
			mAgentOriginGlobal = regionp->getOriginGlobal();

			LLVector3 delta;
			delta.setVec(regionp->getOriginGlobal());

			setPositionAgent(getPositionAgent() - delta);
			LLVector3 camera_position_agent = LLViewerCamera::getInstance()->getOrigin();
			LLViewerCamera::getInstance()->setOrigin(camera_position_agent - delta);

			// Update all of the regions.
			LLWorld::getInstance()->updateAgentOffset(mAgentOriginGlobal);
		}
	}
	mRegionp = regionp;

	// Must shift hole-covering water object locations because local
	// coordinate frame changed.
	LLWorld::getInstance()->updateWaterObjects();

	// keep a list of regions we've been too
	// this is just an interesting stat, logged at the dataserver
	// we could trake this at the dataserver side, but that's harder
	U64 handle = regionp->getHandle();
	mRegionsVisited.insert(handle);

	LLSelectMgr::getInstance()->updateSelectionCenter();
}

//-----------------------------------------------------------------------------
// getRegion()
//-----------------------------------------------------------------------------
LLViewerRegion *LLAgent::getRegion() const
{
	return mRegionp;
}

const LLHost& LLAgent::getRegionHost() const
{
	if (mRegionp)
	{
		return mRegionp->getHost();
	}
	else
	{
		return LLHost::invalid;
	}
}

//-----------------------------------------------------------------------------
// getSLURL()
// returns empty() if getRegion() == NULL
//-----------------------------------------------------------------------------
std::string LLAgent::getSLURL() const
{
	std::string slurl;
	LLViewerRegion *regionp = getRegion();
	if (regionp)
	{
		LLVector3d agentPos = getPositionGlobal();
		S32 x = llround((F32)fmod(agentPos.mdV[VX], (F64)REGION_WIDTH_METERS));
		S32 y = llround((F32)fmod(agentPos.mdV[VY], (F64)REGION_WIDTH_METERS));
		S32 z = llround((F32)agentPos.mdV[VZ]);
		slurl = LLURLDispatcher::buildSLURL(regionp->getName(), x, y, z);
	}
	return slurl;
}

//-----------------------------------------------------------------------------
// inPrelude()
//-----------------------------------------------------------------------------
BOOL LLAgent::inPrelude()
{
	return mRegionp && mRegionp->isPrelude();
}

//-----------------------------------------------------------------------------
// canManageEstate()
//-----------------------------------------------------------------------------

BOOL LLAgent::canManageEstate() const
{
	return mRegionp && mRegionp->canManageEstate();
}

//-----------------------------------------------------------------------------
// sendMessage()
//-----------------------------------------------------------------------------
void LLAgent::sendMessage()
{
	if (gDisconnected)
	{
		llwarns << "Trying to send message when disconnected!" << llendl;
		return;
	}
	if (!mRegionp)
	{
		llerrs << "No region for agent yet!" << llendl;
	}
	gMessageSystem->sendMessage(mRegionp->getHost());
}

//-----------------------------------------------------------------------------
// sendReliableMessage()
//-----------------------------------------------------------------------------
void LLAgent::sendReliableMessage()
{
	if (gDisconnected)
	{
		LL_DEBUGS("Agent") << "Trying to send message when disconnected !"
						   << LL_ENDL;
		return;
	}
	if (!mRegionp)
	{
		LL_DEBUGS("Agent") << "No region for agent yet, not sending message !"
						   << LL_ENDL;
		return;
	}
	gMessageSystem->sendReliable(mRegionp->getHost());
}

//-----------------------------------------------------------------------------
// getVelocity()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::getVelocity() const
{
	if (isAgentAvatarValid())
	{
		return gAgentAvatarp->getVelocity();
	}
	else
	{
		return LLVector3::zero;
	}
}

//-----------------------------------------------------------------------------
// setPositionAgent()
//-----------------------------------------------------------------------------
void LLAgent::setPositionAgent(const LLVector3 &pos_agent)
{
	if (!pos_agent.isFinite())
	{
		llerrs << "setPositionAgent is not a number" << llendl;
	}

	if (isAgentAvatarValid() && gAgentAvatarp->getParent())
	{
		LLVector3 pos_agent_sitting;
		LLVector3d pos_agent_d;
		LLViewerObject *parent = (LLViewerObject*)gAgentAvatarp->getParent();

		pos_agent_sitting = gAgentAvatarp->getPosition() * parent->getRotation() + parent->getPositionAgent();
		pos_agent_d.setVec(pos_agent_sitting);

		mFrameAgent.setOrigin(pos_agent_sitting);
		mPositionGlobal = pos_agent_d + mAgentOriginGlobal;
	}
	else
	{
		mFrameAgent.setOrigin(pos_agent);

		LLVector3d pos_agent_d;
		pos_agent_d.setVec(pos_agent);
		mPositionGlobal = pos_agent_d + mAgentOriginGlobal;
	}
}

//-----------------------------------------------------------------------------
// slamLookAt()
//-----------------------------------------------------------------------------
void LLAgent::slamLookAt(const LLVector3 &look_at)
{
	LLVector3 look_at_norm = look_at;
	look_at_norm.mV[VZ] = 0.f;
	look_at_norm.normalize();
	resetAxes(look_at_norm);
}

//-----------------------------------------------------------------------------
// getPositionGlobal()
//-----------------------------------------------------------------------------
const LLVector3d &LLAgent::getPositionGlobal() const
{
	if (isAgentAvatarValid() && !gAgentAvatarp->mDrawable.isNull())
	{
		mPositionGlobal = getPosGlobalFromAgent(gAgentAvatarp->getRenderPosition());
	}
	else
	{
		mPositionGlobal = getPosGlobalFromAgent(mFrameAgent.getOrigin());
	}

	return mPositionGlobal;
}

//-----------------------------------------------------------------------------
// getPositionAgent()
//-----------------------------------------------------------------------------
const LLVector3 &LLAgent::getPositionAgent()
{
	if (isAgentAvatarValid() && !gAgentAvatarp->mDrawable.isNull())
	{
		mFrameAgent.setOrigin(gAgentAvatarp->getRenderPosition());
	}

	return mFrameAgent.getOrigin();
}

//-----------------------------------------------------------------------------
// getRegionsVisited()
//-----------------------------------------------------------------------------
S32 LLAgent::getRegionsVisited() const
{
	return mRegionsVisited.size();
}

//-----------------------------------------------------------------------------
// getDistanceTraveled()
//-----------------------------------------------------------------------------
F64 LLAgent::getDistanceTraveled() const
{
	return mDistanceTraveled;
}

//-----------------------------------------------------------------------------
// getPosAgentFromGlobal()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::getPosAgentFromGlobal(const LLVector3d &pos_global) const
{
	LLVector3 pos_agent;
	pos_agent.setVec(pos_global - mAgentOriginGlobal);
	return pos_agent;
}

//-----------------------------------------------------------------------------
// getPosGlobalFromAgent()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::getPosGlobalFromAgent(const LLVector3 &pos_agent) const
{
	LLVector3d pos_agent_d;
	pos_agent_d.setVec(pos_agent);
	return pos_agent_d + mAgentOriginGlobal;
}

//-----------------------------------------------------------------------------
// resetAxes()
//-----------------------------------------------------------------------------
void LLAgent::resetAxes()
{
	mFrameAgent.resetAxes();
}

// Copied from LLCamera::setOriginAndLookAt
// Look_at must be unit vector
//-----------------------------------------------------------------------------
// resetAxes()
//-----------------------------------------------------------------------------
void LLAgent::resetAxes(const LLVector3 &look_at)
{
	LLVector3	skyward = getReferenceUpVector();

	// if look_at has zero length, fail
	// if look_at and skyward are parallel, fail
	//
	// Test both of these conditions with a cross product.
	LLVector3 cross(look_at % skyward);
	if (cross.isNull())
	{
		llinfos << "LLAgent::resetAxes cross-product is zero" << llendl;
		return;
	}

	// Make sure look_at and skyward are not parallel
	// and neither are zero length
	LLVector3 left(skyward % look_at);
	LLVector3 up(look_at % left);

	mFrameAgent.setAxes(look_at, left, up);
}

//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(F32 angle, const LLVector3 &axis)
{
	mFrameAgent.rotate(angle, axis);
}

//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(F32 angle, F32 x, F32 y, F32 z)
{
	mFrameAgent.rotate(angle, x, y, z);
}

//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(const LLMatrix3 &matrix)
{
	mFrameAgent.rotate(matrix);
}

//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(const LLQuaternion &quaternion)
{
	mFrameAgent.rotate(quaternion);
}

//-----------------------------------------------------------------------------
// getReferenceUpVector()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::getReferenceUpVector()
{
	// this vector is in the coordinate frame of the avatar's parent object, or the world if none
	LLVector3 up_vector = LLVector3::z_axis;
	if (isAgentAvatarValid() &&
		gAgentAvatarp->getParent() &&
		gAgentAvatarp->mDrawable.notNull())
	{
		U32 camera_mode = mCameraAnimating ? mLastCameraMode : mCameraMode;
		// and in third person...
		if (camera_mode == CAMERA_MODE_THIRD_PERSON)
		{
			// make the up vector point to the absolute +z axis
			up_vector = up_vector * ~((LLViewerObject*)gAgentAvatarp->getParent())->getRenderRotation();
		}
		else if (camera_mode == CAMERA_MODE_MOUSELOOK)
		{
			// make the up vector point to the avatar's +z axis
			up_vector = up_vector * gAgentAvatarp->mDrawable->getRotation();
		}
	}

	return up_vector;
}

// Radians, positive is forward into ground
//-----------------------------------------------------------------------------
// pitch()
//-----------------------------------------------------------------------------
void LLAgent::pitch(F32 angle)
{
	// don't let user pitch if pointed almost all the way down or up
	mFrameAgent.pitch(clampPitchToLimits(angle));
}

// Radians, positive is forward into ground
//-----------------------------------------------------------------------------
// clampPitchToLimits()
//-----------------------------------------------------------------------------
F32 LLAgent::clampPitchToLimits(F32 angle)
{
	// A dot B = mag(A) * mag(B) * cos(angle between A and B)
	// so... cos(angle between A and B) = A dot B / mag(A) / mag(B)
	//                                  = A dot B for unit vectors

	LLVector3 skyward = getReferenceUpVector();

	F32			look_down_limit;
	F32			look_up_limit = 10.f * DEG_TO_RAD;

	F32 angle_from_skyward = acos(mFrameAgent.getAtAxis() * skyward);

	if (isAgentAvatarValid() && gAgentAvatarp->mIsSitting)
	{
		look_down_limit = 130.f * DEG_TO_RAD;
	}
	else
	{
		look_down_limit = 170.f * DEG_TO_RAD;
	}

	// clamp pitch to limits
	if ((angle >= 0.f) && (angle_from_skyward + angle > look_down_limit))
	{
		angle = look_down_limit - angle_from_skyward;
	}
	else if ((angle < 0.f) && (angle_from_skyward + angle < look_up_limit))
	{
		angle = look_up_limit - angle_from_skyward;
	}

    return angle;
}

//-----------------------------------------------------------------------------
// roll()
//-----------------------------------------------------------------------------
void LLAgent::roll(F32 angle)
{
	mFrameAgent.roll(angle);
}

//-----------------------------------------------------------------------------
// yaw()
//-----------------------------------------------------------------------------
void LLAgent::yaw(F32 angle)
{
	if (!rotateGrabbed())
	{
		mFrameAgent.rotate(angle, getReferenceUpVector());
	}
}

// Returns a quat that represents the rotation of the agent in the absolute frame
//-----------------------------------------------------------------------------
// getQuat()
//-----------------------------------------------------------------------------
LLQuaternion LLAgent::getQuat() const
{
	return mFrameAgent.getQuaternion();
}

//-----------------------------------------------------------------------------
// calcFocusOffset()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::calcFocusOffset(LLViewerObject* object,
								   LLVector3 original_focus_point,
								   S32 x, S32 y)
{
	LLVector3 obj_pos = object->getRenderPosition();

	// if is avatar - don't do any funk heuristics to position the focal point
	// see DEV-30589
	if (object->isAvatar() || !gViewerWindow)
	{
		return original_focus_point - obj_pos;
	}

	LLMatrix4 obj_matrix = object->getRenderMatrix();
	LLQuaternion obj_rot = object->getRenderRotation();
	LLViewerCamera* camera = LLViewerCamera::getInstance();

	LLQuaternion inv_obj_rot = ~obj_rot; // get inverse of rotation
	LLVector3 object_extents = object->getScale();
	// make sure they object extents are non-zero
	object_extents.clamp(0.001f, F32_MAX);

	// obj_to_cam_ray is unit vector pointing from object center to camera, in
	// the coordinate frame of the object
	LLVector3 obj_to_cam_ray = obj_pos - camera->getOrigin();
	obj_to_cam_ray.rotVec(inv_obj_rot);
	obj_to_cam_ray.normalize();

	// obj_to_cam_ray_proportions are the (positive) ratios of
	// the obj_to_cam_ray x,y,z components with the x,y,z object dimensions.
	LLVector3 obj_to_cam_ray_proportions;
	obj_to_cam_ray_proportions.mV[VX] = llabs(obj_to_cam_ray.mV[VX] / object_extents.mV[VX]);
	obj_to_cam_ray_proportions.mV[VY] = llabs(obj_to_cam_ray.mV[VY] / object_extents.mV[VY]);
	obj_to_cam_ray_proportions.mV[VZ] = llabs(obj_to_cam_ray.mV[VZ] / object_extents.mV[VZ]);

	// Find the largest ratio stored in obj_to_cam_ray_proportions.
	// This corresponds to the object's local axial plane (XY, YZ, XZ) that is
	// *most* facing the camera
	LLVector3 longest_object_axis;
	// is x-axis longest?
	if (obj_to_cam_ray_proportions.mV[VX] > obj_to_cam_ray_proportions.mV[VY] &&
		obj_to_cam_ray_proportions.mV[VX] > obj_to_cam_ray_proportions.mV[VZ])
	{
		// then grab it
		longest_object_axis.setVec(obj_matrix.getFwdRow4());
	}
	// is y-axis longest?
	else if (obj_to_cam_ray_proportions.mV[VY] > obj_to_cam_ray_proportions.mV[VZ])
	{
		// then grab it
		longest_object_axis.setVec(obj_matrix.getLeftRow4());
	}
	// otherwise, use z axis
	else
	{
		longest_object_axis.setVec(obj_matrix.getUpRow4());
	}

	// Use this axis as the normal to project mouse click on to plane with that
	// normal, at the object center.
	// This generates a point behind the mouse cursor that is approximately in
	// the middle of the object in terms of depth.
	// We do this to allow the camera rotation tool to "tumble" the object by
	// rotating the camera.
	// If the focus point were the object surface under the mouse, camera
	// rotation would introduce an undesirable eccentricity to the object
	// orientation
	LLVector3 focus_plane_normal(longest_object_axis);
	focus_plane_normal.normalize();

	LLVector3d focus_pt_global;
	gViewerWindow->mousePointOnPlaneGlobal(focus_pt_global, x, y,
										   gAgent.getPosGlobalFromAgent(obj_pos),
										   focus_plane_normal);
	LLVector3 focus_pt = gAgent.getPosAgentFromGlobal(focus_pt_global);

	// find vector from camera to focus point in object space
	LLVector3 camera_to_focus_vec = focus_pt - camera->getOrigin();
	camera_to_focus_vec.rotVec(inv_obj_rot);

	// find vector from object origin to focus point in object coordinates
	LLVector3 focus_offset_from_object_center = focus_pt - obj_pos;
	// convert to object-local space
	focus_offset_from_object_center.rotVec(inv_obj_rot);

	// We need to project the focus point back into the bounding box of the
	// focused object.
	// Do this by calculating the XYZ scale factors needed to get focus offset
	// back in bounds along the camera_focus axis
	LLVector3 clip_fraction;

	// for each axis...
	for (U32 axis = VX; axis <= VZ; ++axis)
	{
		//...calculate distance that focus offset sits outside of bounding box
		// along that axis... NOTE: dist_out_of_bounds keeps the sign of
		// focus_offset_from_object_center
		F32 dist_out_of_bounds;
		if (focus_offset_from_object_center.mV[axis] > 0.f)
		{
			dist_out_of_bounds = llmax(0.f,
									   focus_offset_from_object_center.mV[axis] - (object_extents.mV[axis] * 0.5f));
		}
		else
		{
			dist_out_of_bounds = llmin(0.f,
									   focus_offset_from_object_center.mV[axis] + (object_extents.mV[axis] * 0.5f));
		}

		//...then calculate the scale factor needed to push camera_to_focus_vec
		// back in bounds along current axis
		if (llabs(camera_to_focus_vec.mV[axis]) < 0.0001f)
		{
			// don't divide by very small number
			clip_fraction.mV[axis] = 0.f;
		}
		else
		{
			clip_fraction.mV[axis] = dist_out_of_bounds / camera_to_focus_vec.mV[axis];
		}
	}

	LLVector3 abs_clip_fraction = clip_fraction;
	abs_clip_fraction.abs();

	// find axis of focus offset that is *most* outside the bounding box and
	// use that to rescale focus offset to inside object extents
	if (abs_clip_fraction.mV[VX] > abs_clip_fraction.mV[VY] &&
		abs_clip_fraction.mV[VX] > abs_clip_fraction.mV[VZ])
	{
		focus_offset_from_object_center -= clip_fraction.mV[VX] * camera_to_focus_vec;
	}
	else if (abs_clip_fraction.mV[VY] > abs_clip_fraction.mV[VZ])
	{
		focus_offset_from_object_center -= clip_fraction.mV[VY] * camera_to_focus_vec;
	}
	else
	{
		focus_offset_from_object_center -= clip_fraction.mV[VZ] * camera_to_focus_vec;
	}

	// convert back to world space
	focus_offset_from_object_center.rotVec(obj_rot);

	// now, based on distance of camera from object relative to object size
	// push the focus point towards the near surface of the object when
	// (relatively) close to the object or keep the focus point in the object
	// middle when (relatively) far.
	// NOTE: leave focus point in middle of avatars, since the behavior you
	// want when alt-zooming on avatars is almost always "tumble about middle"
	// and not "spin around surface point"
	LLVector3 obj_rel = original_focus_point - object->getRenderPosition();

	// Now that we have the object relative position, we should bias toward the
	// center of the object based on the distance of the camera to the focus
	// point vs. the distance of the camera to the focus.
	F32 relDist = llabs(obj_rel * camera->getAtAxis());
	F32 viewDist = dist_vec(obj_pos + obj_rel, camera->getOrigin());

	LLBBox obj_bbox = object->getBoundingBoxAgent();
	F32 bias = 0.f;

	// virtual_camera_pos is the camera position we are simulating by backing
	// the camera off and adjusting the FOV
	LLVector3 virtual_camera_pos = gAgent.getPosAgentFromGlobal(mFocusTargetGlobal + (getCameraPositionGlobal() - mFocusTargetGlobal) / (1.f + mCameraFOVZoomFactor));

	// If the camera is inside the object (large, hollow objects, for example)
	// leave focus point all the way to destination depth, away from object
	// center
	if (!obj_bbox.containsPointAgent(virtual_camera_pos))
	{
		// Perform magic number biasing of focus point towards surface vs.
		// planar center
		bias = clamp_rescale(relDist/viewDist, 0.1f, 0.7f, 0.0f, 1.0f);
		obj_rel = lerp(focus_offset_from_object_center, obj_rel, bias);
	}

	return obj_rel;
}

//-----------------------------------------------------------------------------
// calcCameraMinDistance()
//-----------------------------------------------------------------------------
BOOL LLAgent::calcCameraMinDistance(F32 &obj_min_distance)
{
	BOOL soft_limit = FALSE; // is the bounding box to be treated literally (volumes) or as an approximation (avatars)

	if (!mFocusObject || mFocusObject->isDead() || mFocusObject->isMesh())
	{
		obj_min_distance = 0.f;
		return TRUE;
	}

	if (mFocusObject->mDrawable.isNull())
	{
#ifdef LL_RELEASE_FOR_DOWNLOAD
		llwarns << "Focus object with no drawable!" << llendl;
#else
		mFocusObject->dump();
		llerrs << "Focus object with no drawable!" << llendl;
#endif
		obj_min_distance = 0.f;
		return TRUE;
	}

	LLQuaternion inv_object_rot = ~mFocusObject->getRenderRotation();
	LLVector3 target_offset_origin = mFocusObjectOffset;
	LLVector3 camera_offset_target(getCameraPositionAgent() - getPosAgentFromGlobal(mFocusTargetGlobal));

	// convert offsets into object local space
	camera_offset_target.rotVec(inv_object_rot);
	target_offset_origin.rotVec(inv_object_rot);

	// push around object extents based on target offset
	LLVector3 object_extents = mFocusObject->getScale();
	if (mFocusObject->isAvatar())
	{
		// fudge factors that lets you zoom in on avatars a bit more (which don't do FOV zoom)
		object_extents.mV[VX] *= AVATAR_ZOOM_MIN_X_FACTOR;
		object_extents.mV[VY] *= AVATAR_ZOOM_MIN_Y_FACTOR;
		object_extents.mV[VZ] *= AVATAR_ZOOM_MIN_Z_FACTOR;
		soft_limit = TRUE;
	}
	LLVector3 abs_target_offset = target_offset_origin;
	abs_target_offset.abs();

	LLVector3 target_offset_dir = target_offset_origin;
	F32 object_radius = mFocusObject->getVObjRadius();

	BOOL target_outside_object_extents = FALSE;

	for (U32 i = VX; i <= VZ; ++i)
	{
		if (abs_target_offset.mV[i] * 2.f > object_extents.mV[i] + OBJECT_EXTENTS_PADDING)
		{
			target_outside_object_extents = TRUE;
		}
		if (camera_offset_target.mV[i] > 0.f)
		{
			object_extents.mV[i] -= target_offset_origin.mV[i] * 2.f;
		}
		else
		{
			object_extents.mV[i] += target_offset_origin.mV[i] * 2.f;
		}
	}

	// don't shrink the object extents so far that the object inverts
	object_extents.clamp(0.001f, F32_MAX);

	// move into first octant
	LLVector3 camera_offset_target_abs_norm = camera_offset_target;
	camera_offset_target_abs_norm.abs();
	// make sure offset is non-zero
	camera_offset_target_abs_norm.clamp(0.001f, F32_MAX);
	camera_offset_target_abs_norm.normalize();

	// find camera position relative to normalized object extents
	LLVector3 camera_offset_target_scaled = camera_offset_target_abs_norm;
	camera_offset_target_scaled.mV[VX] /= object_extents.mV[VX];
	camera_offset_target_scaled.mV[VY] /= object_extents.mV[VY];
	camera_offset_target_scaled.mV[VZ] /= object_extents.mV[VZ];

	if (camera_offset_target_scaled.mV[VX] > camera_offset_target_scaled.mV[VY] &&
		camera_offset_target_scaled.mV[VX] > camera_offset_target_scaled.mV[VZ])
	{
		if (camera_offset_target_abs_norm.mV[VX] < 0.001f)
		{
			obj_min_distance = object_extents.mV[VX] * 0.5f;
		}
		else
		{
			obj_min_distance = object_extents.mV[VX] * 0.5f / camera_offset_target_abs_norm.mV[VX];
		}
	}
	else if (camera_offset_target_scaled.mV[VY] > camera_offset_target_scaled.mV[VZ])
	{
		if (camera_offset_target_abs_norm.mV[VY] < 0.001f)
		{
			obj_min_distance = object_extents.mV[VY] * 0.5f;
		}
		else
		{
			obj_min_distance = object_extents.mV[VY] * 0.5f / camera_offset_target_abs_norm.mV[VY];
		}
	}
	else
	{
		if (camera_offset_target_abs_norm.mV[VZ] < 0.001f)
		{
			obj_min_distance = object_extents.mV[VZ] * 0.5f;
		}
		else
		{
			obj_min_distance = object_extents.mV[VZ] * 0.5f / camera_offset_target_abs_norm.mV[VZ];
		}
	}

	LLVector3 object_split_axis;
	LLVector3 target_offset_scaled = target_offset_origin;
	target_offset_scaled.abs();
	target_offset_scaled.normalize();
	target_offset_scaled.mV[VX] /= object_extents.mV[VX];
	target_offset_scaled.mV[VY] /= object_extents.mV[VY];
	target_offset_scaled.mV[VZ] /= object_extents.mV[VZ];

	if (target_offset_scaled.mV[VX] > target_offset_scaled.mV[VY] &&
		target_offset_scaled.mV[VX] > target_offset_scaled.mV[VZ])
	{
		object_split_axis = LLVector3::x_axis;
	}
	else if (target_offset_scaled.mV[VY] > target_offset_scaled.mV[VZ])
	{
		object_split_axis = LLVector3::y_axis;
	}
	else
	{
		object_split_axis = LLVector3::z_axis;
	}

	LLVector3 camera_offset_object(getCameraPositionAgent() - mFocusObject->getPositionAgent());

	// length projected orthogonal to target offset
	F32 camera_offset_dist = (camera_offset_object - target_offset_dir * (camera_offset_object * target_offset_dir)).magVec();

	// calculate whether the target point would be "visible" if it were outside the bounding box
	// on the opposite of the splitting plane defined by object_split_axis;
	BOOL exterior_target_visible = FALSE;
	if (camera_offset_dist > object_radius)
	{
		// target is visible from camera, so turn off fov zoom
		exterior_target_visible = TRUE;
	}

	F32 camera_offset_clip = camera_offset_object * object_split_axis;
	F32 target_offset_clip = target_offset_dir * object_split_axis;

	// target has moved outside of object extents
	// check to see if camera and target are on same side
	if (target_outside_object_extents)
	{
		if (camera_offset_clip > 0.f && target_offset_clip > 0.f)
		{
			return FALSE;
		}
		else if (camera_offset_clip < 0.f && target_offset_clip < 0.f)
		{
			return FALSE;
		}
	}

	// clamp obj distance to diagonal of 10 by 10 cube
	obj_min_distance = llmin(obj_min_distance, 10.f * F_SQRT3);

	obj_min_distance += LLViewerCamera::getInstance()->getNear() + (soft_limit ? 0.1f : 0.2f);

	return TRUE;
}

F32 LLAgent::getCameraZoomFraction()
{
	// 0.f -> camera zoomed all the way out
	// 1.f -> camera zoomed all the way in
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// already [0,1]
		return mHUDTargetZoom;
	}
	else if (mFocusOnAvatar && cameraThirdPerson())
	{
		return clamp_rescale(mCameraZoomFraction, MIN_ZOOM_FRACTION, MAX_ZOOM_FRACTION, 1.f, 0.f);
	}
	else if (cameraCustomizeAvatar())
	{
		F32 distance = (F32)mCameraFocusOffsetTarget.magVec();
		return clamp_rescale(distance, APPEARANCE_MIN_ZOOM, APPEARANCE_MAX_ZOOM, 1.f, 0.f);
	}
	else
	{
		F32 min_zoom;
		const F32 DIST_FUDGE = 16.f; // meters
		F32 max_zoom = llmin(mDrawDistance - DIST_FUDGE,
								LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE,
								MAX_CAMERA_DISTANCE_FROM_AGENT);

		F32 distance = (F32)mCameraFocusOffsetTarget.magVec();
		if (mFocusObject.notNull())
		{
			if (mFocusObject->isAvatar())
			{
				min_zoom = AVATAR_MIN_ZOOM;
			}
			else
			{
				min_zoom = OBJECT_MIN_ZOOM;
			}
		}
		else
		{
			min_zoom = LAND_MIN_ZOOM;
		}

		return clamp_rescale(distance, min_zoom, max_zoom, 1.f, 0.f);
	}
}

void LLAgent::setCameraZoomFraction(F32 fraction)
{
	// 0.f -> camera zoomed all the way out
	// 1.f -> camera zoomed all the way in
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();

	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		mHUDTargetZoom = fraction;
	}
	else if (mFocusOnAvatar && cameraThirdPerson())
	{
		mCameraZoomFraction = rescale(fraction, 0.f, 1.f, MAX_ZOOM_FRACTION, MIN_ZOOM_FRACTION);
	}
	else if (cameraCustomizeAvatar())
	{
		LLVector3d camera_offset_dir = mCameraFocusOffsetTarget;
		camera_offset_dir.normalize();
		mCameraFocusOffsetTarget = camera_offset_dir * rescale(fraction, 0.f, 1.f, APPEARANCE_MAX_ZOOM, APPEARANCE_MIN_ZOOM);
	}
	else
	{
		F32 min_zoom = LAND_MIN_ZOOM;
		const F32 DIST_FUDGE = 16.f; // meters
		F32 max_zoom = llmin(mDrawDistance - DIST_FUDGE,
								LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE,
								MAX_CAMERA_DISTANCE_FROM_AGENT);

		if (mFocusObject.notNull())
		{
			if (mFocusObject.notNull())
			{
				if (mFocusObject->isAvatar())
				{
					min_zoom = AVATAR_MIN_ZOOM;
				}
				else
				{
					min_zoom = OBJECT_MIN_ZOOM;
				}
			}
		}

		LLVector3d camera_offset_dir = mCameraFocusOffsetTarget;
		camera_offset_dir.normalize();
		mCameraFocusOffsetTarget = camera_offset_dir * rescale(fraction, 0.f, 1.f, max_zoom, min_zoom);
	}
	startCameraAnimation();
}

//-----------------------------------------------------------------------------
// cameraOrbitAround()
//-----------------------------------------------------------------------------
void LLAgent::cameraOrbitAround(const F32 radians)
{
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// do nothing for hud selection
	}
	else if (mFocusOnAvatar && (mCameraMode == CAMERA_MODE_THIRD_PERSON || mCameraMode == CAMERA_MODE_FOLLOW))
	{
		mFrameAgent.rotate(radians, getReferenceUpVector());
	}
	else
	{
		mCameraFocusOffsetTarget.rotVec(radians, 0.f, 0.f, 1.f);

		cameraZoomIn(1.f);
	}
}

//-----------------------------------------------------------------------------
// cameraOrbitOver()
//-----------------------------------------------------------------------------
void LLAgent::cameraOrbitOver(const F32 angle)
{
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// do nothing for hud selection
	}
	else if (mFocusOnAvatar && mCameraMode == CAMERA_MODE_THIRD_PERSON)
	{
		pitch(angle);
	}
	else
	{
		LLVector3 camera_offset_unit(mCameraFocusOffsetTarget);
		camera_offset_unit.normalize();

		F32 angle_from_up = acos(camera_offset_unit * getReferenceUpVector());

		LLVector3d left_axis;
		left_axis.setVec(LLViewerCamera::getInstance()->getLeftAxis());
		F32 new_angle = llclamp(angle_from_up - angle, 1.f * DEG_TO_RAD, 179.f * DEG_TO_RAD);
		mCameraFocusOffsetTarget.rotVec(angle_from_up - new_angle, left_axis);

		cameraZoomIn(1.f);
	}
}

//-----------------------------------------------------------------------------
// cameraZoomIn()
//-----------------------------------------------------------------------------
void LLAgent::cameraZoomIn(const F32 fraction)
{
	if (gDisconnected)
	{
		return;
	}

	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// just update hud zoom level
		mHUDTargetZoom /= fraction;
		return;
	}

	LLVector3d	camera_offset(mCameraFocusOffsetTarget);
	LLVector3d	camera_offset_unit(mCameraFocusOffsetTarget);
	F32 min_zoom = LAND_MIN_ZOOM;
	F32 current_distance = (F32)camera_offset_unit.normalize();
	F32 new_distance = current_distance * fraction;
	static LLCachedControl<bool> disable_camera_constraints(gSavedSettings,
															"DisableCameraConstraints");
	// Don't move through focus point
	if (mFocusObject)
	{
		min_zoom = OBJECT_MIN_ZOOM;
		if (!disable_camera_constraints && mFocusObject->isAvatar())
		{
			calcCameraMinDistance(min_zoom);
		}
	}

	new_distance = llmax(new_distance, min_zoom);

	// Don't zoom too far back
	const F32 DIST_FUDGE = 16.f; // meters
	F32 max_distance = mDrawDistance - DIST_FUDGE;
	if (!disable_camera_constraints)
	{
		max_distance = llmin(max_distance,
							 LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE);
	}
	if (new_distance > max_distance)
	{
		new_distance = max_distance;
	}

	if (cameraCustomizeAvatar())
	{
		new_distance = llclamp(new_distance, APPEARANCE_MIN_ZOOM, APPEARANCE_MAX_ZOOM);
	}

	mCameraFocusOffsetTarget = new_distance * camera_offset_unit;
}

//-----------------------------------------------------------------------------
// cameraOrbitIn()
//-----------------------------------------------------------------------------
void LLAgent::cameraOrbitIn(const F32 meters)
{
	if (mFocusOnAvatar && mCameraMode == CAMERA_MODE_THIRD_PERSON)
	{
		static LLCachedControl<F32> camera_offset_scale(gSavedSettings,
														"CameraOffsetScale");
		F32 camera_offset_dist = llmax(0.001f,
									   mCameraOffsetDefault.magVec() * (F32)camera_offset_scale);

		mCameraZoomFraction = (mTargetCameraDistance - meters) / camera_offset_dist;

		if (!LLPipeline::sFreezeTime && mCameraZoomFraction < MIN_ZOOM_FRACTION && meters > 0.f)
		{
			// No need to animate, camera is already there.
			changeCameraToMouselook(FALSE);
		}

		mCameraZoomFraction = llclamp(mCameraZoomFraction, MIN_ZOOM_FRACTION, MAX_ZOOM_FRACTION);
	}
	else
	{
		LLVector3d	camera_offset(mCameraFocusOffsetTarget);
		LLVector3d	camera_offset_unit(mCameraFocusOffsetTarget);
		F32 current_distance = (F32)camera_offset_unit.normalize();
		F32 new_distance = current_distance - meters;
		F32 min_zoom = LAND_MIN_ZOOM;

		// Don't move through focus point
		if (mFocusObject.notNull())
		{
			if (mFocusObject->isAvatar())
			{
				min_zoom = AVATAR_MIN_ZOOM;
			}
			else
			{
				min_zoom = OBJECT_MIN_ZOOM;
			}
		}

		new_distance = llmax(new_distance, min_zoom);

		// Don't zoom too far back
		const F32 DIST_FUDGE = 16.f; // meters
		F32 max_distance = llmin(mDrawDistance - DIST_FUDGE,
								 LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE);

		if (new_distance > max_distance)
		{
			// Unless camera is unlocked
			static LLCachedControl<bool> disable_camera_constraints(gSavedSettings,
																	"DisableCameraConstraints");
			if (!disable_camera_constraints)
			{
				return;
			}
		}

		if (CAMERA_MODE_CUSTOMIZE_AVATAR == getCameraMode())
		{
			new_distance = llclamp(new_distance, APPEARANCE_MIN_ZOOM, APPEARANCE_MAX_ZOOM);
		}

		// Compute new camera offset
		mCameraFocusOffsetTarget = new_distance * camera_offset_unit;
		cameraZoomIn(1.f);
	}
}

//-----------------------------------------------------------------------------
// cameraPanIn()
//-----------------------------------------------------------------------------
void LLAgent::cameraPanIn(F32 meters)
{
	LLVector3d at_axis;
	at_axis.setVec(LLViewerCamera::getInstance()->getAtAxis());

	mFocusTargetGlobal += meters * at_axis;
	mFocusGlobal = mFocusTargetGlobal;
	// don't enforce zoom constraints as this is the only way for users to get past them easily
	updateFocusOffset();
	// NOTE: panning movements expect the camera to move exactly with the focus target, not animated behind -Nyx
	mCameraSmoothingLastPositionGlobal = calcCameraPositionTargetGlobal();
}

//-----------------------------------------------------------------------------
// cameraPanLeft()
//-----------------------------------------------------------------------------
void LLAgent::cameraPanLeft(F32 meters)
{
	LLVector3d left_axis;
	left_axis.setVec(LLViewerCamera::getInstance()->getLeftAxis());

	mFocusTargetGlobal += meters * left_axis;
	mFocusGlobal = mFocusTargetGlobal;

	// disable smoothing for camera pan, which causes some residents unhappiness
	mCameraSmoothingStop = TRUE;

	cameraZoomIn(1.f);
	updateFocusOffset();
	// NOTE: panning movements expect the camera to move exactly with the focus target, not animated behind - Nyx
	mCameraSmoothingLastPositionGlobal = calcCameraPositionTargetGlobal();
}

//-----------------------------------------------------------------------------
// cameraPanUp()
//-----------------------------------------------------------------------------
void LLAgent::cameraPanUp(F32 meters)
{
	LLVector3d up_axis;
	up_axis.setVec(LLViewerCamera::getInstance()->getUpAxis());

	mFocusTargetGlobal += meters * up_axis;
	mFocusGlobal = mFocusTargetGlobal;

	// disable smoothing for camera pan, which causes some residents unhappiness
	mCameraSmoothingStop = TRUE;

	cameraZoomIn(1.f);
	updateFocusOffset();
	// NOTE: panning movements expect the camera to move exactly with the focus target, not animated behind -Nyx
	mCameraSmoothingLastPositionGlobal = calcCameraPositionTargetGlobal();
}

//-----------------------------------------------------------------------------
// setKey()
//-----------------------------------------------------------------------------
void LLAgent::setKey(const S32 direction, S32 &key)
{
	if (direction > 0)
	{
		key = 1;
	}
	else if (direction < 0)
	{
		key = -1;
	}
	else
	{
		key = 0;
	}
}

//-----------------------------------------------------------------------------
// getControlFlags()
//-----------------------------------------------------------------------------
U32 LLAgent::getControlFlags()
{
/*
	// HACK -- avoids maintenance of control flags when camera mode is turned on or off,
	// only worries about it when the flags are measured
	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		if (!(mControlFlags & AGENT_CONTROL_MOUSELOOK))
		{
			mControlFlags |= AGENT_CONTROL_MOUSELOOK;
		}
	}
*/
	return mControlFlags;
}

//-----------------------------------------------------------------------------
// setControlFlags()
//-----------------------------------------------------------------------------
void LLAgent::setControlFlags(U32 mask)
{
	U32 old_flags = mControlFlags;
	mControlFlags |= mask;
	mbFlagsDirty = mControlFlags ^ old_flags;
}

//-----------------------------------------------------------------------------
// clearControlFlags()
//-----------------------------------------------------------------------------
void LLAgent::clearControlFlags(U32 mask)
{
	U32 old_flags = mControlFlags;
	mControlFlags &= ~mask;
	if (old_flags != mControlFlags)
	{
		mbFlagsDirty = TRUE;
	}
}

//-----------------------------------------------------------------------------
// controlFlagsDirty()
//-----------------------------------------------------------------------------
BOOL LLAgent::controlFlagsDirty() const
{
	return mbFlagsDirty;
}

//-----------------------------------------------------------------------------
// enableControlFlagReset()
//-----------------------------------------------------------------------------
void LLAgent::enableControlFlagReset()
{
	mbFlagsNeedReset = TRUE;
}

//-----------------------------------------------------------------------------
// resetControlFlags()
//-----------------------------------------------------------------------------
void LLAgent::resetControlFlags()
{
	if (mbFlagsNeedReset)
	{
		mbFlagsNeedReset = FALSE;
		mbFlagsDirty = FALSE;
		// reset all of the ephemeral flags
		// some flags are managed elsewhere
		mControlFlags &= AGENT_CONTROL_AWAY | AGENT_CONTROL_FLY | AGENT_CONTROL_MOUSELOOK;
	}
}

//-----------------------------------------------------------------------------
// setAFK()
//-----------------------------------------------------------------------------
void LLAgent::setAFK()
{
	// Drones can't go AFK
	if (gNoRender)
	{
		return;
	}

	if (!gAgent.getRegion())
	{
		// Don't set AFK if we're not talking to a region yet.
		return;
	}

	if (!(mControlFlags & AGENT_CONTROL_AWAY))
	{
		sendAnimationRequest(ANIM_AGENT_AWAY, ANIM_REQUEST_START);
		setControlFlags(AGENT_CONTROL_AWAY | AGENT_CONTROL_STOP);
		gAwayTimer.start();
		if (gAFKMenu)
		{
			//*TODO:Translate
			gAFKMenu->setLabel(std::string("Set Not Away"));
		}
	}
}

//-----------------------------------------------------------------------------
// clearAFK()
//-----------------------------------------------------------------------------
void LLAgent::clearAFK()
{
	gAwayTriggerTimer.reset();

	// Gods can sometimes get into away state (via gestures) without setting
	// the appropriate control flag. JC
	if (mControlFlags & AGENT_CONTROL_AWAY ||
		(isAgentAvatarValid() &&
		 gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_AWAY) !=
			gAgentAvatarp->mSignaledAnimations.end()))
	{
		sendAnimationRequest(ANIM_AGENT_AWAY, ANIM_REQUEST_STOP);
		clearControlFlags(AGENT_CONTROL_AWAY);
		if (gAFKMenu)
		{
			//*TODO:Translate
			gAFKMenu->setLabel(std::string("Set Away"));
		}
	}
}

//-----------------------------------------------------------------------------
// getAFK()
//-----------------------------------------------------------------------------
BOOL LLAgent::getAFK() const
{
	return (mControlFlags & AGENT_CONTROL_AWAY) != 0;
}

//-----------------------------------------------------------------------------
// setBusy()
//-----------------------------------------------------------------------------
void LLAgent::setBusy()
{
	sendAnimationRequest(ANIM_AGENT_BUSY, ANIM_REQUEST_START);
	mIsBusy = TRUE;
	if (gBusyMenu)
	{
		//*TODO:Translate
		gBusyMenu->setLabel(std::string("Set Not Busy"));
	}
	LLFloaterMute::getInstance()->updateButtons();
}

//-----------------------------------------------------------------------------
// clearBusy()
//-----------------------------------------------------------------------------
void LLAgent::clearBusy()
{
	mIsBusy = FALSE;
	sendAnimationRequest(ANIM_AGENT_BUSY, ANIM_REQUEST_STOP);
	if (gBusyMenu)
	{
		//*TODO:Translate
		gBusyMenu->setLabel(std::string("Set Busy"));
	}
	LLFloaterMute::getInstance()->updateButtons();
}

//-----------------------------------------------------------------------------
// getBusy()
//-----------------------------------------------------------------------------
BOOL LLAgent::getBusy() const
{
	return mIsBusy;
}

//-----------------------------------------------------------------------------
// startAutoPilotGlobal()
//-----------------------------------------------------------------------------
void LLAgent::startAutoPilotGlobal(const LLVector3d &target_global,
								   const std::string& behavior_name,
								   const LLQuaternion *target_rotation,
								   void (*finish_callback)(BOOL, void *),
								   void *callback_data,
								   F32 stop_distance,
								   F32 rot_threshold)
{
	if (!isAgentAvatarValid())
	{
		return;
	}

	mAutoPilotFinishedCallback = finish_callback;
	mAutoPilotCallbackData = callback_data;
	mAutoPilotRotationThreshold = rot_threshold;
	mAutoPilotBehaviorName = behavior_name;

	LLVector3d delta_pos(target_global);
	delta_pos -= getPositionGlobal();
	F64 distance = delta_pos.magVec();
	LLVector3d trace_target = target_global;

	trace_target.mdV[VZ] -= 10.f;

	LLVector3d intersection;
	LLVector3 normal;
	LLViewerObject *hit_obj;
	F32 heightDelta = LLWorld::getInstance()->resolveStepHeightGlobal(NULL,
																	  target_global,
																	  trace_target,
																	  intersection,
																	  normal,
																	  &hit_obj);

	if (stop_distance > 0.f)
	{
		mAutoPilotStopDistance = stop_distance;
	}
	else
	{
		// Guess at a reasonable stop distance.
		mAutoPilotStopDistance = (F32)sqrt(distance);
		if (mAutoPilotStopDistance < 0.5f)
		{
			mAutoPilotStopDistance = 0.5f;
		}
	}

	mAutoPilotFlyOnStop = getFlying();

	if (distance > 30.0)
	{
		setFlying(TRUE);
	}

	if (distance > 1.f && heightDelta > (sqrtf(mAutoPilotStopDistance) + 1.f))
	{
		setFlying(TRUE);
		mAutoPilotFlyOnStop = TRUE;
	}

	mAutoPilot = TRUE;
	mAutoPilotTargetGlobal = target_global;

	// trace ray down to find height of destination from ground
	LLVector3d traceEndPt = target_global;
	traceEndPt.mdV[VZ] -= 20.f;

	LLVector3d targetOnGround;
	LLVector3 groundNorm;
	LLViewerObject *obj;

	LLWorld::getInstance()->resolveStepHeightGlobal(NULL, target_global,
													traceEndPt, targetOnGround,
													groundNorm, &obj);
	F64 target_height = llmax((F64)gAgentAvatarp->getPelvisToFoot(),
							  target_global.mdV[VZ] - targetOnGround.mdV[VZ]);

	// clamp z value of target to minimum height above ground
	mAutoPilotTargetGlobal.mdV[VZ] = targetOnGround.mdV[VZ] + target_height;
	mAutoPilotTargetDist = (F32)dist_vec(gAgent.getPositionGlobal(),
										 mAutoPilotTargetGlobal);
	if (target_rotation)
	{
		mAutoPilotUseRotation = TRUE;
		mAutoPilotTargetFacing = LLVector3::x_axis * *target_rotation;
		mAutoPilotTargetFacing.mV[VZ] = 0.f;
		mAutoPilotTargetFacing.normalize();
	}
	else
	{
		mAutoPilotUseRotation = FALSE;
	}

	mAutoPilotNoProgressFrameCount = 0;
}

//-----------------------------------------------------------------------------
// startFollowPilot()
//-----------------------------------------------------------------------------
void LLAgent::startFollowPilot(const LLUUID& leader_id)
{
	if (!mAutoPilot) return;

	mLeaderID = leader_id;
	if (mLeaderID.isNull()) return;

	LLViewerObject* object = gObjectList.findObject(mLeaderID);
	if (!object)
	{
		mLeaderID = LLUUID::null;
		return;
	}

	startAutoPilotGlobal(object->getPositionGlobal());
}

//-----------------------------------------------------------------------------
// stopAutoPilot()
//-----------------------------------------------------------------------------
void LLAgent::stopAutoPilot(BOOL user_cancel)
{
	if (mAutoPilot)
	{
		mAutoPilot = FALSE;
		if (mAutoPilotUseRotation && !user_cancel)
		{
			resetAxes(mAutoPilotTargetFacing);
		}
		//NB: auto pilot can terminate for a reason other than reaching the destination
		if (mAutoPilotFinishedCallback)
		{
			mAutoPilotFinishedCallback(!user_cancel &&
									   dist_vec(gAgent.getPositionGlobal(),
												mAutoPilotTargetGlobal) < mAutoPilotStopDistance,
									   mAutoPilotCallbackData);
		}
		mLeaderID = LLUUID::null;

		// If the user cancelled, don't change the fly state
		if (!user_cancel)
		{
			setFlying(mAutoPilotFlyOnStop);
		}
		setControlFlags(AGENT_CONTROL_STOP);

		if (user_cancel && !mAutoPilotBehaviorName.empty())
		{
			if (mAutoPilotBehaviorName == "Sit")
			{
				LLNotifications::instance().add("CancelledSit");
			}
			else if (mAutoPilotBehaviorName == "Attach")
			{
				LLNotifications::instance().add("CancelledAttach");
			}
			else
			{
				LLNotifications::instance().add("Cancelled");
			}
		}
	}
}

// Returns necessary agent pitch and yaw changes, radians.
//-----------------------------------------------------------------------------
// autoPilot()
//-----------------------------------------------------------------------------
void LLAgent::autoPilot(F32 *delta_yaw)
{
	if (mAutoPilot)
	{
		if (!mLeaderID.isNull())
		{
			LLViewerObject* object = gObjectList.findObject(mLeaderID);
			if (!object)
			{
				stopAutoPilot();
				return;
			}
			mAutoPilotTargetGlobal = object->getPositionGlobal();
		}

		if (!isAgentAvatarValid())
		{
			return;
		}

		if (gAgentAvatarp->mInAir)
		{
			setFlying(TRUE);
		}

		LLVector3 at;
		at.setVec(mFrameAgent.getAtAxis());
		LLVector3 target_agent = getPosAgentFromGlobal(mAutoPilotTargetGlobal);
		LLVector3 direction = target_agent - getPositionAgent();

		F32 target_dist = direction.magVec();

		if (target_dist >= mAutoPilotTargetDist)
		{
			mAutoPilotNoProgressFrameCount++;
			if (mAutoPilotNoProgressFrameCount > AUTOPILOT_MAX_TIME_NO_PROGRESS * gFPSClamped)
			{
				stopAutoPilot();
				return;
			}
		}

		mAutoPilotTargetDist = target_dist;

		// Make this a two-dimensional solution
		at.mV[VZ] = 0.f;
		direction.mV[VZ] = 0.f;

		at.normalize();
		F32 xy_distance = direction.normalize();

		F32 yaw = 0.f;
		if (mAutoPilotTargetDist > mAutoPilotStopDistance)
		{
			yaw = angle_between(mFrameAgent.getAtAxis(), direction);
		}
		else if (mAutoPilotUseRotation)
		{
			// we're close now just aim at target facing
			yaw = angle_between(at, mAutoPilotTargetFacing);
			direction = mAutoPilotTargetFacing;
		}

		yaw = 4.f * yaw / gFPSClamped;

		// figure out which direction to turn
		LLVector3 scratch(at % direction);

		if (scratch.mV[VZ] > 0.f)
		{
			setControlFlags(AGENT_CONTROL_YAW_POS);
		}
		else
		{
			yaw = -yaw;
			setControlFlags(AGENT_CONTROL_YAW_NEG);
		}

		*delta_yaw = yaw;

		// Compute when to start slowing down and when to stop
		F32 stop_distance = mAutoPilotStopDistance;
		F32 slow_distance;
		if (getFlying())
		{
			slow_distance = llmax(6.f, mAutoPilotStopDistance + 5.f);
			stop_distance = llmax(2.f, mAutoPilotStopDistance);
		}
		else
		{
			slow_distance = llmax(3.f, mAutoPilotStopDistance + 2.f);
		}

		// If we're flying, handle autopilot points above or below you.
		if (getFlying() && xy_distance < AUTOPILOT_HEIGHT_ADJUST_DISTANCE)
		{
			if (isAgentAvatarValid())
			{
				F64 current_height = gAgentAvatarp->getPositionGlobal().mdV[VZ];
				F32 delta_z = (F32)(mAutoPilotTargetGlobal.mdV[VZ] - current_height);
				F32 slope = delta_z / xy_distance;
				if (slope > 0.45f && delta_z > 6.f)
				{
					setControlFlags(AGENT_CONTROL_FAST_UP | AGENT_CONTROL_UP_POS);
				}
				else if (slope > 0.002f && delta_z > 0.5f)
				{
					setControlFlags(AGENT_CONTROL_UP_POS);
				}
				else if (slope < -0.45f && delta_z < -6.f &&
						 current_height > AUTOPILOT_MIN_TARGET_HEIGHT_OFF_GROUND)
				{
					setControlFlags(AGENT_CONTROL_FAST_UP | AGENT_CONTROL_UP_NEG);
				}
				else if (slope < -0.002f && delta_z < -0.5f &&
						 current_height > AUTOPILOT_MIN_TARGET_HEIGHT_OFF_GROUND)
				{
					setControlFlags(AGENT_CONTROL_UP_NEG);
				}
			}
		}

		//  calculate delta rotation to target heading
		F32 delta_target_heading = angle_between(mFrameAgent.getAtAxis(),
												 mAutoPilotTargetFacing);

		if (xy_distance > slow_distance && yaw < (F_PI / 10.f))
		{
			// walking/flying fast
			setControlFlags(AGENT_CONTROL_FAST_AT | AGENT_CONTROL_AT_POS);
		}
		else if (mAutoPilotTargetDist > mAutoPilotStopDistance)
		{
			// walking/flying slow
			if (at * direction > 0.9f)
			{
				setControlFlags(AGENT_CONTROL_AT_POS);
			}
			else if (at * direction < -0.9f)
			{
				setControlFlags(AGENT_CONTROL_AT_NEG);
			}
		}

		// check to see if we need to keep rotating to target orientation
		if (mAutoPilotTargetDist < mAutoPilotStopDistance)
		{
			setControlFlags(AGENT_CONTROL_STOP);
			if (!mAutoPilotUseRotation ||
				delta_target_heading < mAutoPilotRotationThreshold)
			{
				stopAutoPilot();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// propagate()
//-----------------------------------------------------------------------------
void LLAgent::propagate(const F32 dt)
{
	// Update UI based on agent motion
	LLFloaterMove *floater_move = LLFloaterMove::getInstance();
	if (floater_move)
	{
		floater_move->mForwardButton   ->setToggleState(mAtKey > 0 || mWalkKey > 0);
		floater_move->mBackwardButton  ->setToggleState(mAtKey < 0 || mWalkKey < 0);
		floater_move->mSlideLeftButton ->setToggleState(mLeftKey > 0);
		floater_move->mSlideRightButton->setToggleState(mLeftKey < 0);
		floater_move->mTurnLeftButton  ->setToggleState(mYawKey > 0.f);
		floater_move->mTurnRightButton ->setToggleState(mYawKey < 0.f);
		floater_move->mMoveUpButton    ->setToggleState(mUpKey > 0);
		floater_move->mMoveDownButton  ->setToggleState(mUpKey < 0);
	}

	// handle rotation based on keyboard levels
	const F32 YAW_RATE = 90.f * DEG_TO_RAD;				// radians per second
	yaw(YAW_RATE * mYawKey * dt);

	const F32 PITCH_RATE = 90.f * DEG_TO_RAD;			// radians per second
	pitch(PITCH_RATE * (F32) mPitchKey * dt);

	// handle auto-land behavior
	static LLCachedControl<bool> automatic_fly(gSavedSettings, "AutomaticFly");
	if (automatic_fly && isAgentAvatarValid() && !gAgentAvatarp->mInAir && mUpKey < 0)
	{
		LLVector3 land_vel = getVelocity();
		land_vel.mV[VZ] = 0.f;
		if (land_vel.magVecSquared() < MAX_VELOCITY_AUTO_LAND_SQUARED)
		{
			// land automatically
			setFlying(FALSE);
		}
	}

	// clear keys
	mAtKey = 0;
	mWalkKey = 0;
	mLeftKey = 0;
	mUpKey = 0;
	mYawKey = 0.f;
	mPitchKey = 0;
}

//-----------------------------------------------------------------------------
// updateAgentPosition()
//-----------------------------------------------------------------------------
void LLAgent::updateAgentPosition(const F32 dt, const F32 yaw_radians, const S32 mouse_x, const S32 mouse_y)
{
	propagate(dt);

	// static S32 cameraUpdateCount = 0;

	rotate(yaw_radians, 0, 0, 1);

	//
	// Check for water and land collision, set underwater flag
	//

	updateLookAt(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------
// updateLookAt()
//-----------------------------------------------------------------------------
void LLAgent::updateLookAt(const S32 mouse_x, const S32 mouse_y)
{
	static LLVector3 last_at_axis;

	if (!isAgentAvatarValid())	// also true when gViewerWindow is NULL (destroyed)
	{
		return;
	}

	LLQuaternion av_inv_rot = ~gAgentAvatarp->mRoot.getWorldRotation();
	LLVector3 root_at = LLVector3::x_axis * gAgentAvatarp->mRoot.getWorldRotation();

	if (gViewerWindow->getMouseVelocityStat()->getCurrent() < 0.01f &&
		root_at * last_at_axis > 0.95f)
	{
		LLVector3 vel = gAgentAvatarp->getVelocity();
		if (vel.magVecSquared() > 4.f)
		{
			setLookAt(LOOKAT_TARGET_IDLE, gAgentAvatarp, vel * av_inv_rot);
		}
		else
		{
			// *FIX: rotate mframeagent by sit object's rotation?
			LLQuaternion look_rotation = gAgentAvatarp->mIsSitting ? gAgentAvatarp->getRenderRotation()
																   : mFrameAgent.getQuaternion(); // use camera's current rotation
			LLVector3 look_offset = LLVector3(2.f, 0.f, 0.f) * look_rotation * av_inv_rot;
			setLookAt(LOOKAT_TARGET_IDLE, gAgentAvatarp, look_offset);
		}
		last_at_axis = root_at;
		return;
	}

	last_at_axis = root_at;

	if (CAMERA_MODE_CUSTOMIZE_AVATAR == getCameraMode())
	{
		setLookAt(LOOKAT_TARGET_NONE, gAgentAvatarp, LLVector3(-2.f, 0.f, 0.f));
	}
	else
	{
		// Move head based on cursor position
		ELookAtType lookAtType = LOOKAT_TARGET_NONE;
		LLVector3 headLookAxis;
		LLCoordFrame frameCamera = *((LLCoordFrame*)LLViewerCamera::getInstance());

		if (cameraMouselook())
		{
			lookAtType = LOOKAT_TARGET_MOUSELOOK;
		}
		else if (cameraThirdPerson())
		{
			// range from -.5 to .5
			F32 x_from_center =
				((F32) mouse_x / (F32) gViewerWindow->getWindowWidth()) - 0.5f;
			F32 y_from_center =
				((F32) mouse_y / (F32) gViewerWindow->getWindowHeight()) - 0.5f;

			static LLCachedControl<F32> yaw_from_mouse_position(gSavedSettings,
																"YawFromMousePosition");
			static LLCachedControl<F32> pitch_from_mouse_position(gSavedSettings,
																  "PitchFromMousePosition");
			frameCamera.yaw(-x_from_center * yaw_from_mouse_position * DEG_TO_RAD);
			frameCamera.pitch(-y_from_center * pitch_from_mouse_position * DEG_TO_RAD);
			lookAtType = LOOKAT_TARGET_FREELOOK;
		}

		headLookAxis = frameCamera.getAtAxis();
		// RN: we use world-space offset for mouselook and freelook
		//headLookAxis = headLookAxis * av_inv_rot;
		setLookAt(lookAtType, gAgentAvatarp, headLookAxis);
	}
}

// friends and operators

std::ostream& operator<<(std::ostream &s, const LLAgent &agent)
{
	// This is unfinished, but might never be used.
	// We'll just leave it for now; we can always delete it.
	s << " { "
	  << "  Frame = " << agent.mFrameAgent << "\n"
	  << " }";
	return s;
}

// ------------------- Beginning of legacy LLCamera hack ----------------------
// This section is included for legacy LLCamera support until
// it is no longer needed.  Some legacy code must exist in
// non-legacy functions, and is labeled with "// legacy" comments.

//-----------------------------------------------------------------------------
// setAvatarObject()
//-----------------------------------------------------------------------------
void LLAgent::setAvatarObject(LLVOAvatarSelf *avatar)
{
	if (!avatar)
	{
		llinfos << "NULL agent pointer passed: ignoring." << llendl;
		return;
	}

	if (!mLookAt)
	{
		mLookAt = (LLHUDEffectLookAt *)LLHUDManager::getInstance()->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_LOOKAT);
	}
	if (!mPointAt)
	{
		mPointAt = (LLHUDEffectPointAt *)LLHUDManager::getInstance()->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_POINTAT);
	}

	if (!mLookAt.isNull())
	{
		mLookAt->setSourceObject(avatar);
	}
	if (!mPointAt.isNull())
	{
		mPointAt->setSourceObject(avatar);
	}
}

// TRUE if your own avatar needs to be rendered.  Usually only
// in third person and build.
//-----------------------------------------------------------------------------
// needsRenderAvatar()
//-----------------------------------------------------------------------------
BOOL LLAgent::needsRenderAvatar()
{
	if (cameraMouselook() && !LLVOAvatar::sVisibleInFirstPerson)
	{
		return FALSE;
	}

	return mShowAvatar && mGenderChosen;
}

// TRUE if we need to render your own avatar's head.
BOOL LLAgent::needsRenderHead()
{
	return (LLVOAvatar::sVisibleInFirstPerson && LLPipeline::sReflectionRender) || (mShowAvatar && !cameraMouselook());
}

//-----------------------------------------------------------------------------
// startTyping()
//-----------------------------------------------------------------------------
void LLAgent::startTyping()
{
	mTypingTimer.reset();

	if (getRenderState() & AGENT_STATE_TYPING)
	{
		// already typing, don't trigger a different animation
		return;
	}
	setRenderState(AGENT_STATE_TYPING);

	if (mChatTimer.getElapsedTimeF32() < 2.f)
	{
		LLVOAvatar* chatter = gObjectList.findAvatar(mLastChatterID);
		if (chatter)
		{
			gAgent.setLookAt(LOOKAT_TARGET_RESPOND, chatter, LLVector3::zero);
		}
	}

	if (gSavedSettings.getBOOL("PlayTypingAnim"))
	{
		sendAnimationRequest(ANIM_AGENT_TYPE, ANIM_REQUEST_START);
	}
	if (gChatBar)
	{
		gChatBar->sendChatFromViewer("", CHAT_TYPE_START, FALSE);
	}
}

//-----------------------------------------------------------------------------
// stopTyping()
//-----------------------------------------------------------------------------
void LLAgent::stopTyping()
{
	if (mRenderState & AGENT_STATE_TYPING)
	{
		clearRenderState(AGENT_STATE_TYPING);
		sendAnimationRequest(ANIM_AGENT_TYPE, ANIM_REQUEST_STOP);
		if (gChatBar)
		{
			gChatBar->sendChatFromViewer("", CHAT_TYPE_STOP, FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
// setRenderState()
//-----------------------------------------------------------------------------
void LLAgent::setRenderState(U8 newstate)
{
	mRenderState |= newstate;
}

//-----------------------------------------------------------------------------
// clearRenderState()
//-----------------------------------------------------------------------------
void LLAgent::clearRenderState(U8 clearstate)
{
	mRenderState &= ~clearstate;
}

//-----------------------------------------------------------------------------
// getRenderState()
//-----------------------------------------------------------------------------
U8 LLAgent::getRenderState()
{
	if (gNoRender || gKeyboard == NULL)
	{
		return 0;
	}

	// *FIX: don't do stuff in a getter!  This is infinite loop city!
	if ((mTypingTimer.getElapsedTimeF32() > TYPING_TIMEOUT_SECS)
		&& (mRenderState & AGENT_STATE_TYPING))
	{
		stopTyping();
	}

	if ((!LLSelectMgr::getInstance()->getSelection()->isEmpty() &&
		 LLSelectMgr::getInstance()->shouldShowSelection()) ||
		LLToolMgr::getInstance()->getCurrentTool()->isEditing())
	{
		setRenderState(AGENT_STATE_EDITING);
	}
	else
	{
		clearRenderState(AGENT_STATE_EDITING);
	}

	return mRenderState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static const LLFloaterView::skip_list_t& get_skip_list()
{
	static LLFloaterView::skip_list_t skip_list;
	skip_list.insert(LLFloaterMap::getInstance());
	return skip_list;
}

//-----------------------------------------------------------------------------
// endAnimationUpdateUI()
//-----------------------------------------------------------------------------
void LLAgent::endAnimationUpdateUI()
{
	if (mCameraMode == mLastCameraMode)
	{
		// We're already done endAnimationUpdateUI for this transition.
		return;
	}

	// clean up UI from mode we're leaving
	if (mLastCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		// show mouse cursor
		if (gViewerWindow)
		{
			gViewerWindow->showCursor();
		}
		// show menus
		if (gMenuBarView)
		{
			gMenuBarView->setVisible(TRUE);
		}
		if (gStatusBar)
		{
			gStatusBar->setVisibleForMouselook(true);
		}

		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);

		// Only pop if we have pushed...
		if (mViewsPushed)
		{
			mViewsPushed = FALSE;
			if (gFloaterView)
			{
				gFloaterView->popVisibleAll(get_skip_list());
			}
		}

		gAgent.setLookAt(LOOKAT_TARGET_CLEAR);
		if (gMorphView)
		{
			gMorphView->setVisible(FALSE);
		}

		// Disable mouselook-specific animations
		if (isAgentAvatarValid())
		{
			if (gAgentAvatarp->isAnyAnimationSignaled(AGENT_GUN_AIM_ANIMS,
													  NUM_AGENT_GUN_AIM_ANIMS))
			{
				LLVOAvatar::AnimIterator end = gAgentAvatarp->mSignaledAnimations.end();
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_AIM_RIFLE_R) != end)
				{
					sendAnimationRequest(ANIM_AGENT_AIM_RIFLE_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_RIFLE_R, ANIM_REQUEST_START);
				}
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_AIM_HANDGUN_R) != end)
				{
					sendAnimationRequest(ANIM_AGENT_AIM_HANDGUN_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_HANDGUN_R, ANIM_REQUEST_START);
				}
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_AIM_BAZOOKA_R) != end)
				{
					sendAnimationRequest(ANIM_AGENT_AIM_BAZOOKA_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_BAZOOKA_R, ANIM_REQUEST_START);
				}
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_AIM_BOW_L) != end)
				{
					sendAnimationRequest(ANIM_AGENT_AIM_BOW_L, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_BOW_L, ANIM_REQUEST_START);
				}
			}
		}
	}
	else if (mLastCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR)
	{
		// make sure we ask to save changes

		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);

		// HACK: If we're quitting, and we were in customize avatar, don't
		// let the mini-map go visible again. JC
        if (!LLAppViewer::instance()->quitRequested())
		{
			LLFloaterMap::getInstance()->popVisible();
		}

		if (gMorphView)
		{
			gMorphView->setVisible(FALSE);
		}

		if (isAgentAvatarValid())
		{
			if (mCustomAnim)
			{
				sendAnimationRequest(ANIM_AGENT_CUSTOMIZE, ANIM_REQUEST_STOP);
				sendAnimationRequest(ANIM_AGENT_CUSTOMIZE_DONE, ANIM_REQUEST_START);

				mCustomAnim = FALSE;
			}

		}
		setLookAt(LOOKAT_TARGET_CLEAR);
	}

	//---------------------------------------------------------------------
	// Set up UI for mode we're entering
	//---------------------------------------------------------------------
	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		// hide menus
		if (gMenuBarView)
		{
			gMenuBarView->setVisible(FALSE);
		}
		if (gStatusBar)
		{
			gStatusBar->setVisibleForMouselook(false);
		}

		// clear out camera lag effect
		mCameraLag.clearVec();

		// JC - Added for always chat in third person option
		gFocusMgr.setKeyboardFocus(NULL);

		LLToolMgr::getInstance()->setCurrentToolset(gMouselookToolset);

		mViewsPushed = TRUE;

		if (gFloaterView)
		{
			gFloaterView->pushVisibleAll(FALSE, get_skip_list());
		}

		if (gMorphView)
		{
			gMorphView->setVisible(FALSE);
		}

		if (gConsole)
		{
			gConsole->setVisible(TRUE);
		}

		if (isAgentAvatarValid())
		{
			// Trigger mouselook-specific animations
			if (gAgentAvatarp->isAnyAnimationSignaled(AGENT_GUN_HOLD_ANIMS,
													  NUM_AGENT_GUN_HOLD_ANIMS))
			{
				LLVOAvatar::AnimIterator end = gAgentAvatarp->mSignaledAnimations.end();
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_HOLD_RIFLE_R) != end)
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_RIFLE_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_RIFLE_R, ANIM_REQUEST_START);
				}
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_HOLD_HANDGUN_R) != end)
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_HANDGUN_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_HANDGUN_R, ANIM_REQUEST_START);
				}
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_HOLD_BAZOOKA_R) != end)
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_BAZOOKA_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_BAZOOKA_R, ANIM_REQUEST_START);
				}
				if (gAgentAvatarp->mSignaledAnimations.find(ANIM_AGENT_HOLD_BOW_L) != end)
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_BOW_L, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_BOW_L, ANIM_REQUEST_START);
				}
			}
			if (gAgentAvatarp->getParent())
			{
				LLVector3 at_axis = LLViewerCamera::getInstance()->getAtAxis();
				LLViewerObject* root_object = (LLViewerObject*)gAgentAvatarp->getRoot();
				if (root_object->flagCameraDecoupled())
				{
					resetAxes(at_axis);
				}
				else
				{
					resetAxes(at_axis * ~((LLViewerObject*)gAgentAvatarp->getParent())->getRenderRotation());
				}
			}
		}

	}
	else if (mCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR)
	{
		LLToolMgr::getInstance()->setCurrentToolset(gFaceEditToolset);

		LLFloaterMap::getInstance()->pushVisible(FALSE);
		/*
		LLView *view;
		for (view = gFloaterView->getFirstChild(); view;
			 view = gFloaterView->getNextChild())
		{
			view->pushVisible(FALSE);
		}
		*/

		if (gMorphView)
		{
			gMorphView->setVisible(TRUE);
		}

		// freeze avatar
		if (isAgentAvatarValid())
		{
			mPauseRequest = gAgentAvatarp->requestPause();
		}
	}

	if (isAgentAvatarValid())
	{
		gAgentAvatarp->updateAttachmentVisibility(mCameraMode);
	}

	if (gFloaterTools)
	{
		gFloaterTools->dirty();
	}

	// Don't let this be called more than once if the camera
	// mode hasn't changed.  --JC
	mLastCameraMode = mCameraMode;
}

//-----------------------------------------------------------------------------
// updateCamera()
//-----------------------------------------------------------------------------
void LLAgent::updateCamera()
{
	// Ventrella - changed camera_skyward to the new global "mCameraUpVector"
	mCameraUpVector = LLVector3::z_axis;
	//LLVector3	camera_skyward(0.f, 0.f, 1.f);
	// end Ventrella

	U32 camera_mode = mCameraAnimating ? mLastCameraMode : mCameraMode;

	validateFocusObject();

	if (isAgentAvatarValid() && gAgentAvatarp->mIsSitting &&
		camera_mode == CAMERA_MODE_MOUSELOOK)
	{
		// Ventrella
		// changed camera_skyward to the new global "mCameraUpVector"
		mCameraUpVector = mCameraUpVector * gAgentAvatarp->getRenderRotation();
		// end Ventrella
	}

	if (cameraThirdPerson() && mFocusOnAvatar &&
		LLFollowCamMgr::getActiveFollowCamParams())
	{
		changeCameraToFollow();
	}

	// Ventrella
	// NOTE - this needs to be integrated into a general upVector system here
	// within llAgent.
	if (camera_mode == CAMERA_MODE_FOLLOW && mFocusOnAvatar)
	{
		mCameraUpVector = mFollowCam.getUpVector();
	}
	// end Ventrella

	if (mSitCameraEnabled)
	{
		if (mSitCameraReferenceObject->isDead())
		{
			setSitCamera(LLUUID::null);
		}
	}

	// Update UI with our camera inputs
	LLFloaterCamera* floater_camera = LLFloaterCamera::getInstance();
	floater_camera->mRotate->setToggleState(mOrbitRightKey > 0.f,	// left
											mOrbitUpKey > 0.f,		// top
											mOrbitLeftKey > 0.f,	// right
											mOrbitDownKey > 0.f);	// bottom

	floater_camera->mZoom->setToggleState(mOrbitInKey > 0.f,		// top
										  mOrbitOutKey > 0.f);		// bottom

	floater_camera->mTrack->setToggleState(mPanLeftKey > 0.f,		// left
										   mPanUpKey > 0.f,			// top
										   mPanRightKey > 0.f,		// right
										   mPanDownKey > 0.f);		// bottom

	// Handle camera movement based on keyboard.
	const F32 ORBIT_OVER_RATE = 90.f * DEG_TO_RAD;			// radians per second
	const F32 ORBIT_AROUND_RATE = 90.f * DEG_TO_RAD;		// radians per second
	const F32 PAN_RATE = 5.f;								// meters per second

	LLViewerCamera* camera = LLViewerCamera::getInstance();

	if (mOrbitUpKey || mOrbitDownKey)
	{
		F32 input_rate = mOrbitUpKey - mOrbitDownKey;
		cameraOrbitOver(input_rate * ORBIT_OVER_RATE / gFPSClamped);
	}

	if (mOrbitLeftKey || mOrbitRightKey)
	{
		F32 input_rate = mOrbitLeftKey - mOrbitRightKey;
		cameraOrbitAround(input_rate * ORBIT_AROUND_RATE / gFPSClamped);
	}

	if (mOrbitInKey || mOrbitOutKey)
	{
		F32 input_rate = mOrbitInKey - mOrbitOutKey;

		LLVector3d to_focus = gAgent.getPosGlobalFromAgent(camera->getOrigin()) - calcFocusPositionTargetGlobal();
		F32 distance_to_focus = (F32)to_focus.magVec();
		// Move at distance (in meters) meters per second
		cameraOrbitIn(input_rate * distance_to_focus / gFPSClamped);
	}

	if (mPanInKey || mPanOutKey)
	{
		F32 input_rate = mPanInKey - mPanOutKey;
		cameraPanIn(input_rate * PAN_RATE / gFPSClamped);
	}

	if (mPanRightKey || mPanLeftKey)
	{
		F32 input_rate = mPanRightKey - mPanLeftKey;
		cameraPanLeft(input_rate * -PAN_RATE / gFPSClamped);
	}

	if (mPanUpKey || mPanDownKey)
	{
		F32 input_rate = mPanUpKey - mPanDownKey;
		cameraPanUp(input_rate * PAN_RATE / gFPSClamped);
	}

	// Clear camera keyboard keys.
	mOrbitLeftKey		= 0.f;
	mOrbitRightKey		= 0.f;
	mOrbitUpKey			= 0.f;
	mOrbitDownKey		= 0.f;
	mOrbitInKey			= 0.f;
	mOrbitOutKey		= 0.f;

	mPanRightKey		= 0.f;
	mPanLeftKey			= 0.f;
	mPanUpKey			= 0.f;
	mPanDownKey			= 0.f;
	mPanInKey			= 0.f;
	mPanOutKey			= 0.f;

	// lerp camera focus offset
	mCameraFocusOffset = lerp(mCameraFocusOffset, mCameraFocusOffsetTarget,
							  LLCriticalDamp::getInterpolant(CAMERA_FOCUS_HALF_LIFE));

	//Ventrella
	if (mCameraMode == CAMERA_MODE_FOLLOW)
	{
		if (isAgentAvatarValid())
		{
			//--------------------------------------------------------------------------------
			// this is where the avatar's position and rotation are given to followCam, and
			// where it is updated. All three of its attributes are updated: (1) position,
			// (2) focus, and (3) upvector. They can then be queried elsewhere in llAgent.
			//--------------------------------------------------------------------------------
			// *TODO: use combined rotation of frameagent and sit object
			LLQuaternion avatarRotationForFollowCam = gAgentAvatarp->mIsSitting ? gAgentAvatarp->getRenderRotation()
																				: mFrameAgent.getQuaternion();

			LLFollowCamParams* current_cam = LLFollowCamMgr::getActiveFollowCamParams();
			if (current_cam)
			{
				mFollowCam.copyParams(*current_cam);
				mFollowCam.setSubjectPositionAndRotation(gAgentAvatarp->getRenderPosition(),
														 avatarRotationForFollowCam);
				mFollowCam.update();
			}
			else
			{
				changeCameraToThirdPerson(TRUE);
			}
		}
	}
	// end Ventrella

	BOOL hit_limit;
	LLVector3d camera_pos_global;
	LLVector3d camera_target_global = calcCameraPositionTargetGlobal(&hit_limit);
	mCameraVirtualPositionAgent = getPosAgentFromGlobal(camera_target_global);
	LLVector3d focus_target_global = calcFocusPositionTargetGlobal();

	// perform field of view correction
	mCameraFOVZoomFactor = calcCameraFOVZoomFactor();
	camera_target_global = focus_target_global + (camera_target_global - focus_target_global) * (1.f + mCameraFOVZoomFactor);

	mShowAvatar = TRUE; // can see avatar by default

	// Adjust position for animation
	if (mCameraAnimating)
	{
		F32 time = mAnimationTimer.getElapsedTimeF32();

		// yet another instance of critically damped motion, hooray!
		// F32 fraction_of_animation = 1.f - pow(2.f, -time / CAMERA_ZOOM_HALF_LIFE);

		// linear interpolation
		F32 fraction_of_animation = time / mAnimationDuration;

		BOOL isfirstPerson = mCameraMode == CAMERA_MODE_MOUSELOOK;
		BOOL wasfirstPerson = mLastCameraMode == CAMERA_MODE_MOUSELOOK;
		F32 fraction_animation_to_skip;

		if (mAnimationCameraStartGlobal == camera_target_global)
		{
			fraction_animation_to_skip = 0.f;
		}
		else
		{
			LLVector3d cam_delta = mAnimationCameraStartGlobal - camera_target_global;
			fraction_animation_to_skip = HEAD_BUFFER_SIZE / (F32)cam_delta.magVec();
		}
		F32 animation_start_fraction = (wasfirstPerson) ? fraction_animation_to_skip : 0.f;
		F32 animation_finish_fraction =  (isfirstPerson) ? (1.f - fraction_animation_to_skip) : 1.f;

		if (fraction_of_animation < animation_finish_fraction)
		{
			if (fraction_of_animation < animation_start_fraction || fraction_of_animation > animation_finish_fraction)
			{
				mShowAvatar = FALSE;
			}

			// ...adjust position for animation
			F32 smooth_fraction_of_animation = llsmoothstep(0.0f, 1.0f, fraction_of_animation);
			camera_pos_global = lerp(mAnimationCameraStartGlobal, camera_target_global, smooth_fraction_of_animation);
			mFocusGlobal = lerp(mAnimationFocusStartGlobal, focus_target_global, smooth_fraction_of_animation);
		}
		else
		{
			// ...animation complete
			mCameraAnimating = FALSE;

			camera_pos_global = camera_target_global;
			mFocusGlobal = focus_target_global;

			endAnimationUpdateUI();
			mShowAvatar = TRUE;
		}

		if (isAgentAvatarValid() && mCameraMode != CAMERA_MODE_MOUSELOOK)
		{
			gAgentAvatarp->updateAttachmentVisibility(mCameraMode);
		}
	}
	else
	{
		camera_pos_global = camera_target_global;
		mFocusGlobal = focus_target_global;
		mShowAvatar = TRUE;
	}

	// smoothing
	if (TRUE)
	{
		LLVector3d agent_pos = getPositionGlobal();
		LLVector3d camera_pos_agent = camera_pos_global - agent_pos;
		// Sitting on what you're manipulating can cause camera jitter with smoothing.
		// This turns off smoothing while editing. -MG
		mCameraSmoothingStop |= (BOOL)LLToolMgr::getInstance()->inBuildMode();

		if (cameraThirdPerson() && !mCameraSmoothingStop)
		{
			const F32 SMOOTHING_HALF_LIFE = 0.02f;

			static LLCachedControl<F32> camera_position_smoothing(gSavedSettings, "CameraPositionSmoothing");
			F32 smoothing = LLCriticalDamp::getInterpolant(camera_position_smoothing * SMOOTHING_HALF_LIFE, FALSE);

			if (!mFocusObject)  // we differentiate on avatar mode
			{
				// for avatar-relative focus, we smooth in avatar space -
				// the avatar moves too jerkily w/r/t global space to smooth there.

				LLVector3d delta = camera_pos_agent - mCameraSmoothingLastPositionAgent;
				if (delta.magVec() < MAX_CAMERA_SMOOTH_DISTANCE)  // only smooth over short distances please
				{
					camera_pos_agent = lerp(mCameraSmoothingLastPositionAgent, camera_pos_agent, smoothing);
					camera_pos_global = camera_pos_agent + agent_pos;
				}
			}
			else
			{
				LLVector3d delta = camera_pos_global - mCameraSmoothingLastPositionGlobal;
				if (delta.magVec() < MAX_CAMERA_SMOOTH_DISTANCE) // only smooth over short distances please
				{
					camera_pos_global = lerp(mCameraSmoothingLastPositionGlobal, camera_pos_global, smoothing);
				}
			}
		}

		mCameraSmoothingLastPositionGlobal = camera_pos_global;
		mCameraSmoothingLastPositionAgent = camera_pos_agent;
		mCameraSmoothingStop = FALSE;
	}

	mCameraCurrentFOVZoomFactor = lerp(mCameraCurrentFOVZoomFactor, mCameraFOVZoomFactor, LLCriticalDamp::getInterpolant(FOV_ZOOM_HALF_LIFE));

//	llinfos << "Current FOV Zoom: " << mCameraCurrentFOVZoomFactor << " Target FOV Zoom: " << mCameraFOVZoomFactor << " Object penetration: " << mFocusObjectDist << llendl;

	F32 ui_offset = 0.f;
	if (CAMERA_MODE_CUSTOMIZE_AVATAR == mCameraMode)
	{
		ui_offset = calcCustomizeAvatarUIOffset(camera_pos_global);
	}

	LLVector3 focus_agent = getPosAgentFromGlobal(mFocusGlobal);

	mCameraPositionAgent	= getPosAgentFromGlobal(camera_pos_global);

	// Move the camera

	//Ventrella
	camera->updateCameraLocation(mCameraPositionAgent, mCameraUpVector, focus_agent);
	//camera->updateCameraLocation(mCameraPositionAgent, camera_skyward, focus_agent);
	//end Ventrella

	//RN: translate UI offset after camera is oriented properly
	camera->translate(camera->getLeftAxis() * ui_offset);

	// Change FOV
	camera->setView(camera->getDefaultFOV() / (1.f + mCameraCurrentFOVZoomFactor));

	// follow camera when in customize mode
	if (cameraCustomizeAvatar())
	{
		setLookAt(LOOKAT_TARGET_FOCUS, NULL, mCameraPositionAgent);
	}

	// update the travel distance stat
	// this isn't directly related to the camera
	// but this seemed like the best place to do this
	LLVector3d global_pos = getPositionGlobal();
	if (! mLastPositionGlobal.isExactlyZero())
	{
		LLVector3d delta = global_pos - mLastPositionGlobal;
		mDistanceTraveled += delta.magVec();
	}
	mLastPositionGlobal = global_pos;

	if (LLVOAvatar::sVisibleInFirstPerson && isAgentAvatarValid() &&
		!gAgentAvatarp->mIsSitting && cameraMouselook())
	{
		LLVector3 head_pos = gAgentAvatarp->mHeadp->getWorldPosition() +
							 LLVector3(0.08f, 0.f, 0.05f) * gAgentAvatarp->mHeadp->getWorldRotation() +
							 LLVector3(0.1f, 0.f, 0.f) * gAgentAvatarp->mPelvisp->getWorldRotation();
		LLVector3 diff = mCameraPositionAgent - head_pos;
		diff = diff * ~gAgentAvatarp->mRoot.getWorldRotation();

		LLJoint* torso_joint = gAgentAvatarp->mTorsop;
		LLJoint* chest_joint = gAgentAvatarp->mChestp;
		LLVector3 torso_scale = torso_joint->getScale();
		LLVector3 chest_scale = chest_joint->getScale();

		// shorten avatar skeleton to avoid foot interpenetration
		if (!gAgentAvatarp->mInAir)
		{
			LLVector3 chest_offset = LLVector3(0.f, 0.f,
											   chest_joint->getPosition().mV[VZ]) *
									 torso_joint->getWorldRotation();
			F32 z_compensate = llclamp(-diff.mV[VZ], -0.2f, 1.f);
			F32 scale_factor = llclamp(1.f - (z_compensate * 0.5f / chest_offset.mV[VZ]),
									   0.5f, 1.2f);
			torso_joint->setScale(LLVector3(1.f, 1.f, scale_factor));

			LLJoint* neck_joint = gAgentAvatarp->mNeckp;
			LLVector3 neck_offset = LLVector3(0.f, 0.f,
											  neck_joint->getPosition().mV[VZ]) *
									chest_joint->getWorldRotation();
			scale_factor = llclamp(1.f - (z_compensate * 0.5f / neck_offset.mV[VZ]),
								   0.5f, 1.2f);
			chest_joint->setScale(LLVector3(1.f, 1.f, scale_factor));
			diff.mV[VZ] = 0.f;
		}

		gAgentAvatarp->mPelvisp->setPosition(gAgentAvatarp->mPelvisp->getPosition() + diff);

		gAgentAvatarp->mRoot.updateWorldMatrixChildren();

		for (LLVOAvatar::attachment_map_t::iterator
				iter = gAgentAvatarp->mAttachmentPoints.begin(),
				end = gAgentAvatarp->mAttachmentPoints.end();
			 iter != end; ++iter)
		{
			LLViewerJointAttachment* attachment = iter->second;
			for (LLViewerJointAttachment::attachedobjs_vec_t::iterator
					attachment_iter = attachment->mAttachedObjects.begin(),
					end = attachment->mAttachedObjects.end();
				 attachment_iter != end; ++attachment_iter)
			{
				LLViewerObject *attached_object = *attachment_iter;
				if (attached_object && !attached_object->isDead() &&
					attached_object->mDrawable.notNull())
				{
					// clear any existing "early" movements of attachment
					attached_object->mDrawable->clearState(LLDrawable::EARLY_MOVE);
					gPipeline.updateMoveNormalAsync(attached_object->mDrawable);
					attached_object->updateText();
				}
			}
		}

		torso_joint->setScale(torso_scale);
		chest_joint->setScale(chest_scale);
	}
}

void LLAgent::updateFocusOffset()
{
	validateFocusObject();
	if (mFocusObject.notNull())
	{
		LLVector3d obj_pos = getPosGlobalFromAgent(mFocusObject->getRenderPosition());
		mFocusObjectOffset.setVec(mFocusTargetGlobal - obj_pos);
	}
}

void LLAgent::validateFocusObject()
{
	if (mFocusObject.notNull() && mFocusObject->isDead())
	{
		mFocusObjectOffset.clearVec();
		clearFocusObject();
		mCameraFOVZoomFactor = 0.f;
	}
}

//-----------------------------------------------------------------------------
// calcCustomizeAvatarUIOffset()
//-----------------------------------------------------------------------------
F32 LLAgent::calcCustomizeAvatarUIOffset(const LLVector3d& camera_pos_global)
{
	F32 ui_offset = 0.f;

	if (gFloaterCustomize && gViewerWindow)
	{
		const LLRect& rect = gFloaterCustomize->getRect();

		// Move the camera so that the avatar isn't covered up by this floater.
		F32 fraction_of_fov = 0.5f - (0.5f * (1.f - llmin(1.f, ((F32)rect.getWidth() / (F32)gViewerWindow->getWindowWidth()))));
		F32 apparent_angle = fraction_of_fov * LLViewerCamera::getInstance()->getView() * LLViewerCamera::getInstance()->getAspect();  // radians
		F32 offset = tan(apparent_angle);

		if (rect.mLeft < (gViewerWindow->getWindowWidth() - rect.mRight))
		{
			// Move the avatar to the right (camera to the left)
			ui_offset = offset;
		}
		else
		{
			// Move the avatar to the left (camera to the right)
			ui_offset = -offset;
		}
	}
	F32 range = (F32)dist_vec(camera_pos_global, gAgent.getFocusGlobal());
	mUIOffset = lerp(mUIOffset, ui_offset, LLCriticalDamp::getInterpolant(0.05f));
	return mUIOffset * range;
}

//-----------------------------------------------------------------------------
// calcFocusPositionTargetGlobal()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::calcFocusPositionTargetGlobal()
{
	if (mFocusObject.notNull() && mFocusObject->isDead())
	{
		clearFocusObject();
	}

	// Ventrella
	if (mCameraMode == CAMERA_MODE_FOLLOW && mFocusOnAvatar)
	{
		mFocusTargetGlobal = gAgent.getPosGlobalFromAgent(mFollowCam.getSimulatedFocus());
		return mFocusTargetGlobal;
	}// End Ventrella
	else if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		LLVector3d at_axis(1.0, 0.0, 0.0);
		LLQuaternion agent_rot = mFrameAgent.getQuaternion();
		if (isAgentAvatarValid() && gAgentAvatarp->getParent())
		{
			LLViewerObject* root_object = (LLViewerObject*)gAgentAvatarp->getRoot();
			if (!root_object->flagCameraDecoupled())
			{
				agent_rot *= ((LLViewerObject*)(gAgentAvatarp->getParent()))->getRenderRotation();
			}
		}
		at_axis = at_axis * agent_rot;
		mFocusTargetGlobal = calcCameraPositionTargetGlobal() + at_axis;
		return mFocusTargetGlobal;
	}
	else if (mCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR)
	{
		return mFocusTargetGlobal;
	}
	else if (!mFocusOnAvatar)
	{
		if (mFocusObject.notNull() && !mFocusObject->isDead() && mFocusObject->mDrawable.notNull())
		{
			LLDrawable* drawablep = mFocusObject->mDrawable;

			if (mTrackFocusObject &&
				drawablep &&
				drawablep->isActive())
			{
				if (!mFocusObject->isAvatar())
				{
					if (mFocusObject->isSelected())
					{
						gPipeline.updateMoveNormalAsync(drawablep);
					}
					else
					{
						if (drawablep->isState(LLDrawable::MOVE_UNDAMPED))
						{
							gPipeline.updateMoveNormalAsync(drawablep);
						}
						else
						{
							gPipeline.updateMoveDampedAsync(drawablep);
						}
					}
				}
			}
			// if not tracking object, update offset based on new object position
			else
			{
				updateFocusOffset();
			}
			LLVector3 focus_agent = mFocusObject->getRenderPosition() + mFocusObjectOffset;
			mFocusTargetGlobal.setVec(getPosGlobalFromAgent(focus_agent));
		}
		return mFocusTargetGlobal;
	}
	else if (mSitCameraEnabled && isAgentAvatarValid() && gAgentAvatarp->mIsSitting && mSitCameraReferenceObject.notNull())
	{
		// sit camera
		LLVector3 object_pos = mSitCameraReferenceObject->getRenderPosition();
		LLQuaternion object_rot = mSitCameraReferenceObject->getRenderRotation();

		LLVector3 target_pos = object_pos + (mSitCameraFocus * object_rot);
		return getPosGlobalFromAgent(target_pos);
	}
	else
	{
		return getPositionGlobal() + calcThirdPersonFocusOffset();
	}
}

LLVector3d LLAgent::calcThirdPersonFocusOffset()
{
	// ...offset from avatar
	LLVector3d focus_offset;
	focus_offset.setVec(mCameraFocusOffsetDefault);

	LLQuaternion agent_rot = mFrameAgent.getQuaternion();
	if (!!isAgentAvatarValid() && gAgentAvatarp->getParent())
	{
		agent_rot *= ((LLViewerObject*)(gAgentAvatarp->getParent()))->getRenderRotation();
	}

	focus_offset = focus_offset * agent_rot;
	return focus_offset;
}

void LLAgent::setupSitCamera()
{
	// agent frame entering this function is in world coordinates
	if (isAgentAvatarValid() && gAgentAvatarp->getParent())
	{
		LLQuaternion parent_rot = ((LLViewerObject*)gAgentAvatarp->getParent())->getRenderRotation();
		// slam agent coordinate frame to proper parent local version
		LLVector3 at_axis = mFrameAgent.getAtAxis();
		at_axis.mV[VZ] = 0.f;
		at_axis.normalize();
		resetAxes(at_axis * ~parent_rot);
	}
}

void LLAgent::setupCameraView(bool reset)
{
	static BOOL rear_view = FALSE;

	BOOL new_rear_view = gSavedSettings.getBOOL("CameraFrontView");
	if (new_rear_view &&
		(mCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR ||
		 mCameraMode == CAMERA_MODE_MOUSELOOK || reset))
	{
		gSavedSettings.setBOOL("CameraFrontView", FALSE);
		new_rear_view = FALSE;
	}
	if (new_rear_view)
	{
		mCameraFocusOffsetDefault = gSavedSettings.getVector3("FocusOffsetFrontView");
		mCameraOffsetDefault = gSavedSettings.getVector3("CameraOffsetFrontView");
	}
	else
	{
		mCameraFocusOffsetDefault = gSavedSettings.getVector3("FocusOffsetDefault");
		mCameraOffsetDefault = gSavedSettings.getVector3("CameraOffsetDefault");
	}
	if (rear_view != new_rear_view)
	{
		rear_view = new_rear_view;
		updateCamera();
	}
}

//-----------------------------------------------------------------------------
// getCameraPositionAgent()
//-----------------------------------------------------------------------------
const LLVector3 &LLAgent::getCameraPositionAgent() const
{
	return LLViewerCamera::getInstance()->getOrigin();
}

//-----------------------------------------------------------------------------
// getCameraPositionGlobal()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::getCameraPositionGlobal() const
{
	return getPosGlobalFromAgent(LLViewerCamera::getInstance()->getOrigin());
}

//-----------------------------------------------------------------------------
// calcCameraFOVZoomFactor()
//-----------------------------------------------------------------------------
F32	LLAgent::calcCameraFOVZoomFactor()
{
	LLVector3 camera_offset_dir;
	camera_offset_dir.setVec(mCameraFocusOffset);

	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		return 0.f;
	}
	else if (mFocusObject.notNull() && !mFocusObject->isAvatar())
	{
		// don't FOV zoom on mostly transparent objects
		LLVector3 focus_offset = mFocusObjectOffset;
		F32 obj_min_dist = 0.f;
		calcCameraMinDistance(obj_min_dist);
		F32 current_distance = llmax(0.001f, camera_offset_dir.magVec());

		mFocusObjectDist = obj_min_dist - current_distance;

		F32 new_fov_zoom = llclamp(mFocusObjectDist / current_distance, 0.f, 1000.f);
		return new_fov_zoom;
	}
	else // focusing on land or avatar
	{
		// keep old field of view until user changes focus explicitly
		return mCameraFOVZoomFactor;
		//return 0.f;
	}
}

//-----------------------------------------------------------------------------
// calcCameraPositionTargetGlobal()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::calcCameraPositionTargetGlobal(BOOL *hit_limit)
{
	// Compute base camera position and look-at points.
	F32			camera_land_height;
	LLVector3d	frame_center_global = !isAgentAvatarValid() ? getPositionGlobal()
															 : getPosGlobalFromAgent(gAgentAvatarp->mRoot.getWorldPosition());

	LLVector3   upAxis = getUpAxis();
	BOOL		isConstrained = FALSE;
	LLVector3d	head_offset;
	head_offset.setVec(mThirdPersonHeadOffset);

	LLVector3d camera_position_global;

	// Ventrella
	if (mCameraMode == CAMERA_MODE_FOLLOW && mFocusOnAvatar)
	{
		camera_position_global = gAgent.getPosGlobalFromAgent(mFollowCam.getSimulatedPosition());
	}// End Ventrella
	else if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		if (!isAgentAvatarValid() || gAgentAvatarp->mDrawable.isNull())
		{
			llwarns << "Null avatar drawable!" << llendl;
			return LLVector3d::zero;
		}
		head_offset.clearVec();
		if (gAgentAvatarp->mIsSitting && gAgentAvatarp->getParent())
		{
			gAgentAvatarp->updateHeadOffset();
			head_offset.mdV[VX] = gAgentAvatarp->mHeadOffset.mV[VX];
			head_offset.mdV[VY] = gAgentAvatarp->mHeadOffset.mV[VY];
			head_offset.mdV[VZ] = gAgentAvatarp->mHeadOffset.mV[VZ] + 0.1f;
			const LLMatrix4& mat = ((LLViewerObject*) gAgentAvatarp->getParent())->getRenderMatrix();
			camera_position_global = getPosGlobalFromAgent
								((gAgentAvatarp->getPosition()+
								 LLVector3(head_offset) * gAgentAvatarp->getRotation()) * mat);
		}
		else
		{
			head_offset.mdV[VZ] = gAgentAvatarp->mHeadOffset.mV[VZ];
			if (gAgentAvatarp->mIsSitting)
			{
				head_offset.mdV[VZ] += 0.1;
			}
			camera_position_global = getPosGlobalFromAgent(gAgentAvatarp->getRenderPosition());//frame_center_global;
			head_offset = head_offset * gAgentAvatarp->getRenderRotation();
			camera_position_global = camera_position_global + head_offset;
		}
	}
	else if (mCameraMode == CAMERA_MODE_THIRD_PERSON && mFocusOnAvatar)
	{
		LLVector3 local_camera_offset;
		F32 camera_distance = 0.f;

		if (mSitCameraEnabled
			&& isAgentAvatarValid()
			&& gAgentAvatarp->mIsSitting
			&& mSitCameraReferenceObject.notNull())
		{
			// sit camera
			LLVector3 object_pos = mSitCameraReferenceObject->getRenderPosition();
			LLQuaternion object_rot = mSitCameraReferenceObject->getRenderRotation();

			LLVector3 target_pos = object_pos + (mSitCameraPos * object_rot);

			camera_position_global = getPosGlobalFromAgent(target_pos);
		}
		else
		{
			static LLCachedControl<F32> camera_offset_scale(gSavedSettings,
															"CameraOffsetScale");
			local_camera_offset = mCameraZoomFraction * mCameraOffsetDefault * camera_offset_scale;

			// are we sitting down?
			if (isAgentAvatarValid() && gAgentAvatarp->getParent())
			{
				LLQuaternion parent_rot = ((LLViewerObject*)gAgentAvatarp->getParent())->getRenderRotation();
				// slam agent coordinate frame to proper parent local version
				LLVector3 at_axis = mFrameAgent.getAtAxis() * parent_rot;
				at_axis.mV[VZ] = 0.f;
				at_axis.normalize();
				resetAxes(at_axis * ~parent_rot);

				local_camera_offset = local_camera_offset * mFrameAgent.getQuaternion() * parent_rot;
			}
			else
			{
				local_camera_offset = mFrameAgent.rotateToAbsolute(local_camera_offset);
			}

			if (!mCameraCollidePlane.isExactlyZero() && (!isAgentAvatarValid() || !gAgentAvatarp->mIsSitting))
			{
				LLVector3 plane_normal;
				plane_normal.setVec(mCameraCollidePlane.mV);

				F32 offset_dot_norm = local_camera_offset * plane_normal;
				if (llabs(offset_dot_norm) < 0.001f)
				{
					offset_dot_norm = 0.001f;
				}

				camera_distance = local_camera_offset.normalize();

				F32 pos_dot_norm = getPosAgentFromGlobal(frame_center_global + head_offset) * plane_normal;

				// if agent is outside the colliding half-plane
				if (pos_dot_norm > mCameraCollidePlane.mV[VW])
				{
					// check to see if camera is on the opposite side (inside) the half-plane
					if (offset_dot_norm + pos_dot_norm < mCameraCollidePlane.mV[VW])
					{
						// diminish offset by factor to push it back outside the half-plane
						camera_distance *= (pos_dot_norm - mCameraCollidePlane.mV[VW] - CAMERA_COLLIDE_EPSILON) / -offset_dot_norm;
					}
				}
				else
				{
					if (offset_dot_norm + pos_dot_norm > mCameraCollidePlane.mV[VW])
					{
						camera_distance *= (mCameraCollidePlane.mV[VW] - pos_dot_norm - CAMERA_COLLIDE_EPSILON) / offset_dot_norm;
					}
				}
			}
			else
			{
				camera_distance = local_camera_offset.normalize();
			}

			mTargetCameraDistance = llmax(camera_distance, MIN_CAMERA_DISTANCE);

			if (mTargetCameraDistance != mCurrentCameraDistance)
			{
				F32 camera_lerp_amt = LLCriticalDamp::getInterpolant(CAMERA_ZOOM_HALF_LIFE);

				mCurrentCameraDistance = lerp(mCurrentCameraDistance, mTargetCameraDistance, camera_lerp_amt);
			}

			// Make the camera distance current
			local_camera_offset *= mCurrentCameraDistance;

			// set the global camera position
			LLVector3d camera_offset;

			LLVector3 av_pos = !isAgentAvatarValid() ? LLVector3::zero : gAgentAvatarp->getRenderPosition();
			camera_offset.setVec(local_camera_offset);
			camera_position_global = frame_center_global + head_offset + camera_offset;

			if (isAgentAvatarValid())
			{
				LLVector3d camera_lag_d;
				F32 lag_interp = LLCriticalDamp::getInterpolant(CAMERA_LAG_HALF_LIFE);
				LLVector3 target_lag;
				LLVector3 vel = getVelocity();

				// lag by appropriate amount for flying
				F32 time_in_air = gAgentAvatarp->mTimeInAir.getElapsedTimeF32();
				if (!mCameraAnimating && gAgentAvatarp->mInAir && time_in_air > GROUND_TO_AIR_CAMERA_TRANSITION_START_TIME)
				{
					LLVector3 frame_at_axis = mFrameAgent.getAtAxis();
					frame_at_axis -= projected_vec(frame_at_axis, getReferenceUpVector());
					frame_at_axis.normalize();

					//transition smoothly in air mode, to avoid camera pop
					F32 u = (time_in_air - GROUND_TO_AIR_CAMERA_TRANSITION_START_TIME) / GROUND_TO_AIR_CAMERA_TRANSITION_TIME;
					u = llclamp(u, 0.f, 1.f);

					lag_interp *= u;

					if (gViewerWindow->getLeftMouseDown() &&
						gViewerWindow->getLastPick().mObjectID == gAgentAvatarp->getID())
					{
						// disable camera lag when using mouse-directed steering
						target_lag.clearVec();
					}
					else
					{
						static LLCachedControl<F32> dynamic_camera_strength(gSavedSettings,
																			"DynamicCameraStrength");
						target_lag = vel * dynamic_camera_strength / 30.f;
					}

					mCameraLag = lerp(mCameraLag, target_lag, lag_interp);

					F32 lag_dist = mCameraLag.magVec();
					if (lag_dist > MAX_CAMERA_LAG)
					{
						mCameraLag = mCameraLag * MAX_CAMERA_LAG / lag_dist;
					}

					// clamp camera lag so that avatar is always in front
					F32 dot = (mCameraLag - (frame_at_axis * (MIN_CAMERA_LAG * u))) * frame_at_axis;
					if (dot < -(MIN_CAMERA_LAG * u))
					{
						mCameraLag -= (dot + (MIN_CAMERA_LAG * u)) * frame_at_axis;
					}
				}
				else
				{
					mCameraLag = lerp(mCameraLag, LLVector3::zero, LLCriticalDamp::getInterpolant(0.15f));
				}

				camera_lag_d.setVec(mCameraLag);
				camera_position_global = camera_position_global - camera_lag_d;
			}
		}
	}
	else
	{
		LLVector3d focusPosGlobal = calcFocusPositionTargetGlobal();
		// camera gets pushed out later wrt mCameraFOVZoomFactor...this is "raw" value
		camera_position_global = focusPosGlobal + mCameraFocusOffset;
	}

	static LLCachedControl<bool> disable_camera_constraints(gSavedSettings,
															"DisableCameraConstraints");
	if (!disable_camera_constraints && !gAgent.isGodlike())
	{
		LLViewerRegion* regionp = LLWorld::getInstance()->getRegionFromPosGlobal(
															camera_position_global);
		bool constrain = true;
		if (regionp && regionp->canManageEstate())
		{
			constrain = false;
		}
		if (constrain)
		{
			F32 max_dist = (CAMERA_MODE_CUSTOMIZE_AVATAR == mCameraMode) ?
				APPEARANCE_MAX_ZOOM : mDrawDistance;

			LLVector3d camera_offset = camera_position_global
				- gAgent.getPositionGlobal();
			F32 camera_distance = (F32)camera_offset.magVec();

			if (camera_distance > max_dist)
			{
				camera_position_global = gAgent.getPositionGlobal() +
					(max_dist / camera_distance) * camera_offset;
				isConstrained = TRUE;
			}
		}

// JC - Could constrain camera based on parcel stuff here.
//			LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromPosGlobal(camera_position_global);
//
//			if (regionp && !regionp->mParcelOverlay->isBuildCameraAllowed(regionp->getPosRegionFromGlobal(camera_position_global)))
//			{
//				camera_position_global = last_position_global;
//
//				isConstrained = TRUE;
//			}
	}

	// Don't let camera go underground
	F32 camera_min_off_ground = getCameraMinOffGround();

	camera_land_height = LLWorld::getInstance()->resolveLandHeightGlobal(camera_position_global);

	if (camera_position_global.mdV[VZ] < camera_land_height + camera_min_off_ground)
	{
		camera_position_global.mdV[VZ] = camera_land_height + camera_min_off_ground;
		isConstrained = TRUE;
	}

	if (hit_limit)
	{
		*hit_limit = isConstrained;
	}

	return camera_position_global;
}

//-----------------------------------------------------------------------------
// handleScrollWheel()
//-----------------------------------------------------------------------------
void LLAgent::handleScrollWheel(S32 clicks)
{
	if (mCameraMode == CAMERA_MODE_FOLLOW && gAgent.getFocusOnAvatar())
	{
		if (! mFollowCam.getPositionLocked()) // not if the followCam position is locked in place
		{
			mFollowCam.zoom(clicks);
			if (mFollowCam.isZoomedToMinimumDistance())
			{
				changeCameraToMouselook(FALSE);
			}
		}
	}
	else
	{
		LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
		const F32 ROOT_ROOT_TWO = sqrt(F_SQRT2);

		// Block if camera is animating
		if (mCameraAnimating)
		{
			return;
		}

		if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
		{
			F32 zoom_factor = (F32)pow(0.8, -clicks);
			cameraZoomIn(zoom_factor);
		}
		else if (mFocusOnAvatar && mCameraMode == CAMERA_MODE_THIRD_PERSON)
		{
			static LLCachedControl<F32> camera_offset_scale(gSavedSettings,
															"CameraOffsetScale");
			F32 current_zoom_fraction = mTargetCameraDistance / (mCameraOffsetDefault.magVec() * camera_offset_scale);
			current_zoom_fraction *= 1.f - pow(ROOT_ROOT_TWO, clicks);
			cameraOrbitIn(current_zoom_fraction * mCameraOffsetDefault.magVec() * camera_offset_scale);
		}
		else
		{
			F32 current_zoom_fraction = (F32)mCameraFocusOffsetTarget.magVec();
			cameraOrbitIn(current_zoom_fraction * (1.f - pow(ROOT_ROOT_TWO, clicks)));
		}
	}
}

//-----------------------------------------------------------------------------
// getCameraMinOffGround()
//-----------------------------------------------------------------------------
F32 LLAgent::getCameraMinOffGround()
{
	static LLCachedControl<bool> disable_camera_constraints(gSavedSettings,
															"DisableCameraConstraints");
	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		return 0.f;
	}
	else
	{
		if (disable_camera_constraints)
		{
			return -1000.f;
		}
		else
		{
			return 0.5f;
		}
	}
}

//-----------------------------------------------------------------------------
// resetCamera()
//-----------------------------------------------------------------------------
void LLAgent::resetCamera()
{
	// Remove any pitch from the avatar
	LLVector3 at = mFrameAgent.getAtAxis();
	at.mV[VZ] = 0.f;
	at.normalize();
	gAgent.resetAxes(at);
	// have to explicitly clear field of view zoom now
	mCameraFOVZoomFactor = 0.f;

	updateCamera();
}

//-----------------------------------------------------------------------------
// changeCameraToMouselook()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToMouselook(BOOL animate)
{
	if (!gViewerWindow ||
		LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	// visibility changes at end of animation
	gViewerWindow->getWindow()->resetBusyCount();

	// Menus should not remain open on switching to mouselook...
	LLMenuGL::sMenuContainer->hideMenus();

	// unpause avatar animation
	mPauseRequest = NULL;

	LLToolMgr::getInstance()->setCurrentToolset(gMouselookToolset);

	gSavedSettings.setBOOL("BuildBtnState", FALSE);

	setupCameraView(true);	// Reset the view to rear view.

	if (isAgentAvatarValid())
	{
		gAgentAvatarp->stopMotion(ANIM_AGENT_BODY_NOISE);
		gAgentAvatarp->stopMotion(ANIM_AGENT_BREATHE_ROT);
	}

	//gViewerWindow->stopGrab();
	LLSelectMgr::getInstance()->deselectAll();
	gViewerWindow->hideCursor();
	gViewerWindow->moveCursorToCenter();

	if (mCameraMode != CAMERA_MODE_MOUSELOOK)
	{
		gFocusMgr.setKeyboardFocus(NULL);

		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_MOUSELOOK;
		U32 old_flags = mControlFlags;
		setControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

		if (animate)
		{
			startCameraAnimation();
		}
		else
		{
			mCameraAnimating = FALSE;
			endAnimationUpdateUI();
		}
		gViewerWindow->resetMouselookFadeTimer();
	}
}

//-----------------------------------------------------------------------------
// changeCameraToDefault()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToDefault()
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	if (LLFollowCamMgr::getActiveFollowCamParams())
	{
		changeCameraToFollow();
	}
	else
	{
		changeCameraToThirdPerson();
	}
}

// Ventrella
//-----------------------------------------------------------------------------
// changeCameraToFollow()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToFollow(BOOL animate)
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	if (mCameraMode != CAMERA_MODE_FOLLOW)
	{
		if (mCameraMode == CAMERA_MODE_MOUSELOOK)
		{
			animate = FALSE;
		}
		startCameraAnimation();

		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_FOLLOW;

		// bang-in the current focus, position, and up vector of the follow cam
		mFollowCam.reset(mCameraPositionAgent, LLViewerCamera::getInstance()->getPointOfInterest(), LLVector3::z_axis);

		if (gBasicToolset)
		{
			LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		}

		if (isAgentAvatarValid())
		{
			gAgentAvatarp->mPelvisp->setPosition(LLVector3::zero);
			gAgentAvatarp->startMotion(ANIM_AGENT_BODY_NOISE);
			gAgentAvatarp->startMotion(ANIM_AGENT_BREATHE_ROT);
		}

		gSavedSettings.setBOOL("BuildBtnState", FALSE);

		// unpause avatar animation
		mPauseRequest = NULL;

		U32 old_flags = mControlFlags;
		clearControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

		if (animate)
		{
			startCameraAnimation();
		}
		else
		{
			mCameraAnimating = FALSE;
			endAnimationUpdateUI();
		}
	}
}

//-----------------------------------------------------------------------------
// changeCameraToThirdPerson()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToThirdPerson(BOOL animate)
{
	if (!gViewerWindow ||
		LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	gViewerWindow->getWindow()->resetBusyCount();

	mCameraZoomFraction = INITIAL_ZOOM_FRACTION;

	if (isAgentAvatarValid())
	{
		if (!gAgentAvatarp->mIsSitting)
		{
			gAgentAvatarp->mPelvisp->setPosition(LLVector3::zero);
		}
		gAgentAvatarp->startMotion(ANIM_AGENT_BODY_NOISE);
		gAgentAvatarp->startMotion(ANIM_AGENT_BREATHE_ROT);
	}

	gSavedSettings.setBOOL("BuildBtnState", FALSE);

	LLVector3 at_axis;

	// unpause avatar animation
	mPauseRequest = NULL;

	if (mCameraMode != CAMERA_MODE_THIRD_PERSON)
	{
		if (gBasicToolset)
		{
			LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		}

		mCameraLag.clearVec();
		if (mCameraMode == CAMERA_MODE_MOUSELOOK)
		{
			mCurrentCameraDistance = MIN_CAMERA_DISTANCE;
			mTargetCameraDistance = MIN_CAMERA_DISTANCE;
			animate = FALSE;
		}
		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_THIRD_PERSON;
		U32 old_flags = mControlFlags;
		clearControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

	}

	// Remove any pitch from the avatar
	if (isAgentAvatarValid() && gAgentAvatarp->getParent())
	{
		LLQuaternion obj_rot = ((LLViewerObject*)gAgentAvatarp->getParent())->getRenderRotation();
		at_axis = LLViewerCamera::getInstance()->getAtAxis();
		at_axis.mV[VZ] = 0.f;
		at_axis.normalize();
		resetAxes(at_axis * ~obj_rot);
	}
	else
	{
		at_axis = mFrameAgent.getAtAxis();
		at_axis.mV[VZ] = 0.f;
		at_axis.normalize();
		resetAxes(at_axis);
	}

	if (animate)
	{
		startCameraAnimation();
	}
	else
	{
		mCameraAnimating = FALSE;
		endAnimationUpdateUI();
	}
}

//-----------------------------------------------------------------------------
// changeCameraToCustomizeAvatar()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToCustomizeAvatar(BOOL avatar_animate, BOOL camera_animate)
{
	if (!gViewerWindow) return;

	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

//MK
	if (gRRenabled && gAgent.mRRInterface.mContainsUnsit)
	{
		return;
	}
//mk
	setControlFlags(AGENT_CONTROL_STAND_UP); // force stand up
//MK
	if (gRRenabled && gAgent.mRRInterface.contains("standtp"))
	{
		gAgent.mRRInterface.mSnappingBackToLastStandingLocation = true;
		gAgent.teleportViaLocationLookAt(gAgent.mRRInterface.mLastStandingLocation);
		gAgent.mRRInterface.mSnappingBackToLastStandingLocation = false;
	}
//mk

	gViewerWindow->getWindow()->resetBusyCount();

	if (gFaceEditToolset)
	{
		LLToolMgr::getInstance()->setCurrentToolset(gFaceEditToolset);
	}

	gSavedSettings.setBOOL("BuildBtnState", FALSE);

	if (camera_animate)
	{
		startCameraAnimation();
	}

	// Remove any pitch from the avatar
	//LLVector3 at = mFrameAgent.getAtAxis();
	//at.mV[VZ] = 0.f;
	//at.normalize();
	//gAgent.resetAxes(at);

	if (mCameraMode != CAMERA_MODE_CUSTOMIZE_AVATAR)
	{
		setupCameraView(true);	// Reset the view to rear view.
		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_CUSTOMIZE_AVATAR;
		U32 old_flags = mControlFlags;
		clearControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

		gFocusMgr.setKeyboardFocus(NULL);
		gFocusMgr.setMouseCapture(NULL);

		LLVOAvatarSelf::onCustomizeStart();
	}

	if (isAgentAvatarValid())
	{
		if (avatar_animate && gSavedSettings.getBOOL("AppearanceAnimation")
			/*&& gSavedSettings.getBOOL("AppearanceForceStand")*/)
		{
				// Remove any pitch from the avatar
			LLVector3 at = mFrameAgent.getAtAxis();
			at.mV[VZ] = 0.f;
			at.normalize();
			gAgent.resetAxes(at);

			sendAnimationRequest(ANIM_AGENT_CUSTOMIZE, ANIM_REQUEST_START);
			mCustomAnim = TRUE;
			gAgentAvatarp->startMotion(ANIM_AGENT_CUSTOMIZE);
			LLMotion* turn_motion = gAgentAvatarp->findMotion(ANIM_AGENT_CUSTOMIZE);

			if (turn_motion)
			{
				mAnimationDuration = turn_motion->getDuration() + CUSTOMIZE_AVATAR_CAMERA_ANIM_SLOP;

			}
			else
			{
				mAnimationDuration = gSavedSettings.getF32("ZoomTime");
			}
		}

		gAgent.setFocusGlobal(LLVector3d::zero);
	}
	else
	{
		mCameraAnimating = FALSE;
		endAnimationUpdateUI();
	}
}

//
// Focus point management
//

//-----------------------------------------------------------------------------
// startCameraAnimation()
//-----------------------------------------------------------------------------
void LLAgent::startCameraAnimation()
{
	mAnimationCameraStartGlobal = getCameraPositionGlobal();
	mAnimationFocusStartGlobal = mFocusGlobal;
	mAnimationTimer.reset();
	mCameraAnimating = TRUE;
	static LLCachedControl<F32> zoom_time(gSavedSettings, "ZoomTime");
	mAnimationDuration = zoom_time;
}

//-----------------------------------------------------------------------------
// stopCameraAnimation()
//-----------------------------------------------------------------------------
void LLAgent::stopCameraAnimation()
{
	mCameraAnimating = FALSE;
}

void LLAgent::clearFocusObject()
{
	if (mFocusObject.notNull())
	{
		startCameraAnimation();

		setFocusObject(NULL);
		mFocusObjectOffset.clearVec();
	}
}

void LLAgent::setFocusObject(LLViewerObject* object)
{
	mFocusObject = object;
}

// Focus on a point, but try to keep camera position stable.
//-----------------------------------------------------------------------------
// setFocusGlobal()
//-----------------------------------------------------------------------------
void LLAgent::setFocusGlobal(const LLPickInfo& pick)
{
	LLViewerObject* objectp = gObjectList.findObject(pick.mObjectID);

	if (objectp)
	{
		// focus on object plus designated offset
		// which may or may not be same as pick.mPosGlobal
		setFocusGlobal(objectp->getPositionGlobal() + LLVector3d(pick.mObjectOffset), pick.mObjectID);
	}
	else
	{
		// focus directly on point where user clicked
		setFocusGlobal(pick.mPosGlobal, pick.mObjectID);
	}
}

void LLAgent::setFocusGlobal(const LLVector3d& focus, const LLUUID &object_id)
{
	setFocusObject(gObjectList.findObject(object_id));
	LLVector3d old_focus = mFocusTargetGlobal;
	LLViewerObject *focus_obj = mFocusObject;

	// if focus has changed
	if (old_focus != focus)
	{
		if (focus.isExactlyZero())
		{
			if (isAgentAvatarValid())
			{
				mFocusTargetGlobal = getPosGlobalFromAgent(gAgentAvatarp->mHeadp->getWorldPosition());
			}
			else
			{
				mFocusTargetGlobal = getPositionGlobal();
			}
			mCameraFocusOffsetTarget = getCameraPositionGlobal() - mFocusTargetGlobal;
			mCameraFocusOffset = mCameraFocusOffsetTarget;
			setLookAt(LOOKAT_TARGET_CLEAR);
		}
		else
		{
			mFocusTargetGlobal = focus;
			if (!focus_obj)
			{
				mCameraFOVZoomFactor = 0.f;
			}

			mCameraFocusOffsetTarget = gAgent.getPosGlobalFromAgent(mCameraVirtualPositionAgent) - mFocusTargetGlobal;

			startCameraAnimation();

			if (focus_obj)
			{
				if (focus_obj->isAvatar())
				{
					setLookAt(LOOKAT_TARGET_FOCUS, focus_obj);
				}
				else
				{
					setLookAt(LOOKAT_TARGET_FOCUS, focus_obj, (getPosAgentFromGlobal(focus) - focus_obj->getRenderPosition()) * ~focus_obj->getRenderRotation());
				}
			}
			else
			{
				setLookAt(LOOKAT_TARGET_FOCUS, NULL, getPosAgentFromGlobal(mFocusTargetGlobal));
			}
		}
	}
	else // focus == mFocusTargetGlobal
	{
		if (focus.isExactlyZero())
		{
			if (isAgentAvatarValid())
			{
				mFocusTargetGlobal = getPosGlobalFromAgent(gAgentAvatarp->mHeadp->getWorldPosition());
			}
			else
			{
				mFocusTargetGlobal = getPositionGlobal();
			}
		}
		mCameraFocusOffsetTarget = (getCameraPositionGlobal() - mFocusTargetGlobal) / (1.f + mCameraFOVZoomFactor);;
		mCameraFocusOffset = mCameraFocusOffsetTarget;
	}

	if (mFocusObject.notNull())
	{
		// for attachments, make offset relative to avatar, not the attachment
		if (mFocusObject->isAttachment())
		{
			while (mFocusObject.notNull()		// DEV-29123 - can crash with a messed-up attachment
				&& !mFocusObject->isAvatar())
			{
				mFocusObject = (LLViewerObject*) mFocusObject->getParent();
			}
			setFocusObject((LLViewerObject*)mFocusObject);
		}
		updateFocusOffset();
	}
}

// Used for avatar customization
//-----------------------------------------------------------------------------
// setCameraPosAndFocusGlobal()
//-----------------------------------------------------------------------------
void LLAgent::setCameraPosAndFocusGlobal(const LLVector3d& camera_pos, const LLVector3d& focus, const LLUUID &object_id)
{
	LLVector3d old_focus = mFocusTargetGlobal;

	F64 focus_delta_squared = (old_focus - focus).magVecSquared();
	const F64 ANIM_EPSILON_SQUARED = 0.0001;
	if (focus_delta_squared > ANIM_EPSILON_SQUARED)
	{
		startCameraAnimation();

		if (CAMERA_MODE_CUSTOMIZE_AVATAR == mCameraMode)
		{
			// Compensate for the fact that the camera has already been offset to make room for LLFloaterCustomize.
			mAnimationCameraStartGlobal -= LLVector3d(LLViewerCamera::getInstance()->getLeftAxis() * calcCustomizeAvatarUIOffset(mAnimationCameraStartGlobal));
		}
	}

	//LLViewerCamera::getInstance()->setOrigin(gAgent.getPosAgentFromGlobal(camera_pos));
	setFocusObject(gObjectList.findObject(object_id));
	mFocusTargetGlobal = focus;
	mCameraFocusOffsetTarget = camera_pos - focus;
	mCameraFocusOffset = mCameraFocusOffsetTarget;

	if (mFocusObject)
	{
		if (mFocusObject->isAvatar())
		{
			setLookAt(LOOKAT_TARGET_FOCUS, mFocusObject);
		}
		else
		{
			setLookAt(LOOKAT_TARGET_FOCUS, mFocusObject, (getPosAgentFromGlobal(focus) - mFocusObject->getRenderPosition()) * ~mFocusObject->getRenderRotation());
		}
	}
	else
	{
		setLookAt(LOOKAT_TARGET_FOCUS, NULL, getPosAgentFromGlobal(mFocusTargetGlobal));
	}

	if (mCameraAnimating)
	{
		const F64 ANIM_METERS_PER_SECOND = 10.0;
		const F64 MIN_ANIM_SECONDS = 0.5;
		F64 anim_duration = llmax(MIN_ANIM_SECONDS, sqrt(focus_delta_squared) / ANIM_METERS_PER_SECOND);
		setAnimationDuration((F32)anim_duration);
	}

	updateFocusOffset();
}

//-----------------------------------------------------------------------------
// setSitCamera()
//-----------------------------------------------------------------------------
void LLAgent::setSitCamera(const LLUUID &object_id, const LLVector3 &camera_pos, const LLVector3 &camera_focus)
{
	BOOL camera_enabled = !object_id.isNull();

	if (camera_enabled)
	{
		LLViewerObject *reference_object = gObjectList.findObject(object_id);
		if (reference_object)
		{
			//convert to root object relative?
			mSitCameraPos = camera_pos;
			mSitCameraFocus = camera_focus;
			mSitCameraReferenceObject = reference_object;
			mSitCameraEnabled = TRUE;
		}
	}
	else
	{
		mSitCameraPos.clearVec();
		mSitCameraFocus.clearVec();
		mSitCameraReferenceObject = NULL;
		mSitCameraEnabled = FALSE;
	}
}

//-----------------------------------------------------------------------------
// setFocusOnAvatar()
//-----------------------------------------------------------------------------
void LLAgent::setFocusOnAvatar(BOOL focus_on_avatar, BOOL animate)
{
	if (focus_on_avatar != mFocusOnAvatar)
	{
		if (animate)
		{
			startCameraAnimation();
		}
		else
		{
			stopCameraAnimation();
		}
	}

	//RN: when focused on the avatar, we're not "looking" at it
	// looking implies intent while focusing on avatar means
	// you're just walking around with a camera on you...eesh.
	if (!mFocusOnAvatar && focus_on_avatar)
	{
		setFocusGlobal(LLVector3d::zero);
		mCameraFOVZoomFactor = 0.f;
		if (mCameraMode == CAMERA_MODE_THIRD_PERSON)
		{
			LLVector3 at_axis;
			if (isAgentAvatarValid() && gAgentAvatarp->getParent())
			{
				LLQuaternion obj_rot = ((LLViewerObject*)gAgentAvatarp->getParent())->getRenderRotation();
				at_axis = LLViewerCamera::getInstance()->getAtAxis();
				at_axis.mV[VZ] = 0.f;
				at_axis.normalize();
				resetAxes(at_axis * ~obj_rot);
			}
			else
			{
				at_axis = LLViewerCamera::getInstance()->getAtAxis();
				at_axis.mV[VZ] = 0.f;
				at_axis.normalize();
				resetAxes(at_axis);
			}
		}
	}
	// unlocking camera from avatar
	else if (mFocusOnAvatar && !focus_on_avatar)
	{
		// keep camera focus point consistent, even though it is now unlocked
		setFocusGlobal(getPositionGlobal() + calcThirdPersonFocusOffset(), gAgent.getID());
	}

	mFocusOnAvatar = focus_on_avatar;
}

//-----------------------------------------------------------------------------
// heardChat()
//-----------------------------------------------------------------------------
void LLAgent::heardChat(const LLUUID& id)
{
	// log text and voice chat to speaker mgr
	// for keeping track of active speakers, etc.
	LLLocalSpeakerMgr::getInstance()->speakerChatted(id);

	// don't respond to your own voice
	if (id == getID()) return;

	if (ll_rand(2) == 0)
	{
		LLViewerObject *chatter = gObjectList.findObject(mLastChatterID);
		setLookAt(LOOKAT_TARGET_AUTO_LISTEN, chatter, LLVector3::zero);
	}

	mLastChatterID = id;
	mChatTimer.reset();
}

//-----------------------------------------------------------------------------
// lookAtLastChat()
//-----------------------------------------------------------------------------
void LLAgent::lookAtLastChat()
{
	// Block if camera is animating or not in normal third person camera mode
	if (mCameraAnimating || !cameraThirdPerson())
	{
		return;
	}

	LLViewerObject *chatter = gObjectList.findObject(mLastChatterID);
	if (chatter)
	{
		LLVector3 delta_pos;
		if (chatter->isAvatar())
		{
			LLVOAvatar *chatter_av = (LLVOAvatar*)chatter;
			if (isAgentAvatarValid() && chatter_av->mHeadp)
			{
				delta_pos = chatter_av->mHeadp->getWorldPosition() - gAgentAvatarp->mHeadp->getWorldPosition();
			}
			else
			{
				delta_pos = chatter->getPositionAgent() - getPositionAgent();
			}
			delta_pos.normalize();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = gAgentAvatarp->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normalize();
			LLVector3 up = left % delta_pos;
			up.normalize();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;
			if (chatter_av->mHeadp)
			{
				setFocusGlobal(getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition()), mLastChatterID);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - gAgent.getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition());
			}
			else
			{
				setFocusGlobal(chatter->getPositionGlobal(), mLastChatterID);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
			}
			setFocusOnAvatar(FALSE, TRUE);
		}
		else
		{
			delta_pos = chatter->getRenderPosition() - getPositionAgent();
			delta_pos.normalize();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = gAgentAvatarp->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normalize();
			LLVector3 up = left % delta_pos;
			up.normalize();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;

			setFocusGlobal(chatter->getPositionGlobal(), mLastChatterID);
			mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
			setFocusOnAvatar(FALSE, TRUE);
		}
	}
}

void LLAgent::lookAtObject(LLUUID object_id, ECameraPosition camera_pos)
{
	// Block if camera is animating or not in normal third person camera mode
	if (mCameraAnimating || !cameraThirdPerson())
	{
		return;
	}

	LLViewerObject *chatter = gObjectList.findObject(object_id);
	if (chatter)
	{
		LLVector3 delta_pos;
		if (chatter->isAvatar())
		{
			LLVOAvatar *chatter_av = (LLVOAvatar*)chatter;
			if (!!isAgentAvatarValid() && chatter_av->mHeadp)
			{
				delta_pos = chatter_av->mHeadp->getWorldPosition() - gAgentAvatarp->mHeadp->getWorldPosition();
			}
			else
			{
				delta_pos = chatter->getPositionAgent() - getPositionAgent();
			}
			delta_pos.normVec();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = gAgentAvatarp->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normVec();
			LLVector3 up = left % delta_pos;
			up.normVec();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;

			F32 radius = chatter_av->getVObjRadius();
			LLVector3d view_dist(radius, radius, 0.0f);

			if (chatter_av->mHeadp)
			{
				setFocusGlobal(getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition()), object_id);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - gAgent.getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition());

				switch (camera_pos)
				{
					case CAMERA_POSITION_SELF:
						mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - gAgent.getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition());
						break;
					case CAMERA_POSITION_OBJECT:
						mCameraFocusOffsetTarget =  view_dist;
						break;
				}
			}
			else
			{
				setFocusGlobal(chatter->getPositionGlobal(), object_id);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();

				switch (camera_pos)
				{
					case CAMERA_POSITION_SELF:
						mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
						break;
					case CAMERA_POSITION_OBJECT:
						mCameraFocusOffsetTarget = view_dist;
						break;
				}
			}
			setFocusOnAvatar(FALSE, TRUE);
		}
		else
		{
			delta_pos = chatter->getRenderPosition() - getPositionAgent();
			delta_pos.normVec();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = gAgentAvatarp->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normVec();
			LLVector3 up = left % delta_pos;
			up.normVec();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;

			setFocusGlobal(chatter->getPositionGlobal(), object_id);

			switch (camera_pos)
			{
				case CAMERA_POSITION_SELF:
					mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
					break;
				case CAMERA_POSITION_OBJECT:
					F32 radius = chatter->getVObjRadius();
					LLVector3d view_dist(radius, radius, 0.0f);
					mCameraFocusOffsetTarget = view_dist;
					break;
			}

			setFocusOnAvatar(FALSE, TRUE);
		}
	}
}

LLSD ll_sdmap_from_vector3(const LLVector3& vec)
{
	LLSD ret;
	ret["X"] = vec.mV[VX];
	ret["Y"] = vec.mV[VY];
	ret["Z"] = vec.mV[VZ];
	return ret;
}

void LLAgent::setStartPosition(U32 location_id)
{
	if (gAgentID == LLUUID::null || gObjectList.findAvatar(gAgentID) == NULL)
	{
        llwarns << "Can't find agent viewer object id " << gAgentID
				<< ". Operation aborted." << llendl;
        return;
    }

	// we've got the viewer object
	// Sometimes the agent can be velocity interpolated off of
	// this simulator.  Clamp it to the region the agent is
	// in, a little bit in on each side.
	const F32 INSET = 0.5f; //meters
	const F32 REGION_WIDTH = LLWorld::getInstance()->getRegionWidthInMeters();

	LLVector3 agent_pos = getPositionAgent();

	if (isAgentAvatarValid())
	{
		// the z height is at the agent's feet
		agent_pos.mV[VZ] -= 0.5f * gAgentAvatarp->mBodySize.mV[VZ];
	}

	agent_pos.mV[VX] = llclamp(agent_pos.mV[VX], INSET, REGION_WIDTH - INSET);
	agent_pos.mV[VY] = llclamp(agent_pos.mV[VY], INSET, REGION_WIDTH - INSET);

	// Don't let them go below ground, or too high.
	agent_pos.mV[VZ] = llclamp(agent_pos.mV[VZ],
							   mRegionp->getLandHeightRegion(agent_pos),
							   LLWorld::getInstance()->getRegionMaxHeight());

	std::string url = gAgent.getRegion()->getCapability("HomeLocation");
	if (!url.empty())
	{
		// Send the CapReq

		LLSD request;
		LLSD body;
		LLSD homeLocation;

		homeLocation["LocationId"] = LLSD::Integer(location_id);
		homeLocation["LocationPos"] = ll_sdmap_from_vector3(agent_pos);
		homeLocation["LocationLookAt"] = ll_sdmap_from_vector3(mFrameAgent.getAtAxis());

		body["HomeLocation"] = homeLocation;

		LLHTTPClient::post(url, body, new LLHomeLocationResponder());
	}
	else
	{
		// Old message based method

		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_SetStartLocationRequest);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->nextBlockFast(_PREHASH_StartLocationData);
		// corrected by sim
		msg->addStringFast(_PREHASH_SimName, "");
		msg->addU32Fast(_PREHASH_LocationID, location_id);
		msg->addVector3Fast(_PREHASH_LocationPos, agent_pos);
		msg->addVector3Fast(_PREHASH_LocationLookAt, mFrameAgent.getAtAxis());

		// Reliable only helps when setting home location. Last location is
		// sent on quit, and we don't have time to ack the packets.
		msg->sendReliable(mRegionp->getHost());
	}

	const U32 HOME_INDEX = 1;
	if (HOME_INDEX == location_id)
	{
		setHomePosRegion(mRegionp->getHandle(), getPositionAgent());
	}
}

void LLAgent::requestStopMotion(LLMotion* motion)
{
	// Notify all avatars that a motion has stopped.
	// This is needed to clear the animation state bits
	LLUUID anim_state = motion->getID();
	onAnimStop(motion->getID());

	// if motion is not looping, it could have stopped by running out of time
	// so we need to tell the server this
//	llinfos << "Sending stop for motion " << motion->getName() << llendl;
	sendAnimationRequest(anim_state, ANIM_REQUEST_STOP);
}

void LLAgent::onAnimStop(const LLUUID& id)
{
	// handle automatic state transitions (based on completion of animation
	// playback)
	if (id == ANIM_AGENT_STAND)
	{
		stopFidget();
	}
	else if (id == ANIM_AGENT_AWAY)
	{
		clearAFK();
	}
	else if (id == ANIM_AGENT_STANDUP)
	{
		// send stand up command
		setControlFlags(AGENT_CONTROL_FINISH_ANIM);

		// now trigger dusting self off animation
		if (isAgentAvatarValid() && !gAgentAvatarp->mBelowWater && rand() % 3 == 0)
			sendAnimationRequest(ANIM_AGENT_BRUSH, ANIM_REQUEST_START);
	}
	else if (id == ANIM_AGENT_PRE_JUMP || id == ANIM_AGENT_LAND ||
			 id == ANIM_AGENT_MEDIUM_LAND)
	{
		setControlFlags(AGENT_CONTROL_FINISH_ANIM);
	}
}

bool LLAgent::isGodlike() const
{
	return mAgentAccess->isGodlike();
}

bool LLAgent::isGodlikeWithoutAdminMenuFakery() const
{
	return mAgentAccess->isGodlikeWithoutAdminMenuFakery();
}

U8 LLAgent::getGodLevel() const
{
	return mAgentAccess->getGodLevel();
}

bool LLAgent::wantsPGOnly() const
{
	return mAgentAccess->wantsPGOnly();
}

bool LLAgent::canAccessMature() const
{
	return mAgentAccess->canAccessMature();
}

bool LLAgent::canAccessAdult() const
{
	return mAgentAccess->canAccessAdult();
}

bool LLAgent::canAccessMaturityInRegion(U64 region_handle) const
{
	LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromHandle(region_handle);
	if (regionp)
	{
		switch (regionp->getSimAccess())
		{
			case SIM_ACCESS_MATURE:
				if (!canAccessMature())
					return false;
				break;
			case SIM_ACCESS_ADULT:
				if (!canAccessAdult())
					return false;
				break;
			default:
				// Oh, go on and hear the silly noises.
				break;
		}
	}

	return true;
}

bool LLAgent::canAccessMaturityAtGlobal(LLVector3d pos_global) const
{
	U64 region_handle = to_region_handle_global(pos_global.mdV[0], pos_global.mdV[1]);
	return canAccessMaturityInRegion(region_handle);
}

bool LLAgent::prefersPG() const
{
	return mAgentAccess->prefersPG();
}

bool LLAgent::prefersMature() const
{
	return mAgentAccess->prefersMature();
}

bool LLAgent::prefersAdult() const
{
	return mAgentAccess->prefersAdult();
}

bool LLAgent::isTeen() const
{
	return mAgentAccess->isTeen();
}

bool LLAgent::isMature() const
{
	return mAgentAccess->isMature();
}

bool LLAgent::isAdult() const
{
	return mAgentAccess->isAdult();
}

void LLAgent::setTeen(bool teen)
{
	mAgentAccess->setTeen(teen);
}

//static
U8 LLAgent::convertTextToMaturity(char text)
{
	return LLAgentAccess::convertTextToMaturity(text);
}

bool LLAgent::sendMaturityPreferenceToServer(U8 preferredMaturity)
{
	// Update agent access preference on the server
	std::string url = getRegion()->getCapability("UpdateAgentInformation");
	if (!url.empty())
	{
		// Set new access preference
		LLSD access_prefs = LLSD::emptyMap();
		if (preferredMaturity == SIM_ACCESS_PG)
		{
			access_prefs["max"] = "PG";
		}
		else if (preferredMaturity == SIM_ACCESS_MATURE)
		{
			access_prefs["max"] = "M";
		}
		if (preferredMaturity == SIM_ACCESS_ADULT)
		{
			access_prefs["max"] = "A";
		}

		LLSD body = LLSD::emptyMap();
		body["access_prefs"] = access_prefs;
		llinfos << "Sending access prefs update to " << (access_prefs["max"].asString()) << " via capability to: "
		<< url << llendl;
		LLHTTPClient::post(url, body, new LLHTTPClient::Responder());    // Ignore response
		return true;
	}
	return false;
}

BOOL LLAgent::getAdminOverride() const
{
	return mAgentAccess->getAdminOverride();
}

void LLAgent::setMaturity(char text)
{
	mAgentAccess->setMaturity(text);
}

void LLAgent::setAdminOverride(BOOL b)
{
	mAgentAccess->setAdminOverride(b);
}

void LLAgent::setGodLevel(U8 god_level)
{
	mAgentAccess->setGodLevel(god_level);
}

const LLAgentAccess& LLAgent::getAgentAccess()
{
	return *mAgentAccess;
}

bool LLAgent::validateMaturity(const LLSD& newvalue)
{
	return mAgentAccess->canSetMaturity(newvalue.asInteger());
}

void LLAgent::handleMaturity(const LLSD& newvalue)
{
	sendMaturityPreferenceToServer(newvalue.asInteger());
}

void LLAgent::buildFullname(std::string& name) const
{
	if (isAgentAvatarValid())
	{
		name = gAgentAvatarp->getFullname();
	}
}

void LLAgent::buildFullnameAndTitle(std::string& name) const
{
	if (isGroupMember())
	{
		name = mGroupTitle;
		name += ' ';
	}
	else
	{
		name.erase(0, name.length());
	}

	if (isAgentAvatarValid())
	{
		name += gAgentAvatarp->getFullname();
	}
}

BOOL LLAgent::isInGroup(const LLUUID& group_id) const
{
	if (isGodlike())
		return true;

	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			return TRUE;
		}
	}
	return FALSE;
}

// This implementation should mirror LLAgentInfo::hasPowerInGroup
BOOL LLAgent::hasPowerInGroup(const LLUUID& group_id, U64 power) const
{
	if (isGodlike())
		return true;

	// GP_NO_POWERS can also mean no power is enough to grant an ability.
	if (GP_NO_POWERS == power) return FALSE;

	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			return (BOOL)((mGroups.get(i).mPowers & power) > 0);
		}
	}
	return FALSE;
}

BOOL LLAgent::hasPowerInActiveGroup(U64 power) const
{
	return (mGroupID.notNull() && (hasPowerInGroup(mGroupID, power)));
}

U64 LLAgent::getPowerInGroup(const LLUUID& group_id) const
{
	if (isGodlike())
		return GP_ALL_POWERS;

	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			return (mGroups.get(i).mPowers);
		}
	}

	return GP_NO_POWERS;
}

BOOL LLAgent::getGroupData(const LLUUID& group_id, LLGroupData& data) const
{
	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			data = mGroups.get(i);
			return TRUE;
		}
	}
	return FALSE;
}

S32 LLAgent::getGroupContribution(const LLUUID& group_id) const
{
	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			S32 contribution = mGroups.get(i).mContribution;
			return contribution;
		}
	}
	return 0;
}

BOOL LLAgent::setGroupContribution(const LLUUID& group_id, S32 contribution)
{
	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			mGroups.get(i).mContribution = contribution;
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessage("SetGroupContribution");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgentID);
			msg->addUUID("SessionID", gAgentSessionID);
			msg->nextBlock("Data");
			msg->addUUID("GroupID", group_id);
			msg->addS32("Contribution", contribution);
			sendReliableMessage();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LLAgent::setUserGroupFlags(const LLUUID& group_id, BOOL accept_notices, BOOL list_in_profile)
{
	S32 count = mGroups.count();
	for (S32 i = 0; i < count; ++i)
	{
		if (mGroups.get(i).mID == group_id)
		{
			mGroups.get(i).mAcceptNotices = accept_notices;
			mGroups.get(i).mListInProfile = list_in_profile;
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessage("SetGroupAcceptNotices");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgentID);
			msg->addUUID("SessionID", gAgentSessionID);
			msg->nextBlock("Data");
			msg->addUUID("GroupID", group_id);
			msg->addBOOL("AcceptNotices", accept_notices);
			msg->nextBlock("NewData");
			msg->addBOOL("ListInProfile", list_in_profile);
			sendReliableMessage();

			update_group_floaters(group_id);

			return TRUE;
		}
	}
	return FALSE;
}

// utility to build a location string
void LLAgent::buildLocationString(std::string& str)
{
	const LLVector3& agent_pos_region = getPositionAgent();
	S32 pos_x = S32(agent_pos_region.mV[VX]);
	S32 pos_y = S32(agent_pos_region.mV[VY]);
	S32 pos_z = S32(agent_pos_region.mV[VZ]);

	// Round the numbers based on the velocity
	LLVector3 agent_velocity = getVelocity();
	F32 velocity_mag_sq = agent_velocity.magVecSquared();

	const F32 FLY_CUTOFF = 6.f;		// meters/sec
	const F32 FLY_CUTOFF_SQ = FLY_CUTOFF * FLY_CUTOFF;
	const F32 WALK_CUTOFF = 1.5f;	// meters/sec
	const F32 WALK_CUTOFF_SQ = WALK_CUTOFF * WALK_CUTOFF;

	if (velocity_mag_sq > FLY_CUTOFF_SQ)
	{
		pos_x -= pos_x % 4;
		pos_y -= pos_y % 4;
	}
	else if (velocity_mag_sq > WALK_CUTOFF_SQ)
	{
		pos_x -= pos_x % 2;
		pos_y -= pos_y % 2;
	}

	// create a defult name and description for the landmark
	std::string buffer;
	if (LLViewerParcelMgr::getInstance()->getAgentParcelName().empty())
	{
		// the parcel doesn't have a name
		buffer = llformat("%.32s (%d, %d, %d)",
						  getRegion()->getName().c_str(),
						  pos_x, pos_y, pos_z);
	}
	else
	{
		// the parcel has a name, so include it in the landmark name
		buffer = llformat("%.32s, %.32s (%d, %d, %d)",
						  LLViewerParcelMgr::getInstance()->getAgentParcelName().c_str(),
						  getRegion()->getName().c_str(),
						  pos_x, pos_y, pos_z);
	}
	str = buffer;
}

LLQuaternion LLAgent::getHeadRotation()
{
	if (!isAgentAvatarValid() || !gAgentAvatarp->mPelvisp || !gAgentAvatarp->mHeadp)
	{
		return LLQuaternion::DEFAULT;
	}

	if (!gAgent.cameraMouselook())
	{
		return gAgentAvatarp->getRotation();
	}

	// We must be in mouselook
	LLVector3 look_dir(LLViewerCamera::getInstance()->getAtAxis());
	LLVector3 up = look_dir % mFrameAgent.getLeftAxis();
	LLVector3 left = up % look_dir;

	LLQuaternion rot(look_dir, left, up);
	if (gAgentAvatarp->getParent())
	{
		rot = rot * ~gAgentAvatarp->getParent()->getRotation();
	}

	return rot;
}

void LLAgent::sendAnimationRequests(LLDynamicArray<LLUUID> &anim_ids, EAnimRequest request)
{
	if (gAgentID.isNull())
	{
		return;
	}

	S32 num_valid_anims = 0;

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_AgentAnimation);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

	for (S32 i = 0, count = anim_ids.count(); i < count; ++i)
	{
		if (anim_ids[i].isNull())
		{
			continue;
		}
		msg->nextBlockFast(_PREHASH_AnimationList);
		msg->addUUIDFast(_PREHASH_AnimID, (anim_ids[i]));
		msg->addBOOLFast(_PREHASH_StartAnim, (request == ANIM_REQUEST_START) ? TRUE : FALSE);
		num_valid_anims++;
	}

	msg->nextBlockFast(_PREHASH_PhysicalAvatarEventList);
	msg->addBinaryDataFast(_PREHASH_TypeData, NULL, 0);
	if (num_valid_anims)
	{
		sendReliableMessage();
	}
}

void LLAgent::sendAnimationRequest(const LLUUID &anim_id, EAnimRequest request)
{
	if (gAgentID.isNull() || anim_id.isNull() || !mRegionp)
	{
		return;
	}

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_AgentAnimation);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

	msg->nextBlockFast(_PREHASH_AnimationList);
	msg->addUUIDFast(_PREHASH_AnimID, (anim_id));
	msg->addBOOLFast(_PREHASH_StartAnim, (request == ANIM_REQUEST_START) ? TRUE : FALSE);

	msg->nextBlockFast(_PREHASH_PhysicalAvatarEventList);
	msg->addBinaryDataFast(_PREHASH_TypeData, NULL, 0);
	sendReliableMessage();
}

void LLAgent::sendWalkRun(bool running)
{
	LLMessageSystem* msgsys = gMessageSystem;
	if (msgsys)
	{
		msgsys->newMessageFast(_PREHASH_SetAlwaysRun);
		msgsys->nextBlockFast(_PREHASH_AgentData);
		msgsys->addUUIDFast(_PREHASH_AgentID, getID());
		msgsys->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msgsys->addBOOLFast(_PREHASH_AlwaysRun, BOOL(running));
		sendReliableMessage();
	}
}

void LLAgent::friendsChanged()
{
	LLCollectProxyBuddies collector;
	LLAvatarTracker::instance().applyFunctor(collector);
	mProxyForAgents = collector.mProxy;
}

BOOL LLAgent::isGrantedProxy(const LLPermissions& perm)
{
	return (mProxyForAgents.count(perm.getOwner()) > 0);
}

BOOL LLAgent::allowOperation(PermissionBit op,
							 const LLPermissions& perm,
							 U64 group_proxy_power,
							 U8 god_minimum)
{
	// Check god level.
	if (getGodLevel() >= god_minimum) return TRUE;

	if (!perm.isOwned()) return FALSE;

	// A group member with group_proxy_power can act as owner.
	BOOL is_group_owned;
	LLUUID owner_id;
	perm.getOwnership(owner_id, is_group_owned);
	LLUUID group_id(perm.getGroup());
	LLUUID agent_proxy(getID());

	if (is_group_owned)
	{
		if (hasPowerInGroup(group_id, group_proxy_power))
		{
			// Let the member assume the group's id for permission requests.
			agent_proxy = owner_id;
		}
	}
	else
	{
		// Check for granted mod permissions.
		if ((PERM_OWNER != op) && isGrantedProxy(perm))
		{
			agent_proxy = owner_id;
		}
	}

	// This is the group id to use for permission requests.
	// Only group members may use this field.
	LLUUID group_proxy = LLUUID::null;
	if (group_id.notNull() && isInGroup(group_id))
	{
		group_proxy = group_id;
	}

	// We now have max ownership information.
	if (PERM_OWNER == op)
	{
		// This this was just a check for ownership, we can now return the answer.
		return (agent_proxy == owner_id);
	}

	return perm.allowOperationBy(op, agent_proxy, group_proxy);
}

void LLAgent::getName(std::string& name)
{
	name.clear();

	if (isAgentAvatarValid())
	{
		LLNameValue *first_nv = gAgentAvatarp->getNVPair("FirstName");
		LLNameValue *last_nv = gAgentAvatarp->getNVPair("LastName");
		if (first_nv && last_nv)
		{
			name = first_nv->printData() + " " + last_nv->printData();
		}
		else
		{
			llwarns << "Agent is missing FirstName and/or LastName nv pair." << llendl;
		}
	}
	else
	{
		name = gSavedSettings.getString("FirstName") + " " + gSavedSettings.getString("LastName");
	}
}

const LLColor4 &LLAgent::getEffectColor()
{
	return mEffectColor;
}

void LLAgent::setEffectColor(const LLColor4 &color)
{
	mEffectColor = color;
}

void LLAgent::initOriginGlobal(const LLVector3d &origin_global)
{
	mAgentOriginGlobal = origin_global;
}

void update_group_floaters(const LLUUID& group_id)
{
	LLFloaterGroupInfo::refreshGroup(group_id);

	// update avatar info
	LLFloaterAvatarInfo* fa = LLFloaterAvatarInfo::getInstance(gAgent.getID());
	if (fa)
	{
		fa->resetGroupList();
	}

	if (gIMMgr)
	{
		// update the talk view
		gIMMgr->refresh();
	}

	gAgent.fireEvent(new LLEvent(&gAgent, "new group"), "");
}

// static
void LLAgent::processAgentDropGroup(LLMessageSystem *msg, void **)
{
	LLUUID	agent_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id);

	if (agent_id != gAgentID)
	{
		llwarns << "processAgentDropGroup for agent other than me" << llendl;
		return;
	}

	LLUUID	group_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_GroupID, group_id);

	// Remove the group if it already exists remove it and add the new data to pick up changes.
	LLGroupData gd;
	gd.mID = group_id;
	S32 index = gAgent.mGroups.find(gd);
	if (index != -1)
	{
		gAgent.mGroups.remove(index);
		if (gAgent.getGroupID() == group_id)
		{
			gAgent.mGroupID.setNull();
			gAgent.mGroupPowers = 0;
			gAgent.mGroupName.clear();
			gAgent.mGroupTitle.clear();
		}

		// refresh all group information
		gAgent.sendAgentDataUpdateRequest();

		LLGroupMgr::getInstance()->clearGroupData(group_id);
		// close the floater for this group, if any.
		LLFloaterGroupInfo::closeGroup(group_id);
		// refresh the group panel of the search window, if necessary.
		LLFloaterDirectory::refreshGroup(group_id);
	}
	else
	{
		llwarns << "processAgentDropGroup, agent is not part of group "
				<< group_id << llendl;
	}
}

class LLAgentDropGroupViewerNode : public LLHTTPNode
{
	virtual void post(LLHTTPNode::ResponsePtr response,
					  const LLSD& context,
					  const LLSD& input) const
	{
		if (!input.isMap() || !input.has("body"))
		{
			//what to do with badly formed message?
			response->statusUnknownError(400);
			response->result(LLSD("Invalid message parameters"));
		}

		LLSD body = input["body"];
		if (body.has("body"))
		{
			//stupid message system doubles up the "body"s
			body = body["body"];
		}

		if (body.has("AgentData") &&
			body["AgentData"].isArray() &&
			body["AgentData"][0].isMap())
		{
			llinfos << "VALID DROP GROUP" << llendl;

			//there is only one set of data in the AgentData block
			LLSD agent_data = body["AgentData"][0];
			LLUUID agent_id;
			LLUUID group_id;

			agent_id = agent_data["AgentID"].asUUID();
			group_id = agent_data["GroupID"].asUUID();

			if (agent_id != gAgentID)
			{
				llwarns << "AgentDropGroup for agent other than me" << llendl;

				response->notFound();
				return;
			}

			// Remove the group if it already exists remove it
			// and add the new data to pick up changes.
			LLGroupData gd;
			gd.mID = group_id;
			S32 index = gAgent.mGroups.find(gd);
			if (index != -1)
			{
				gAgent.mGroups.remove(index);
				if (gAgent.getGroupID() == group_id)
				{
					gAgent.mGroupID.setNull();
					gAgent.mGroupPowers = 0;
					gAgent.mGroupName.clear();
					gAgent.mGroupTitle.clear();
				}

				// refresh all group information
				gAgent.sendAgentDataUpdateRequest();

				LLGroupMgr::getInstance()->clearGroupData(group_id);
				// close the floater for this group, if any.
				LLFloaterGroupInfo::closeGroup(group_id);
				// refresh the group panel of the search window,
				//if necessary.
				LLFloaterDirectory::refreshGroup(group_id);
			}
			else
			{
				llwarns
					<< "AgentDropGroup, agent is not part of group "
					<< group_id << llendl;
			}

			response->result(LLSD());
		}
		else
		{
			//what to do with badly formed message?
			response->statusUnknownError(400);
			response->result(LLSD("Invalid message parameters"));
		}
	}
};

LLHTTPRegistration<LLAgentDropGroupViewerNode>
	gHTTPRegistrationAgentDropGroupViewerNode("/message/AgentDropGroup");

// static
void LLAgent::processAgentGroupDataUpdate(LLMessageSystem *msg, void **)
{
	LLUUID	agent_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id);

	if (agent_id != gAgentID)
	{
		llwarns << "processAgentGroupDataUpdate for agent other than me" << llendl;
		return;
	}

	S32 count = msg->getNumberOfBlocksFast(_PREHASH_GroupData);
	LLGroupData group;
	S32 index = -1;
	bool need_floater_update = false;
	for (S32 i = 0; i < count; ++i)
	{
		msg->getUUIDFast(_PREHASH_GroupData, _PREHASH_GroupID, group.mID, i);
		msg->getUUIDFast(_PREHASH_GroupData, _PREHASH_GroupInsigniaID, group.mInsigniaID, i);
		msg->getU64(_PREHASH_GroupData, "GroupPowers", group.mPowers, i);
		msg->getBOOL(_PREHASH_GroupData, "AcceptNotices", group.mAcceptNotices, i);
		msg->getS32(_PREHASH_GroupData, "Contribution", group.mContribution, i);
		msg->getStringFast(_PREHASH_GroupData, _PREHASH_GroupName, group.mName, i);

		if (group.mID.notNull())
		{
			need_floater_update = true;
			// Remove the group if it already exists remove it and add the new data to pick up changes.
			index = gAgent.mGroups.find(group);
			if (index != -1)
			{
				gAgent.mGroups.remove(index);
			}
			gAgent.mGroups.put(group);
		}
		if (need_floater_update)
		{
			update_group_floaters(group.mID);
		}
	}
}

class LLAgentGroupDataUpdateViewerNode : public LLHTTPNode
{
	virtual void post(LLHTTPNode::ResponsePtr response,
					  const LLSD& context,
					  const LLSD& input) const
	{
		LLSD body = input["body"];
		if (body.has("body"))
		{
			body = body["body"];
		}
		LLUUID agent_id = body["AgentData"][0]["AgentID"].asUUID();

		if (agent_id != gAgentID)
		{
			llwarns << "processAgentGroupDataUpdate for agent other than me"
					<< llendl;
			return;
		}

		LLSD group_data = body["GroupData"];

		LLSD::array_iterator iter_group = group_data.beginArray();
		LLSD::array_iterator end_group = group_data.endArray();
		S32 group_index = 0;
		for ( ; iter_group != end_group; ++iter_group)
		{

			LLGroupData group;
			S32 index = -1;
			bool need_floater_update = false;

			group.mID = (*iter_group)["GroupID"].asUUID();
			group.mPowers = ll_U64_from_sd((*iter_group)["GroupPowers"]);
			group.mAcceptNotices = (*iter_group)["AcceptNotices"].asBoolean();
			group.mListInProfile = body["NewGroupData"][group_index]["ListInProfile"].asBoolean();
			group.mInsigniaID = (*iter_group)["GroupInsigniaID"].asUUID();
			group.mName = (*iter_group)["GroupName"].asString();
			group.mContribution = (*iter_group)["Contribution"].asInteger();

			++group_index;

			if (group.mID.notNull())
			{
				need_floater_update = true;
				// Remove the group if it already exists remove it and add the
				// new data to pick up changes.
				index = gAgent.mGroups.find(group);
				if (index != -1)
				{
					gAgent.mGroups.remove(index);
				}
				gAgent.mGroups.put(group);
			}
			if (need_floater_update)
			{
				update_group_floaters(group.mID);
			}
		}
	}
};

LLHTTPRegistration<LLAgentGroupDataUpdateViewerNode >
	gHTTPRegistrationAgentGroupDataUpdateViewerNode ("/message/AgentGroupDataUpdate");

// static
void LLAgent::processAgentDataUpdate(LLMessageSystem *msg, void **)
{
	LLUUID	agent_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id);

	if (agent_id != gAgentID)
	{
		llwarns << "processAgentDataUpdate for agent other than me" << llendl;
		return;
	}

	msg->getStringFast(_PREHASH_AgentData, _PREHASH_GroupTitle, gAgent.mGroupTitle);
	LLUUID active_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_ActiveGroupID, active_id);

	if (active_id.notNull())
	{
		gAgent.mGroupID = active_id;
		msg->getU64(_PREHASH_AgentData, "GroupPowers", gAgent.mGroupPowers);
		msg->getString(_PREHASH_AgentData, _PREHASH_GroupName, gAgent.mGroupName);
	}
	else
	{
		gAgent.mGroupID.setNull();
		gAgent.mGroupPowers = 0;
		gAgent.mGroupName.clear();
	}

	update_group_floaters(active_id);
}

// static
void LLAgent::processScriptControlChange(LLMessageSystem *msg, void **)
{
	S32 block_count = msg->getNumberOfBlocks("Data");
	for (S32 block_index = 0; block_index < block_count; ++block_index)
	{
		BOOL take_controls;
		U32	controls;
		BOOL passon;
		U32 i;
		msg->getBOOL("Data", "TakeControls", take_controls, block_index);
		if (take_controls)
		{
			// take controls
			msg->getU32("Data", "Controls", controls, block_index);
			msg->getBOOL("Data", "PassToAgent", passon, block_index);
			U32 total_count = 0;
			for (i = 0; i < TOTAL_CONTROLS; ++i)
			{
				if (controls & (1 << i))
				{
					if (passon)
					{
						++gAgent.mControlsTakenPassedOnCount[i];
					}
					else
					{
						++gAgent.mControlsTakenCount[i];
					}
					++total_count;
				}
			}

			// Any control taken?  If so, might be first time.
			if (total_count > 0)
			{
				LLFirstUse::useOverrideKeys();
			}
		}
		else
		{
			// release controls
			msg->getU32("Data", "Controls", controls, block_index);
			msg->getBOOL("Data", "PassToAgent", passon, block_index);
			for (i = 0; i < TOTAL_CONTROLS; ++i)
			{
				if (controls & (1 << i))
				{
					if (passon)
					{
						--gAgent.mControlsTakenPassedOnCount[i];
						if (gAgent.mControlsTakenPassedOnCount[i] < 0)
						{
							gAgent.mControlsTakenPassedOnCount[i] = 0;
						}
					}
					else
					{
						--gAgent.mControlsTakenCount[i];
						if (gAgent.mControlsTakenCount[i] < 0)
						{
							gAgent.mControlsTakenCount[i] = 0;
						}
					}
				}
			}
		}
	}
}

//static
void LLAgent::processAgentCachedTextureResponse(LLMessageSystem* mesgsys, void** user_data)
{
	gAgentQueryManager.mNumPendingQueries--;
	LL_DEBUGS("Agent") << "Remaining pending queries: "
					   << gAgentQueryManager.mNumPendingQueries << LL_ENDL;

	if (!isAgentAvatarValid())
	{
		llwarns << "No avatar for user in cached texture update!" << llendl;
		return;
	}

	if (!gAgentAvatarp->isUsingBakedTextures())
	{
		// ignore baked textures when in customize mode
		LL_DEBUGS("Agent") << "Agent in customize mode, not uploading baked textures."
						   << LL_ENDL;
		return;
	}

	S32 query_id;
	mesgsys->getS32Fast(_PREHASH_AgentData, _PREHASH_SerialNum, query_id);

	S32 num_texture_blocks = mesgsys->getNumberOfBlocksFast(_PREHASH_WearableData);

	S32 num_results = 0;
	for (S32 texture_block = 0; texture_block < num_texture_blocks;
		 ++texture_block)
	{
		LLUUID texture_id;
		U8 texture_index;

		mesgsys->getUUIDFast(_PREHASH_WearableData, _PREHASH_TextureID,
							 texture_id, texture_block);
		mesgsys->getU8Fast(_PREHASH_WearableData, _PREHASH_TextureIndex,
						   texture_index, texture_block);

		if ((S32)texture_index < TEX_NUM_INDICES)
		{
			const LLVOAvatarDictionary::TextureEntry* texture_entry;
			texture_entry = LLVOAvatarDictionary::instance().getTexture((ETextureIndex)texture_index);
			if (texture_entry)
			{
				EBakedTextureIndex baked_index = texture_entry->mBakedTextureIndex;

				if (gAgentQueryManager.mActiveCacheQueries[baked_index] == query_id)
				{
					if (texture_id.notNull())
					{
						LL_DEBUGS("Agent") << "Received cached texture "
										   << (U32)texture_index << ": "
										   << texture_id << LL_ENDL;
						gAgentAvatarp->setCachedBakedTexture((ETextureIndex)texture_index, texture_id);
						//gAgentAvatarp->setTETexture(LLVOAvatar::sBakedTextureIndices[texture_index], texture_id);
						gAgentQueryManager.mActiveCacheQueries[baked_index] = 0;
						num_results++;
					}
				}
				else
				{
					// no cache of this bake. request upload.
					LL_DEBUGS("Agent") << "No cache for baked index "
									   << (U32)baked_index
									   << ", invalidating composite to trigger rebake..."
									   << LL_ENDL;
					gAgentAvatarp->invalidateComposite(gAgentAvatarp->getLayerSet(baked_index),
													   TRUE);
				}
			}
			else
			{
				LL_DEBUGS("Agent") << "No texture entry found for index "
								   << (U32)texture_index  << " !!!"<< LL_ENDL;
			}
		}
	}

	llinfos << "Received cached texture response for " << num_results
			<< " textures." << llendl;

	gAgentAvatarp->updateMeshTextures();

	if (gAgentQueryManager.mNumPendingQueries == 0)
	{
		// RN: not sure why composites are disabled at this point
		gAgentAvatarp->setCompositeUpdatesEnabled(TRUE);
		gAgent.sendAgentSetAppearance();
	}
}

BOOL LLAgent::anyControlGrabbed() const
{
	U32 i;
	for (i = 0; i < TOTAL_CONTROLS; ++i)
	{
		if (gAgent.mControlsTakenCount[i] > 0)
			return TRUE;
		if (gAgent.mControlsTakenPassedOnCount[i] > 0)
			return TRUE;
	}
	return FALSE;
}

BOOL LLAgent::isControlGrabbed(S32 control_index) const
{
	return mControlsTakenCount[control_index] > 0;
}

void LLAgent::forceReleaseControls()
{
	gMessageSystem->newMessage("ForceScriptControlRelease");
	gMessageSystem->nextBlock("AgentData");
	gMessageSystem->addUUID("AgentID", getID());
	gMessageSystem->addUUID("SessionID", getSessionID());
	sendReliableMessage();
}

void LLAgent::setHomePosRegion(const U64& region_handle, const LLVector3& pos_region)
{
	mHaveHomePosition = TRUE;
	mHomeRegionHandle = region_handle;
	mHomePosRegion = pos_region;
}

BOOL LLAgent::getHomePosGlobal(LLVector3d* pos_global)
{
	if (!mHaveHomePosition)
	{
		return FALSE;
	}
	F32 x = 0;
	F32 y = 0;
	from_region_handle(mHomeRegionHandle, &x, &y);
	pos_global->setVec(x + mHomePosRegion.mV[VX], y + mHomePosRegion.mV[VY], mHomePosRegion.mV[VZ]);
	return TRUE;
}

void LLAgent::clearVisualParams(void *data)
{
	if (isAgentAvatarValid())
	{
		gAgentAvatarp->clearVisualParamWeights();
		gAgentAvatarp->updateVisualParams();
	}
}

//---------------------------------------------------------------------------
// Teleport
//---------------------------------------------------------------------------

// teleportCore() - stuff to do on any teleport
// protected
bool LLAgent::teleportCore(bool is_local)
{
	if (TELEPORT_NONE != mTeleportState)
	{
		llwarns << "Attempt to teleport when already teleporting." << llendl;
		return false;
	}

#if 0
	// This should not exist. It has been added, removed, added, and now removed again.
	// This change needs to come from the simulator. Otherwise, the agent ends up out of
	// sync with other viewers. Discuss in DEV-14145/VWR-6744 before reenabling.

	// Stop all animation before actual teleporting
	if (isAgentAvatarValid())
	{
		for (LLVOAvatar::AnimIterator
				anim_it = gAgentAvatarp->mPlayingAnimations.begin();
		     anim_it != gAgentAvatarp->mPlayingAnimations.end(); ++anim_it)
		{
			gAgentAvatarp->stopMotion(anim_it->first);
		}
		gAgentAvatarp->processAnimationStateChanges();
	}
#endif

	// Don't call LLFirstUse::useTeleport because we don't know/ yet if the
	// teleport will succeed. Look in process_teleport_location_reply

	// close the map and find panels so we can see our destination
	LLFloaterWorldMap::hide(NULL);
	LLFloaterDirectory::hide(NULL);

	// hide land floater too - it'll be out of date
	LLFloaterLand::hideInstance();

	LLViewerParcelMgr::getInstance()->deselectLand();
	LLViewerMediaFocus::getInstance()->setFocusFace(false, NULL, 0, NULL);

	// Close all pie menus, deselect land, etc.
	// Don't change the camera until we know teleport succeeded. JC
	resetView(FALSE);

	// local logic
	LLViewerStats::getInstance()->incStat(LLViewerStats::ST_TELEPORT_COUNT);
	if (is_local)
	{
		gAgent.setTeleportState(LLAgent::TELEPORT_LOCAL);
	}
	else
	{
		gTeleportDisplay = TRUE;
		gAgent.setTeleportState(LLAgent::TELEPORT_START);

		//release geometry from old location
		gPipeline.resetVertexBuffers();

		if (gSavedSettings.getBOOL("SpeedRez"))
		{
			F32 draw_distance = gSavedSettings.getF32("RenderFarClip");
			if (gSavedDrawDistance < draw_distance)
			{
				gSavedDrawDistance = draw_distance;
			}
			gSavedSettings.setF32("SavedRenderFarClip", gSavedDrawDistance);
			gSavedSettings.setF32("RenderFarClip", 32.0f);
		}
		make_ui_sound("UISndTeleportOut");
	}

	// MBW -- Let the voice client know a teleport has begun so it can leave
	// the existing channel.
	// This was breaking the case of teleporting within a single sim. Backing
	// it out for now.
	//LLVoiceClient::getInstance()->leaveChannel();

	return true;
}

void LLAgent::teleportRequest(const U64& region_handle,
							  const LLVector3& pos_local,
							  bool look_at_from_camera)
{
	LLViewerRegion* regionp = getRegion();
	bool is_local = (region_handle == to_region_handle(getPositionGlobal()));
	if (regionp && teleportCore(is_local))
	{
		llinfos << "TeleportLocationRequest: '" << region_handle << "':" << pos_local
				<< llendl;
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessage("TeleportLocationRequest");
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->nextBlockFast(_PREHASH_Info);
		msg->addU64("RegionHandle", region_handle);
		msg->addVector3("Position", pos_local);
		LLVector3 look_at(0,1,0);
		if (look_at_from_camera)
		{
			look_at = LLViewerCamera::getInstance()->getAtAxis();
		}
		msg->addVector3("LookAt", look_at);
		sendReliableMessage();
	}
}

// Landmark ID = LLUUID::null means teleport home
void LLAgent::teleportViaLandmark(const LLUUID& landmark_asset_id)
{
//MK
	if (gRRenabled &&
		(LLStartUp::getStartupState() != STATE_STARTED ||
		 (gViewerWindow && gViewerWindow->getShowProgress()) ||
		 gAgent.mRRInterface.contains("tplm") ||
		 (gAgent.mRRInterface.mContainsUnsit &&
		  isAgentAvatarValid() && gAgentAvatarp->mIsSitting)))
	{
		return;
	}
//mk
	LLViewerRegion *regionp = getRegion();
	if (regionp && teleportCore())
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_TeleportLandmarkRequest);
		msg->nextBlockFast(_PREHASH_Info);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->addUUIDFast(_PREHASH_LandmarkID, landmark_asset_id);
		sendReliableMessage();
	}
}

void LLAgent::teleportViaLure(const LLUUID& lure_id, BOOL godlike)
{
	LLViewerRegion* regionp = getRegion();
	if (regionp && teleportCore())
	{
		U32 teleport_flags = 0x0;
		if (godlike)
		{
			teleport_flags |= TELEPORT_FLAGS_VIA_GODLIKE_LURE;
			teleport_flags |= TELEPORT_FLAGS_DISABLE_CANCEL;
		}
		else
		{
			teleport_flags |= TELEPORT_FLAGS_VIA_LURE;
		}

		// send the message
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_TeleportLureRequest);
		msg->nextBlockFast(_PREHASH_Info);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->addUUIDFast(_PREHASH_LureID, lure_id);
		// teleport_flags is a legacy field, now derived sim-side:
		msg->addU32("TeleportFlags", teleport_flags);
		sendReliableMessage();
	}
}

// James Cook, July 28, 2005
void LLAgent::teleportCancel()
{
	LLViewerRegion* regionp = getRegion();
	if (regionp)
	{
		// send the message
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessage("TeleportCancel");
		msg->nextBlockFast(_PREHASH_Info);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		sendReliableMessage();
	}
	gTeleportDisplay = FALSE;
	gAgent.setTeleportState(LLAgent::TELEPORT_NONE);
}

void LLAgent::teleportViaLocation(const LLVector3d& pos_global)
{
//MK
	if (gRRenabled &&
		(LLStartUp::getStartupState() != STATE_STARTED ||
		 (gViewerWindow && gViewerWindow->getShowProgress()) ||
		 gAgent.mRRInterface.contains("tploc") ||
		 (gAgent.mRRInterface.mContainsUnsit &&
		  isAgentAvatarValid() && gAgentAvatarp->mIsSitting)))
	{
		return;
	}
//mk
	LLViewerRegion* regionp = getRegion();
	U64 handle = to_region_handle(pos_global);
	LLSimInfo* info = LLWorldMap::getInstance()->simInfoFromHandle(handle);
	if (regionp && info)
	{
		LLVector3d region_origin = info->getGlobalOrigin();
		LLVector3 pos_local((F32)(pos_global.mdV[VX] - region_origin.mdV[VX]),
							(F32)(pos_global.mdV[VY] - region_origin.mdV[VY]),
							(F32)(pos_global.mdV[VZ]));
		teleportRequest(handle, pos_local);
	}
	else if (regionp &&
			 teleportCore(regionp->getHandle() == to_region_handle_global((F32)pos_global.mdV[VX],
																		  (F32)pos_global.mdV[VY])))
	{
		llwarns << "Using deprecated teleportlocationrequest." << llendl;
		// send the message
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_TeleportLocationRequest);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

		msg->nextBlockFast(_PREHASH_Info);
		F32 width = regionp->getWidth();
		LLVector3 pos(fmod((F32)pos_global.mdV[VX], width),
					  fmod((F32)pos_global.mdV[VY], width),
					  (F32)pos_global.mdV[VZ]);
		F32 region_x = (F32)(pos_global.mdV[VX]);
		F32 region_y = (F32)(pos_global.mdV[VY]);
		U64 region_handle = to_region_handle_global(region_x, region_y);
		msg->addU64Fast(_PREHASH_RegionHandle, region_handle);
		msg->addVector3Fast(_PREHASH_Position, pos);
		pos.mV[VX] += 1;
		msg->addVector3Fast(_PREHASH_LookAt, pos);
		sendReliableMessage();
	}
}

// Teleport to global position, but keep facing in the same direction
void LLAgent::teleportViaLocationLookAt(const LLVector3d& pos_global)
{
//MK
	if (gRRenabled)
	{
		// Do not perform these checks if we are automatically snapping back to the last standing location
		if (!gAgent.mRRInterface.mSnappingBackToLastStandingLocation)
		{
			// Can't TP if we can't sittp, unsit, tp to a location or when the
			// forward control is taken (and not passed), and something is locked
			if (gAgent.mRRInterface.contains("tploc") ||
				(gAgent.forwardGrabbed() && gAgent.mRRInterface.mContainsDetach) ||
				 gAgent.mRRInterface.contains("sittp") ||
				 (gAgent.mRRInterface.mContainsUnsit &&
				  isAgentAvatarValid() && gAgentAvatarp->mIsSitting))
			{
				return;
			}
		}
	}
//mk
	mbTeleportKeepsLookAt = true;
	setFocusOnAvatar(FALSE, ANIMATE);	// detach camera form avatar, so it keeps direction
	U64 region_handle = to_region_handle(pos_global);
	LLVector3 pos_local = (LLVector3)(pos_global - from_region_handle(region_handle));
	teleportRequest(region_handle, pos_local, getTeleportKeepsLookAt());
}

void LLAgent::setTeleportState(ETeleportState state)
{
	mTeleportState = state;
	if (mTeleportState > TELEPORT_NONE && LLPipeline::sFreezeTime)
	{
		LLFloaterSnapshot::hide(0);
	}
	if (mTeleportState == TELEPORT_NONE)
	{
		mbTeleportKeepsLookAt = false;
	}
	// OGPX : Only compute a 'slurl' in non-OGP mode. In OGP, set it to
	// regionuri in floaterteleport.
	if (mTeleportState == TELEPORT_MOVING &&
		!gSavedSettings.getBOOL("OpenGridProtocol"))
	{
		// We're outa here. Save "back" slurl.
		mTeleportSourceSLURL = getSLURL();
	}
}

void LLAgent::stopCurrentAnimations()
{
	// This function stops all current overriding animations on this avatar,
	// propagating this change back to the server.

	if (isAgentAvatarValid())
	{
		for (LLVOAvatar::AnimIterator
				anim_it = gAgentAvatarp->mPlayingAnimations.begin();
			 anim_it != gAgentAvatarp->mPlayingAnimations.end(); ++anim_it)
		{
			// don't cancel a ground-sit anim, as viewers use this animation's
			// status in determining whether we're sitting.
			if (anim_it->first != ANIM_AGENT_SIT_GROUND_CONSTRAINED)
			{
				// stop this animation locally
				gAgentAvatarp->stopMotion(anim_it->first, TRUE);
				// ...and tell the server to tell everyone.
				sendAnimationRequest(anim_it->first, ANIM_REQUEST_STOP);
			}
		}

		// re-assert at least the default standing animation, because viewers
		// get confused by avs with no associated anims.
		sendAnimationRequest(ANIM_AGENT_STAND, ANIM_REQUEST_START);
	}
}

void LLAgent::fidget()
{
	if (!getAFK())
	{
		F32 curTime = mFidgetTimer.getElapsedTimeF32();
		if (curTime > mNextFidgetTime)
		{
			// pick a random fidget anim here
			S32 oldFidget = mCurrentFidget;

			mCurrentFidget = ll_rand(NUM_AGENT_STAND_ANIMS);

			if (mCurrentFidget != oldFidget)
			{
				LLAgent::stopFidget();

				switch (mCurrentFidget)
				{
				case 0:
					mCurrentFidget = 0;
					break;
				case 1:
					sendAnimationRequest(ANIM_AGENT_STAND_1, ANIM_REQUEST_START);
					mCurrentFidget = 1;
					break;
				case 2:
					sendAnimationRequest(ANIM_AGENT_STAND_2, ANIM_REQUEST_START);
					mCurrentFidget = 2;
					break;
				case 3:
					sendAnimationRequest(ANIM_AGENT_STAND_3, ANIM_REQUEST_START);
					mCurrentFidget = 3;
					break;
				case 4:
					sendAnimationRequest(ANIM_AGENT_STAND_4, ANIM_REQUEST_START);
					mCurrentFidget = 4;
					break;
				}
			}

			// calculate next fidget time
			mNextFidgetTime = curTime + ll_frand(MAX_FIDGET_TIME - MIN_FIDGET_TIME) + MIN_FIDGET_TIME;
		}
	}
}

void LLAgent::stopFidget()
{
	LLDynamicArray<LLUUID> anims;
	anims.put(ANIM_AGENT_STAND_1);
	anims.put(ANIM_AGENT_STAND_2);
	anims.put(ANIM_AGENT_STAND_3);
	anims.put(ANIM_AGENT_STAND_4);

	gAgent.sendAnimationRequests(anims, ANIM_REQUEST_STOP);
}

void LLAgent::requestEnterGodMode()
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_RequestGodlikePowers);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_RequestBlock);
	msg->addBOOLFast(_PREHASH_Godlike, TRUE);
	msg->addUUIDFast(_PREHASH_Token, LLUUID::null);

	// simulators need to know about your request
	sendReliableMessage();
}

void LLAgent::requestLeaveGodMode()
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_RequestGodlikePowers);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_RequestBlock);
	msg->addBOOLFast(_PREHASH_Godlike, FALSE);
	msg->addUUIDFast(_PREHASH_Token, LLUUID::null);

	// simulator needs to know about your request
	sendReliableMessage();
}

//-----------------------------------------------------------------------------
// sendAgentSetAppearance()
//-----------------------------------------------------------------------------
void LLAgent::sendAgentSetAppearance()
{
	if (!isAgentAvatarValid() || gAgentWearables.isSettingOutfit() ||
		(gAgentQueryManager.mNumPendingQueries > 0 &&
		 gAgentAvatarp->isUsingBakedTextures()))
	{
		return;
	}

	llinfos << "TAT: Sent AgentSetAppearance: "
			<< gAgentAvatarp->getBakedStatusForPrintout() << llendl;
	//dumpAvatarTEs("sendAgentSetAppearance()");

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_AgentSetAppearance);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

	// Correct for the collision tolerance (to make it look like the agent is
	// actually walking on the ground/object).
	LLVector3 body_size = gAgentAvatarp->mBodySize;
	body_size.mV[VZ] += gSavedSettings.getF32("AvatarOffsetZ");
	msg->addVector3Fast(_PREHASH_Size, body_size);

	// To guard against out of order packets.
	// Note: always start by sending 1. This resets the server's count. 0 on
	// the server means "uninitialized"
	mAppearanceSerialNum++;
	msg->addU32Fast(_PREHASH_SerialNum, mAppearanceSerialNum);

	// is texture data current relative to wearables?
	// KLW - TAT this will probably need to check the local queue.
	BOOL textures_current = gAgentAvatarp->areTexturesCurrent();

	for (U8 baked_index = 0; baked_index < BAKED_NUM_INDICES;
		 ++baked_index)
	{
		const ETextureIndex texture_index = LLVOAvatarDictionary::bakedToLocalTextureIndex((EBakedTextureIndex)baked_index);

		// if we're not wearing a skirt, we don't need the texture to be baked
		if (texture_index == TEX_SKIRT_BAKED &&
			!gAgentAvatarp->isWearingWearableType(LLWearableType::WT_SKIRT))
		{
			continue;
		}

		// IMG_DEFAULT_AVATAR means not baked. 0 index should be ignored for
		// baked textures
		if (!gAgentAvatarp->isTextureDefined(texture_index, 0))
		{
			textures_current = FALSE;
			break;
		}
	}

	// only update cache entries if we have all our baked textures
	if (textures_current)
	{
		llinfos << "TAT: Sending cached texture data" << llendl;
		for (U8 baked_index = 0; baked_index < BAKED_NUM_INDICES;
			 ++baked_index)
		{
			BOOL generate_valid_hash = TRUE;
			if (isAgentAvatarValid() &&
				!gAgentAvatarp->isBakedTextureFinal((LLVOAvatarDefines::EBakedTextureIndex)baked_index))
			{
				generate_valid_hash = FALSE;
				llinfos << "Not caching baked texture upload for "
						<< (U32)baked_index
						<< " due to being uploaded at low resolution." << llendl;
			}

			const LLUUID hash = gAgentWearables.computeBakedTextureHash((EBakedTextureIndex)baked_index,
																		generate_valid_hash);
			if (hash.notNull())
			{
				ETextureIndex texture_index = LLVOAvatarDictionary::bakedToLocalTextureIndex((EBakedTextureIndex) baked_index);
				msg->nextBlockFast(_PREHASH_WearableData);
				msg->addUUIDFast(_PREHASH_CacheID, hash);
				msg->addU8Fast(_PREHASH_TextureIndex, (U8)texture_index);
			}
		}
		msg->nextBlockFast(_PREHASH_ObjectData);
		gAgentAvatarp->sendAppearanceMessage(gMessageSystem);
	}
	else
	{
		// If the textures aren't baked, send NULL for texture IDs
		// This means the baked texture IDs on the server will be untouched.
		// Once all textures are baked, another AvatarAppearance message will be sent to update the TEs
		msg->nextBlockFast(_PREHASH_ObjectData);
		gMessageSystem->addBinaryDataFast(_PREHASH_TextureEntry, NULL, 0);
	}

	const bool physics_on = LLVOAvatar::sAvatarPhysics &&
							gAgentWearables.selfHasWearable(LLWearableType::WT_PHYSICS);
	for (LLViewerVisualParam* param = (LLViewerVisualParam*)gAgentAvatarp->getFirstVisualParam();
		 param;
		 param = (LLViewerVisualParam*)gAgentAvatarp->getNextVisualParam())
	{
		// do not transmit params of group
		// VISUAL_PARAM_GROUP_TWEAKABLE_NO_TRANSMIT
		if (param->getGroup() == VISUAL_PARAM_GROUP_TWEAKABLE)
		{
			// A hack to prevent ruthing on older viewers when physics is not
			// in use for our avatar.
			if (!physics_on && param->getID() >= 10000) break;

			msg->nextBlockFast(_PREHASH_VisualParam);
			// We don't send the param ids. Instead, we assume that the receiver
			// has the same params in the same sequence.
			const F32 param_value = param->getWeight();
			const U8 new_weight = F32_to_U8(param_value, param->getMinWeight(),
											param->getMaxWeight());
			msg->addU8Fast(_PREHASH_ParamValue, new_weight);
		}
	}

	sendReliableMessage();
}

void LLAgent::sendAgentDataUpdateRequest()
{
	gMessageSystem->newMessageFast(_PREHASH_AgentDataUpdateRequest);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	sendReliableMessage();
}

void LLAgent::observeFriends()
{
	if (!mFriendObserver)
	{
		mFriendObserver = new LLAgentFriendObserver;
		LLAvatarTracker::instance().addObserver(mFriendObserver);
		friendsChanged();
	}
}

void LLAgent::parseTeleportMessages(const std::string& xml_filename)
{
	LLXMLNodePtr root;
	BOOL success = LLUICtrlFactory::getLayeredXMLNode(xml_filename, root);

	if (!success || !root || !root->hasName("teleport_messages"))
	{
		llerrs << "Problem reading teleport string XML file: "
			   << xml_filename << llendl;
		return;
	}

	for (LLXMLNode* message_set = root->getFirstChild();
		 message_set != NULL;
		 message_set = message_set->getNextSibling())
	{
		if (!message_set->hasName("message_set")) continue;

		std::map<std::string, std::string> *teleport_msg_map = NULL;
		std::string message_set_name;

		if (message_set->getAttributeString("name", message_set_name))
		{
			// now we loop over all the string in the set and add them to the
			// appropriate set
			if (message_set_name == "errors")
			{
				teleport_msg_map = &sTeleportErrorMessages;
			}
			else if (message_set_name == "progress")
			{
				teleport_msg_map = &sTeleportProgressMessages;
			}
		}

		if (!teleport_msg_map) continue;

		std::string message_name;
		for (LLXMLNode* message_node = message_set->getFirstChild();
			 message_node != NULL;
			 message_node = message_node->getNextSibling())
		{
			if (message_node->hasName("message") &&
				message_node->getAttributeString("name", message_name))
			{
				(*teleport_msg_map)[message_name] =
					message_node->getTextContents();
			}
		}
	}
}

// OGPX - This code will change when capabilities get refactored.
// Right now this is used for capabilities that we get from OGP agent domain
void LLAgent::setCapability(const std::string& name, const std::string& url)
{
#if 0 // OGPX : I think (hope?) we don't need this
	  //    but I'm leaving it here commented out because I'm not quite
	  //    sure why the region capabilities code had it wedged in setCap call
	  //    Maybe the agent domain capabilities will need something like this as well

	if (name == "EventQueueGet")
	{
		delete mEventPoll;
		mEventPoll = NULL;
		mEventPoll = new LLEventPoll(url, getHost());
	}
	else if (name == "UntrustedSimulatorMessage")
	{
		LLHTTPSender::setSender(mHost, new LLCapHTTPSender(url));
	}
	else
#endif
	{
		mCapabilities[name] = url;
	}
}

//OGPX : Agent Domain capabilities...  this needs to be refactored
std::string LLAgent::getCapability(const std::string& name) const
{
	CapabilityMap::const_iterator iter = mCapabilities.find(name);
	if (iter == mCapabilities.end())
	{
		return "";
	}
	return iter->second;
}
/********************************************************************************/

LLAgentQueryManager gAgentQueryManager;

LLAgentQueryManager::LLAgentQueryManager()
:	mWearablesCacheQueryID(0),
	mNumPendingQueries(0),
	mUpdateSerialNum(0)
{
	for (U32 i = 0; i < BAKED_NUM_INDICES; ++i)
	{
		mActiveCacheQueries[i] = 0;
	}
}

LLAgentQueryManager::~LLAgentQueryManager()
{
}

// EOF
